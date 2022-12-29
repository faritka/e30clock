/*
 * The class for the display
 * 
 */
#ifndef __CLOCK_DISPLAY_H
#define __CLOCK_DISPLAY_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <time.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ClockTemperature.h>
#include <ClockTime.h>
#include <ClockTimeLib.h>

#include <ClockLightSensor.h>

class ClockDisplay
{
    public:
        //the operating mode of the display: time, date, temperature
        uint8_t mode = 0;

        static const uint8_t modeTime = 0;
        static const uint8_t modeDate = 1;
        static const uint8_t modeTemp = 2;

        //setting the time modes
        static const uint8_t modeYear = 10;
        static const uint8_t modeMonth = 11;
        static const uint8_t modeDay = 12;
        static const uint8_t modeHour = 13;
        static const uint8_t modeMinute = 14;

        //setting the daylight saving start time for a timezone
        static const uint8_t modeDstWeek = 30;
        static const uint8_t modeDstWeekday = 31;
        static const uint8_t modeDstMonth = 32;
        static const uint8_t modeDstHour = 33;
        static const uint8_t modeDstOffset = 34;

        //setting the standard start time for a timezone
        static const uint8_t modeStdWeek = 40;
        static const uint8_t modeStdWeekday = 41;
        static const uint8_t modeStdMonth = 42;
        static const uint8_t modeStdHour = 43;
        static const uint8_t modeStdOffset = 44;

        //setting an hourly alarm
        static const uint8_t modeHourlyAlarm = 50;

        //setting the frequency correction offset
        static const uint8_t modeCorrectionOffset = 55;

        //shows the light sensor value
        static const uint8_t modeLightSensorValue = 57;
        
        //the display device
        const struct device *display;

        //the mutex to limit simultaneous access to the display hardware
        struct k_mutex mutexDisplay;


        ClockDisplay(ClockSettings *clockSettings, ClockTime *clockTime, ClockTemperature *clockTemperature);

        //show different screens depending on the mode
        void show(bool showTitle = true);

        //sets the brightness level from luxes
        void setBrightness(uint8_t lux);

        //sets the background light for the buttons
        void setBackgroundLight(bool onOff);

        //sets the mode
        void inline setMode(uint8_t mode) {
            this->mode = mode;
        }

        //gets the mode
        uint8_t inline getMode() {
            return this->mode;
        }

        //gets the settings object
        ClockSettings inline *getClockSettings()
        {
            return this->clockSettings;
        }

        /**
         * Clears the screen buffer
         */
        void clearScreen();


        /**
         * Sets the thread ID
         */
        void inline setThreadId(k_tid_t threadId)
        {
            this->threadId = threadId;
        }

        /**
         * Wakes up the display from the while cycle sleep
         */
        void inline wakeup()
        {
            k_wakeup(this->threadId);
        }
        
        /**
         * Gets the sleep time before the next display refresh
         *
         * @return uint32_t The sleep time in ms
         */
        uint32_t getSleepTime();

    private:
        //the clockSettings object
        ClockSettings *clockSettings;

        //the clockTime object
        ClockTime *clockTime;

        //the clockTemperature object
        ClockTemperature *clockTemperature;

        //saves the current thread ID
        k_tid_t threadId;

        //the background light sensor interrupt data
        static struct gpio_callback backgroundLightCallbackData;

        //the background light interrupt was called
        static bool backgroundLightInterruptCalled;
        
        //the background light is on or off
        bool backgroundLightOn = false;

        struct display_buffer_descriptor bufDesc;

        //the display width in pixels
        const static uint8_t displayWidth = 30;

        //the font width
        const static uint8_t fontWidth = 5;

        //the buffer that holds pixels before they are send to the display
        uint8_t bufDisplay[30];

        //FIFO for button presses
        struct k_fifo *pressedButtons;

        //the font
        const static uint8_t font[][5];

        //sends the contents of the buffer to the display
        void render();


        /**
         * Draws a character at the position X
         *
         * @param uint8_t c The character to display
         * @param uint8_t x The position at the display from which to display
         */
        void drawChar(uint8_t c, uint8_t x); 

        /**
         * Draws and displays the string
         */
        void drawString(const char *displayStr);

        /**
         * Draws and displays the string scrolling it to the left
         */
        void drawStringScrolling(const char *displayStr);

        /**
         * Gets the background light sensor
         */
        const struct gpio_dt_spec getBackgroundLightSensor();

        /**
         * Inits the background change interrupt
         */
        int initBackgroundLightInterrupt();

        /**
         * Background light interrupt function
         */
        static void backgroundLightChanged(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

};

#endif

