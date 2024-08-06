#include "mymem.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList {
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  int size;    // How many bytes in this block?
  char alloc;  // 1 if this block is allocated,
               // 0 if this block is free.
  void *ptr;   // location of block in memory pool.
};

strategies myStrategy = NotSet;  // Current strategy

size_t mySize;
void *myMemory = NULL;
int memoryAllocated;
int holes;

static struct memoryList *head;
static struct memoryList *next;
static struct memoryList *lastAllocatedBlock;

// Prototypes for helper functions
void *allocateFirstFit(size_t requested);
void *allocateBestFit(size_t requested);
void *allocateWorstFit(size_t requested);
void *allocateNextFit(size_t requested);
void *allocateBlock(struct memoryList *node, int requested);
int findBestFitSize(size_t requested);
int findLargestFreeSize();

/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
        - "best" (best-fit)
        - "worst" (worst-fit)
        - "first" (first-fit)
        - "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all
   mymalloc requests.
*/

void initmem(strategies strategy, size_t sz) {
  myStrategy = strategy;
  mySize = sz;

  // Free the memory if it has already been allocated
  if (myMemory != NULL) {
    free(myMemory);
    myMemory = NULL;
  }

  // Free the linked list if it has already been allocated
  if (head != NULL) {
    head->last->next = NULL;  // Break the circularity
    struct memoryList *nextNode;
    while (head != NULL) {
      nextNode = head;
      head = head->next;
      free(nextNode);
    }
  }

  // Allocate the memory and initialise the linked list
  myMemory = malloc(sz);

  memoryAllocated = 0;
  holes = 1;

  // Initialise the linked list
  head = malloc(sizeof(struct memoryList));
  head->next = head->last = head;
  head->size = mySize;
  head->alloc = 0;
  head->ptr = myMemory;

  // Initialise the next pointer
  lastAllocatedBlock = head;
}

