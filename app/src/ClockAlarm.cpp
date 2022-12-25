#include <ClockAlarm.h>

ClockAlarm::ClockAlarm(ClockSettings *clockSettings, ClockTime *clockTime)
{
    printk("Init ClockAlarm.\n");

    this->clockSettings = clockSettings;
    this->clockTime = clockTime;
}

void ClockAlarm::process()
{
    printk("Process ClockAlarm thread.\n");

    while (1) {
        k_sem_take(&clockTime->alarmSemaphore, K_FOREVER);

        printk("Processing in ClockAlarm thread\n");

        if (clockSettings->getHourlyAlarm()) {
            //plays the gong sound named T1
            ClockGong clockGong;
            clockGong.playT1();
        }

        k_sem_reset(&clockTime->alarmSemaphore);

        printk("End processing in ClockAlarm\n");
    }

}