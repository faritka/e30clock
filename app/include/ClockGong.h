/*
 * The class for the gong
 * 
 */
#ifndef __CLOCK_GONG_H
#define __CLOCK_GONG_H

#include <zephyr/sys/printk.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>


class ClockGong
{
    public:
        ClockGong();

        //play the Gong T1 sound
        void playT1();

        //play the Gong T2 sound
        void playT2();
        

    private:

};

#endif

