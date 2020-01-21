#include <string.h>
#include <assert.h>
#include "my_malloc.h"

//check
void check(meta_free *alloc){
    assert((alloc->next_add == NULL && alloc == tail) ||alloc->next_add ==(void *)alloc + split_threshold + alloc->size);
}

//malloc:
//ff_malloc
void *ff_malloc(size_t size) {
    if (!head){
        //no free block, need append
        meta_free *new_meta = append_block(size);
        if (new_meta == NULL) return NULL;
        else {
            return new_meta + 1;
        }
    }
    // find first match free block
    meta_free *find_res = head;
    while (find_res != NULL){
        if (find_res->size >= size){
            break;
        }
        find_res = find_res->next;
    }
    // check the found block
    if (find_res ==  NULL){
        // found no free block is big enough
        find_res = append_block(size);
        if (find_res == NULL) return NULL;
        else
            return find_res + 1;
    }
    else {
        // found the block, but the block maybe big enough to split
        if (find_res->size <= size + split_threshold) {
            // no need to split
            // remove from the free list           
            find_res->is_free = 0;
            if (find_res == head){
                //remove the first block
                head = find_res->next;
                find_res->next = NULL;
                find_res->prev = NULL;
            }
            else if (find_res->next != NULL){
                //rm the other block
                find_res->prev->next = find_res->next;
                find_res->next->prev = find_res->prev;
                find_res->next = NULL;
                find_res->prev = NULL;
            }
            else {
                //rm last block
                find_res->prev->next = NULL;
                find_res->next = NULL;
                find_res->prev = NULL;
            }
            //size_t block_size = find_res->size;
            data_segment_free_space_size -= find_res->size + split_threshold;
            return find_res + 1;
        }
        else {
            //big enough to split.
            //1st part: free block; 2nd part: allocated block
            //no need to change the list
            size_t offset = find_res->size - size;
            meta_free *alloc = (void *)find_res + offset;
            //alloc as the allocated part of big block
            alloc->next = NULL;
            alloc->prev = NULL;
            alloc->size = size;
            find_res->size -= size + split_threshold;
            alloc->is_free = 0;
            alloc->prev_add = find_res;
            if (find_res == tail){
                // split the last block
                alloc->next_add = NULL;
                find_res->next_add = alloc;
                tail = alloc;
            }
            else {
                alloc->next_add = find_res->next_add;
                find_res->next_add->prev_add = alloc;
                find_res->next_add = alloc;
            }
            
            data_segment_free_space_size -= (size + split_threshold);
            return alloc + 1;  //return allocated part
        }
    }
}

//bf_malloc
void *bf_malloc(size_t size) {
    if (!head){
        //no free block, need append
        meta_free *new_meta = append_block(size);
        if (new_meta == NULL) return NULL;
        else {
            return new_meta + 1; //return the address of the start of data, skip the meta
        }
    }
    // find best match free block
    meta_free *find = head;
    meta_free *find_res = NULL; //the best match
    // find the best match, find_res
    while (find != NULL){
        // speed up the match
        // Since sizeof(meta)=48
        // make a threshold = split_threshold//3
        if (find->size >= size && find->size <= size + 16) {
            find_res = find;
            // find a better one, sacrifice the optimal for the speed.
            break;
        }       
        // 
        if(find->size > size + split_threshold){
            if (find_res == NULL || find->size < find_res->size){
                find_res = find;
            }
        }
        //
        find = find->next;
    }
    // check the found block
    if (find_res ==  NULL){
        // found no free block is big enough
        find_res = append_block(size);
        if (find_res == NULL) return NULL;
        else
            return find_res + 1;
    }
    else {
        // found the block, but the block maybe big enough to split
        if (find_res->size <= size + split_threshold) {
            // no need to split
            // remove from the free list           
            find_res->is_free = 0;
            if (find_res == head){
                //remove the first block
                head = find_res->next;
                find_res->next = NULL;
                find_res->prev = NULL;
            }
            else if (find_res->next != NULL){
                //rm the other block
                find_res->prev->next = find_res->next;
                find_res->next->prev = find_res->prev;
                find_res->next = NULL;
                find_res->prev = NULL;
            }
            else {
                //rm last block
                find_res->prev->next = NULL;
                find_res->next = NULL;
                find_res->prev = NULL;
            }
            //size_t block_size = find_res->size;
            data_segment_free_space_size -= find_res->size + split_threshold;
            return find_res + 1;
        }
        else {
            //big enough to split
            //1st part: free block; 2nd part: allocated block
            //no need to change the list
            size_t offset = find_res->size - size;
            meta_free *alloc = (void *)find_res + offset;
            //alloc as the allocated part of big block
            alloc->next = NULL;
            alloc->prev = NULL;
            alloc->size = size;
            find_res->size -= size + split_threshold;
            alloc->is_free = 0;
            alloc->prev_add = find_res;
            if (find_res == tail){
                // split the last block
                alloc->next_add = NULL;
                find_res->next_add = alloc;
                tail = alloc;
            }
            else {
                alloc->next_add = find_res->next_add;
                find_res->next_add->prev_add = alloc;
                find_res->next_add = alloc;
            }            
            data_segment_free_space_size -= (size + split_threshold);
            return alloc + 1;  //return allocated part
        }
    }
}


