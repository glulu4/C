// This adds coalescing of free blocks.
// Improves performance to 54/100 ... takes less time.

/*-------------------------------------------------------------------
 *  Malloc Lab Starter code:
 *        single doubly-linked free block list with LIFO policy
 *        with support for coalescing adjacent free blocks
 *make
 * Terminology:
 * o We will implement an explicit free list allocator.
 * o We use "next" and "previous" to refer to blocks as ordered in
 *   the free list.
 * o We use "following" and "preceding" to refer to adjacent blocks
 *   in memory.
 *-------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/* Macros for unscaled pointer arithmetic to keep other code cleaner.
   Casting to a char* has the effect that pointer arithmetic happens at
   the byte granularity (i.e. POINTER_ADD(0x1, 1) would be 0x2).  (By
   default, incrementing a pointer in C has the effect of incrementing
   it by the size of the type to which it points (e.g. Block).)
   We cast the result to void* to force you to cast back to the
   appropriate type and ensure you don't accidentally use the resulting
   pointer as a char* implicitly.
*/
#define UNSCALED_POINTER_ADD(p, x) ((void*)((char*)(p) + (x)))
#define UNSCALED_POINTER_SUB(p, x) ((void*)((char*)(p) - (x)))


/******** FREE LIST IMPLEMENTATION ***********************************/


/* An BlockInfo contains information about a block, including the size
   as well as pointers to the next and previous blocks in the free list.
   This is similar to the "explicit free list" structure illustrated in
   the lecture slides.

   Note that the next pointer are only needed when the block is free. To
   achieve better utilization, mm_malloc should use the space for next as
   part of the space it returns.

   +--------------+
   |     size     |  <-  Block pointers in free list point here
   |              |
   |   (header)   |
   |              |
   |     prev     |
   +--------------+
   |   nextFree   |  <-  Pointers returned by mm_malloc point here
   |   prevFree   |
   +--------------+      (allocated blocks do not have a 'nextFree' field)
   |  space and   |      (this is a space optimization...)
   |   padding    |
   |     ...      |      Free blocks write their nextFree/prevFree pointers in
   |     ...      |      this space.
   +--------------+

*/
typedef struct _BlockInfo {
  // Size of the block and whether or not the block is in use or free.
  // When the size is negative, the block is currently free.
  long int size;
  // Pointer to the previous block in the list.
  struct _Block* prev;
} BlockInfo;

/* A FreeBlockInfo structure contains metadata just for free blocks.
 * When you are ready, you can improve your naive implementation by
 * using these to maintain a separate list of free blocks.
 *
 * These are "kept" in the region of memory that is normally used by
 * the program when the block is allocated. That is, since that space
 * is free anyway, we can make good use of it to improve our malloc.
 */
typedef struct _FreeBlockInfo {
  // Pointer to the next free block in the list.
  struct _Block* nextFree;
  // Pointer to the previous free block in the list.
  struct _Block* prevFree;
} FreeBlockInfo;

/* This is a structure that can serve as all kinds of nodes.
 */
typedef struct _Block {
  BlockInfo info;
  FreeBlockInfo freeNode;
} Block;

/* Pointer to the first FreeBlockInfo in the free list, the list's head. */
static Block* free_list_head = NULL;
static Block* malloc_list_tail = NULL;

static size_t heap_size = 0;

/* Size of a word on this architecture. */
#define WORD_SIZE sizeof(void*)

/* Alignment of blocks returned by mm_malloc.
 * (We need each allocation to at least be big enough for the free space
 * metadata... so let's just align by that.)  */
#define ALIGNMENT (sizeof(FreeBlockInfo))

/* This function will have the OS allocate more space for our heap.
 *
 * It returns a pointer to that new space. That pointer will always be
 * larger than the last request and be continuous in memory.
 */
void* requestMoreSpace(size_t reqSize);

/* This function will get the first block or returns NULL if there is not
 * one.
 *
 * You can use this to start your through search for a block.
 */
Block* first_block();

