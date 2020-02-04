#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size_t)(size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define SIZE_PTR(p)  ((size_t*)(((char*)(p)) - SIZE_T_SIZE))

#define WSIZE    8      
#define DSIZE    16     
#define CHUNKSIZE   (1<<12) 
#define OVERHEAD    16    

#define MAX(x, y) ((x) > (y)? (x) : (y))  

#define PACK(size, alloc)  ((size) | (alloc))

#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))  
#define GET_SIZE(p)  (GET(p) & ~(0xF))
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(p)       ((char *)(p) - WSIZE)  
#define FTRP(p)       ((char *)(p) + GET_SIZE(HDRP(p)) - DSIZE)

#define NEXTB(p)  ((char *)(p) + GET_SIZE(((char *)(p) - WSIZE)))
#define PREVB(p)  ((char *)(p) - GET_SIZE(((char *)(p) - DSIZE)))

#define NEXT(p)  (*(char**)(p + WSIZE))
#define PREV(p)  (*(char**)p)

static char *heap_listp = 0;  
static char *free_listp = 0;  

static void add_block(void *p);
static void remove_block(void *p);
static void *find_fit(size_t nsize);
static void *extend_heap(size_t words);
static void place(void *p, size_t nsize);
static void *coalesce(void *p);
//static void printblock(void *bp);
//static void checkblock(void *bp); 

/*
 * mm_init - Called when a new trace starts.
 */
int mm_init(void)
{
        heap_listp = mem_sbrk(4*DSIZE);
        if(!heap_listp)
                return -1;
        PUT(heap_listp, 0);
        PUT(heap_listp + 2*WSIZE, 0);
	PUT(heap_listp+WSIZE+DSIZE, PACK(0, 1));
        PUT(heap_listp+WSIZE, PACK(OVERHEAD, 1));
        PUT(heap_listp+DSIZE, PACK(OVERHEAD, 1));
        free_listp = heap_listp + DSIZE;
        if (!(extend_heap(CHUNKSIZE/WSIZE)))
                return -1;
        return 0;
}

void *mm_malloc(size_t size)
{
	size_t nsize; 
	if (!heap_listp)
		mm_init(); 
	if (size <= 0)
		return NULL; 
	else if (size <= DSIZE)
		nsize = DSIZE + OVERHEAD; 
	else
		nsize = ((DSIZE + OVERHEAD + size - 1)/DSIZE) * DSIZE; 

	char *p = find_fit(nsize); 
	if (p)
	{
		place(p, nsize); 
		return p; 
	}
	size_t ext = MAX(nsize, CHUNKSIZE); 
	p = extend_heap(ext/WSIZE); 
	if (!p)
		return NULL;
	else
	{
		place(p, nsize); 
		return p; 	
	}
}


//9
void mm_free(void *ptr)
{
	size_t s;
	if(!ptr)	
		return;
	s = GET_SIZE(HDRP(ptr)); 
	if (!heap_listp)
	    mm_init();
	PUT(FTRP(ptr), PACK(s, 0)); 
	PUT(HDRP(ptr), PACK(s,0)); 
	coalesce(ptr); 
}

static void *extend_heap(size_t words)
{
    void *ret_p;
	void *bp;
	size_t size;
	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
	if((long)(bp = mem_sbrk(size)) < 0)
		return NULL;
	PUT(HDRP(bp), PACK(size, 0));

	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXTB(bp)), PACK(0,1));
	ret_p = coalesce(bp);
	return ret_p;
}

