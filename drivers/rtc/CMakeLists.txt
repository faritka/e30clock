# Copyright (c) 2021 
# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_STM32L_RTC stm32l_rtc)
add_subdirectory_ifdef(CONFIG_MICROCRYSTAL_RV3032 rv3032)