/* This function will get the adjacent block or returns NULL if there is not
 * one.
 *
 * You can use this to move along your malloc list one block at a time.
 */
Block* next_block(Block* block);

/* Use this function to print a thorough listing of your heap data structures.
 */
void examine_heap();

/* Checks the heap for any issues and prints out errors as it finds them.
 *
 * Use this when you are debugging to check for consistency issues. */
int check_heap();
void addFreeBlock( Block * ptrFreeBlock );
void removeFreeBlock( Block * ptrFreeBlock );

Block* searchList(size_t reqSize) {
  Block* ptrFreeBlock = first_block();

  long int checkSize = -reqSize;


  Block* curr_block = ptrFreeBlock;
  // while the next block is not null, ie at the end of the list
  while ( curr_block  != NULL ){
    long int curr_block_size = curr_block->info.size; // gets size of current block, neg if its free
    if ( curr_block_size > 0 ) { // if the size is pos, ie the curr block is not free
      curr_block = next_block( curr_block );
      continue; // if teh current block is not free, we go to next iteration, we look at next block
    }

    // if curr_block_size is less than checkSize... because negative we're checking if it's number is bigger
    if ( curr_block_size <= checkSize ){
      return curr_block; // this block can fit the requested size, we return it
    }
    curr_block = next_block( curr_block );
  }
  return NULL;
}

/* Find a free block of at least the requested size in the free list.  Returns
   NULL if no free block is large enough. */
Block* searchFreeList(size_t reqSize) {
  Block* ptrFreeBlock = free_list_head;
  long int checkSize = -reqSize;

  Block* curr_block = ptrFreeBlock;
  //searching free list, while the block is not at the end of the free list
  while ( curr_block != NULL  ){
    long int curr_block_size = curr_block->info.size; // will be negative since we searching da free list

    // if the current blocks size is exactly what we need
    if ( curr_block_size == checkSize ){
      return curr_block;
    }
    // ok not exact fit, but is the current blocks size bigger than, ie less than, the requested size
    if ( curr_block_size <= checkSize ){
      return curr_block; // this block can fit the requested size, we return it
    }

    // incrementing block by searching for the next block
    curr_block = curr_block->freeNode.nextFree;
  }
  // no free blocks son
  return NULL;
}

// TOP-LEVEL ALLOCATOR INTERFACE ------------------------------------

/* Allocate a block of size size and return a pointer to it. If size is zero,
 * returns null.
 */
void* mm_malloc(size_t size) {
  // examine_heap();
  Block* ptrFreeBlock = NULL;
  long int reqSize;

  // Zero-size requests get NULL.
  if (size == 0) {
    return NULL;
  }



  // Determine the amount of memory we want to allocate
  reqSize = size;
  // Round up for correct alignment
  reqSize = ALIGNMENT * ((reqSize + ALIGNMENT - 1) / ALIGNMENT);

  ptrFreeBlock = searchFreeList(reqSize);

  // no space in the heap for requested size
  if ( ptrFreeBlock == NULL ){
    // pointing free block to newly allocated space
    ptrFreeBlock = requestMoreSpace(reqSize+sizeof(BlockInfo));// if there is no space, allocate more with teh exact size yuou need!

    ptrFreeBlock->info.prev = malloc_list_tail; // linking new node's prev pointer to last tail 
    malloc_list_tail = ptrFreeBlock; // updating tail
    ptrFreeBlock->info.size = reqSize;
    return (void*)UNSCALED_POINTER_ADD(ptrFreeBlock, sizeof(BlockInfo));
  }
    
  
  
  // can we split?
  // if we can at least a block of size zero we can split 
  // after we split, we put the data in the first one ( on the left )
  if ( -(ptrFreeBlock->info.size) - reqSize >= sizeof(Block)){
    // we cast to a char so we move exactly that many bytes
    Block * node_post_split = (Block*)( ((char*)ptrFreeBlock) + (reqSize) + sizeof(BlockInfo)); // making two identical blocks WTF!
    node_post_split->info.size = ptrFreeBlock->info.size + reqSize + sizeof(BlockInfo);
    // we do this after because we need the whole size above to set size of the next block post split 
    ptrFreeBlock->info.size = reqSize; // making size of free block allocated
    node_post_split->info.prev = ptrFreeBlock;


    // updating tail
    Block * next_node = next_block(node_post_split);
    // if the next node is null, ie node_post_split is at the end, we make it the tail... 
    if ( next_node == NULL ){
      malloc_list_tail = node_post_split;
    }
    else{
      // connecting the next node back to the node post split
      next_node->info.prev = node_post_split;
    }


    // ok did we allocate the first block? if so we need to move the free list head and remove that node 
    // they have the same address
    removeFreeBlock(ptrFreeBlock);
    addFreeBlock(node_post_split);

    // ptrFreeBlock->info.size = (ptrFreeBlock->info.size);
    return (void*)UNSCALED_POINTER_ADD(ptrFreeBlock, sizeof(BlockInfo));
  }

  ptrFreeBlock->info.size = -(ptrFreeBlock->info.size);
  removeFreeBlock(ptrFreeBlock);
  
    
  return (void*)UNSCALED_POINTER_ADD(ptrFreeBlock, sizeof(BlockInfo));
 } // end of mm_malloc

