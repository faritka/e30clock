#include <ClockTime.h>

// Sunday is 0
const char ClockTime::weekdayNames[][4] = {"Sun", "Mon", "Tue", "Wed", 
    "Thu", "Fri", "Sat"};

const char ClockTime::monthNames[][4] = {"Jan", "Feb", "Mar", "Apr", "May", 
    "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

struct rtc_alarm_cfg ClockTime::alarmConf = {alarmCallback, NULL};

struct k_sem ClockTime::alarmSemaphore;

ClockTime::ClockTime(ClockSettings *clockSettings) 
    : tm(), tmUtc(), timezone(&clockSettings->dstRule, &clockSettings->stdRule, 2020)
{
    k_mutex_init(&mutexRtc);

    k_sem_init(&alarmSemaphore, 0, 1);

    this->clockSettings = clockSettings;

    printk("\nget device rv_3032\n");
    rtc = device_get_binding(DT_LABEL(DT_INST(0, microcrystal_rv3032)));
    
    if (!rtc) {
        printk("Device Microcrystal_rv3032 not found.\n");
        return;
    }

    //sets the interrupt for hourly alarms
    setAlarmInterrupt();

    //the default time
/*
    setSecond(0);
    setMinute(5);
    setHour(18);
    setDay(19);
    setMonth(4);
    setYear(2022);

    setRtcTime();
*/

    printk("Settime Time: %.2d:%.2d:%.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    printk(", Date: %.2d-%.2d-%.2d\n", tm.tm_year, tm.tm_mon, tm.tm_mday);

    getRtcTime();
};

int ClockTime::getRtcTime()
{
    printk("Getting time\n");
    
    if (k_mutex_lock(&mutexRtc, K_MSEC(300)) == 0) {
        //read the UTC time from RTC
        rtc_get_time(rtc, &tmUtc);

        k_mutex_unlock(&mutexRtc);
    } else {
        printk("Cannot lock RTC for reading\n");
    }

    //convert the UTC time to the UNIX format
    int64_t currentTime = ClockTimeLib::mktime(&tmUtc);
    //get the local time in the UNIX format
    int64_t currentTimeLocal = timezone.toLocal(currentTime, getYear());

    printk("currentTime: %lld\n", currentTime);
    printk("currentTimeLocal: %lld\n", currentTimeLocal);

    //convert the UNIX time to the struct tm
    ClockTimeLib::gmtime(currentTimeLocal, &tm);

    printk("getRtcTime: %.2d:%.2d:%.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    printk(", Date: %.2d-%.2d-%.2d\n", tm.tm_year, tm.tm_mon, tm.tm_mday);

    return 0;
}


int ClockTime::setRtcTime()
{
    printk("Setting RTC time\n");

    //get the UNIX time in the local timezone
    int64_t timeLocal = ClockTimeLib::mktime(&tm);

    //get the UNIX time in UTC
    int64_t timeUtc = timezone.toUtc(timeLocal, getYear());
    //convert the UTC UNIX time to the tm structure
    ClockTimeLib::gmtime(timeUtc, &tmUtc);

    printk("Before timeUtc: %lld\n", timeUtc);

    printk("Before setRtcTime: %.2d:%.2d:%.2d", tmUtc.tm_hour, tmUtc.tm_min, tmUtc.tm_sec);

    printk(", Date: %.2d-%.2d-%.2d\n", tmUtc.tm_year, tmUtc.tm_mon, tmUtc.tm_mday);


    if (k_mutex_lock(&mutexRtc, K_MSEC(300)) == 0) {
        //send the utc time to RTC
        rtc_set_time(rtc, &tmUtc);
        
        k_mutex_unlock(&mutexRtc);
    }

    printk("timeUtc: %lld\n", timeUtc);

    printk("setRtcTime: %.2d:%.2d:%.2d", tmUtc.tm_hour, tmUtc.tm_min, tmUtc.tm_sec);

    printk(", Date: %.2d-%.2d-%.2d\n", tmUtc.tm_year, tmUtc.tm_mon, tmUtc.tm_mday);

    return 0;
}
 
uint8_t ClockTime::getDaysInMonth()
{
    uint8_t lastDay = ClockTimeLib::monthDays[tm.tm_mon];

    //February
    if ((tm.tm_mon == 1) && ClockTimeLib::isLeapYear(tm.tm_year)) { 
        lastDay = 29;
    }

    return lastDay;
}

void ClockTime::alarmCallback(const struct device *dev, void *user_data)
{
    printk("=========\nIn alarmCallback\n");


    //send the semaphore signal to the ClockAlarm thread
    k_sem_give(&alarmSemaphore);
}

void ClockTime::setAlarmInterrupt()
{
    if (clockSettings->getHourlyAlarm()) {
        //alarm at the start of each hour
        struct tm tmAlarm;
        tmAlarm.tm_sec = 0;
        tmAlarm.tm_min = 0;
        tmAlarm.tm_hour = 0;
        tmAlarm.tm_wday = 0;
        tmAlarm.tm_mday = 1;

        //mask - do not care about dates and hours
        rtc_set_alarm(rtc, &alarmConf, &tmAlarm, 110);
    } else {
        rtc_cancel_alarm(rtc);
    }
}

/** 
 * Returns the frequency correction offset from the RTC
 */
int8_t ClockTime::getCorrectionOffset()
{
    int8_t offset = 0;

    int ret = rtc_offset_read(rtc, &offset);
    if (ret != 0) {
        return 0;
    }

    return offset;
}

/**
 * Sets the frequency correction offset to the RTC
 * @param int8_t The offset is from -32 to +31
 */
void ClockTime::setCorrectionOffset(int8_t offset) 
{
    int ret = rtc_offset_write(rtc, offset);
    if (ret != 0) {
        return;
    }
}
