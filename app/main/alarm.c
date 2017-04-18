#include <stdint.h>
#include <string.h>
#include "clock.h"
#include "alarm.h"

static alarm_union_t alarms[MAX_ALARM_NUM];
uint8_t alarm_later;
uint8_t alarm_num = 0;
alarm_union_t* get_alarm(uint8_t index)
{
    if(index >= MAX_ALARM_NUM) {
        return NULL;
    }
    return &(alarms[index]);
}

uint32_t set_alarm(uint8_t index,alarm_union_t *alarm)
{
    if(index >= MAX_ALARM_NUM) {
        return 1;
    }
    if(NULL == alarm) {
        return 1;
    }
    memcpy(&(alarms[index]),alarm,sizeof(alarm_union_t));
    return 0;
}
void clear_alarms()
{
    alarm_num = 0;
}
uint32_t add_alarm(alarm_union_t *alarm)
{
    if(alarm_num >= MAX_ALARM_NUM)
    {
        return 1;
    }
    else
    {
        memcpy(&(alarms[alarm_num]),alarm,sizeof(alarm_union_t));
        alarm_num ++ ;
        return 0;
    }
}
uint32_t get_alarm_count()
{
    return alarm_num;
}
void check_alarm(void)
{
    uint8_t index;
    static UTCTime  pre_SecondCountRTC;
    DAY_OF_WEEK day_of_week;
    UTCTimeStruct *tm;

    tm = get_clock_time();
    if(alarm_later) {
        alarm_later --;
        if(pre_SecondCountRTC == get_clock_time_counter() - 180) {
            pre_SecondCountRTC = get_clock_time_counter();

        }
    }

    for(index =0; index < alarm_num; index ++) {
        if(tm->hour == get_alarm(index)->bit_field.hour && tm->minutes == get_alarm(index)->bit_field.minute) // time
        {
            day_of_week = get_day_of_week(get_clock_time_counter()); // day of week


            if((get_alarm(index)->bit_field.day_repeat_flag & (1 << day_of_week)) ||
               (tm->day == get_alarm(index)->bit_field.day && tm->month == get_alarm(index)->bit_field.month && tm->year == get_alarm(index)->bit_field.year + 2000))
            {
                pre_SecondCountRTC = get_clock_time_counter();

                alarm_later = 0;

            }
        }
    }
}
