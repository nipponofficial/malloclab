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

// #define WSIZE 4
// #define DSIZE 8
// #define CHUNKSIZE (1 << 12)

// #define MAX(x, y)  ((x) > (y) ? (x) : (y))

// #define PACK(size, alloc) ((size) | (alloc))

// #define GET(p) (*(unsigned int *)(p))
// #define PUT(p, val) ((*(unsigned int *)(p)) = (val))

// #define GET_SIZE(p)   (GET(p) & ~0x07)
// #define GET_ALLOC(p)  (GET(p) & 0x1)

// #define HDRP(bp)     ((char*)(bp) - WSIZE)
// #define FTRP(bp)     ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// #define NEXT_BLKP(bp)    ((char*)(bp) + GET_SIZE(HDRP(bp)))  //计算后块的地址
// #define PREV_BLKP(bp)    ((char*)(bp) - GET_SIZE((char*)(bp) - DSIZE))  //计算前块的地址

// static char *heap_list, *prev_list;

// static void *extend_heap(size_t size);     //拓展堆块
// static void *find_fit(size_t size);        //寻找空闲块
// static void *coalesce(void *bp);           //合并空闲块
// static void place(char *bp, size_t size);  //分割空闲块

// /*
//  * mm_init - initialize the malloc package.
//  */
// int mm_init(void) { 
// 		if((heap_list = mem_sbrk(4*WSIZE)) == (void *)-1)
// 				return -1;
// 		PUT(heap_list, 0);                              
// 		PUT(heap_list + (1*WSIZE), PACK(DSIZE, 1));
// 		PUT(heap_list + (2*WSIZE), PACK(DSIZE, 1));
// 		PUT(heap_list + (3*WSIZE), PACK(DSIZE, 1));
// 		heap_list += (2*WSIZE);

// 		prev_list = heap_list;

// 		if (extend_heap(CHUNKSIZE/WSIZE) == NULL)   //拓展堆块
// 				return -1;
// 		return 0;
// }


// /*
//  * mm_malloc - Allocate a block by incrementing the brk pointer.
//  *     Always allocate a block whose size is a multiple of the alignment.
//  */
// void *mm_malloc(size_t size) {
// 		size_t asize;
// 		size_t extendsize;
// 		void *bp = NULL;

// 		if (size == 0) return NULL;

// 		if (size < DSIZE) 
// 				asize = 2 * DSIZE;
// 		else 
// 				// asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
// 				asize = ALIGN(size + 8);

// 		if ((bp = find_fit(asize)) != NULL){
// 				place((char *)bp, asize);
// 				return bp;
// 		}

// 		extendsize = MAX(asize, CHUNKSIZE);
// 		if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
// 				return NULL;
// 		place(bp, asize);
		
// 		return bp;
// }


// /*
//  * mm_free - Freeing a block does nothing.
//  */
// void mm_free(void *ptr) {
// 		if (ptr == NULL)
// 				return;

// 		size_t size = GET_SIZE(HDRP(ptr));
// 		PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(size)), 0));
// 		PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(size)), 0));
// 		coalesce(ptr);
// }


// /*
//  * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
//  */
// void *mm_realloc(void *ptr, size_t size) {
// 		if (ptr == NULL)
//         return mm_malloc(size);

//     if (size == 0) {
//         mm_free(ptr);
//         return NULL;
//     }

//     void *oldptr = ptr;
//     void *newptr;
//     size_t copySize;
    
//     newptr = mm_malloc(size);
//     if (newptr == NULL)
//         return NULL;
//     copySize = GET_SIZE(HDRP(newptr));
//     if (size < copySize)
//         copySize = size;
//     memcpy(newptr, oldptr, copySize);
//     mm_free(oldptr);
//     return newptr;
// }


// static void *extend_heap(size_t size) {
// 		size_t asize;   
// 		void *bp;

// 		asize = (size % 2) ? (size + 1) * WSIZE : size * WSIZE;

// 		if ((long)(bp = mem_sbrk(asize)) == -1)
// 				return NULL;

// 		PUT(HDRP(bp), PACK(asize, 0));          //HDRP(bp)指向原结尾块
// 		PUT(FTRP(bp), PACK(asize, 0));          
// 		PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));   //新结尾块
// 		return coalesce(bp);
// }


// static void *find_fit(size_t size) {         

// 		void *bp;
//     for (bp = prev_list; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
//         if (!GET_ALLOC(HDRP(bp)) && (size <= GET_SIZE(HDRP(bp)))) {
//             prev_list = bp;
//             return bp;
//         }
//     }
//     for (bp = heap_list; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
//         if (!GET_ALLOC(HDRP(bp)) && (size <= GET_SIZE(HDRP(bp)))) {
//             prev_list = bp;
//             return bp;
//         }
//     }
//     return NULL;
// } 