void removeFreeBlock( Block * ptrFreeBlock ){

  // if we are removing the head
  if ( ptrFreeBlock == free_list_head){
    if (free_list_head->freeNode.nextFree == NULL ){
      free_list_head = NULL;
    }
    else{
      Block * new_head = free_list_head->freeNode.nextFree;
      free_list_head->freeNode.nextFree = NULL;
      free_list_head = new_head;
      new_head->freeNode.prevFree = NULL;
    }
  }
  else{
    Block * next = ptrFreeBlock->freeNode.nextFree;
    Block * prev = ptrFreeBlock->freeNode.prevFree;

    prev->freeNode.nextFree = next;
    if ( next ){
      next->freeNode.prevFree = prev;
    }

    ptrFreeBlock->freeNode.nextFree = NULL;
    ptrFreeBlock->freeNode.prevFree = NULL;
  }
}


void coalesce(Block* blockInfo) {
  
  // blockInfo is made negative before it enters the function
  //examine_heap();
  Block* nextBlock = next_block(blockInfo);
  Block* previousBlock = blockInfo->info.prev;

  // if prev block is within the heap and is free
  // current into the prev
  if ( previousBlock && previousBlock->info.size < 0 ){
    previousBlock->info.size += blockInfo->info.size - sizeof(BlockInfo);
    if ( nextBlock )
      nextBlock->info.prev = previousBlock;
    else
      malloc_list_tail = previousBlock;
    removeFreeBlock(blockInfo);
    blockInfo = previousBlock;
  }
  // if next block is not null and is free
  
  if ( nextBlock != NULL && nextBlock->info.size < 0 ){

    blockInfo->info.size += nextBlock->info.size - sizeof(BlockInfo);
    Block * nextNext = next_block(nextBlock);
    if ( nextNext )
      nextNext->info.prev = blockInfo;
    else
      malloc_list_tail = blockInfo;
  
    removeFreeBlock(nextBlock);
  }
}

/* Free the block referenced by ptr. */
void mm_free(void* ptr) {

  Block* blockInfo = (Block*)UNSCALED_POINTER_SUB(ptr, sizeof(BlockInfo));
  blockInfo->info.size = -(blockInfo->info.size);
  addFreeBlock(blockInfo);
  coalesce(blockInfo);
  
}

void addFreeBlock( Block * ptrFreeBlock ){

  // list is currently empty
  if ( free_list_head == NULL ){
    free_list_head = ptrFreeBlock;
    free_list_head->freeNode.prevFree = NULL;
    free_list_head->freeNode.nextFree = NULL;
  }

  else{
    Block * prev_head = free_list_head;
    free_list_head = ptrFreeBlock;
    free_list_head->freeNode.prevFree = NULL;
    free_list_head->freeNode.nextFree = prev_head;

    prev_head->freeNode.prevFree = free_list_head;

  }
}

