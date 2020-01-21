/*
    ECE 650 HW1 by Tongbo Liu
    Copyright (c) Tongbo Liu All rights reserved.
*/

#include <stdio.h>
#include <unistd.h>

// set a threshold which define if the block should be splitted
// the threshold is sizeof(meta_free) since if we split, the other block will 
// be big enough to hold more than a (meta + data)
#ifndef split_threshold
#define split_threshold sizeof(meta_free)
#endif
// create a meta information of a free block
typedef struct Meta_Free {
  size_t size;
  // for creating a list of blocks (random order)
  struct Meta_Free *next;
  struct Meta_Free *prev;
  // set the address of the neighbor free block (physically consecutive)
  struct Meta_Free *next_add;
  struct Meta_Free *prev_add;
  // set if it's free
  int is_free;
} meta_free;

// global arguments
unsigned long data_segment_size = 0;
long long data_segment_free_space_size = 0;
meta_free *head = NULL;
meta_free *tail = NULL; // point to the last blk, including allocated

//functions
//First Fit malloc/free
void *ff_malloc(size_t size); 
void ff_free(void *ptr);
//Best Fit malloc/free
void *bf_malloc(size_t size); 
void bf_free(void *ptr);
//performance
unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes
//functions for abstract
meta_free *append_block (size_t size);
void merge_free(meta_free *free_blk);
void merge_help (meta_free *first, meta_free *second);
//check
void check(meta_free *check_blk);