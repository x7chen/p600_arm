#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nrf_soc.h"
#include "pstorage.h"
#include "storage_queue.h"

#define FLASH_PAGE_SIZE             (NRF_FICR->CODEPAGESIZE)
#define STORAGE_QUEUE_MAX_PAGE      30
#define STORAGE_QUEUE_MAX_SIZE      (FLASH_PAGE_SIZE*FLASH_STORAGE_MAX_PAGE)
#define STORAGE_QUEUE_MAX_COUNT     4
#define MAX_ITEM_SIZE               16
#define QUEUE_BUFF_MASK             0x07
#define QUEUE_BUFF_SIZE             (QUEUE_BUFF_MASK+1)

__align(4) uint8_t storage_queue_buff[QUEUE_BUFF_SIZE*MAX_ITEM_SIZE];
uint8_t storage_queue_buff_index;
static uint32_t flash_storage_count;
static uint32_t flash_storage_used_page_count;
static bool Storage_Queue_is_Initialized = false;
pstorage_handle_t   f_pstorage_handle;
static void  f_pstorage_cb( pstorage_handle_t  * handle,
                            uint8_t              op_code,
                            uint32_t             result,
                            uint8_t            * p_data,
                            uint32_t             data_len)
{

}
uint32_t storage_queue_init()
{
    uint32_t err_code = 0;
    
    flash_storage_count = 0;
    flash_storage_used_page_count = 0;
    pstorage_module_param_t param;
    param.block_size = FLASH_PAGE_SIZE;
    param.block_count = STORAGE_QUEUE_MAX_PAGE;
    param.cb = f_pstorage_cb;
//    pstorage_init();          //invoked by device_manager_init() in ble_init file , so mask it here
    err_code = pstorage_register(&param,&f_pstorage_handle);
    Storage_Queue_is_Initialized = true;
    return err_code;
}


uint32_t storage_queue_clear(storage_queue_t *queue_handle)
{
    uint32_t err_code = 0;
    pstorage_handle_t block_handle;
    queue_handle->read_page = queue_handle->base_page;
    queue_handle->read_offset = 0;
    queue_handle->write_page = queue_handle->base_page;
    queue_handle->write_offset = 0;
    queue_handle->item_count = 0;
    pstorage_block_identifier_get(&f_pstorage_handle,queue_handle->write_page,&block_handle);
    err_code = pstorage_clear(&block_handle,FLASH_PAGE_SIZE);
    return err_code;
}

uint32_t storage_queue_create(storage_queue_init_t * init,storage_queue_t *queue_handle)
{
    uint32_t block_count = 0;
    if(!Storage_Queue_is_Initialized)
    {
        storage_queue_init();
    }
    if(init->queue_size<=0)
    {
        return 2;
    }
    if(init->item_size > MAX_ITEM_SIZE)
    {
        return 3;
    }
    if((flash_storage_count+1)>=STORAGE_QUEUE_MAX_COUNT)
    {
        return 4;
    }
    block_count = init->queue_size/FLASH_PAGE_SIZE;
    if(init->queue_size%FLASH_PAGE_SIZE!=0)
    {
        block_count++;
    }
    if((flash_storage_used_page_count + block_count) > STORAGE_QUEUE_MAX_PAGE)
    {
        return 5;
    }
    queue_handle->item_size = init->item_size;
    queue_handle->occupy_size = ((init->item_size+3)/4)*4;
    queue_handle->base_page = flash_storage_used_page_count;
    queue_handle->queue_id = flash_storage_count;
    queue_handle->max_item_count = block_count*(FLASH_PAGE_SIZE/queue_handle->occupy_size);
    queue_handle->page_count = block_count;
    queue_handle->page_size = FLASH_PAGE_SIZE;
    storage_queue_clear(queue_handle);
    flash_storage_used_page_count += block_count;
    flash_storage_count++;

    return 0;
}
uint32_t set_storage_next_read_page(storage_queue_t *queue_handle)
{
    uint32_t err_code = 0;
    pstorage_handle_t block_handle;
    pstorage_block_identifier_get(&f_pstorage_handle,queue_handle->write_page,&block_handle);
    err_code = pstorage_clear(&block_handle,FLASH_PAGE_SIZE);
    if(queue_handle->read_page==queue_handle->base_page+queue_handle->page_count)
    {
        queue_handle->read_page = queue_handle->base_page;
    }
    else
    {
        queue_handle->read_page ++ ;
    }
    queue_handle->read_offset = 0;
    return err_code;
}
uint32_t set_storage_next_write_page(storage_queue_t *queue_handle)
{
    if(queue_handle->write_page==queue_handle->base_page+queue_handle->page_count)
    {
        queue_handle->write_page = queue_handle->base_page;
    }
    else
    {
        queue_handle->write_page ++ ;
    }
    queue_handle->write_offset = 0;
    return 0;
}
uint32_t storage_queue_in(storage_queue_t *queue_handle,uint8_t *data)
{
    uint32_t err_code =0;
    pstorage_handle_t block_handle;
    storage_queue_buff_index++;
    storage_queue_buff_index &= QUEUE_BUFF_MASK;
    memcpy(storage_queue_buff+storage_queue_buff_index*MAX_ITEM_SIZE,data,queue_handle->item_size);
    //如果读写块相遇
    //如果写指针小于读指针，需要将读指针移到下一个块，然后清空当前块
    //如果队列为空，直接写，不需要移动读指针
    if((queue_handle->write_page == queue_handle->read_page)\
        &&(queue_handle->write_offset <= queue_handle->read_offset)\
        &&(queue_handle->item_count != 0))
    { 
        queue_handle->item_count -= ((queue_handle->page_size-queue_handle->read_offset)/queue_handle->occupy_size);
        set_storage_next_read_page(queue_handle);    
    }
    //写操作
    pstorage_block_identifier_get(&f_pstorage_handle,queue_handle->write_page,&block_handle);
    err_code = pstorage_store(&block_handle,storage_queue_buff+storage_queue_buff_index*MAX_ITEM_SIZE,queue_handle->occupy_size,queue_handle->write_offset);
    queue_handle->item_count++;
    queue_handle->write_offset += queue_handle->occupy_size;
    if(queue_handle->write_offset>(queue_handle->page_size-queue_handle->occupy_size))
    {
       set_storage_next_write_page(queue_handle); 
    }
    return err_code;
}

uint32_t storage_queue_out(storage_queue_t *queue_handle,uint8_t *data)
{
    uint32_t err_code = 0;
    pstorage_handle_t block_handle;
    if(queue_handle->item_count<=0)
    {
        return 1;
    }
    storage_queue_buff_index++;
    storage_queue_buff_index &= QUEUE_BUFF_MASK;
    //读操作
    pstorage_block_identifier_get(&f_pstorage_handle,queue_handle->read_page,&block_handle);
    err_code = pstorage_load(storage_queue_buff+storage_queue_buff_index*MAX_ITEM_SIZE,&block_handle,queue_handle->occupy_size,queue_handle->read_offset);
    queue_handle->item_count--;
    queue_handle->read_offset += queue_handle->occupy_size;
    if(queue_handle->read_offset>(queue_handle->page_size-queue_handle->occupy_size))
    {
       set_storage_next_read_page(queue_handle); 
    }
    memcpy(data,storage_queue_buff+storage_queue_buff_index*MAX_ITEM_SIZE,queue_handle->item_size);
    return err_code;
}

