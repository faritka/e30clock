/*
 * Copyright (c) 2021 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "rv3032.h"

#ifdef CONFIG_PM_DEVICE
/**
 * Turns on the chip from sleep
 * @param dev Pointer to data structure
 */
static int rv3032_exit_sleep(struct rv3032_data *data)
{
    return 0;
}

/**
 * Turns off the chip into sleep
 * @param dev Pointer to data structure
 */
static int rv3032_enter_sleep(struct rv3032_data *data)
{
    return 0;
}
#endif

#ifdef CONFIG_PM_DEVICE
static int rv3032_pm_action(const struct device *dev,
        enum pm_device_action action)
{
    int ret = 0;
    struct rv3032_data *data = (struct rv3032_data *)dev->data;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        ret = rv3032_exit_sleep(data);
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        ret = rv3032_enter_sleep(data);
        break;
    default:
        ret = -ENOTSUP;
        break;
    }

    return ret;
}
#endif /* CONFIG_PM_DEVICE */

/**
 * Reads a byte from EEPROM
 * @param data Pointer to data structure
 * @param uint8_t reg_addr The address of the EEPROM register
 * @param uint8_t value The value to be read from the register
 */
static int rv3032_eeprom_read(const struct device *dev, const uint8_t reg_addr, uint8_t *value)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;

    int ret = 0;
    uint8_t control1_register;
    uint8_t temperature_lsb_register;
    
    k_mutex_lock(&data->lock, K_FOREVER);

    //EERD - EEPROM Memory Refresh Disable bit. When 1, disables the automatic refresh of the
    //Configuration Registers from the EEPROM Memory
    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL1, &control1_register);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading EERD from RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    WRITE_BIT(control1_register, 2, 1);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL1, control1_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EERD to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //check if the EEPROM is not busy
    //by reading EEbusy - EEPROM Memory Busy Status Bit
    while(1) {
        printk("Waiting for EEbusy\n");
        ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_TEMP_LSB, &temperature_lsb_register);
        if (ret != 0) {
            LOG_ERR("read block failed");
            printk("Error reading EEbusy from RTC\n");
            k_mutex_unlock(&data->lock);
            return ret;
        }

        if (!(temperature_lsb_register & BIT(2))) {
            break;
        }
        
        k_msleep(2);
    }

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_EEADDR, reg_addr);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EEADDR to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_EECMD, 0x22);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EECMD to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //t_read = ~1.1 ms.
    k_msleep(2);
    
    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_EEDATA, value);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading EEDATA from RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //enable auto refresh
    WRITE_BIT(control1_register, 2, 0);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL1, control1_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EERD to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    return 0;
}

/**
 * Writes a byte to EEPROM
 * @param data Pointer to data structure
 * @param uint8_t reg_addr The address of the EEPROM register
 * @param uint8_t value The value to be written to the register
 */
static int rv3032_eeprom_write(const struct device *dev, const uint8_t reg_addr, const uint8_t value)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;

    int ret = 0;
    uint8_t control1_register;
    uint8_t temperature_lsb_register;
    
    k_mutex_lock(&data->lock, K_FOREVER);

    //EERD - EEPROM Memory Refresh Disable bit. When 1, disables the automatic refresh of the
    //Configuration Registers from the EEPROM Memory
    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL1, &control1_register);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading EERD from RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    WRITE_BIT(control1_register, 2, 1);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL1, control1_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EERD to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //check if the EEPROM is not busy
    //by reading EEbusy - EEPROM Memory Busy Status Bit
    while(1) {
        printk("Waiting for EEbusy\n");
        ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_TEMP_LSB, &temperature_lsb_register);
        if (ret != 0) {
            LOG_ERR("read block failed");
            printk("Error reading EEbusy from RTC\n");
            k_mutex_unlock(&data->lock);
            return ret;
        }

        if (!(temperature_lsb_register & BIT(2))) {
            break;
        }
        
        k_msleep(2);
    }

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_EEADDR, reg_addr);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EEADDR to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    ret = i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_EEDATA, value);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading EEDATA from RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_EECMD, 0x21);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EECMD to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    //t_write = ~4.8 ms.
    k_msleep(6);

    //enable auto refresh
    WRITE_BIT(control1_register, 2, 0);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL1, control1_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing EERD to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    return 0;
}

