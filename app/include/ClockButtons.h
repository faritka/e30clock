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

        int initButtonInterrupt(const struct gpio_dt_spec *button, struct gpio_callback *callback, 
            gpio_callback_handler_t callbackFunction);

        static inline void hButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = hButtonId;
            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void minButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = minButtonId;
            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void hourButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = hourButtonId;
            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void dateButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = dateButtonId;
            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void memoButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = memoButtonId;
            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        static inline void tempButtonPressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
        {
            struct pressedButton eventPressedButton;
            eventPressedButton.id = tempButtonId;
            k_fifo_put(&pressedButtons, &eventPressedButton);
            printk("Button pressed: %s, pins: %zu, ID: %u\n", dev->name, pins, eventPressedButton.id);
        }

        void hButtonProcess(void);

        void minButtonProcess(void);

        void dateButtonProcess(void);

        void hourButtonProcess(void);

        void tempButtonProcess(void);

        void memoButtonProcess(void);

};

#endif

