#include <ClockButtons.h>

gpio_callback ClockButtons::hButtonCallbackData;
gpio_callback ClockButtons::minButtonCallbackData;
gpio_callback ClockButtons::hourButtonCallbackData;
gpio_callback ClockButtons::dateButtonCallbackData;
gpio_callback ClockButtons::memoButtonCallbackData;
gpio_callback ClockButtons::tempButtonCallbackData;

struct k_fifo ClockButtons::pressedButtons;

ClockButtons::ClockButtons(ClockSettings *clockSettings, ClockTime *clockTime, ClockDisplay *clockDisplay)
{
    this->clockSettings = clockSettings;
    this->clockTime = clockTime;
    this->clockDisplay = clockDisplay;

    printk("Init Buttons.\n");
    k_fifo_init(&pressedButtons);
    
    const struct gpio_dt_spec hButton = GPIO_DT_SPEC_GET(DT_NODELABEL(button_h), gpios);
    const struct gpio_dt_spec minButton = GPIO_DT_SPEC_GET(DT_NODELABEL(button_min), gpios);
    const struct gpio_dt_spec hourButton = GPIO_DT_SPEC_GET(DT_NODELABEL(button_hour), gpios);
    const struct gpio_dt_spec dateButton = GPIO_DT_SPEC_GET(DT_NODELABEL(button_date), gpios);
    const struct gpio_dt_spec memoButton = GPIO_DT_SPEC_GET(DT_NODELABEL(button_memo), gpios);
    const struct gpio_dt_spec tempButton = GPIO_DT_SPEC_GET(DT_NODELABEL(button_temp), gpios);

    int ret = initButtonInterrupt(&hButton, &hButtonCallbackData, hButtonPressed);
    ret = initButtonInterrupt(&minButton, &minButtonCallbackData, minButtonPressed);
    ret = initButtonInterrupt(&hourButton, &hourButtonCallbackData, hourButtonPressed);
    ret = initButtonInterrupt(&dateButton, &dateButtonCallbackData, dateButtonPressed);
    ret = initButtonInterrupt(&tempButton, &tempButtonCallbackData, tempButtonPressed);
    ret = initButtonInterrupt(&memoButton, &memoButtonCallbackData, memoButtonPressed);

}

int ClockButtons::initButtonInterrupt(const struct gpio_dt_spec *button, struct gpio_callback *callback, 
    gpio_callback_handler_t callbackFunction)
{
    if (!device_is_ready(button->port)) {
        printk("Error: Button device %s is not ready\n", button->port->name);
        return 1;
    }

    int ret = gpio_pin_configure_dt(button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
           ret, button->port->name, button->pin);
        return 1;
    }

    ret = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, button->port->name, button->pin);
        return 1;
    }

    gpio_init_callback(callback, callbackFunction, BIT(button->pin));
    gpio_add_callback(button->port, callback);
    printk("Set up button at %s pin %d\n", button->port->name, button->pin);

    return 0;
}

void ClockButtons::processButtonActions()
{
    struct pressedButton *eventPressedButton;

    while (1) {
        eventPressedButton = (pressedButton*)k_fifo_get(&pressedButtons, K_FOREVER);
        if (!eventPressedButton) {
            k_msleep(50);
            continue;
        }

       printk("Received a button press %u\n", eventPressedButton->id);

        switch(eventPressedButton->id) {
            case hButtonId:
                hButtonProcess();
                break;
            case minButtonId:
                minButtonProcess();
                break;
            case dateButtonId:
                //debounce the button
                if (clockDisplay->getMode() != ClockDisplay::modeDate) {
                    dateButtonProcess();
                }
                break;
            case hourButtonId:
                if (clockDisplay->getMode() != ClockDisplay::modeHour) {
                    hourButtonProcess();
                }
                break;
            case tempButtonId:
                if (clockDisplay->getMode() != ClockDisplay::modeTemp) {
                    tempButtonProcess();
                }
                break;
            case memoButtonId:
                memoButtonProcess();
                break;
            default:
                printk("The button is not processed\n");
                continue;
        }
    }
}

