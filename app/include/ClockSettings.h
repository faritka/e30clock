/*
 * The class for the settings
 * 
 */
#ifndef __CLOCK_SETTINGS_H
#define __CLOCK_SETTINGS_H

#include <sys/printk.h>

#include <device.h>
#include <devicetree.h>

#include <drivers/i2c.h>
#include <drivers/eeprom.h>

#include <drivers/gpio.h>

#include <ClockTimezone.h>

class ClockSettings
{
    public:
        
        //temperature in Fahrenheit
        static const uint8_t formatFahrenheit = 10;
        //temperature in Celsius
        static const uint8_t formatCelsius = 15;

        //12-hour clock
        static const uint8_t formatHour12 = 20;
        //24-hour clock
        static const uint8_t formatHour24 = 25;


        // The daylight time start
        TimeChangeRule dstRule;
        
        // The standard time start
        TimeChangeRule stdRule;

        // If the hourly alarm is on or off
        bool hourlyAlarm = false;

        //how to show temperature: Celsius or Fahrenheit
        uint8_t formatTemperature;
        //12-hour or 24-hour
        uint8_t formatHour;

        ClockSettings();

        /**
         * Loads the variables of the class from the EEPROM
        */
        int load();

        /**
         * Saves the variables of the class  into the EEPROM
         */
        int save();

        /**
         * Sets the daylight weekday number
         */
        inline void setDstWeekday(uint8_t dow)
        {
            dstRule.dow = dow;
        }

        /**
         * Sets the standard weekday number
         */
        inline void setStdWeekday(uint8_t dow)
        {
            stdRule.dow = dow;
        }

        /**
         * Sets the daylight month number
         */
        inline void setDstMonth(uint8_t month)
        {
            dstRule.month = month;
        }

        /**
         * Sets the standard month number
         */
        inline void setStdMonth(uint8_t month)
        {
            stdRule.month = month;
        }

        /**
         * Sets the daylight week number
         */
        inline void setDstWeek(uint8_t week)
        {
            dstRule.week = week;
        }

        /**
         * Sets the standard week number
         */
        inline void setStdWeek(uint8_t week)
        {
            stdRule.week = week;
        }

        /**
         * Sets the daylight time start hour
         */
        inline void setDstHour(uint8_t hour)
        {
            dstRule.hour = hour;
        }

        /**
         * Sets the standard time hour
         */
        inline void setStdHour(uint8_t hour)
        {
            stdRule.hour = hour;
        }

        /**
         * Sets the daylight time offset in minutes
         */
        inline void setDstOffset(int offset)
        {
            dstRule.offset = offset;
        }

        /**
         * Sets the standard time offset in minutes
         */
        inline void setStdOffset(int offset)
        {
            stdRule.offset = offset;
        }

        /**
         * Sets the hourly alarm
         */
        inline void setHourlyAlarm(bool hourlyAlarm)
        {
            this->hourlyAlarm = hourlyAlarm;
        }

        /**
         * Gets the hourly alarm
         */
        inline bool getHourlyAlarm()
        {
            return this->hourlyAlarm;
        }


    private:
        //the settings ID to verify that the data is written correctly
        uint32_t settingsId;

        //sets default values
        void setDefaultValues();

        //get the EEPROM device
        const struct device *getEepromDevice();

        //read the temperature and 12-hour/24-hour format (Celsius/Fahrenheit 12/24 hour) setting pins 
        void readFormat();

        //read one of the format setting pins
        uint8_t readFormatPin(const struct gpio_dt_spec *gpio);
};

#endif

