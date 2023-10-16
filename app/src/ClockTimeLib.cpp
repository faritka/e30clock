#include <ClockTimeLib.h>

const uint8_t ClockTimeLib::monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

uint64_t ClockTimeLib::daysFromCivil(uint32_t year, uint8_t month, uint8_t day)
{
    year -= month <= 2;

    int64_t era = (year >= 0 ? year : year - 399) / 400;
    unsigned int yoe = year - era * 400;
    unsigned int doy = (153U * (month + (month > 2 ? -3 : 9)) + 2U) / 5U + day;
    unsigned int doe = yoe * 365U + yoe / 4U - yoe / 100U + doy;

    return (era * 146097 + (int64_t)doe - 719468);
}

int64_t ClockTimeLib::mktime(struct tm *tm)
{
    int64_t y = 1900 + (int64_t)tm->tm_year;
    unsigned int m = tm->tm_mon + 1;
    unsigned int d = tm->tm_mday - 1;
    int64_t ndays = daysFromCivil(y, m, d);
    int64_t time = tm->tm_sec;

    time += 60LL * (tm->tm_min + 60LL * tm->tm_hour);
    time += 86400LL * ndays;

    tm->tm_wday = (ndays >= -4 ? (ndays + 4) % 7 : (ndays + 5) % 7 + 6);

    return time;
}

void ClockTimeLib::gmtime(int64_t timeInput, struct tm *tm)
{
    //unix time starts from 1970, tm->tm_year from 1900
    uint32_t year = 70;
    uint8_t month, monthLength;
    int64_t time = 0;
    uint32_t days = 0;

    time = timeInput;
    tm->tm_sec = time % 60;

    // now it is minutes
    time /= 60;
    tm->tm_min = time % 60;

    // now it is hours
    time /= 60;
    tm->tm_hour = time % 24;

    // now it is days
    time /= 24; 
    // Sunday is day 0
    tm->tm_wday = ((time + 4) % 7);  
  
    while((unsigned)(days += (isLeapYear(year) ? 366 : 365)) <= time) {
        year++;
    }
    // year is offset from 1900 
    tm->tm_year = year; 
  
    days -= isLeapYear(year) ? 366 : 365;
    
    // now it is days in this year, starting at 0
    time -= days;
  
    days = 0;
    month = 0;
    monthLength = 0;
    for (month = 0; month < 12; month++) {
        // February
        if (month == 1) {
            if (isLeapYear(year)) {
                monthLength = 29;
            } else {
                monthLength = 28;
            }
        } else {
            monthLength = monthDays[month];
        }
    
        if (time >= monthLength) {
            time -= monthLength;
        } else {
            break;
        }
    }

    //January is month 0
    tm->tm_mon = month;

    // day of month, from 1
    tm->tm_mday = time + 1;
}