/**
 * Reads a byte from the RAM register
 * There are 16 RAM registers: 0x40-0x4f
 * @param data Pointer to data structure
 * @param uint8_t reg_addr The address of the RAM register
 * @param uint8_t value The value to be read from the register
 */

static int rv3032_ram_read(const struct device *dev, uint8_t reg_addr, uint8_t *value)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    int ret = 0;

    k_mutex_lock(&data->lock, K_FOREVER);

    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, reg_addr, value);
    if (ret != 0) {
        LOG_ERR("read register failed");
        printk("Error reading from RAM %u\n", reg_addr);
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    printk("Read the RAM register %u, value: %u\n", reg_addr, *value);

    return 0;
}

/**
 * Writes a byte to the RAM register
 * @param data Pointer to data structure
 * @param uint8_t reg_addr The address of the RAM register
 * @param uint8_t value The value to be written to the register
 */

static int rv3032_ram_write(const struct device *dev, uint8_t reg_addr, uint8_t value)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    int ret = 0;

    k_mutex_lock(&data->lock, K_FOREVER);

    ret = i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, reg_addr, value);
    if (ret != 0) {
        LOG_ERR("Write RAM register failed");
        printk("Error writing to RAM %u\n", reg_addr);
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    printk("Wrote to the RAM register %u, value: %u\n", reg_addr, value);

    return 0;
}


/**
 * Reads the frequency correction setting from the EEPROM
 * @param data Pointer to data structure
 * @param int8_t offset The offset value to be read from the register
 */
static int rv3032_offset_read(const struct device *dev, int8_t *offset)
{
    int ret = 0;
    uint8_t offset_register;

    ret = rv3032_eeprom_read(dev, RV3032_EEPROM_OFFSET, &offset_register);
    if (ret != 0) {
        return ret;
    }
    printk("Offset register read: %u\n", offset_register);

    //read only the last 6 bits
    *offset = offset_register & 0x3f;

    //negative values
    if (*offset >= 32) {
        *offset -= 64;
    }

    printk("Offset value read: %d\n", (int)*offset);

    return 0;
}

/**
 * Writes the value to the frequency correction register in the EEPROM
 * @param data Pointer to data structure
 * @param int8_t offset The offset value to be written to the register
 */
static int rv3032_offset_write(const struct device *dev, int8_t offset)
{
    int ret = 0;
    int8_t offset_register = 0;

    //first read the currect value of the offset register
    ret = rv3032_eeprom_read(dev, RV3032_EEPROM_OFFSET, &offset_register);
    if (ret != 0) {
        return ret;
    }
    printk("Offset register old in write: %u\n", offset_register);

    //the range is from -32 to +31
    if (offset > 31) {
        offset = 31;
    } else if (offset < -32) {
        offset = -32;
    }

    //two's complement number for negative numbers
    if ((offset >= -32) && (offset <= -1)) {
        offset += 64;
    }

    printk("Offset new: %d\n", (int)offset);
    
    offset_register = (offset_register & 0xc0) | offset;

    printk("Offset register new: %u\n", offset_register);

    ret = rv3032_eeprom_write(dev, RV3032_EEPROM_OFFSET, offset_register);
    if (ret != 0) {
        return ret;
    }

    return 0;
}


/**
  * Starts the RTC
  * @param dev Pointer to device structure
  */
static int rv3032_start(const struct device *dev)
{
    ARG_UNUSED(dev);


    return 0;
}

/**
 * Stops the RTC
  * @param dev Pointer to device structure
 */
static int rv3032_stop(const struct device *dev)
{
    ARG_UNUSED(dev);

    return 0;
}

/**
  * Starts the RTC calibration frequency output
  * @param dev Pointer to device structure
  */
