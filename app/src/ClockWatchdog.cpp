#include <ClockWatchdog.h>

ClockWatchdog::ClockWatchdog()
{
    printk("Init Device Watchdog.\n");

    watchdog = DEVICE_DT_GET(DT_INST(0, st_stm32_watchdog));
    
    if (!watchdog) {
        printk("Device Watchdog IWDT not found.\n");
        return;
    }

    init();
}

int ClockWatchdog::init()
{
    printk("Initializing the Watchdog\n");

    if (!watchdog) {
        return -1;
    }

    struct wdt_timeout_cfg watchdogConfig = {
        // Expire watchdog after max window
        .window = {
            .min = 0U,
            .max = 26214,
        },
        //no callback for IWDT
        .callback = NULL,
        // Reset SoC when watchdog timer expires.
        .flags = WDT_FLAG_RESET_SOC,
    };

    watchdogChannelId = wdt_install_timeout(watchdog, &watchdogConfig);

    if (watchdogChannelId < 0) {
        printk("Watchdog install error\n");
        return -1;
    }

    feed();

    return 0;
}

void ClockWatchdog::feed()
{
    printk("===Feeding the Watchdog===\n");

    if (!watchdog) {
        return;
    }

    wdt_feed(watchdog, watchdogChannelId);
}
