#ifndef _ALARM_H_
#define _ALARM_H_

#define MAX_ALARM_NUM       8
typedef struct
{
    uint64_t day_repeat_flag    :7;
    uint64_t reserved           :4;
    uint64_t id                 :3;
    uint64_t minute             :6;
    uint64_t hour               :5;
    uint64_t day                :5;
    uint64_t month              :4;
    uint64_t year               :6;
}alarm_bit_field_t;

typedef union {
    uint64_t data;
    alarm_bit_field_t bit_field;
} alarm_union_t;
alarm_union_t* get_alarm(uint8_t index);
uint32_t set_alarm(uint8_t index,alarm_union_t *alarm);
void clear_alarms(void);
uint32_t add_alarm(alarm_union_t *alarm);
uint32_t get_alarm_count(void);
void check_alarm(void);

#endif
