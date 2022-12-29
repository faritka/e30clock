/*
 * The class to define timezones
 * 
 */
#ifndef __CLOCK_TIMEZONE_H
#define __CLOCK_TIMEZONE_H

#include <time.h>

#include <zephyr/sys/printk.h>
#include <ClockTimeLib.h>

// Constants for TimeChangeRules
enum week_t {Last, First, Second, Third, Fourth}; 
enum dow_t {Sun=0, Mon, Tue, Wed, Thu, Fri, Sat};
enum month_t {Jan=0, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec};


// Structure to describe rules for when daylight/summer time begins,
// or when standard time begins.
struct TimeChangeRule
{
    // five chars max
    char abbrev[6];
    // First, Second, Third, Fourth, or Last week of the month
    uint8_t week;
    // day of week, 0=Sun, 1=Mon, ... 6=Sat
    uint8_t dow;
    // 1=Jan, 2=Feb, ... 12=Dec
    uint8_t month;
    // 0-23
    uint8_t hour;
    // offset from UTC in minutes
    int offset;
};

class ClockTimezone 
{
    public:
        ClockTimezone(TimeChangeRule *dstRule, TimeChangeRule *stdRule, uint16_t year);

        void setRules(TimeChangeRule *dstRule, TimeChangeRule *stdRule, uint16_t year);

        /**
         * Converts the UTC UNIX time to the local time
         */
        int64_t toLocal(int64_t utc, uint16_t year);

        /**
         * Checks if the UTC time is inside the daylight saving interval or not
         */
        bool isDstUtc(int64_t utc, uint16_t year);

        /**
         * Converts the local time to the UTC UNIX time
         */
        int64_t toUtc(int64_t local, uint16_t year);

        /**
         * Checks if the local time is inside the daylight saving interval or not
         */
        bool isDstLocal(int64_t local, uint16_t year);


        /**
         * Sets the daylight weekday number
         */
        inline void setDstWeekday(uint8_t dow)
        {
            dstRule->dow = dow;
        }

        /**
         * Gets the daylight weekday number
         */
        inline const uint8_t getDstWeekday()
        {
            return dstRule->dow;
        }

        /**
         * Returns the daylight weekday name
         */
        inline const char *getDstWeekdayName()
        {
            return (ClockTimezone::weekdayNames[getDstWeekday()]);
        }

        /**
         * Sets the standard weekday number
         */
        inline void setStdWeekday(uint8_t dow)
        {
            stdRule->dow = dow;
        }

        /**
         * Gets the standard weekday number
         */
        inline const uint8_t getStdWeekday()
        {
            return stdRule->dow;
        }

        /**
         * Returns the standard weekday name
         */
        inline const char *getStdWeekdayName()
        {
            return (ClockTimezone::weekdayNames[getStdWeekday()]);
        }

        /**
         * Sets the daylight month number
         */
        inline void setDstMonth(uint8_t month)
        {
            dstRule->month = month;
        }

        /**
         * Gets the daylight month number
         */
        inline const uint8_t getDstMonth()
        {
            return dstRule->month;
        }

        /**
         * Returns the daylight month name
         */
        inline const char *getDstMonthName()
        {
            return (ClockTimezone::monthNames[getDstMonth()]);
        }

        /**
         * Sets the standard month number
         */
        inline void setStdMonth(uint8_t month)
        {
            stdRule->month = month;
        }

        /**
         * Gets the standard month number
         */
        inline const uint8_t getStdMonth()
        {
            return stdRule->month;
        }

        /**
         * Returns the standard month name
         */
        inline const char *getStdMonthName()
        {
            return (ClockTimezone::monthNames[getStdMonth()]);
        }

        /**
         * Sets the daylight week number
         */
        inline void setDstWeek(uint8_t week)
        {
            dstRule->week = week;
        }

        /**
         * Gets the daylight week number
         */
        inline const uint8_t getDstWeek()
        {
            return dstRule->week;
        }

        /**
         * Returns the daylight time week name
         */
        inline const char *getDstWeekName()
        {
            return (ClockTimezone::weekNames[getDstWeek()]);
        }

        /**
         * Sets the standard time week number
         */
        inline void setStdWeek(uint8_t week)
        {
            stdRule->week = week;
        }

        /**
         * Gets the standard time week number
         */
        inline const uint8_t getStdWeek()
        {
            return stdRule->week;
        }

        /**
         * Returns the standard time week name
         */
        inline const char *getStdWeekName()
        {
            return (ClockTimezone::weekNames[getStdWeek()]);
        }

        /**
         * Sets the daylight time start hour
         */
        inline void setDstHour(uint8_t hour)
        {
            dstRule->hour = hour;
        }

        /**
         * Gets the daylight time start hour
         */
        inline const uint8_t getDstHour()
        {
            return dstRule->hour;
        }

        /**
         * Sets the standard time hour
         */
        inline void setStdHour(uint8_t hour)
        {
            stdRule->hour = hour;
        }

        /**
         * Gets the standard time start hour
         */
        inline const uint8_t getStdHour()
        {
            return stdRule->hour;
        }

        /**
         * Sets the daylight time offset in minutes
         */
        inline void setDstOffset(int offset)
        {
            dstRule->offset = offset;
        }

        /**
         * Gets the daylight time offset in minutes
         */
        inline const int getDstOffset()
        {
            return dstRule->offset;
        }

        /**
         * Sets the standard time offset in minutes
         */
        inline void setStdOffset(int offset)
        {
            stdRule->offset = offset;
        }

        /**
         * Gets the standard time offset in minutes
         */
        inline const int getStdOffset()
        {
            return stdRule->offset;
        }

        /**
         * Gets the number of available offsets
         */
        inline const int getNumberOfOffsets()
        {
            return sizeof(offsets)/sizeof(*offsets);
        }

        /**
         * Gets the number of the offset
         */
        inline const uint8_t getOffsetNumber(int offset)
        {
            printk("numberofoffsets: %d\n", getNumberOfOffsets());

            for (uint8_t i = 0; i < getNumberOfOffsets(); i++) {
                if (offsets[i] == offset) {
                    printk("offsets[i]: %d\n", offsets[i]);
                    return i;
                }
            }

            return 0;
        }

        /**
         * Gets the offset by its number in the array
         */
        inline const int getOffsetByNumber(uint8_t offsetNumber)
        {
            return (offsets[offsetNumber]);
        }

    private:
        //the current year
        uint16_t year;

        //the daylight saving start rule
        TimeChangeRule *dstRule;
        //the standard time start rule
        TimeChangeRule *stdRule;

        // the start of the daylight saving time for the year
        int64_t startDst;
        // the start of the daylight saving time for the year in UTC
        int64_t startDstUtc;

        // the start of the standard time for the year
        int64_t startStd;
        // the start of the standard time for the year in UTC
        int64_t startStdUtc;

        //the week names for timezone changes
        static const char weekNames[5][6];

        //the weekday names
        static const char weekdayNames[8][4];

        //the month names
        static const char monthNames[12][4];

        //the offsets
        static const int offsets[38];

        /**
         * Calculates start and end times of daylight saving times for the year
         */
        void calculateTimeChange();

        /**
         * Convert the rule to the UNIX time
         */
        int64_t mktime(const TimeChangeRule *rule);

};

#endif