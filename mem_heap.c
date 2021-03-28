/***************************
*by meilanli
*ԭ��ο�RT-Thread С�ڴ�����㷨
****************************/
#include "mem_heap.h"
#include "stddef.h"
#include "stdio.h"
#include "string.h"

//�ֽڶ���
#define ALIGN_SIZE  4   //4�ֽڶ���
#define MEM_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))
#define MEM_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))


struct MemHeapList
{
    uint32_t used;
    struct MemHeapList *next;   //����ָ��
    struct MemHeapList *prev;   //ǰ��ָ��
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
        mem_heap_init();    //�ѳ�ʼ��
    }
    
    //���ҿ��пռ�
    size = MEM_ALIGN(size, ALIGN_SIZE);
    p0 = s_list_head;
    while(p0)
    {
        if( (p0->used) == 0 ) //�����ǰ�ڴ�ûʹ��
        {
            next = p0->next;  //��һ���ڴ�
            //��ǰ�ڴ��Ŀռ�
            block_size = (next?(uint32_t)next:MEM_HEAP_END) - (uint32_t)p0;
            //��� ��ǰ�ڴ��ʣ��ռ� ���ڵ��� ���볤��
            if(  block_size - SIZEOF_STRUCT_HEAP_LIST >= size  )
            {
                //����������ڴ�󣬻�ʣ���ڴ�
                //��ǰ�ڴ�ռ��һ��ָ��γ��ȣ������ʣ���ڴ�ҲҪ�½�һ��ָ���
                if( block_size - 2*SIZEOF_STRUCT_HEAP_LIST > size )
                {
                    //�����ʣ���ڴ���������ڴ��,��ͷ������ָ���
                    pnew = (struct MemHeapList *)((uint32_t)p0+size+SIZEOF_STRUCT_HEAP_LIST);
                    pnew->used = 0;
                    pnew->next = next;
                    pnew->prev = p0;
                    p0->next = pnew;
                    if(next)  
                    {
                        next->prev = pnew;
                    }
                    //ָ��γ���������ʹ�ÿռ�
                    s_heap_info.used_size += SIZEOF_STRUCT_HEAP_LIST;
                }
                
                p0->used = 1;
                s_heap_info.used_num ++;
                s_heap_info.used_size += size;
                
                memset((void *)((uint32_t)p0+SIZEOF_STRUCT_HEAP_LIST), 0, size);
                //���ո��û�ʹ�õ��ڴ��ַ��ָ���֮��
                return (void *)((uint32_t)p0+SIZEOF_STRUCT_HEAP_LIST);
            }
        }
        //ָ����һ���ڴ�
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
    
    //�����һ���ڴ��ǿ���,�뵱ǰ�ڴ��ϲ�
    if(next && !next->used)
    {
        p0->next = next->next;
        if(next->next)
        {
            next->next->prev = p0;
        }
    }
    
    //�����һ���ڴ��ǿ���,�뵱ǰ�ڴ��ϲ�
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
        mem_heap_init();    //�ѳ�ʼ��
    }
    return &s_heap_info;
}