// static void place(char *bp, size_t asize) {
// 		size_t total_size = GET_SIZE(HDRP(bp));
// 		size_t remainder_size = total_size - asize;

// 		if (remainder_size >= 2*DSIZE) {
// 				PUT(HDRP(bp), PACK(asize, 1));
// 				PUT(FTRP(bp), PACK(asize, 1));
// 				bp = NEXT_BLKP(bp);
// 				PUT(HDRP(bp), PACK(remainder_size, 0));
// 				PUT(FTRP(bp), PACK(remainder_size, 0));
// 		} else {
// 				PUT(HDRP(bp), PACK(total_size, 1));
// 				PUT(FTRP(bp), PACK(total_size, 1));
// 		}
// 		prev_list = bp;
// }

// static void *coalesce(void *bp) {
		   
// 		size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
//     size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
//     size_t size = GET_SIZE(HDRP(bp));

//     if (prev_alloc && next_alloc) {                     /* Case 1 */
//         /* Do nothing */
//     } else if (prev_alloc && !next_alloc) {             /* Case 2 */
//         size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
//         PUT(HDRP(bp), PACK(size, 0));
//         PUT(FTRP(bp), PACK(size, 0));
//     } else if (!prev_alloc && next_alloc) {             /* Case 3 */
//         size += GET_SIZE(HDRP(PREV_BLKP(bp)));
//         PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//         PUT(FTRP(bp), PACK(size, 0));
//         bp = PREV_BLKP(bp);
//     } else {                                            /* Case 4 */
//         size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(HDRP(PREV_BLKP(bp)));
//         PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//         PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
//         bp = PREV_BLKP(bp);
//     }
//     prev_list = bp;
//     return bp;
// }

//* Basic constants and macros: */
#define WSIZE      4          /* Word and header/footer size (bytes) */
#define DSIZE      8          /* Doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)    /* Extend heap by this amount (bytes) */
#define MINBLOCKSIZE 16

/* Max value of 2 values */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p. */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)   (GET(p) & ~0x7) /* Take attention!!! size = header + payload + footer */
#define GET_ALLOC(p)  (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)  ((char *)(bp) - WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

/* Global declarations */
static char *heap_listp, *prev_listp;

/* Function prototypes for internal helper routines */
static void *coalesce(void *bp);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1)
        return -1;
    PUT(heap_listp, 0);                             /* alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(DSIZE, 1));    /* Epilogue header */
    heap_listp += (2*WSIZE);    /* make heap_listp points to epilogue header */
    prev_listp = heap_listp;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *             Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if not fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0) {
        return NULL;
    }

    /* Adjust block size to include overhead and alignment reqs */
    if (size < DSIZE)
        asize = 2 * DSIZE; /* Minimum block size: WORD(HDR) + DWORD(Payload) + WORD(FTR) */
    else
        asize = ALIGN(size + 8);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block and coalesce if necessary.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
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

/*
 * coalesce - Merge freed block.
 */
static void* coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    if (prev_alloc && next_alloc)
        return bp;
    else if (prev_alloc && !next_alloc) {
        if (NEXT_BLKP(bp) == prev_listp)
            prev_listp = bp;
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        prev_listp = bp;
    }
    else if (!prev_alloc && next_alloc) {
        if (PREV_BLKP(bp) == prev_listp)
            prev_listp = bp;
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        prev_listp = bp;
    }
    else {
        if (PREV_BLKP(bp) == prev_listp)
            prev_listp = bp;
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        prev_listp = bp;
    }
    // prev_listp = bp;
    return bp;
}

/*
 * extend_heap - Extend the heap with a free block and return that block's payload address.
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* Find freeblock that fits the request size and return it's bp */
static void *find_fit(size_t asize)
{
    // /* Find free block that fits the requsted asize. (first fit)*/
    // for(char* bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    //     if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize) {
    //         return bp;
    //     }
    // }
    // return NULL;

    /* next fit */
    for (char* bp = prev_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
        {
            prev_listp = bp;
            return bp;
        }
    }

    for (char* bp = heap_listp; bp != prev_listp; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= asize)
        {
            prev_listp = bp;
            return bp;
        }
    }
    return NULL;
}

/* Place requested block in a free block, split if necessary */
static void place(void *bp, size_t asize)
{
    size_t blk_size = GET_SIZE(HDRP(bp));
    size_t remain_size = blk_size - asize;
    
    /* split if we have space space */
    if ( remain_size >= 16) {
        /* set up malloced block */
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        /* set up spilited block */
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
    }
    else {
        /* set up malloced block */
        PUT(HDRP(bp), PACK(blk_size, 1));
        PUT(FTRP(bp), PACK(blk_size, 1));
    }
    prev_listp = bp;
}