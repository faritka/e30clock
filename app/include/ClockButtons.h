/*
 * The class for the buttons
 * 
 */
#ifndef __CLOCK_BUTTONS_H
#define __CLOCK_BUTTONS_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/drivers/gpio.h>

#include <ClockDisplay.h>
#include <ClockTime.h>
#include <ClockTimezone.h>

/**
 * FIFO to store the list of pressed buttons
 */
struct pressedButton {
    /* 1st word reserved for use by FIFO */
    void *fifoReserved;
    uint8_t id;
};


class ClockButtons
{
    public:

        ClockButtons(ClockSettings *clockSettings, ClockTime *clockTime, ClockDisplay *clockDisplay);

        void processButtonActions();

    private:

        /**
         * FIFO to store clicked buttons events for further processing
         */
        static struct k_fifo pressedButtons;

        /**
         * The last time a button was pressed. For debouncing
         */
        static int64_t lastPressedButtonTime;

        /**
         * The last pressed button ID
         */
        static uint8_t lastPressedButtonId;

        /**
         * The debouncer delay interval to wait for the next press in ms
         */
        static const uint16_t debouncerDelay = 50;

        ClockSettings *clockSettings;

        ClockTime *clockTime;

        ClockDisplay *clockDisplay;

        static const uint8_t hButtonId = 1;
        static const uint8_t minButtonId = 2;
        static const uint8_t hourButtonId = 3;
        static const uint8_t dateButtonId = 4;
        static const uint8_t memoButtonId = 5;
        static const uint8_t tempButtonId = 6;
        

        static struct gpio_callback hButtonCallbackData;
        static struct gpio_callback minButtonCallbackData;
        static struct gpio_callback hourButtonCallbackData;
        static struct gpio_callback dateButtonCallbackData;
        static struct gpio_callback memoButtonCallbackData;
        static struct gpio_callback tempButtonCallbackData;

        /**
         * The debouncer function
         */
        static inline int buttonDebouncer(const uint8_t pressedButtonId) 
        {
            int result = 0;
            int64_t pressedButtonTime = k_uptime_get();

            if ((pressedButtonId == lastPressedButtonId) && ((pressedButtonTime - debouncerDelay) <= lastPressedButtonTime)) {
                result = 1;
            }

            lastPressedButtonTime = pressedButtonTime;
            lastPressedButtonId = pressedButtonId;

            return result;
        }

        int initButtonInterrupt(const struct gpio_dt_spec *button, struct gpio_callback *callback, 
            gpio_callback_handler_t callbackFunction);

        static inline void hButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = hButtonId;

            if (buttonDebouncer(eventPressedButton.id)) {
                printk("The button was debounced, ID: %u\n", eventPressedButton.id);
                return;
            }

            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void minButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = minButtonId;

            if (buttonDebouncer(eventPressedButton.id)) {
                printk("The button was debounced, ID: %u\n", eventPressedButton.id);
                return;
            }

            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void hourButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = hourButtonId;

            if (buttonDebouncer(eventPressedButton.id)) {
                printk("The button was debounced, ID: %u\n", eventPressedButton.id);
                return;
            }

            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void dateButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = dateButtonId;

            if (buttonDebouncer(eventPressedButton.id)) {
                printk("The button was debounced, ID: %u\n", eventPressedButton.id);
                return;
            }

            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void memoButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = memoButtonId;

            if (buttonDebouncer(eventPressedButton.id)) {
                printk("The button was debounced, ID: %u\n", eventPressedButton.id);
                return;
            }

            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void tempButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = tempButtonId;

            if (buttonDebouncer(eventPressedButton.id)) {
                printk("The button was debounced, ID: %u\n", eventPressedButton.id);
                return;
            }

            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        void hButtonProcess(void);

        void minButtonProcess(void);

        void dateButtonProcess(void);

        void hourButtonProcess(void);

        void tempButtonProcess(void);

        void memoButtonProcess(void);

        /**
         * Process the hour changing buttons
         */
        void processHourChange();
};

#endif

