#include <ClockGong.h>

ClockGong::ClockGong()
{
}

void ClockGong::playT1()
{
    int ret = 0;

    printk("Playing gong T1\n");
    const struct gpio_dt_spec gongT1 = GPIO_DT_SPEC_GET(DT_PATH(bmw_gong), t1_gpios);

    if (!device_is_ready(gongT1.port)) {
        printk("Error: GPIO gongT1 %s is not ready\n", gongT1.port->name);
    } else {    
        //active when the level is low
        ret = gpio_pin_configure_dt(&gongT1, GPIO_OUTPUT_LOW);
        if (ret != 0) {
            printk("Error %d: failed to configure %s pin %d\n",
                ret, gongT1.port->name, gongT1.pin);
        }
    }

    ret = gpio_pin_set_dt(&gongT1, 1);
    if (ret != 0) {
        printk("Error %d: pin set T1 %s pin %d\n",
            ret, gongT1.port->name, gongT1.pin);
    }

    k_msleep(500);

    ret = gpio_pin_set_dt(&gongT1, 0);
    if (ret != 0) {
        printk("Error %d: pin set T1 %s pin %d\n",
            ret, gongT1.port->name, gongT1.pin);
    }

}

void ClockGong::playT2()
{
    int ret = 0;

    printk("Playing gong T2\n");
    const struct gpio_dt_spec gongT2 = GPIO_DT_SPEC_GET(DT_PATH(bmw_gong), t2_gpios);

    if (!device_is_ready(gongT2.port)) {
        printk("Error: GPIO gongT2 %s is not ready\n", gongT2.port->name);
    } else {    
        //active when the level is low
        ret = gpio_pin_configure_dt(&gongT2, GPIO_OUTPUT_LOW);
        if (ret != 0) {
            printk("Error %d: failed to configure %s pin %d\n",
                ret, gongT2.port->name, gongT2.pin);
        }
    }

    ret = gpio_pin_set_dt(&gongT2, 1);
    if (ret != 0) {
        printk("Error %d: pin set T2 %s pin %d\n",
            ret, gongT2.port->name, gongT2.pin);
    }

    k_msleep(500);

    ret = gpio_pin_set_dt(&gongT2, 0);
    if (ret != 0) {
        printk("Error %d: pin set T2 %s pin %d\n",
            ret, gongT2.port->name, gongT2.pin);
    }
}



