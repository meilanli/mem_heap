/***************************
*by meilanli
*原理参考RT-Thread 小内存管理算法
****************************/
#include "mem_heap.h"
#include "stddef.h"
#include "stdio.h"
#include "string.h"

//字节对齐
#define ALIGN_SIZE  4   //4字节对齐
#define MEM_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))
#define MEM_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))


struct MemHeapList
{
    uint32_t used;
    struct MemHeapList *next;   //后向指针
    struct MemHeapList *prev;   //前向指针
};

static struct MemHeapList *s_list_head = NULL;
static struct MemHeapInfo s_heap_info={0,0,0};

#define SIZEOF_STRUCT_HEAP_LIST     MEM_ALIGN(sizeof(struct MemHeapList), ALIGN_SIZE)

static void mem_heap_init(void)
{
    s_list_head = (struct MemHeapList *)(MEM_ALIGN((uint32_t)MEM_HEAP_BEGIN, ALIGN_SIZE));

    s_list_head->used = 0;
    s_list_head->next = NULL;
    s_list_head->prev = NULL;
    
    s_heap_info.used_size = SIZEOF_STRUCT_HEAP_LIST;
    s_heap_info.total_size = MEM_HEAP_END - (uint32_t)MEM_HEAP_BEGIN;
}


void *heap_malloc(uint32_t size)
{
    struct MemHeapList *p0,*next,*pnew;
    uint32_t block_size=0;
    
    if(s_list_head == NULL)
    {
        mem_heap_init();    //堆初始化
    }
    
    //查找空闲空间
    size = MEM_ALIGN(size, ALIGN_SIZE);
    p0 = s_list_head;
    while(p0)
    {
        if( (p0->used) == 0 ) //如果当前内存没使用
        {
            next = p0->next;  //下一块内存
            //当前内存块的空间
            block_size = (next?(uint32_t)next:MEM_HEAP_END) - (uint32_t)p0;
            //如果 当前内存块剩余空间 大于等于 申请长度
            if(  block_size - SIZEOF_STRUCT_HEAP_LIST >= size  )
            {
                //如果分配了内存后，还剩有内存
                //当前内存占用一个指针段长度，分配后剩余内存也要新建一个指针段
                if( block_size - 2*SIZEOF_STRUCT_HEAP_LIST > size )
                {
                    //分配后剩余内存独立成新内存块,在头部建立指针段
                    pnew = (struct MemHeapList *)((uint32_t)p0+size+SIZEOF_STRUCT_HEAP_LIST);
                    pnew->used = 0;
                    pnew->next = next;
                    pnew->prev = p0;
                    p0->next = pnew;
                    if(next)  
                    {
                        next->prev = pnew;
                    }
                    //指针段长度算入已使用空间
                    s_heap_info.used_size += SIZEOF_STRUCT_HEAP_LIST;
                }
                
                p0->used = 1;
                s_heap_info.used_num ++;
                s_heap_info.used_size += size;
                
                memset((void *)((uint32_t)p0+SIZEOF_STRUCT_HEAP_LIST), 0, size);
                //最终给用户使用的内存地址是指针段之后
                return (void *)((uint32_t)p0+SIZEOF_STRUCT_HEAP_LIST);
            }
        }
        //指向下一块内存
        p0 = p0->next;
    }

    return NULL;
}



void heap_free(void *free)
{
    struct MemHeapList *p0,*next,*prev;
    
    if( ((uint32_t)free) < (uint32_t)s_list_head + SIZEOF_STRUCT_HEAP_LIST) return;
    if( ((uint32_t)free) > (uint32_t)MEM_HEAP_END ) return;
    
    p0 = (struct MemHeapList *)((uint32_t)free-SIZEOF_STRUCT_HEAP_LIST);

    next = (struct MemHeapList *)(p0->next);
    prev = (struct MemHeapList *)(p0->prev);
    
    //如果下一块内存是空闲,与当前内存块合并
    if(next && !next->used)
    {
        p0->next = next->next;
        if(next->next)
        {
            next->next->prev = p0;
        }
    }
    
    //如果上一块内存是空闲,与当前内存块合并
    if(prev && !prev->used)
    {
        prev->next = p0->next;
        if(p0->next)
        {
            p0->next->prev = prev;
        }
    }
    
    p0->used = 0;
    s_heap_info.used_num --;
    s_heap_info.used_size -= (next?(uint32_t)next:MEM_HEAP_END) -(uint32_t)p0;
}

struct MemHeapInfo *heap_get_info(void)
{
    if(s_list_head == NULL)
    {
        mem_heap_init();    //堆初始化
    }
    return &s_heap_info;
}


