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

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) ((*(unsigned int *)(p)) = (val))

#define HDRP(bp)     ((char*)(bp) - WSIZE)                       //获得头部的地址
#define FTRP(bp)     ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)  //获得脚部的地址, 与宏定义HDRP有耦合

static char *heap_list;
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) { 
  if((heap_list = mem_sbrk(4*WSIZE)) == (void *)-1)
    return -1;
  PUT(heap_list, 0);                              
  PUT(heap_list + 1*WSIZE, PACK(8, 1));
  PUT(heap_list + 2*WSIZE, PACK(8, 1));
  PUT(heap_list, PACK(0, 1));
  heap_list += (2*WSIZE);

  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
  size_t asize;
  size_t extendsize;
  char *bp = NULL;

  if(size == 0)
    return NULL;

  if((bp = find_fit(asize)) != NULL){
    place(bp, asize);
    return bp;
  }

  extendsize = MAX(asize, CHUNKSIZE);
  if((bp = extend_heap(extendsize)) == NULL)
    return NULL;
  place(bp, asize);
  
  return bp;
}

static void *find_fit(size_t size) {         
    void *bp;
    for (bp = heap_list; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (GET_SIZE(HDRP(bp)) >= size)) 
          return bp;
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
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
  if(ptr==0)
    return;
  size_t size = GET_SIZE(HDRP(ptr));

  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));
  coalesce(ptr);
}

static void *coalesce(void *bp) {          
    int pre_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    int post_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (pre_alloc && post_alloc) {
        return bp;
    } else if (pre_alloc && !post_alloc) {   
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    } else if (!pre_alloc && post_alloc) {   
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
    } else {  
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
size_t old_size, new_size, extendsize;
    void *old_ptr, *new_ptr;

    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    new_size = ALIGN(size + 2*WSIZE);
    old_size = GET_SIZE(HDRP(ptr));
    old_ptr = ptr;
    if (old_size >= new_size) {
        if (old_size - new_size >= DSIZE + 2*WSIZE) {  
            place(old_ptr, new_size);
            return old_ptr;
        } else {   
            return old_ptr;
        }
    } else {
        if ((new_ptr = find_fit(new_size)) == NULL) {
            extendsize = MAX(new_size, CHUNKSIZE);
            if ((new_ptr = extend_heap(extendsize)) == NULL)
                return NULL;
        }
        place(new_ptr, new_size);
        memcpy(new_ptr, old_ptr, old_size - 2*WSIZE);
        mm_free(old_ptr);
        return new_ptr;
    }
}