void ClockButtons::minButtonProcess(void)
{
    if ((clockDisplay->getMode() == ClockDisplay::modeTime) || (clockDisplay->getMode() == ClockDisplay::modeMinute)) {
        clockTime->getRtcTime();

        uint8_t minute = clockTime->getMinute();

        clockTime->setMinute((minute < 59) ? minute + 1 : 0);
        clockTime->setSecond(0);

        clockTime->setRtcTime();

        printk("In minButtonProcess Time: %.2d:%.2d:%.2d\n", clockTime->getHour(), clockTime->getMinute(), clockTime->getSecond());
    } else if (clockDisplay->getMode() == ClockDisplay::modeHour) {
        //get the current time
        clockTime->getRtcTime();

        uint8_t hour = clockTime->getHour();

        clockTime->setHour((hour < 23) ? hour + 1 : 0);

        //write the new time to RTC
        clockTime->setRtcTime();

        printk("In minButtonProcess Time: %.2d:%.2d:%.2d\n", clockTime->getHour(), clockTime->getMinute(), clockTime->getSecond());
    } else if (clockDisplay->getMode() == ClockDisplay::modeYear) {
        //get the current time
        clockTime->getRtcTime();

        uint16_t year = clockTime->getYear();

        printk("Year: %4d\n", year);

        clockTime->setYear((year < 2099) ? year + 1 : 2021);

        //write the new time to RTC
        clockTime->setRtcTime();

        printk("In minButtonProcess Year: %.4d\n", clockTime->getYear());
    } else if (clockDisplay->getMode() == ClockDisplay::modeMonth) {
        //get the current time
        clockTime->getRtcTime();

        uint8_t month = clockTime->getMonth();

        printk("Month: %2d\n", month);

        clockTime->setMonth((month < 12) ? month + 1 : 1);

        //write the new time to RTC
        clockTime->setRtcTime();

        printk("In minButtonProcess Month: %.2d\n", clockTime->getMonth());

    } else if (clockDisplay->getMode() == ClockDisplay::modeDay) {
        //get the current time
        clockTime->getRtcTime();

        uint8_t day = clockTime->getDay();
        uint8_t daysInMonth = clockTime->getDaysInMonth();

        printk("Day: %2d, daysInMonth: %2d\n", day, daysInMonth);

        clockTime->setDay((day < (daysInMonth - 1)) ? day + 1 : 1);

        //write the new time to RTC
        clockTime->setRtcTime();

        printk("In minButtonProcess Day: %.2d\n", clockTime->getDay());

    } else if (clockDisplay->getMode() == ClockDisplay::modeMinute) {
        //get the current time
        clockTime->getRtcTime();

        uint8_t minute = clockTime->getMinute();

        printk("Minute: %2d\n", minute);

        clockTime->setMinute((minute < 59) ? minute + 1 : 0);
        clockTime->setSecond(0);

        //write the new time to RTC
        clockTime->setRtcTime();

        printk("In minButtonProcess Minute: %.2d\n", clockTime->getMinute());

    } else if (clockDisplay->getMode() == ClockDisplay::modeDstWeek) {
        uint8_t week = clockTime->getTimezone()->getDstWeek();
        printk("Week: %2d\n", week);

        clockTime->getTimezone()->setDstWeek((week < 4) ? week + 1 : 0);

        clockSettings->setDstWeek(clockTime->getTimezone()->getDstWeek());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess DstWeek: %.2d\n", clockTime->getTimezone()->getDstWeek());

    } else if (clockDisplay->getMode() == ClockDisplay::modeDstWeekday) {
        uint8_t weekday = clockTime->getTimezone()->getDstWeekday();
        printk("Weekday: %2d\n", weekday);

        clockTime->getTimezone()->setDstWeekday((weekday < 6) ? weekday + 1 : 0);

        clockSettings->setDstWeekday(clockTime->getTimezone()->getDstWeekday());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess DstWeekday: %.2d\n", clockTime->getTimezone()->getDstWeekday());

    } else if (clockDisplay->getMode() == ClockDisplay::modeDstMonth) {
        uint8_t month = clockTime->getTimezone()->getDstMonth();
        printk("Month: %2d\n", month);

        clockTime->getTimezone()->setDstMonth((month < 11) ? month + 1 : 0);

        clockSettings->setDstMonth(clockTime->getTimezone()->getDstMonth());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess DstMonth: %.2d\n", clockTime->getTimezone()->getDstMonth());

    } else if (clockDisplay->getMode() == ClockDisplay::modeDstHour) {
        uint8_t hour = clockTime->getTimezone()->getDstHour();
        printk("Hour: %2d\n", hour);

        clockTime->getTimezone()->setDstHour((hour < 23) ? hour + 1 : 0);

        clockSettings->setDstHour(clockTime->getTimezone()->getDstHour());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess DstHour: %.2d\n", clockTime->getTimezone()->getDstHour());

    } else if (clockDisplay->getMode() == ClockDisplay::modeDstOffset) {
        int offset = clockTime->getTimezone()->getDstOffset();
        printk("Offset: %2d\n", offset);

        uint8_t offsetNumber = clockTime->getTimezone()->getOffsetNumber(offset);
        printk("Offset: %2d, number: %2d\n", offset, offsetNumber);

        //calculate the new offset number
        offsetNumber = (offsetNumber < (clockTime->getTimezone()->getNumberOfOffsets() - 1)) ? offsetNumber + 1 : 0;

        printk("New offset number: %2d\n", offsetNumber);

        clockTime->getTimezone()->setDstOffset(clockTime->getTimezone()->getOffsetByNumber(offsetNumber));

        clockSettings->setDstOffset(clockTime->getTimezone()->getDstOffset());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess DstOffset: %.2d\n", clockTime->getTimezone()->getDstOffset());
    } else if (clockDisplay->getMode() == ClockDisplay::modeStdWeek) {
        uint8_t week = clockTime->getTimezone()->getStdWeek();
        printk("Week: %2d\n", week);

        clockTime->getTimezone()->setStdWeek((week < 4) ? week + 1 : 0);

        clockSettings->setStdWeek(clockTime->getTimezone()->getStdWeek());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess stdWeek: %.2d\n", clockTime->getTimezone()->getStdWeek());

    } else if (clockDisplay->getMode() == ClockDisplay::modeStdWeekday) {
        uint8_t weekday = clockTime->getTimezone()->getStdWeekday();
        printk("Weekday: %2d\n", weekday);

        clockTime->getTimezone()->setStdWeekday((weekday < 6) ? weekday + 1 : 0);

        clockSettings->setStdWeekday(clockTime->getTimezone()->getStdWeekday());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess stdWeekday: %.2d\n", clockTime->getTimezone()->getStdWeekday());

    } else if (clockDisplay->getMode() == ClockDisplay::modeStdMonth) {
        uint8_t month = clockTime->getTimezone()->getDstMonth();
        printk("Month: %2d\n", month);

        clockTime->getTimezone()->setStdMonth((month < 11) ? month + 1 : 0);

        clockSettings->setStdMonth(clockTime->getTimezone()->getStdMonth());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess stdMonth: %.2d\n", clockTime->getTimezone()->getStdMonth());

    } else if (clockDisplay->getMode() == ClockDisplay::modeStdHour) {
        uint8_t hour = clockTime->getTimezone()->getStdHour();
        printk("Hour: %2d\n", hour);

        clockTime->getTimezone()->setStdHour((hour < 23) ? hour + 1 : 0);

        clockSettings->setStdHour(clockTime->getTimezone()->getStdHour());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess stdHour: %.2d\n", clockTime->getTimezone()->getStdHour());

    } else if (clockDisplay->getMode() == ClockDisplay::modeStdOffset) {
        int offset = clockTime->getTimezone()->getStdOffset();
        printk("Offset: %2d\n", offset);

        uint8_t offsetNumber = clockTime->getTimezone()->getOffsetNumber(offset);
        printk("Offset: %2d, number: %2d\n", offset, offsetNumber);

        //calculate the new offset number
        offsetNumber = (offsetNumber < (clockTime->getTimezone()->getNumberOfOffsets() - 1)) ? offsetNumber + 1 : 0;

        printk("New offset number: %2d\n", offsetNumber);

        clockTime->getTimezone()->setStdOffset(clockTime->getTimezone()->getOffsetByNumber(offsetNumber));

        clockSettings->setStdOffset(clockTime->getTimezone()->getStdOffset());

        //write the new settings to EEPROM
        clockSettings->save();

        printk("In minButtonProcess stdOffset: %.2d\n", clockTime->getTimezone()->getStdOffset());
    } else if (clockDisplay->getMode() == ClockDisplay::modeHourlyAlarm) {
        bool hourlyAlarm = clockSettings->getHourlyAlarm();
        printk("HourlyAlarm: %2d\n", hourlyAlarm);

        //invert bool
        hourlyAlarm = !hourlyAlarm;

        clockSettings->setHourlyAlarm(hourlyAlarm);
        clockSettings->save();

        //enable or disable hourly interrupts
        clockTime->setAlarmInterrupt();

        printk("In minButtonProcess hourlyAlarm: %.2d\n", clockSettings->getHourlyAlarm());
    } else if (clockDisplay->getMode() == ClockDisplay::modeCorrectionOffset) {
        int8_t offset = clockTime->getCorrectionOffset();
        printk("Time Correction Offset: %2d\n", offset);

        //the offset is from -32 to +31
        clockTime->setCorrectionOffset((offset < 31) ? offset + 1 : -32);

        printk("In minButtonProcess Correction Offset: %+.2d\n", clockTime->getCorrectionOffset());
    } else if (clockDisplay->getMode() == ClockDisplay::modeLightSensorValue) {
        printk("In minButtonProcess LightSensorValue. Do nothing, just display the value\n");
    }

    //update display immediately
    clockDisplay->show(false);

}

