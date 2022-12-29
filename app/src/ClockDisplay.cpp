#include <ClockDisplay.h>
#include <font_5x7.h>

bool ClockDisplay::backgroundLightInterruptCalled = true;
gpio_callback ClockDisplay::backgroundLightCallbackData;

ClockDisplay::ClockDisplay(ClockSettings *clockSettings, ClockTime *clockTime, ClockTemperature *clockTemperature)
{
    struct display_capabilities capabilities;

    this->clockSettings = clockSettings;
    this->clockTime = clockTime;
    this->clockTemperature = clockTemperature;

    k_mutex_init(&mutexDisplay);

    threadId = k_current_get();

    //the default mode is time
    setMode(ClockDisplay::modeTime);

    //display = device_get_binding(DT_LABEL(DT_INST(0, holtek_ht1632c)));
    display = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht1632c));

    if (display == NULL) {
        printk("Device HT1632C not found.\n");
        return;
    }

    display_get_capabilities(display, &capabilities);
    printk("Display width %d\n", capabilities.x_resolution);
    printk("Display height %d\n", capabilities.y_resolution);

    //32 rows of 8-bit or 24 rows of 16-bit
    bufDesc.buf_size = displayWidth * capabilities.y_resolution;

    bufDesc.height = capabilities.y_resolution;
    bufDesc.pitch = 1;
    bufDesc.width = displayWidth;

    clearScreen();

    //initBackgroundLightInterrupt();
}

void ClockDisplay::show(bool showTitle)
{
    char displayStr[7];
    memset(displayStr, 0, sizeof(displayStr));

    if (mode == modeTime) {
        uint8_t hour = clockTime->getHour();

        printk("showTime Time: %.2d:%.2d:%.2d", clockTime->getHour(), clockTime->getMinute(), clockTime->getSecond());        
        printk(", Date: %.2d-%.2d-%.2d, Weekday: %.2d\n", clockTime->getYear(), clockTime->getMonth(), clockTime->getDay(), clockTime->getWeekday());

        //convert the hour to the 12-hour clock
        if (this->clockSettings->formatHour == ClockSettings::formatHour12) {
            hour %= 12;
            hour = ((hour == 0) ? 12 : hour);
        }

        sprintf(displayStr, "%2hu:%02hu", hour % 24, clockTime->getMinute() % 60);

        drawString(displayStr);
    } else if (mode == modeDate) {
        clearScreen();

        sprintf(displayStr, "%s%2u", clockTime->getWeekdayName(), clockTime->getDay());
        drawString(displayStr);
    } else if (mode == modeTemp) {
        int temperature = clockTemperature->getTemperature();
        unsigned char degree = 248;

        //show the temperature in Fahrenheit
        if (this->clockSettings->formatTemperature == ClockSettings::formatFahrenheit) {
            temperature = (int)(temperature * 1.8) + 32;
        }

        sprintf(displayStr, "%+.2d%c", temperature, degree);
        printk("displayStr: %s\n", displayStr);
        drawString(displayStr);
    } else if (mode == modeYear) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Year");
        }

        sprintf(displayStr, "%04u", clockTime->getYear());
        drawString(displayStr);
    } else if (mode == modeMonth) {
        clearScreen();
        if (showTitle) {
            drawStringScrolling("Month");
        }

        sprintf(displayStr, "%s", clockTime->getMonthName());
        drawString(displayStr);
    } else if (mode == modeDay) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Day");
        }

        sprintf(displayStr, "%.u", clockTime->getDay());
        drawString(displayStr);
    } else if (mode == modeHour) {
        clearScreen();
        if (showTitle) {
            drawStringScrolling("Hour");
        }

        sprintf(displayStr, "%.2u", clockTime->getHour());
        drawString(displayStr);
    } else if (mode == modeMinute) {
        clearScreen();
        if (showTitle) {
            drawStringScrolling("Minute");
        }

        sprintf(displayStr, "%.2u", clockTime->getMinute());
        drawString(displayStr);
    } else if (mode == modeDstWeek) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Daylight Start Week");
        }

        sprintf(displayStr, "%s", clockTime->getTimezone()->getDstWeekName());
        drawString(displayStr);
    } else if (mode == modeDstWeekday) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Daylight Start Weekday");
        }

        sprintf(displayStr, "%s", clockTime->getTimezone()->getDstWeekdayName());
        drawString(displayStr);
    } else if (mode == modeDstMonth) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Daylight Start Month");
        }

        sprintf(displayStr, "%s", clockTime->getTimezone()->getDstMonthName());
        drawString(displayStr);
    } else if (mode == modeDstHour) {
        if (showTitle) {
            drawStringScrolling("Daylight Start Hour");
        }

        sprintf(displayStr, "%.u", clockTime->getTimezone()->getDstHour());
        drawString(displayStr);
    } else if (mode == modeDstOffset) {
        if (showTitle) {
            drawStringScrolling("Daylight Time Offset");
        }

        int offset = clockTime->getTimezone()->getDstOffset();

        printk("Offset: %d\n", offset);

        sprintf(displayStr, "%+.2hd.%.2hu", offset/60, abs(offset)%60);
        printk("displayStr: %s\n", displayStr);
        drawString(displayStr);
    } else if (mode == modeStdWeek) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Daylight End Week");
        }

        sprintf(displayStr, "%s", clockTime->getTimezone()->getStdWeekName());
        drawString(displayStr);
    } else if (mode == modeStdWeekday) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Daylight End Weekday");
        }

        sprintf(displayStr, "%s", clockTime->getTimezone()->getStdWeekdayName());
        drawString(displayStr);
    } else if (mode == modeStdMonth) {
        clearScreen();

        if (showTitle) {
            drawStringScrolling("Daylight End Month");
        }

        sprintf(displayStr, "%s", clockTime->getTimezone()->getStdMonthName());
        drawString(displayStr);
    } else if (mode == modeStdHour) {
        if (showTitle) {
            drawStringScrolling("Daylight End Hour");
        }

        sprintf(displayStr, "%.u", clockTime->getTimezone()->getStdHour());
        drawString(displayStr);
    } else if (mode == modeStdOffset) {
        if (showTitle) {
            drawStringScrolling("Standard Time Offset");
        }

        int offset = clockTime->getTimezone()->getStdOffset();

        printk("Offset: %d\n", offset);

        sprintf(displayStr, "%+.2hd.%.2hu", offset/60, abs(offset)%60);
        printk("displayStr: %s\n", displayStr);
        drawString(displayStr);
    } else if (mode == modeHourlyAlarm) {
        if (showTitle) {
            drawStringScrolling("Alarm Hourly");
        }

        clearScreen();

        bool hourlyAlarm = clockSettings->getHourlyAlarm();

        printk("HourlyAlarm: %d\n", hourlyAlarm);

        sprintf(displayStr, "%s", (hourlyAlarm ? "Yes" : "No "));
        printk("displayStr: %s\n", displayStr);

        drawString(displayStr);
    } else if (mode == modeCorrectionOffset) {
        if (showTitle) {
            drawStringScrolling("Time Correction Offset (+ faster or - slower)");
        }

        clearScreen();

        int8_t offset = clockTime->getCorrectionOffset();

        printk("Time Correction Offset (+ faster or - slower): %d\n", offset);

        sprintf(displayStr, "%+.2d", offset);
        printk("displayStr: %s\n", displayStr);
        drawString(displayStr);
    } else if (mode == modeLightSensorValue) {
        if (showTitle) {
            drawStringScrolling("Light Sensor Value");
        }

        clearScreen();

        ClockLightSensor lightSensor;
        uint8_t lux = lightSensor.getLightLux();
        sprintf(displayStr, "%04u", lux);

        printk("displayStr: %s\n", displayStr);
        drawString(displayStr);
    }


}

