#ifndef __MEM_HEAP_H
#define __MEM_HEAP_H
#include "stdint.h"

//以下为了方便MDK环境下自动获取剩余RAM起始地址
//其它平台，自行定义MEM_HEAP_BEGIN，MEM_HEAP_END即可
#define RAM_SIZE   48    //RAM内存总KB
#define RAM_BEGIN  0x20000000   //内存起始地址
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define __CLANG_ARM
#endif

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define MEM_HEAP_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define MEM_HEAP_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define MEM_HEAP_BEGIN    (&__bss_end)
#endif

#ifdef __ICCARM__
// Use *.icf ram symbal, to avoid hardcode.
extern char __ICFEDIT_region_RAM_end__;
#define MEM_SRAM_END          &__ICFEDIT_region_RAM_end__
#else
#define MEM_HEAP_END          (RAM_BEGIN + RAM_SIZE * 1024)
#endif


struct MemHeapInfo
{
    uint32_t used_num;
    uint32_t used_size;
    uint32_t total_size;
};

void *heap_malloc(uint32_t size);
void heap_free(void *free);
struct MemHeapInfo *heap_get_info(void);

#endif
