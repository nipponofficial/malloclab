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
		""
};

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

#define GET_SIZE(p)   (GET(p) & ~0x07)
#define GET_ALLOC(p)  (GET(p) & 0x1)

#define HDRP(bp)     ((char*)(bp) - WSIZE)
#define FTRP(bp)     ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define FDP(bp)  (*(char **)(bp + WSIZE))
#define BKP(bp)  (*(char **)(bp))

#define NEXT_BLKP(bp)    ((char*)(bp) + GET_SIZE(HDRP(bp)))  //计算后块的地址
#define PREV_BLKP(bp)    ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))  //计算前块的地址

#define SET_BKP(bp, bkp) (BKP(bp) = bkp)
#define SET_FDP(bp, fdp) (FDP(bp) = fdp)

static char *heap_list, *free_list;

static void *extend_heap(size_t size);     //拓展堆块
static void *find_fit(size_t size);        //寻找空闲块
static void *coalesce(void *bp);           //合并空闲块
static void place(char *bp, size_t size);  //分割空闲块
static void insert(void *bp); /* insert a free block to free list */
static void delete(void *bp); /* delete a free block from free list */
static void mm_checkheap(int verbose);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) { 
		if((heap_list = mem_sbrk(4*WSIZE)) == (void *)-1)
				return -1;
		PUT(heap_list, 0);                              
		PUT(heap_list + (1*WSIZE), PACK(DSIZE, 1));
		PUT(heap_list + (2*WSIZE), PACK(DSIZE, 1));
		PUT(heap_list + (3*WSIZE), PACK(DSIZE, 1));
		heap_list += (2*WSIZE);

		free_list = NULL;

		if (extend_heap(CHUNKSIZE/WSIZE) == NULL)   //拓展堆块
				return -1;
		return 0;
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
			
		if ((bp = find_fit(asize)) != NULL){
				place((char *)bp, asize);
				return bp;
		}

		extendsize = MAX(asize, CHUNKSIZE);
		if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
				return NULL;
		place(bp, asize);
		
		return bp;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
		if (ptr == 0)
				return;
		size_t size = GET_SIZE(HDRP(ptr));

		PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
		PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
		coalesce(ptr);
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
		memcpy(newptr, oldptr, copySize - WSIZE);
		mm_free(oldptr);

		return newptr;
}


static void *extend_heap(size_t size) {
		size_t asize;   
		void *bp;

		size = (size % 2) ? (size + 1) * WSIZE : size * WSIZE;

		if ((long)(bp = mem_sbrk(asize)) == -1)
				return NULL;

		PUT(HDRP(bp), PACK(size, 0));          //HDRP(bp)指向原结尾块
		PUT(FTRP(bp), PACK(size, 0));          
		PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   //新结尾块
		return coalesce(bp);
}


static void *find_fit(size_t size) {         

		for (char* bp = free_list; bp != NULL; bp = FDP(bp)) {
				if (GET_SIZE(HDRP(bp)) >= size) {
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
				PUT(HDRP(bp), PACK(remainder_size, 0));
				PUT(FTRP(bp), PACK(remainder_size, 0));
				coalesce(NEXT_BLKP(bp));
		} else {
				PUT(HDRP(bp), PACK(total_size, 1));
				PUT(FTRP(bp), PACK(total_size, 1));
		}
}

static void *coalesce(void *bp) {
		   
		void* prev_bp = PREV_BLKP(bp);
    void* next_bp = NEXT_BLKP(bp);

    size_t prev_alloc = GET_ALLOC(FTRP(prev_bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));

    size_t current_size = GET_SIZE(HDRP(bp));

    /* Case 0: no need to coalesce */
    if (prev_alloc && next_alloc) {
        DBG_PRINTF("no merge\n");
        insert(bp); 
        return bp; 
    }
    /* 
     * Case 1, previous block is free
     */
    else if (!prev_alloc && next_alloc) {
        DBG_PRINTF("merge prev(%p)\n", prev_bp);
        /* setup merged block */
        current_size += GET_SIZE(HDRP(prev_bp));
        PUT(HDRP(prev_bp), PACK(current_size, 0));
        PUT(FTRP(bp), PACK(current_size, 0));
        return prev_bp;
    }
    /* 
     * Case 2, next block is free, we need to delete next block from free list.
     */
    else if (prev_alloc && !next_alloc) {
        DBG_PRINTF("merge next(%p)\n", next_bp);
        /* Delete next block from free list */
        delete(next_bp);
        /* setup merged block */
        current_size += GET_SIZE(HDRP(next_bp));
        PUT(HDRP(bp), PACK(current_size, 0));
        PUT(FTRP(bp), PACK(current_size, 0));
        insert(bp);
        return bp;
    }
    /* 
     * Case 3, previous and next block both are free, we need to delete next block from free list.
     */
    else {
        DBG_PRINTF("merge prev(%p) and next(%p)\n", prev_bp, next_bp);
        /* Delete next block from free list */
        delete(next_bp);
        /* setup merged block */
        current_size += GET_SIZE(HDRP(prev_bp));
        current_size += GET_SIZE(FTRP(next_bp));
        PUT(HDRP(prev_bp), PACK(current_size, 0));
        PUT(FTRP(next_bp), PACK(current_size, 0));
        return prev_bp;
    }   
}

/*
 * insert - Insert given block pointer to the head of free list.
 * insert() is called by free() or place()
 */
static void insert(void* bp)
{
    DBG_PRINTF("Entering insert(%p)\n", bp);
    if (free_list == NULL) {
        DBG_PRINTF("Free list is NULL, make %p the head of free list\n", bp);
        free_list = bp;
        SET_FDP(bp, NULL);
        SET_BKP(bp, NULL);
        CHECKHEAP(0);
        return;
    }
    /* Set up current block */
    SET_FDP(bp, free_list);
    SET_BKP(bp, 0);
    /* Set next block */
    SET_BKP(free_list, bp);
    /* Free list head is np now */
    free_list = bp;
    CHECKHEAP(0);
}
/*
 * delete - Remove given block pointer from free list.
 * delete() is called by place() or coalesce();
 * we dont' call delete() when there's no free block, so what we deleted is what we inserted.
 */
static void delete(void* bp)
{
    DBG_PRINTF("Entering delete(%p)\n", bp);
    /* Only one free block */
    if(BKP(bp) == NULL && FDP(bp) == NULL) {
        free_list = NULL;
    }
    /* More than one free block, delete the first block */
    else if (BKP(bp) == NULL) {
        /* Fix free list head */
        SET_BKP(FDP(bp), NULL);
        free_list = FDP(bp);
    }
    /* More than one free block, delete the last block */
    else if (FDP(bp) == NULL) {
        SET_FDP(BKP(bp), NULL);
    }
    /* More than two free block, delete the middle block */
    else {
        SET_FDP(BKP(bp), FDP(bp));
        SET_BKP(FDP(bp), BKP(bp));
    }
    CHECKHEAP(0);
}

static void mm_checkheap(int verbose)
{
    DBG_PRINTF("---------------CHECK HEAP START---------------------\n");
    DBG_PRINTF("freelist_headp: %p\n", free_list);
    if (free_list) {
        DBG_PRINTF("Free List: %p<-", BKP(free_list));
        for (char * tmp = free_list; tmp != NULL; tmp = FDP(tmp)) {
            DBG_PRINTF("%p(size: %u)->", tmp, GET_SIZE(HDRP(tmp)));
        }
        DBG_PRINTF("%p\n", NULL);
    }
    DBG_PRINTF("---------------CHECK HEAP END----------------------\n");
}