static int rv3032_start_calibration(const struct device *dev)
{
    int ret = 0;
    uint8_t pmu_register = 0;
    uint8_t pmu_register_old = 0;
    uint8_t clkout2_register = 0;
    uint8_t clkout2_register_old = 0;

    //read and set the Clkout 2 EEPROM register
    ret = rv3032_eeprom_read(dev, RV3032_EEPROM_CLKOUT2, &clkout2_register_old);
    if (ret != 0) {
        return ret;
    }

    printk("Clockout 2 register old %u\n", clkout2_register_old);

    //OS bit to 0 (XTAL), FD field to 11 (1 Hz)
    clkout2_register = 0x60;

    printk("RV-3032 clkout2_register: %u\n", clkout2_register);

    if (clkout2_register != clkout2_register_old) {
        ret = rv3032_eeprom_write(dev, RV3032_EEPROM_PMU, pmu_register);
        if (ret != 0) {
            return ret;
        }
    }

    //read and set the Power Management Unit EEPROM register
    ret = rv3032_eeprom_read(dev, RV3032_EEPROM_PMU, &pmu_register_old);
    if (ret != 0) {
        return ret;
    }

    printk("PMU register old %u\n", pmu_register_old);

    pmu_register = pmu_register_old;
    //set to 0 to enable the output
    pmu_register |= (0 << 6);

    printk("RV-3032 pmu_register: %u\n", pmu_register);

    if (pmu_register != pmu_register_old) {
        ret = rv3032_eeprom_write(dev, RV3032_EEPROM_PMU, pmu_register);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

/**
  * Stops the RTC calibration frequency output
  * @param dev Pointer to device structure
  */
static int rv3032_stop_calibration(const struct device *dev)
{
    int ret = 0;
    uint8_t pmu_register = 0;
    uint8_t pmu_register_old = 0;

    //read and set the Power Management Unit EEPROM register
    ret = rv3032_eeprom_read(dev, RV3032_EEPROM_PMU, &pmu_register_old);
    if (ret != 0) {
        return ret;
    }

    printk("PMU register old %u\n", pmu_register_old);

    pmu_register = pmu_register_old;
    //set to 1 to disable the output
    pmu_register |= (1 << 6);

    printk("RV-3032 pmu_register: %u\n", pmu_register);

    if (pmu_register != pmu_register_old) {
        ret = rv3032_eeprom_write(dev, RV3032_EEPROM_PMU, pmu_register);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}


/**
 * Processes alarms
 */
static void rv3032_irq_thread(struct rv3032_data *data)
{
    int ret = 0;
    int is_alarm = 0;
    uint8_t status_register;
    rtc_alarm_callback_t alarm_callback = data->callback;

    printk("Starting rv3032_irq_thread\n");
    while (1) {
        is_alarm = 0;
        k_sem_take(&data->irq_sem, K_FOREVER);

        printk("Processing in rv3032_irq_thread\n");
        k_mutex_lock(&data->lock, K_FOREVER);

        //AF status. Alarm Flag. Disable it.
        ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_STATUS, &status_register);
        if (ret != 0) {
            LOG_ERR("read block failed");
            printk("Error reading AF\n");
            k_mutex_unlock(&data->lock);
            continue;
        }

        printk("status_register: %u\n", status_register);

        printk("bit(3): %lu, & %lu\n", BIT(3), (status_register & BIT(3)));

        if (status_register & BIT(3)) {
            is_alarm = 1;
    
            WRITE_BIT(status_register, 3, 0);

            i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_STATUS, status_register);
            if (ret != 0) {
                LOG_ERR("write block failed");
                printk("Error writing AF to RTC\n");
                k_mutex_unlock(&data->lock);
                continue;
            }
        }
        
        k_mutex_unlock(&data->lock);

        if (is_alarm && (alarm_callback != NULL)) {
            printk("alarm callback is called\n");
            alarm_callback(data->i2c, data->user_data);
            printk("alarm callback is finished\n");
        }

        k_sem_reset(&data->irq_sem);

        printk("End processing\n");
    }
}


/**
 * Gets the RTC time
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 */
static int rv3032_get_time(const struct device *dev, struct tm *tm)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    int ret = 0;
    //read starting from the seconds register
    uint8_t reg = RV3032_SECONDS;
    uint8_t time_buf[7];

    k_mutex_lock(&data->lock, K_FOREVER);

    ret = i2c_write_read(data->i2c, RV3032_I2C_ADDRESS, &reg, 1, time_buf, sizeof(time_buf));

    if (ret < 0) {
        LOG_ERR("read block failed");
        printk("Error reading from RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    tm->tm_sec = bcd2bin(time_buf[0]);
    tm->tm_min = bcd2bin(time_buf[1]);
    tm->tm_hour = bcd2bin(time_buf[2]);
    tm->tm_wday = bcd2bin(time_buf[3]);
    tm->tm_mday = bcd2bin(time_buf[4]);
    tm->tm_mon = bcd2bin(time_buf[5]) - 1;
    tm->tm_year = bcd2bin(time_buf[6]) + 100;

    printk("\nrtc_get_time: %.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    printk(", Date: %.2d-%.2d-%.2d\n", tm->tm_mday, tm->tm_mon, tm->tm_year);

    return 0;
}

/**
 * Sets the RTC time
 * @param dev Pointer to device structure
 * @param tm Pointer to tm structure
 */
static int rv3032_set_time(const struct device *dev, struct tm *tm)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    int ret = 0;

    printk("In rtc_settime Time: %.2d:%.2d:%.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);

    printk(", Date: %.2d-%.2d-%.2d, Weekday: %.2d\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday);


    //the buffer that contains values for the time and date
    //it will be sent to the chip 
    //0 - seconds, 1 - minutes, 2 - hours, 3 - weekday, 4 - day, 5 - month, 6 - year
    uint8_t time_buf[8] = {
        //start writing from the seconds address
        RV3032_SECONDS,
        bin2bcd(tm->tm_sec), 
        bin2bcd(tm->tm_min),
        bin2bcd(tm->tm_hour),
        bin2bcd(tm->tm_wday),
        bin2bcd(tm->tm_mday),
        bin2bcd(tm->tm_mon + 1),
        bin2bcd(tm->tm_year - 100),
    };

    k_mutex_lock(&data->lock, K_FOREVER);

    ret = i2c_write(data->i2c, time_buf, sizeof(time_buf), RV3032_I2C_ADDRESS);
    if (ret < 0) {
        LOG_ERR("write block failed");
        printk("Error writing to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    return 0;
}

/**
 * Sets the alarm
 */
static int rv3032_set_alarm(const struct device *dev, 
    const struct rtc_alarm_cfg *alarm_cfg, struct tm *tm, uint32_t mask)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    int ret = 0;
    uint8_t status_register;
    uint8_t control2_register;
    

    data->callback = alarm_cfg->callback;
    data->user_data = alarm_cfg->user_data;

    //registers for alarms
    //bit 7 - AE_M - Minutes Alarm Enable bit, 0 - enabled, 1 - disabled
    //bits 6:0 Minutes Alarm, 00 to 59 in BCD
    uint8_t minutes_alarm = 0;
    //bit 7 - AE_H - Hours Alarm Enable bit, 0 - enabled, 1 -disabled
    //bits 5:0 - Hours Alarm, 00 to 23 in BCD
    uint8_t hours_alarm = 0;
    //bit 7 - AE_D - Date Alarm Enable bit
    //bits 5:0 - Date Alarm, 01 to 31 in BCD (actually, day of month) 
    uint8_t date_alarm = 0;

    minutes_alarm = bin2bcd(tm->tm_min);
    hours_alarm = bin2bcd(tm->tm_hour);
    date_alarm = bin2bcd(tm->tm_mday);

    /* Mask bits
    AE_D AE_H AE_M
     0    0    0   When minutes, hours and day match (once per month)(1) – Default value
     0    0    1   When hours and day match (once per month) 
     0    1    0   When minutes and day match (once per hour per month)
     0    1    1   When day matches (once per month)
     1    0    0   When minutes and hours match (once per day)
     1    0    1   When hours match (once per day)
     1    1    0   When minutes match (once per hour)
     1    1    1   Every minute
    */
    WRITE_BIT(minutes_alarm, 7, (mask & BIT(0)));
    WRITE_BIT(hours_alarm, 7, (mask & BIT(1)));
    WRITE_BIT(date_alarm, 7, (mask & BIT(2)));

    printk("RV-3032 minutes_alarm: %u, hours_alarm: %u, date_alarm: %u\n", minutes_alarm, hours_alarm, date_alarm);

    
    k_mutex_lock(&data->lock, K_FOREVER);

    //AIE Alarm Interrupt Enable bit.
    //0 - No interrupt signal is generated on INT̅pin when an Alarm event occurs
    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL2, &control2_register);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading AIE\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    WRITE_BIT(control2_register, 3, 0);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL2, control2_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing AIE to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //AF status. Alarm Flag. Disable it.
    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_STATUS, &status_register);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading AF\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    WRITE_BIT(status_register, 3, 0);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_STATUS, status_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing AF to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //write in the Minutes Alarm register
    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_MINUTES_ALARM, minutes_alarm);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing minutes alarm to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    //write in the Hours Alarm register
    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_HOURS_ALARM, hours_alarm);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing hours alarm to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    //write in the Date Alarm register
    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_DATE_ALARM, date_alarm);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing date alarm to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    WRITE_BIT(control2_register, 3, 1);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL2, control2_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing AIE to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    rv3032_irq_config(dev);

    printk("Alarm has been set in RV-3032\n");
    return 0;
}