// PROVIDED FUNCTIONS -----------------------------------------------
//
// You do not need to modify these, but they might be helpful to read
// over.

/* Get more heap space of exact size reqSize. */
void* requestMoreSpace(size_t reqSize) {
  void* ret = UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
  heap_size += reqSize;

  void* mem_sbrk_result = mem_sbrk(reqSize);
  if ((size_t)mem_sbrk_result == -1) {
    printf("ERROR: mem_sbrk failed in requestMoreSpace\n");
    exit(0);
  }

  return ret;
}

/* Initialize the allocator. */
int mm_init() {
  free_list_head = NULL;
  malloc_list_tail = NULL;
  heap_size = 0;

  return 0;
}

/* Gets the first block in the heap or returns NULL if there is not one. */
Block* first_block() {
  Block* first = (Block*)mem_heap_lo();
  if (heap_size == 0) {
    return NULL;
  }

  return first;
}

/* Gets the adjacent block or returns NULL if there is not one. */
Block* next_block(Block* block) {
  size_t distance = (block->info.size > 0) ? block->info.size : -block->info.size;

  Block* end = (Block*)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
  Block* next = (Block*)UNSCALED_POINTER_ADD(block, sizeof(BlockInfo) + distance);
  if (next >= end) {
    return NULL;
  }

  return next;
}

/* Print the heap by iterating through it as an implicit free list. */
void examine_heap() {
  /* print to stderr so output isn't buffered and not output if we crash */
  Block* curr = (Block*)mem_heap_lo();
  Block* end = (Block*)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
  fprintf(stderr, "heap size:\t0x%lx\n", heap_size);
  fprintf(stderr, "heap start:\t%p\n", curr);
  fprintf(stderr, "heap end:\t%p\n", end);

  fprintf(stderr, "free_list_head: %p\n", (void*)free_list_head);

  fprintf(stderr, "malloc_list_tail: %p\n", (void*)malloc_list_tail);

  while(curr && curr < end) {
    /* print out common block attributes */
    fprintf(stderr, "%p: %ld\t", (void*)curr, curr->info.size);

    /* and allocated/free specific data */
    if (curr->info.size > 0) {
      fprintf(stderr, "ALLOCATED\tprev: %p\n", (void*)curr->info.prev);
    } else {
      fprintf(stderr, "FREE\tnextFree: %p, prevFree: %p, prev: %p\n", (void*)curr->freeNode.nextFree, (void*)curr->freeNode.prevFree, (void*)curr->info.prev);
    }

    curr = next_block(curr);
  }
  fprintf(stderr, "END OF HEAP\n\n");

  curr = free_list_head;
  fprintf(stderr, "Head ");
  while(curr) {
    fprintf(stderr, "-> %p ", curr);
    curr = curr->freeNode.nextFree;
  }
  fprintf(stderr, "\n");
}

/* Checks the heap data structure for consistency. */
int check_heap() {
  Block* curr = (Block*)mem_heap_lo();
  Block* end = (Block*)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
  Block* last = NULL;
  long int free_count = 0;

  while(curr && curr < end) {
    if (curr->info.prev != last) {
      fprintf(stderr, "check_heap: Error: previous link not correct.\n");
      examine_heap();
    }

    if (curr->info.size <= 0) {
      // Free
      free_count++;
    }

    last = curr;
    curr = next_block(curr);
  }

  curr = free_list_head;
  last = NULL;
  while(curr) {
    if (curr == last) {
      fprintf(stderr, "check_heap: Error: free list is circular.\n");
      examine_heap();
    }
    last = curr;
    curr = curr->freeNode.nextFree;
    if (free_count == 0) {
      fprintf(stderr, "check_heap: Error: free list has more items than expected.\n");
      examine_heap();
    }
    free_count--;
  }

  return 0;
}
