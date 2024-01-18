/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Wenying Li",
    /* First member's email address */
    "2020080108",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* 16 bytes alignment */
#define ALIGNMENT 16

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
#define PACK(size, alloc) ((size) | (alloc))

#define MAX(x, y)  ((x) > (y) ? (x) : (y))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = (val))

#define GET_SIZE(p)   (GET(p) & ~0x07)    //得到地址p处的 size
#define GET_ALLOC(p)  (GET(p) & 0x1)      //得到地址p处的 allocated bit

#define HDRP(bp)     ((char*)(bp) - WSIZE)                       //获得头部的地址
#define FTRP(bp)     ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)  //获得脚部的地址, 与宏定义HDRP有耦合

#define NEXT_BLKP(bp)    ((char*)(bp) + GET_SIZE((char*)(bp) - WSIZE))  //计算后块的地址
#define PREV_BLKP(bp)    ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))  //计算前块的地址

static char *heap_list, *prev_list;

static void *extend_heap(size_t size);     //拓展堆块
static void *find_fit(size_t size);        //寻找空闲块
static void place(char *bp, size_t size);  //分割空闲块
static void *coalesce(void *bp);           //合并空闲块

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) { 
  if((heap_list = mem_sbrk(4*WSIZE)) == (void *)-1)
    return -1;
  PUT(heap_list, 0);                              
  PUT(heap_list + 1*WSIZE, PACK(DSIZE, 1));
  PUT(heap_list + 2*WSIZE, PACK(DSIZE, 1));
  PUT(heap_list + 3*WSIZE, PACK(DSIZE, 1));
  heap_list += (2*WSIZE);
  prev_list = heap_list;

  if (extend_heap(CHUNKSIZE/WSIZE) == NULL)   //拓展堆块
    return -1;
  return 0;
}

static void *extend_heap(size_t size) {
    size_t asize;   
    void *bp;

    asize = ALIGN(size);
     //printf("extend %d\n", asize);
    if ((long)(bp = mem_sbrk(asize)) == -1)
        return NULL;

    PUT(HDRP(bp), PACK(asize, 0));          //HDRP(bp)指向原结尾块
    PUT(FTRP(bp), PACK(asize, 0));          
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   //新结尾块
    return coalesce(bp);
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
  size_t asize;
  size_t extendsize;
  void *bp = NULL;

  if (size == 0)
    return NULL;
  if (size < DSIZE) 
    asize = 2 * DSIZE;
  else 
    asize = ALIGN(size + 8);
    
  asize = ALIGN(size + 2*WSIZE);

  if ((bp = find_fit(asize)) != NULL){
    place((char *)bp, asize);
    return bp;
  }

  extendsize = MAX(asize, CHUNKSIZE);
  if((bp = extend_heap(extendsize/WSIZE)) == NULL)
    return NULL;
  place(bp, asize);
  
  return bp;
}

static void *find_fit(size_t size) {         
    for (char* bp = prev_list; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= size)){
          prev_list = bp;
          return bp;
        }
    }
    for (char* bp = heap_list; bp != prev_list; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= size)){
          prev_list = bp;
          return bp;
        }
    }
    return NULL;
} 

static void place(char *bp, size_t asize) {
    size_t total_size = GET_SIZE(HDRP(bp));
    size_t remainder_size = total_size - asize;

    if (remainder_size >= 2*DSIZE) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(remainder_size, 0));
        PUT(FTRP(bp), PACK(remainder_size, 0));
    } else {
        PUT(HDRP(bp), PACK(total_size, 1));
        PUT(FTRP(bp), PACK(total_size, 1));
    }
    prev_list = bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
  if (ptr == 0)
    return;
  size_t size = GET_SIZE(HDRP(ptr));

  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));
  coalesce(ptr);
}

static void *coalesce(void *bp) {          
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    if (prev_alloc && next_alloc)
        return bp;
    else if (prev_alloc && !next_alloc) {
        if (NEXT_BLKP(bp) == prev_list)
            prev_list = bp;
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        prev_list = bp;
    }
    else if (!prev_alloc && next_alloc) {
        if (PREV_BLKP(bp) == prev_list)
            prev_list = bp;
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        prev_list = bp;
    }
    else {
        if (PREV_BLKP(bp) == prev_list)
            prev_list = bp;
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        prev_list = bp;
    }
    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
  void *oldptr = ptr;
  void *newptr;
  size_t copySize;

  newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  size = GET_SIZE(HDRP(oldptr));
  copySize = GET_SIZE(HDRP(newptr));
  if (size < copySize)
    copySize = size;
  memcpy(newptr, oldptr, copySize-WSIZE);
  mm_free(oldptr);
  return newptr;
}