static int rv3032_cancel_alarm(const struct device *dev)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    int ret = 0;
    uint8_t control2_register;

    data->callback = NULL;

    k_mutex_lock(&data->lock, K_FOREVER);

    //AIE Alarm Interrupt Enable bit.
    //0 - No interrupt signal is generated on INT̅pin when an Alarm event occurs
    ret = i2c_reg_read_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL2, &control2_register);
    if (ret != 0) {
        LOG_ERR("read block failed");
        printk("Error reading AIE\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }
    
    WRITE_BIT(control2_register, 3, 0);

    i2c_reg_write_byte(data->i2c, RV3032_I2C_ADDRESS, RV3032_CONTROL2, control2_register);
    if (ret != 0) {
        LOG_ERR("write block failed");
        printk("Error writing AIE to RTC\n");
        k_mutex_unlock(&data->lock);
        return ret;
    }

    k_mutex_unlock(&data->lock);

    return 0;
}

static void rv3032_int_gpio_callback_handler(const struct device *port,
    struct gpio_callback *cb, gpio_port_pins_t pins)
{
    struct rv3032_data *data = CONTAINER_OF(cb, struct rv3032_data, int_gpio_cb);

    ARG_UNUSED(pins);

    printk("In ALARM gpio_callback_handler\n");

    //send the semaphore signal to the child thread
    k_sem_give(&data->irq_sem);
}


int rv3032_irq_config(const struct device *dev)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    const struct rv3032_config *config = (struct rv3032_config *)dev->config;
    int ret = 0;

    printk("RV-3032 irq config\n");

    k_sem_init(&data->irq_sem, 0, 1);

    if (config->int_gpio.port != NULL) {
        if (!device_is_ready(config->int_gpio.port)) {
            LOG_ERR("INT GPIO not ready");
            printk("Interrupt gpio is not ready");
            return -EINVAL;
        }

        ret = gpio_pin_configure_dt(&config->int_gpio, GPIO_INPUT | GPIO_ACTIVE_LOW);
        if (ret < 0) {
            LOG_ERR("failed to configure INT GPIO (err %d)", ret);
            printk("failed to configure INT GPIO (err %d)", ret);
            return ret;
        }

        ret = gpio_pin_interrupt_configure_dt(&config->int_gpio, GPIO_INT_EDGE_TO_ACTIVE);
        if (ret < 0) {
            printk("Error %d: failed to configure interrupt on %s pin %d\n",
                ret, config->int_gpio.port->name, config->int_gpio.pin);
            return ret;
        }


        gpio_init_callback(&data->int_gpio_cb, rv3032_int_gpio_callback_handler,
           BIT(config->int_gpio.pin));

        ret = gpio_add_callback(config->int_gpio.port, &data->int_gpio_cb);
        if (ret < 0) {
            LOG_ERR("failed to add INT GPIO callback (err %d)", ret);
            printk("Failed to add Interrupt GPIO callback\n");
            return ret;
        }

        printk("RV-3032 interrupt callback has been set\n");
    } else {
        printk("The interrupt port for RTC is not set\n");
    }

    //creates a thread to process an alarm. It gets the signal from the IRQ function.
    k_thread_create(&data->irq_thread, data->irq_thread_stack,
        RV3032_IRQ_THREAD_STACK_SIZE,
        (k_thread_entry_t)rv3032_irq_thread, data, NULL, NULL,
        K_PRIO_COOP(2),
        0, K_NO_WAIT);

    return 0;
}