static void *coalesce(void *p)
{
	size_t prev = GET_ALLOC(FTRP(PREVB(p))) || PREVB(p) == p;
	size_t next = GET_ALLOC(HDRP(NEXTB(p))); 
	size_t size = GET_SIZE(HDRP(p)); 

	if (!next && !prev)
	{
		size += GET_SIZE(HDRP(PREVB(p))) + GET_SIZE(HDRP(NEXTB(p))); 
		remove_block(PREVB(p)); 
		remove_block(NEXTB(p)); 	
		p = PREVB(p); 
		PUT(HDRP(p), PACK(size,0)); 
		PUT(FTRP(p), PACK(size,0)); 
		add_block(p); 
		return p; 
	}
	else if(prev && !next)
    {
        size += GET_SIZE(HDRP(NEXTB(p)));
        remove_block(NEXTB(p));
        PUT(HDRP(p), PACK(size,0));
        PUT(FTRP(p), PACK(size,0));
		add_block(p); 
		return p; 
	}
	else if (!prev && next)
	{
		size += GET_SIZE(HDRP(PREVB(p))); 
		p = PREVB(p); 
		remove_block(p); 
		PUT(HDRP(p), PACK(size, 0)); 
		PUT(FTRP(p), PACK(size, 0)); 
		add_block(p); 
		return p; 
	}
	else
	{
		add_block(p); 
		return p; 
	}
}


static void place(void *p, size_t nsize)
{
	size_t csize = GET_SIZE(HDRP(p));
	if((csize -nsize) >= (OVERHEAD + 2*DSIZE)) 
	{
		PUT(HDRP(p), PACK(nsize, 1));
		PUT(FTRP(p), PACK(nsize, 1));
		remove_block(p);
		p = NEXTB(p); 
		PUT(HDRP(p), PACK(csize-nsize, 0));
        	PUT(FTRP(p), PACK(csize-nsize, 0));
		add_block(p);
	} 	
	else
	{
		remove_block(p);
		PUT(HDRP(p), PACK(csize, 1)); 
		PUT(FTRP(p), PACK(csize, 1));
	}
}


static void *find_fit(size_t nsize)
{
	void* bp;
	for (bp = free_listp; GET_SIZE(HDRP(bp)) > 0 && GET_ALLOC(HDRP(bp)) == 0; bp = NEXT(bp))
	{
		if (nsize <= GET_SIZE(HDRP(bp)))
			return bp; 
	}
	return NULL; 	
}





void *mm_calloc (size_t nmemb, size_t size)
{
  void *newptr;
  newptr = mm_malloc(nmemb*size);
  bzero(newptr, nmemb*size);

  return newptr;
}

void *mm_realloc(void *oldptr, size_t size)
{
  size_t oldsize;
  void *newptr;

  if(size == 0) {
    mm_free(oldptr);
    return 0;
  }

  if(!oldptr)
    return mm_malloc(size);
  newptr = mm_malloc(size);

  if(newptr == NULL)
    return 0;

  oldsize = GET_SIZE(HDRP(oldptr));
  if(oldsize > size) 
	oldsize = size;
  memcpy(newptr, oldptr, oldsize);

  mm_free(oldptr);
  return newptr;
}


static void add_block(void *p)
{
	NEXT(p) = free_listp; 
	PREV(free_listp) = p; 
	PREV(p) = NULL; 
	free_listp = p; 
}

void remove_block(void *p)
{
	if (!PREV(p))
		free_listp = NEXT(p); 
	else
		NEXT(PREV(p)) = NEXT(p); 
	PREV(NEXT(p)) = PREV(p); 	
}
/*
static void printblock(void *bp)
{
  size_t hsize;// halloc, fsize, falloc;

  hsize = GET_SIZE(HDRP(bp));

  if (hsize == 0) {
    printf("%p: eol\n", bp);
    return;
  }

}

static void checkblock(void *bp)
{
  if ((size_t)bp % 8)
    printf("Error: %p isn't  doubleword aligned\n", bp);
  if (GET(HDRP(bp)) != GET(FTRP(bp)))
    printf("Error: header doesn't  match footer\n");
}


*/ 

void mm_checkheap(int verbose)
{
	
/*	char *bp = heap_listp;

  if (verbose)
    printf("Heap (%p):\n", heap_listp);

  if ((GET_SIZE(HDRP(heap_listp)) != 24) || !GET_ALLOC(HDRP(heap_listp)))
      checkblock(heap_listp);

  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
      if (verbose)
      {
	    printblock(bp);
      checkblock(bp);
    }
    } 

  if (verbose)
    printblock(bp);
  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
      printf("Bad epilogue header\n");
    
*/
}
