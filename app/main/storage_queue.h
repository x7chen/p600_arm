#ifndef STORAGE_QUEUE_H
#define STORAGE_QUEUE_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pstorage.h"

typedef struct
{
    uint32_t    queue_id;             //  id
    uint32_t    page_size;              //  page size 
    uint32_t    page_count;             //  page count
    uint32_t    read_page;              //  read page
    uint32_t    write_page;             //  write page
    uint32_t    read_offset;            //  read offset
    uint32_t    write_offset;           //  write offset
    uint32_t    base_page;              //  base page
    uint32_t    item_size;              //  item size
    uint32_t    occupy_size;            //  item occupy size
    uint32_t    max_item_count;         //  max items count
    uint32_t    item_count;             //  item count
}storage_queue_t;

typedef struct 
{
    uint32_t    queue_size;
    uint32_t    item_size;
}storage_queue_init_t;

uint32_t storage_queue_create(storage_queue_init_t * init,storage_queue_t *storage_handle);
uint32_t storage_queue_in(storage_queue_t *storage_handle,uint8_t *data);
uint32_t storage_queue_out(storage_queue_t *storage_handle,uint8_t *data);
uint32_t storage_queue_clear(storage_queue_t *storage_handle);

#endif

