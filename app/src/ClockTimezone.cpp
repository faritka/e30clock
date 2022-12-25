#include <ClockTimezone.h>

#include <sys/printk.h>

// Week names for timezone changes
const char ClockTimezone::weekNames[][6] = {"Last", "First", "Sec", "Third", "Four"};


// Sunday is 0
const char ClockTimezone::weekdayNames[][4] = {"Sun", "Mon", "Tue", "Wed", 
    "Thu", "Fri", "Sat"};

const char ClockTimezone::monthNames[][4] = {"Jan", "Feb", "Mar", "Apr", "May", 
    "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const int ClockTimezone::offsets[] = {-720, -660, -600, -570, -540, -480, -420, -360, -300, -240, -210, -180, -120, -60, 
    0, 60, 120, 180, 210, 240, 270, 300, 330, 345, 360, 390, 420, 480, 525, 540, 570, 600, 630, 660, 720, 765, 780, 840};

/**
 * 
 * Create a ClockTimezone object from the given time change rules. 
 */
ClockTimezone::ClockTimezone(TimeChangeRule *dstRule, TimeChangeRule *stdRule, uint16_t year)
{
    this->year = 2021;
    setRules(dstRule, stdRule, year);
}


void ClockTimezone::setRules(TimeChangeRule *dstRule, TimeChangeRule *stdRule, uint16_t year)
{
    this->dstRule = dstRule;
    this->stdRule = stdRule;

    //the year has changed
    if (year != this->year) {
        this->year = year;
        calculateTimeChange();
    }
}

void ClockTimezone::calculateTimeChange()
{
    printk("Timezone year: %ld\n", (long)year);

    startDst = mktime(dstRule);
    startDstUtc = startDst - stdRule->offset * 60;

    startStd = mktime(stdRule);
    startStdUtc = startStd - dstRule->offset * 60;

    printk("startDst time: %lld\n", startDst);
    printk("startStd time: %lld\n", startStd);

    printk("startDstUtc time: %lld\n", startDstUtc);
    printk("startStdUtc time: %lld\n", startStdUtc);

}

int64_t ClockTimezone::mktime(const TimeChangeRule *rule)
{
    int64_t t = 0;
    uint16_t tmpYear = year;

    // temp copies of r.month and r.week
    uint8_t month = rule->month;
    uint8_t week = rule->week;

    // calculate first day of the month, or for "Last" rules, first day of the next month
    tm tm;
    
    // is this a "Last week" rule?
    if (week == 0) {
        // yes, for "Last", go to the next month
        if (++month > 12) {
            month = 1;
            ++tmpYear;
        }
        // and treat as first week of next month, subtract 7 days later
        week = 1;
    }

    tm.tm_hour = rule->hour;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_mday = 1;
    tm.tm_mon = month;
    tm.tm_year = tmpYear - 1900;

    t = ClockTimeLib::mktime(&tm);

    // add offset from the first of the month to rule.dow, and offset for the given week
    t += ((rule->dow - tm.tm_wday + 7) % 7 + (week - 1) * 7 ) * 86400;

    // back up a week if this is a "Last" rule
    if (rule->week == 0) {
        t -= 7 * 86400;
    }

    return t;
}

int64_t ClockTimezone::toLocal(int64_t utc, uint16_t year)
{
    // If the year has changed, calculate new time change periods
    if (year != this->year) {
        this->year = year;
        calculateTimeChange();
    }

    if (isDstUtc(utc, year)) {
        return (utc + dstRule->offset * 60LL);
    } else {
        return (utc + stdRule->offset * 60LL);
    }
}

int64_t ClockTimezone::toUtc(int64_t local, uint16_t year)
{
    // If the year has changed, calculate new time change periods
    if (year != this->year) {
        this->year = year;
        calculateTimeChange();
    }

    if (isDstLocal(local, year)) {
        return (local - dstRule->offset * 60);
    } else {
        return (local - stdRule->offset * 60);
    }
}

bool ClockTimezone::isDstUtc(int64_t utc, uint16_t year)
{
    // If the year has changed, calculate new time change periods
    if (year != this->year) {
        this->year = year;
        calculateTimeChange();
    }

    // daylight time not observed in this tz
    if (startDstUtc == startStdUtc) {
        return false;
    } // Northern hemisphere 
    else if (startStdUtc > startDstUtc) {
        return ((utc >= startDstUtc) && (utc < startStdUtc));
    } // Southern hemisphere
    else {
        return !((utc >= startStdUtc) && (utc < startDstUtc));
    }
}

bool ClockTimezone::isDstLocal(int64_t local, uint16_t year)
{
    // If the year has changed, calculate new time change periods
    if (year != this->year) {
        this->year = year;
        calculateTimeChange();
    }

    // daylight time not observed in this tz
    if (startDstUtc == startStdUtc) {
        return false;
    } // Northern hemisphere
    else if (startStd > startDst) {
        return ((local >= startDst) && (local < startStd));
    } // Southern hemisphere 
    else {
        return !((local >= startStd) && (local < startDst));
    }
}
