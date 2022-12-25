/**
 * @file
 * @brief RTC public API header file.
 */

/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_INCLUDE_DRIVERS_RTC_H_
#define APP_INCLUDE_DRIVERS_RTC_H_

#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Alarm callback
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param user_data User data.
 */
typedef void (*rtc_alarm_callback_t)(const struct device *dev,
        void *user_data);

/** @brief Alarm callback structure.
 *
 * @param callback Callback called on alarm (cannot be NULL).
 * @param ticks Number of ticks that triggers the alarm. It can be relative (to
 * @param user_data User data returned in callback.
 */
struct rtc_alarm_cfg {
    rtc_alarm_callback_t callback;
    void *user_data;
};

/**
 * @typedef rtc_start_api
 * @brief Callback API to start the RTC
 */
typedef int (*rtc_start_api)(const struct device *dev);

/**
 * @typedef rtc_stop_api
 * @brief Callback API to stop the RTC
 */
typedef int (*rtc_stop_api)(const struct device *dev);

/**
 * @typedef rtc_get_time_api
 * @brief Callback API to get the time
 */
typedef int (*rtc_get_time_api)(const struct device *dev, struct tm *tm);

/**
 * @typedef rtc_set_time_api
 * @brief Callback API to set the time
 */
typedef int (*rtc_set_time_api)(const struct device *dev, struct tm *tm);

/**
 * @typedef rtc_set_alarm_api
 * @brief Callback API to set an alarm
 */
typedef int (*rtc_set_alarm_api)(const struct device *dev,
        const struct rtc_alarm_cfg *alarm_cfg, struct tm *tm, uint32_t mask);

/**
 * @typedef rtc_cancel_alarm_api
 * @brief Callback API to cancel the alarm
 */
typedef int (*rtc_cancel_alarm_api)(const struct device *dev);

/**
 * @typedef rtc_eeprom_write_api
 * @brief Callback API to write a value to a EEPROM register
 */
typedef int (*rtc_eeprom_write_api)(const struct device *dev,
    const uint8_t reg_addr, const uint8_t value);

/**
 * @typedef rtc_eeprom_read_api
 * @brief Callback API to read the value from a EEPROM register
 */
typedef int (*rtc_eeprom_read_api)(const struct device *dev,
    const uint8_t reg_addr, uint8_t *value);

/**
 * @typedef rtc_offset_write_api
 * @brief Callback API to write a value to the frequency correction offset register
 */
typedef int (*rtc_offset_write_api)(const struct device *dev,
    int8_t offset);

/**
 * @typedef rtc_offset_read_api
 * @brief Callback API to read the value from the frequency correction offset register
 */
typedef int (*rtc_offset_read_api)(const struct device *dev,
    int8_t *offset);

/**
 * @typedef rtc_ram_write_api
 * @brief Callback API to write a value to a RAM register
 */
typedef int (*rtc_ram_write_api)(const struct device *dev,
    const uint8_t reg_addr, const uint8_t value);

/**
 * @typedef rtc_ram_read_api
 * @brief Callback API to read the value from a RAM register
 */
typedef int (*rtc_ram_read_api)(const struct device *dev,
    const uint8_t reg_addr, uint8_t *value);

/**
 * @typedef rtc_start_calibration_api
 * @brief Callback API to start the calibration frequency output
 */
typedef int (*rtc_start_calibration_api)(const struct device *dev);

/**
 * @typedef rtc_stop_calibration_api
 * @brief Callback API to stop the calibration frequency output
 */
typedef int (*rtc_stop_calibration_api)(const struct device *dev);


/**
 * @brief RTC driver API
 * API which an RTC driver should expose
 */
struct rtc_driver_api {
    rtc_start_api start;
    rtc_stop_api stop;
    rtc_get_time_api get_time;
    rtc_set_time_api set_time;
    rtc_set_alarm_api set_alarm;
    rtc_cancel_alarm_api cancel_alarm;
    rtc_eeprom_read_api eeprom_read;
    rtc_eeprom_write_api eeprom_write;
    rtc_ram_read_api ram_read;
    rtc_ram_write_api ram_write;
    rtc_offset_read_api offset_read;
    rtc_offset_write_api offset_write;
    rtc_start_calibration_api start_calibration;
    rtc_stop_calibration_api stop_calibration;
};

/**
 * @brief Starts the RTC
 *
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_start(const struct device *dev)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->start(dev);
}

/**
 * @brief Stops the RTC
 *
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_stop(const struct device *dev)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->stop(dev);
}

/**
 * @brief Gets the time from RTC
 *
 *
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_get_time(const struct device *dev, struct tm *tm)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->get_time(dev, tm);
}

/**
 * @brief Sets the time in RTC
 *
 *
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_set_time(const struct device *dev, struct tm *tm)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->set_time(dev, tm);
}

/**
 * @brief Sets the alarm in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_set_alarm(const struct device *dev, 
        const struct rtc_alarm_cfg *alarm_cfg, struct tm *tm, uint32_t mask)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->set_alarm(dev, alarm_cfg, tm, mask);
}

/**
 * @brief Cancels the alarm in the RTC
 *
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_cancel_alarm(const struct device *dev)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->cancel_alarm(dev);
}

/**
 * @brief Writes to the EEPROM register in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param uint8_t The address of the EEPROM register
 * @param uint8_t The value to write
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_eeprom_write(const struct device *dev,
    const uint8_t reg_addr, const uint8_t value)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->eeprom_write(dev, reg_addr, value);
}

/**
 * @brief Reads from the EEPROM register in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param uint8_t The address of the EEPROM register
 * @param uint8_t The value to which to read
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_eeprom_read(const struct device *dev,
    const uint8_t reg_addr, uint8_t *value)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->eeprom_read(dev, reg_addr, value);
}

/**
 * @brief Writes to the RAM register in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param uint8_t The address of the RAM register
 * @param uint8_t The value to write
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_ram_write(const struct device *dev,
    const uint8_t reg_addr, const uint8_t value)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->ram_write(dev, reg_addr, value);
}

/**
 * @brief Reads from the RAM register in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param uint8_t The address of the RAM register
 * @param uint8_t The value to which to read
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_ram_read(const struct device *dev,
    const uint8_t reg_addr, uint8_t *value)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->ram_read(dev, reg_addr, value);
}

/**
 * @brief Writes to the time frequency correction offset register in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param int8_t The offset value to write
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_offset_write(const struct device *dev,
    int8_t offset)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->offset_write(dev, offset);
}

/**
 * @brief Reads from the frequency correction offset register in the RTC
 *
 *
 * @param dev Pointer to device structure
 * @param int8_t The offset value to which to read
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_offset_read(const struct device *dev,
    int8_t *offset)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->offset_read(dev, offset);
}


/**
 * @brief Starts the calibration signal frequency output
 *
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_start_calibration(const struct device *dev)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->start_calibration(dev);
}

/**
 * @brief Stops the calibration signal frequency output
 *
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static inline int rtc_stop_calibration(const struct device *dev)
{
    const struct rtc_driver_api *api =
        (const struct rtc_driver_api *)dev->api;

    return api->stop_calibration(dev);
}


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  /* APP_INCLUDE_DRIVERS_RTC_H_ */
