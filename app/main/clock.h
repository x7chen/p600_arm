#ifndef _CLOCK_H_
#define _CLOCK_H_
#include <stdint.h>
#include <stdbool.h>
#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define YearLength(yr) (IsLeapYear(yr) ? 366 : 365)

// number of seconds since 0 hrs, 0 minutes, 0 seconds, on the
// 1st of January 2000 UTC

typedef uint32_t UTCTime; /* used to store the second counts for RTC */

#define BEGYEAR         2000     // UTC started at 00:00:00 January 1, 2000

#define DAY             86400UL  // 24 hours * 60 minutes * 60 seconds


// To be used with
typedef struct
{
    uint16_t year;    // 2000+
    uint8_t month;    // 0-11
    uint8_t day;      // 0-30
    uint8_t seconds;  // 0-59
    uint8_t minutes;  // 0-59
    uint8_t hour;     // 0-23
}UTCTimeStruct;
typedef enum {
    MOn  = 0,
    Tues = 1,
    Wed  = 2,
    Thur = 3,
    Fri  = 4,
    Sat  = 5,
    Sun  = 6
}DAY_OF_WEEK;
typedef void (*clock_callback_t)(uint32_t content);
typedef struct{
    clock_callback_t on_second_update;
    clock_callback_t on_minute_update;
    clock_callback_t on_hour_update;
    clock_callback_t on_day_update;
    clock_callback_t on_month_update;
    clock_callback_t on_year_update;
}clock_callbacks_t;

void system_clock_init(void);
void register_clock_callbacks(clock_callbacks_t *callbacks);
void set_system_clock(UTCTimeStruct *time);
UTCTimeStruct * get_clock_time(void);
DAY_OF_WEEK get_day_of_week(UTCTime secTime);//used to calculate day of week

uint8_t monthLength( uint8_t lpyr, uint8_t mon );
UTCTime get_clock_time_counter(void);
void set_clock_time_counter(UTCTime counter);
bool is_systme_clock_valid(void);
UTCTime convert_time_to_Second(UTCTimeStruct *time);
void ConvertToUTCTime( UTCTimeStruct *tm, UTCTime secTime );

//2000-01-01 is sat
#define SYSTEM_ORIGIN_DAY_OF_WEEK (Sat)
#define LENGTH_OF_WEEK      (7)

#endif 