void ClockButtons::hButtonProcess(void)
{
    switch (clockDisplay->getMode()) {
        case ClockDisplay::modeTime: {
            //get the current time
            clockTime->getRtcTime();

            uint8_t hour = clockTime->getHour();

            clockTime->setHour((hour < 23) ? hour + 1 : 0);

            //write the new time to RTC
            clockTime->setRtcTime();

            printk("In hButtonProcess Time: %.2d:%.2d:%.2d\n", clockTime->getHour(), clockTime->getMinute(), clockTime->getSecond());

            break;
        }
        //switches the display menu
        case ClockDisplay::modeYear:
            clockDisplay->setMode(ClockDisplay::modeMonth);
            break;
        case ClockDisplay::modeMonth:
            clockDisplay->setMode(ClockDisplay::modeDay);
            break;
        case ClockDisplay::modeDay:
            clockDisplay->setMode(ClockDisplay::modeHour);
            break;
        case ClockDisplay::modeHour:
            clockDisplay->setMode(ClockDisplay::modeMinute);
            break;
        case ClockDisplay::modeMinute:
            clockDisplay->setMode(ClockDisplay::modeHourlyAlarm);
            break;
        case ClockDisplay::modeHourlyAlarm:
            clockDisplay->setMode(ClockDisplay::modeDstWeek);
            break;
        case ClockDisplay::modeDstWeek:
            clockDisplay->setMode(ClockDisplay::modeDstWeekday);
            break;
        case ClockDisplay::modeDstWeekday:
            clockDisplay->setMode(ClockDisplay::modeDstMonth);
            break;
        case ClockDisplay::modeDstMonth:
            clockDisplay->setMode(ClockDisplay::modeDstHour);
            break;
        case ClockDisplay::modeDstHour:
            clockDisplay->setMode(ClockDisplay::modeDstOffset);
            break;
        case ClockDisplay::modeDstOffset:
            clockDisplay->setMode(ClockDisplay::modeStdWeek);
            break;
        case ClockDisplay::modeStdWeek:
            clockDisplay->setMode(ClockDisplay::modeStdWeekday);
            break;
        case ClockDisplay::modeStdWeekday:
            clockDisplay->setMode(ClockDisplay::modeStdMonth);
            break;
        case ClockDisplay::modeStdMonth:
            clockDisplay->setMode(ClockDisplay::modeStdHour);
            break;
        case ClockDisplay::modeStdHour:
            clockDisplay->setMode(ClockDisplay::modeStdOffset);
            break;
        case ClockDisplay::modeStdOffset:
            clockDisplay->setMode(ClockDisplay::modeCorrectionOffset);
            break;
        case ClockDisplay::modeCorrectionOffset:
            clockDisplay->setMode(ClockDisplay::modeLightSensorValue);
            break;
        case ClockDisplay::modeLightSensorValue:
            clockDisplay->setMode(ClockDisplay::modeYear);
            break;
    }

    clockDisplay->show(true);
}

void ClockButtons::dateButtonProcess(void)
{
    printk("In dateButtonPress\n");
    clockDisplay->clearScreen();

    clockDisplay->setMode(ClockDisplay::modeDate);
    clockDisplay->show();
}

void ClockButtons::hourButtonProcess(void)
{
    printk("In hourButtonPress\n");
    clockDisplay->clearScreen();

    clockDisplay->setMode(ClockDisplay::modeTime);
    clockDisplay->show();

    this->clockDisplay->wakeup();
}

void ClockButtons::tempButtonProcess(void)
{
    printk("In tempButtonPress\n");
    clockDisplay->clearScreen();

    clockDisplay->setMode(ClockDisplay::modeTemp);
    clockDisplay->show();
}

void ClockButtons::memoButtonProcess(void)
{
    printk("In memoButtonPress\n");

    clockDisplay->setMode(ClockDisplay::modeYear);
    clockDisplay->show();
}


