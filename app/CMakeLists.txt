#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C VERSION 1.0.0)

configure_file(app_version.h.in ${CMAKE_BINARY_DIR}/app/include/app_version.h)
target_include_directories(app PRIVATE ${CMAKE_BINARY_DIR}/app/include include src)

FILE(GLOB app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})
