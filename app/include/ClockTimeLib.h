/*
 * The class for time functions
 * 
 */
#ifndef __CLOCK_TIMELIB_H
#define __CLOCK_TIMELIB_H

#include <time.h>
#include <sys/printk.h>


class ClockTimeLib 
{
    public:
        /**
         * Converts from UNIX time to the structure tm elements
         */
        static void gmtime(int64_t timeInput, struct tm *tm);

        /**
         * Convert time elements from the structure tm into the UNIX time
         */
        static int64_t mktime(struct tm *tm);

        /**
         * Checks if the year is leap
         */
        static bool isLeapYear(uint16_t year)
        {
            year = 1900 + year;

            return (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0));
        }


        /** Convert a civil (proleptic Gregorian) date to days relative to
         * 1970-01-01.
         *  
         * @param year the calendar year
         * @param month the calendar month, in the range [1, 12]
         * @param day the day of the month, in the range [1, last_day_of_month(y, m)]
         *
         * @return the signed number of days between the specified day and
         * 1970-01-01
         */
        static uint64_t daysFromCivil(uint32_t year, uint8_t month, uint8_t day);

        static const uint8_t monthDays[];

    private:

};

#endif