// This function finds the correct size of the best fitting free block
int mem_best_fit(int size) {
  if (!mem_free()) return 0;

  int bestSize = -1;
  struct memoryList *current = head;

  do {
    if (!current->alloc && current->size >= size &&
        (current->size < bestSize || bestSize == -1))
      bestSize = current->size;

    current = current->next;
  } while (current != head);

  return bestSize;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1
 */

void *assignBlock(struct memoryList *node, int requested) {
  struct memoryList *temp = malloc(sizeof(struct memoryList));
  *temp = (struct memoryList){.next = node,
                              .last = node->last,
                              .size = requested,
                              .alloc = 1,
                              .ptr = node->ptr};

  node->last->next = temp;
  node->last = temp;
  node->ptr += requested;
  node->size -= requested;

  if (node == head) {
    head = temp;
  }

  memoryAllocated += requested;
  lastAllocatedBlock = temp;

  return temp->ptr;
}

void *mymalloc(size_t requested) {
  assert((int)myStrategy > 0);

  switch (myStrategy) {
    case NotSet:
      return NULL;
    case First:
      return allocateFirstFit(requested);
    case Best:
      return allocateBestFit(requested);
    case Worst:
      return allocateWorstFit(requested);
    case Next:
      return allocateNextFit(requested);
    default:
      return NULL;
  }
}

void *allocateFirstFit(size_t requested) {
  // Find the first free block that is large enough
  struct memoryList *current = head;

  // Loop through the list until we find a block that is large enough
  do {
    // If the block is not allocated and is large enough
    if (!current->alloc && current->size >= requested) {
      return allocateBlock(current, requested);
    }
    current = current->next;
  } while (current != head);

  return NULL;
}

void *allocateBestFit(size_t requested) {
  // Find the best fitting free block
  int bestSize = findBestFitSize(requested);

  // If there is no block that is large enough
  if (bestSize == -1) {
    return NULL;
  }

  struct memoryList *current = head;

  // Loop through the list until we find a block that is large enough
  while (current->size != bestSize || current->alloc) {
    current = current->next;
  }

  // If the block is the correct size, allocate it
  return (requested == bestSize) ? allocateBlock(current, requested)
                                 : assignBlock(current, requested);
}

void *allocateWorstFit(size_t requested) {
  // Find the worst fitting free block
  int worstSize = findLargestFreeSize();

  // If there is no block that is large enough
  if (worstSize == 0) {
    return NULL;
  }

  struct memoryList *current = head;

  // Loop through the list until we find a block that is large enough
  while (current->size != worstSize || current->alloc) {
    current = current->next;
  }

  // If the block is the correct size, allocate it
  return (requested == worstSize) ? allocateBlock(current, requested)
                                  : assignBlock(current, requested);
}

void *allocateNextFit(size_t requested) {
  // Find the first free block that is large enough
  struct memoryList *current = lastAllocatedBlock->next;

  // Loop through the list until we find a block that is large enough
  do {
    if (!current->alloc && current->size >= requested) {
      return (requested == current->size) ? allocateBlock(current, requested)
                                          : assignBlock(current, requested);
    }
    current = current->next;
  } while (current != lastAllocatedBlock->next);

  return NULL;
}

void *allocateBlock(struct memoryList *node, int requested) {
  // If the block is the correct size, allocate it
  if (node->size > requested) {
    return assignBlock(node, requested);
  } else {
    node->alloc = 1;
    holes--;
    memoryAllocated += requested;
    lastAllocatedBlock = node;

    return node->ptr;
  }
}

int findLargestFreeSize() {
  // Find the largest free block
  int largestSize = 0;
  struct memoryList *current = head;

  // Loop through the list until we find a block that is large enough
  do {
    if (current->size > largestSize && !current->alloc) {
      largestSize = current->size;
    }
    current = current->next;
  } while (current != head);

  return largestSize;
}

int findBestFitSize(size_t requested) {
  // Find the best fitting free block
  int bestSize = -1;
  struct memoryList *current = head;

  // Loop through the list until we find a block that is large enough
  do {
    if (!current->alloc && current->size >= requested &&
        (current->size < bestSize || bestSize == -1)) {
      bestSize = current->size;
    }

    current = current->next;
  } while (current != head);

  return bestSize;
}

// Joins the node with the previous node
void join(struct memoryList *node) {
  // If the previous node is allocated, we cannot join
  if (node->last->alloc == 1) {
    return;
  }

  // Update the nodes to join them
  struct memoryList *prevNode = node->last;
  struct memoryList *nextNode = node->next;

  prevNode->size += node->size;
  prevNode->next = nextNode;
  nextNode->last = prevNode;

  // Update the head and lastAllocatedBlock pointers if necessary
  if (node == head) {
    head = prevNode;
  }

  if (node == lastAllocatedBlock) {
    lastAllocatedBlock = prevNode;
  }

  // Free the node
  free(node);
}

/* Finds the block in the linked list and marks it as free */
struct memoryList *findBlock(void *block) {
  struct memoryList *current = head;

  // Loop through the list until we find the block
  while (current->ptr != block) current = current->next;

  // Update the head and lastAllocatedBlock pointers if necessary
  if (current == lastAllocatedBlock) lastAllocatedBlock = current->next;

  // Mark the block as free
  current->alloc = 0;
  memoryAllocated -= current->size;

  return current;
}

/* Joins the block with the previous and next blocks if they are free */
int joinAdjacentBlocks(struct memoryList *current) {
  int numberOfAdjacentFreeBlocks = 0;

  if ((current != head) && (!current->last->alloc)) {
    numberOfAdjacentFreeBlocks++;
    current = current->last;
    join(current->next);
  }

  if ((current->next != head) && (!current->next->alloc)) {
    numberOfAdjacentFreeBlocks++;
    join(current->next);
  }

  return numberOfAdjacentFreeBlocks;
}

/* Updates the holes count based on the number of adjacent free blocks */
void updateHolesCount(int numberOfAdjacentFreeBlocks) {
  if (!numberOfAdjacentFreeBlocks) {
    holes++;
  }
  if (numberOfAdjacentFreeBlocks == 2) {
    holes--;
  }
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void *block) {
  struct memoryList *current = findBlock(block);
  int numberOfAdjacentFreeBlocks = joinAdjacentBlocks(current);
  updateHolesCount(numberOfAdjacentFreeBlocks);
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when we refer to "memory" here, we mean the
 * memory pool this module manages via initmem/mymalloc/myfree.
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes() { return holes; }

/* Get the number of bytes allocated */
int mem_allocated() { return memoryAllocated; }

/* Number of non-allocated bytes */
int mem_free() {
  int totalFree = 0;
  struct memoryList *current = head;

  do {
    if (!current->alloc) totalFree += current->size;

    current = current->next;
  } while (current != head);

  return totalFree;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free() {
  if (!mem_free()) return 0;
  int c = 0;
  next = head;
  do {
    if (next->size > c && !next->alloc) c = next->size;
    next = next->next;
  } while (next != head);
  return c;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size) {
  int c = 0;
  next = head;
  do {
    if ((next->size <= size) && (!next->alloc)) c += 1;
    next = next->next;
  } while (next != head);
  return c;
}

char mem_is_alloc(void *ptr) {
  while (next->ptr != ptr) next = next->next;
  return next->alloc;
}

// Returns a pointer to the memory pool.
void *mem_pool() { return myMemory; }

// Returns the total number of bytes in the memory pool. */
int mem_total() { return mySize; }

// Get string name for a strategy.
char *strategy_name(strategies strategy) {
  switch (strategy) {
    case Best:
      return "best";
    case Worst:
      return "worst";
    case First:
      return "first";
    case Next:
      return "next";
    default:
      return "unknown";
  }
}

// Get strategy from name.
strategies strategyFromString(char *strategy) {
  if (!strcmp(strategy, "best")) {
    return Best;
  } else if (!strcmp(strategy, "worst")) {
    return Worst;
  } else if (!strcmp(strategy, "first")) {
    return First;
  } else if (!strcmp(strategy, "next")) {
    return Next;
  } else {
    return 0;
  }
}

/*
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory() {
  // Complete this function
  struct memoryList *current = head;
  do {
    printf("Size: %d, Alloc: %d, Ptr: %p\n", current->size, current->alloc,
           current->ptr);
    current = current->next;
  } while (current != head);
}

/* Use this function to track memory allocation performance.
 * This function does not depend on your implementation,
 * but on the functions you wrote above.
 */
void print_memory_status() {
  printf("%d out of %d bytes allocated.\n", mem_allocated(), mem_total());
  printf(
      "%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",
      mem_free(), mem_holes(), mem_largest_free());
  printf("Average hole size is %f.\n\n", ((float)mem_free()) / mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
  strategies strat;
  void *a, *b, *c, *d, *e;
  if (argc > 1)
    strat = strategyFromString(argv[1]);
  else
    strat = First;

  /* A simple example.
     Each algorithm should produce a different layout. */

  initmem(strat, 500);

  a = mymalloc(100);
  b = mymalloc(100);
  c = mymalloc(100);
  myfree(b);
  d = mymalloc(50);
  myfree(a);
  e = mymalloc(25);

  print_memory();
  print_memory_status();
}