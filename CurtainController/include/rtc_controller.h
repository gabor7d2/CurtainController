/* DS3231 Real Time Clock AVR Lirary
 *
 * Copyright (C) 2016-2017 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by Adafruit Industries. MIT license.
 */

//////////////////////////////////////////////////////////////////////////
///
/// Real time clock module for communicating with DS3231 RTC
///
/// Keeps the current time in memory by requesting it every 500 ms from
/// the RTC using TaskScheduler.
///
/// Call RTC_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef __DS3231_H__
#define __DS3231_H__

#include "utils.h"

#define RTC_WADDR 0b11010000
#define RTC_RADDR 0b11010001

typedef struct rtc_time
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
} rtc_time;

typedef struct rtc_date
{
	uint8_t wday;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} rtc_date;

/*
 * Clock initialization
 */
void RTC_Init();

/**
 * Gets the current date.
 */
rtc_date RTC_GetDate();

/**
 * Gets the current time.
 */
rtc_time RTC_GetTime();

/**
 * Set date of RTC
 * @param time The new date
 */
void RTC_SetDate(rtc_date *date);

/**
 * Set time of RTC
 * @param time The new time
 */
void RTC_SetTime(rtc_time *time);


#endif
