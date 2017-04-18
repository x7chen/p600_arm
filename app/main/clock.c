#include <stdint.h>
#include <string.h>
#include "nrf51.h"
#include "app_timer.h"
#include "clock.h"
#include "app_scheduler.h"

#define APP_TIMER_PRESCALER 0
#define ONE_MINUTE_INTERVAL         APP_TIMER_TICKS(1000*60, APP_TIMER_PRESCALER)
#define ONE_SECOND_INTERVAL         APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)
APP_TIMER_DEF(clockID);
static clock_callbacks_t *clock_callbacks = NULL;
static UTCTimeStruct Global_Time    = {	.year = 2000,
                                        .month = 0,
                                        .day = 0,
                                        .hour = 0,
                                        .minutes = 0,
                                        .seconds = 0};
static UTCTime SecondCountRTC;
void clock_callback_is_null()
{

}
void register_clock_callbacks(clock_callbacks_t *callbacks)
{
    clock_callbacks = callbacks;
    if(clock_callbacks->on_second_update == NULL)
    {
        clock_callbacks->on_second_update = (clock_callback_t)clock_callback_is_null;
    }
    if(clock_callbacks->on_minute_update == NULL)
    {
        clock_callbacks->on_minute_update = (clock_callback_t)clock_callback_is_null;
    }
    if(clock_callbacks->on_hour_update == NULL)
    {
        clock_callbacks->on_hour_update = (clock_callback_t)clock_callback_is_null;
    }
    if(clock_callbacks->on_day_update == NULL)
    {
        clock_callbacks->on_day_update = (clock_callback_t)clock_callback_is_null;
    }
    if(clock_callbacks->on_month_update == NULL)
    {
        clock_callbacks->on_month_update = (clock_callback_t)clock_callback_is_null;
    }
    if(clock_callbacks->on_year_update == NULL)
    {
        clock_callbacks->on_year_update = (clock_callback_t)clock_callback_is_null;
    }
}

/*********************************************************************
 * Get month length
 *********************************************************************/
uint8_t monthLength(uint8_t lpyr, uint8_t mon)
{
	uint8_t days = 31;

	if (mon == 1)
	{
		days = (28 + lpyr);
	}
	else
	{
		if (mon > 6)
		{
			mon--;
		}

		if (mon & 1)
		{
			days = 30;
		}
	}
	return (days);
}

/**************************************************************************
 * Calculte UTCTime
 ***************************************************************************/
void ConvertToUTCTime(UTCTimeStruct *tm, UTCTime secTime)
{
	// calculate the time less than a day - hours, minutes, seconds
	{
		uint32_t day = secTime % DAY;
		tm->seconds = day % 60UL;
		tm->minutes = (day % 3600UL) / 60UL;
		tm->hour = day / 3600UL;
	}

	// Fill in the calendar - day, month, year
	{
		uint16_t numDays = secTime / DAY;
		tm->year = BEGYEAR;
		while (numDays >= YearLength(tm->year))
		{
			numDays -= YearLength(tm->year);
			tm->year++;
		}

		tm->month = 0;
		while (numDays >= monthLength(IsLeapYear(tm->year), tm->month))
		{
			numDays -= monthLength(IsLeapYear(tm->year), tm->month);
			tm->month++;
		}

		tm->day = numDays;
	}
}
void update_clock_scheduler()
{
   
    UTCTimeStruct *UTC_Time;
    static UTCTimeStruct old_UTC_Time;
    static uint32_t seconds=0,minutes=0,hour=0,day=0,month=0,year=0;
	SecondCountRTC++;
	if(clock_callbacks != NULL)
    {
        UTC_Time = get_clock_time();
        clock_callbacks->on_second_update(seconds++);
        if(UTC_Time->minutes != old_UTC_Time.minutes)
        {
            clock_callbacks->on_minute_update(minutes++);
        }
        if(UTC_Time->hour != old_UTC_Time.hour)
        {
            hour++;
            clock_callbacks->on_hour_update(hour++);
        }
        if(UTC_Time->day != old_UTC_Time.day)
        {
            clock_callbacks->on_day_update(day++);
        }
        if(UTC_Time->month != old_UTC_Time.month)
        {
            clock_callbacks->on_month_update(month++);
        }
        if(UTC_Time->year != old_UTC_Time.year)
        {
            clock_callbacks->on_year_update(year++);
        }
        memcpy((uint8_t *)&old_UTC_Time,(uint8_t *)UTC_Time,sizeof(UTCTimeStruct));
    }
}
/**********************************************
 * Time out handle
 ***********************************************/
static void update_clock(void *p_context)
{
    (void) p_context;
    app_sched_event_put(NULL,0,(app_sched_event_handler_t)update_clock_scheduler);
}

/**************************************************************************
 * system clock init
 ***************************************************************************/
void system_clock_init(void)
{
	uint32_t err_code;
	/* Init ticks from RTC */
	SecondCountRTC = 1; /* This should read from flash */
	err_code = app_timer_create(&clockID, APP_TIMER_MODE_REPEATED,update_clock);
	APP_ERROR_CHECK(err_code);
	err_code = app_timer_start(clockID, ONE_SECOND_INTERVAL, NULL);
	APP_ERROR_CHECK(err_code);

}

UTCTime convert_time_to_Second(UTCTimeStruct *time)
{
	uint32_t i = 0;
	UTCTime offset = 0;

	//day time
	offset += time->seconds;
	offset += time->minutes * 60;
	offset += time->hour * 60 * 60;

	uint8_t leapYear = IsLeapYear(time->year + 2000);

	offset += DAY * (time->day - 1);

	for (i = 0; i < time->month - 1; ++i)
	{ //month start from 1
		offset += monthLength(leapYear, i) * DAY;
	}

	for (i = 0; i < time->year; ++i)
	{
		if (IsLeapYear(i + 2000))
		{
			offset += DAY * 366;
		}
		else
		{
			offset += DAY * 365;
		}
	}

	return offset;
}

void set_system_clock(UTCTimeStruct *time)
{
	SecondCountRTC = convert_time_to_Second(time);
	app_timer_stop(clockID);
	app_timer_start(clockID, ONE_SECOND_INTERVAL, NULL);
}

UTCTimeStruct *get_clock_time(void)
{
	ConvertToUTCTime(&Global_Time, SecondCountRTC);
	Global_Time.month += 1; //calibration
	Global_Time.day += 1; //calibration
	return &Global_Time;
}

/* calculate day of week */
DAY_OF_WEEK get_day_of_week(UTCTime secTime)
{
	uint32_t day = secTime / DAY;
	DAY_OF_WEEK today = (DAY_OF_WEEK) (((day % LENGTH_OF_WEEK)
			+ SYSTEM_ORIGIN_DAY_OF_WEEK) %
	LENGTH_OF_WEEK);
	return today;
}

UTCTime get_clock_time_counter(void)
{
	return SecondCountRTC;
}

/******************************************************
 * update utctime only used in restart set time
 *******************************************************/
void set_clock_time_counter(UTCTime counter)
{
	SecondCountRTC = counter;
	app_timer_stop(clockID);
	app_timer_start(clockID, ONE_SECOND_INTERVAL, NULL);
}

bool is_systme_clock_valid(void)
{
	UTCTimeStruct *tm = get_clock_time();
	if (2000 == tm->year)
		return false;
	else
		return true;
}

