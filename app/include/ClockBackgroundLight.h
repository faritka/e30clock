/*
 * The class for the background light
 * 
 */
#ifndef __CLOCK_BACKGROUND_LIGHT_H
#define __CLOCK_BACKGROUND_LIGHT_H

#include <sys/printk.h>

#include <device.h>
#include <devicetree.h>
#include <drivers/display.h>
#include <drivers/gpio.h>

#include <ClockDisplay.h>

class ClockBackgroundLight
{
    public:
        ClockBackgroundLight(ClockDisplay *clockDisplay);

        //the semaphore to tell the ClockBackgroundLight thread that an interrupt was triggered
        static struct k_sem interruptSemaphore;

        //sets the background light for the buttons
        void set(bool onOff);

        //the main loop
        void process();


    private:
        //the clockDisplay object
        ClockDisplay *clockDisplay;

        //the background light sensor interrupt data
        static struct gpio_callback callbackData;
        
        //the background light is on or off
        bool lightOn = false;

        struct display_buffer_descriptor bufDesc;

        //the display width in pixels for the background light info
        const static uint8_t displayWidth = 2;

        //the buffer that holds pixels before they are sent to the display
        uint8_t bufDisplay[2];

        /**
         * Gets the background light sensor
         */
        const struct gpio_dt_spec getSensor();

        /**
         * Inits the background change interrupt
         */
        int initInterrupt();

        /**
         * Background light interrupt function
         */
        static void changed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

        //sends the background light info to the display
        void render();

};

#endif