void ClockDisplay::render()
{
/*
    //if the background light interrupt was called
    if (backgroundLightInterruptCalled) {
        printk("backgroundLightInterruptCalled was On\n");

        const struct gpio_dt_spec lightSensor = getBackgroundLightSensor();

        int level = gpio_pin_get_dt(&lightSensor);

        printk("Input Level: %d\n", level);

        backgroundLightOn = (level ? true : false);

        backgroundLightInterruptCalled = false;
    }

    setBackgroundLight(backgroundLightOn);
*/

    //wait for milleseconds until the display is free
    //if not, if can display the time later
    if (k_mutex_lock(&mutexDisplay, K_MSEC(300)) == 0) {
        display_write(display, 0, 0, &bufDesc, bufDisplay);

        k_mutex_unlock(&mutexDisplay);
    } else {
        printk("Mutex timeout\n");
    }

}

/**
 * Clears the screen buffer
 */
void ClockDisplay::clearScreen()
{
    for (size_t i = 0; i < displayWidth; i++) {
        bufDisplay[i] = 0x00;
    }
}

/**
 * Draws a character at the position X
 *
 * @param uint8_t c The character to display
 * @param uint8_t x The position at the display from which to display
 */
void ClockDisplay::drawChar(uint8_t c, uint8_t x) 
{
    // Convert the character to an index
    if (c < ' ') {
        c = 0;
    } else {
        c -= ' ';
    }

    //printk("Character index: %d\n", c);

    for (int i = x, j = 0; j < fontWidth; i++, j++) {
        bufDisplay[i] = font[c][j];
        //printk("i: %d, j: %d, char: %x\n", i, j, (unsigned char)font[c][j]);
    }
}