/**
 * @brief Inits the Micro Crystal RV-3032-C7 RTC
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static int rv3032_init(const struct device *dev)
{
    struct rv3032_data *data = (struct rv3032_data *)dev->data;
    const struct rv3032_config *config = (struct rv3032_config *)dev->config;
    int ret;
    uint8_t pmu_register = 0, pmu_register_old = 0;

    k_mutex_init(&data->lock);

    /* Get the I2C device */
    data->i2c = device_get_binding(config->i2c_name);
    if (data->i2c == NULL) {
        LOG_ERR("Could not find I2C device");
        return -EINVAL;
    }

    printk("RV-3032 was found\n");

    //read and set the Power Management Unit EEPROM register
    ret = rv3032_eeprom_read(dev, RV3032_EEPROM_PMU, &pmu_register_old);

    printk("PMU %u\n", pmu_register_old);

    printk("RV-3032 nclke: %u, bsm: %u, tcm: %u, tcr: %u\n", config->nclke, config->bsm, config->tcm, config->tcr);

    pmu_register |= config->tcm | (config->tcr << 2) | (config->bsm << 4) | (config->nclke << 6);

    printk("RV-3032 pmu_register: %u\n", pmu_register);

    if (pmu_register != pmu_register_old) {
        ret = rv3032_eeprom_write(dev, RV3032_EEPROM_PMU, pmu_register);
    }

    int8_t offset_value;
    ret = rv3032_offset_read(dev, &offset_value);

    //uint8_t ram_value = 155;
    //ret = rv3032_ram_write(dev, 0x41, ram_value);
    //ret = rv3032_ram_read(dev, 0x41, &ram_value);