//free:
// ff_free
void ff_free(void *ptr) {
    if (ptr == NULL) {
        printf("No block to be freed.\n");
        return;
    }
    // add the freed block to the head of the list
    // back to the start of meta
    meta_free * free_blk = ptr - split_threshold;
    // memset(ptr,0,free_blk.size + split_threshold);
    free_blk->is_free = 1;
    if (head == NULL) {
        // no allocated after free
        head = free_blk;  
        free_blk->next = NULL;
        free_blk->prev = NULL;
    }
    else {
        free_blk->next = head;
        head->prev = free_blk;
        head = free_blk; 
    }
    data_segment_free_space_size += free_blk->size + split_threshold;
    // merge free block
    merge_free(free_blk);
    //data_segment_free_space_size += free_blk->size + split_threshold;
}

// bf_free
void bf_free(void *ptr){
    // the same as bf_free
    ff_free(ptr);
}

// merge block functions
void merge_free(meta_free *free_blk) {
    // check the next block can be merged
    // 1. exist 2. is free
    if (free_blk->next_add && free_blk->next_add->is_free == 1){
        //data_segment_free_space_size += split_threshold;
        meta_free *second = free_blk->next_add;
        merge_help(free_blk,second);
    }
    // check the previous block can be merged
    if (free_blk->prev_add && free_blk->prev_add->is_free == 1){
        //data_segment_free_space_size += split_threshold;
        meta_free *first = free_blk->prev_add;
        merge_help(first,free_blk);
    }  
}

void merge_help (meta_free *first, meta_free *second){
    first->size += second->size + split_threshold;
    //data_segment_free_space_size += first->size; 
    //first means the used block, second is the block to be merged. 
    if (first->next_add != tail){
        second->next_add->prev_add = first;
        first->next_add = second->next_add;
    }
    else {
        tail = first;
        first->next_add = NULL;
    }
    // rm the second blk from fee list
    second->is_free = 0;
    if (second == head){
        //remove the first block
        head = second->next;
        second->next = NULL;
        second->prev = NULL;
    }
    else if (second->next != NULL){
        //rm the internal block
        second->prev->next = second->next;
        second->next->prev = second->prev;
        second->next = NULL;
        second->prev = NULL;
    }
    else {
        //rm last block
        second->prev->next = NULL;
        second->next = NULL;
        second->prev = NULL;            
    }
}
// sbrk 
meta_free *append_block (size_t size) {
    void *last_add = sbrk(size + sizeof(meta_free));
    // check sbrk
    if (last_add == (void *)-1) {
        perror("sbrk");
        return NULL;
    }
    //a new meta
    meta_free *new_meta = last_add;
    new_meta->is_free = 0;  //allocated
    new_meta->size = size;
    // allocated, not add to list
    new_meta->next = NULL;
    new_meta->prev = NULL;   
    // set previous block address
    if (tail != NULL) {
        new_meta->prev_add = tail;
        new_meta->next_add = NULL;
        // update previous tail
        tail->next_add = new_meta;
    } 
    else {
        // 1st allocated block
        new_meta->prev_add = NULL;
        new_meta->next_add = NULL;
    }
    tail = new_meta;   
    //data_segment_size += size + sizeof(meta_free);
    data_segment_size += size + split_threshold;
    return new_meta;
}

//performance
unsigned long get_data_segment_size() { 
    return data_segment_size; 
}
unsigned long get_data_segment_free_space_size() {
  return data_segment_free_space_size;
}