/**
 * Draws the string on the display and renders it
 */
void ClockDisplay::drawString(const char *displayStr)
{
    //coordinate x
    uint8_t x = 1;

    //1 display indicator width in pixels
    const uint8_t indicatorWidth = 6;

    for (uint8_t i = 0; i < strlen(displayStr); i++) {
        //draw in the special row for a dot '.' character
        if (displayStr[i] == '.') {
            bufDisplay[x - 1] = 0xff;
        } else {
            drawChar(displayStr[i], x);
            x += indicatorWidth;
        }
    }

    render();
}

/**
 * Draws and displays the string scrolling it to the left
 */
void ClockDisplay::drawStringScrolling(const char *displayStr)
{
    //coordinate x
    uint8_t x = 1;

    //1 display indicator width in pixels
    const uint8_t indicatorWidth = 6;
    //the number of the indicators in the display
    const uint8_t numberOfIndicators = 5;


    for (uint8_t k = 0; k < strlen(displayStr); k++) {
        x = 1;
        clearScreen();

        for (uint8_t i = k; i < k + numberOfIndicators; i++) {

            printk("k: %d, i: %d, c: %c\n", k, i, displayStr[i]);
            drawChar(displayStr[i], x);

            if (displayStr[i] == '\0') {
                break;
            }
            x += indicatorWidth;
        }
        render();
        
        k_msleep(300);
    }

}

/**
 * Sets the brightness level
 * The HT1632C display supports 16 levels, so the brightness in luxes must be converted.
 *
 * @param uint16_t The brightness
 */
void ClockDisplay::setBrightness(uint8_t brightness)
{
    printk("Setting brightness %u\n", (unsigned int)brightness);

    if (k_mutex_lock(&mutexDisplay, K_MSEC(300)) == 0) {
        //accepts the value between 0 and 255
        display_set_brightness(display, brightness);

        k_mutex_unlock(&mutexDisplay);
    }

    printk("After setting brightness\n");

}

/**
 * Sets the background light for the buttons
 *
 * @param bool onOff If true, turn it on
 */
void ClockDisplay::setBackgroundLight(bool onOff)
{
/*
    //Sets the ROW30 and ROW31 and COL7
    if (onOff == true) {
        bufBackgroundLightInfo[0] = bufBackgroundLightInfo[1] = 0x01;
    } else {
        bufBackgroundLightInfo[0] = bufBackgroundLightInfo[1] = 0x00;
    }
*/
}

/**
 * Gets the background light sensor
 */
const struct gpio_dt_spec ClockDisplay::getBackgroundLightSensor()
{
    const struct gpio_dt_spec backgroundLightSensor = GPIO_DT_SPEC_GET(DT_PATH(bmw_clock), background_gpios);

    return backgroundLightSensor;
}

/**
 * Inits the background light sensor interrupt
 */
int ClockDisplay::initBackgroundLightInterrupt()
{
    const struct gpio_dt_spec gpio = getBackgroundLightSensor();

    if (!device_is_ready(gpio.port)) {
        printk("Error: GPIO light device %s is not ready\n", gpio.port->name);
        return 1;
    }

    //active when the level is low
    int ret = gpio_pin_configure_dt(&gpio, GPIO_INPUT | GPIO_ACTIVE_LOW | GPIO_PULL_UP);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
           ret, gpio.port->name, gpio.pin);
        return 1;
    }

    ret = gpio_pin_interrupt_configure_dt(&gpio, GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, gpio.port->name, gpio.pin);
        return 1;
    }

    gpio_init_callback(&backgroundLightCallbackData, backgroundLightChanged, BIT(gpio.pin));
    gpio_add_callback(gpio.port, &backgroundLightCallbackData);

    printk("Set up background light GPIO at %s pin %d\n", gpio.port->name, gpio.pin);

    uint8_t levelRaw = gpio_pin_get_raw(gpio.port, gpio.pin);
    printk("\nLevelRaw: %u\n", levelRaw);

    int level = gpio_pin_get_dt(&gpio);
    printk("\nLevel: %d\n", level);

    //check the background light sensor level the first time
    backgroundLightInterruptCalled = true;

    return 0;
}

void ClockDisplay::backgroundLightChanged(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    backgroundLightInterruptCalled = true;

    printk("Background light GPIO interrupt: %s, pins: %zu\n", dev->name, pins);
}
/**
 * Gets the sleep time for the display depending on the data displayed
 */
uint32_t ClockDisplay::getSleepTime()
{
    uint32_t sleepTime = 1000;

    //refresh often only in the time mode
    if (getMode() != ClockDisplay::modeTime) {
        sleepTime = 60000 * 5;
    }

    return sleepTime;
}