#ifdef CONFIG_PM_DEVICE
    data->pm_state = PM_DEVICE_STATE_ACTIVE;
#endif

    return 0;
}

static const struct rtc_driver_api rv3032_api = {
    .start = rv3032_start,
    .stop = rv3032_stop,
    .get_time = rv3032_get_time,
    .set_time = rv3032_set_time,
    .set_alarm = rv3032_set_alarm,
    .cancel_alarm = rv3032_cancel_alarm,
    .eeprom_write = rv3032_eeprom_write,
    .eeprom_read = rv3032_eeprom_read,
    .ram_write = rv3032_ram_write,
    .ram_read = rv3032_ram_read,
    .offset_read = rv3032_offset_read,
    .offset_write = rv3032_offset_write,
    .start_calibration = rv3032_start_calibration,
    .stop_calibration = rv3032_stop_calibration,
};

static struct rv3032_data rv3032_data = {};

static struct rv3032_config rv3032_config = {
    .i2c_name = DT_INST_BUS_LABEL(0),
    .int_gpio = GPIO_DT_SPEC_INST_GET_OR(0, int_gpios, { 0 }),
    .nclke = DT_INST_PROP(0, nclke),
    .bsm = DT_INST_PROP(0, bsm),
    .tcm = DT_INST_PROP(0, tcm),
    .tcr = DT_INST_PROP(0, tcr),
};

PM_DEVICE_DT_INST_DEFINE(0, rv3032_pm_action);

DEVICE_DT_INST_DEFINE(0, &rv3032_init,
          PM_DEVICE_DT_INST_REF(0), &rv3032_data, &rv3032_config, APPLICATION,
          CONFIG_APPLICATION_INIT_PRIORITY, &rv3032_api);
