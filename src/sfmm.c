/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sfmm.h"

/**
 * You should store the head of your free list in this variable.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
sf_free_header* freelist_head = NULL;
void* bottomOfHeap = NULL;

#define PAGE_SIZE 4096

/* TODO Make sure to properly update the freelist when stuff is added. 
		Right now I am not properly using next and prev. */
void* sf_malloc(size_t size) {
	if(size == 0){
		return NULL;
	} else {
		if(freelist_head == NULL){
			/* Get the current pointer to the heap and assign it to freelist_head*/
			bottomOfHeap = sf_sbrk(0);
			void* pointer = sf_sbrk(PAGE_SIZE);
			pointer += -8;
			freelist_head = (sf_free_header*) pointer;
			pointer += 8;
			(*freelist_head).header.alloc = 0;
			(*freelist_head).header.block_size = PAGE_SIZE;
			(*freelist_head).header.block_size = (*freelist_head).header.block_size >> 4;
			int payloadSize = (*freelist_head).header.block_size << 4;
			void* footerPointer = pointer;
			footerPointer += payloadSize -16;
			struct sf_footer * footer = (sf_footer*) footerPointer;
			(*footer).alloc = 0;
			(*footer).block_size = (*freelist_head).header.block_size;

			//printf("%p\n", freelist_head);

			/* Freelist now instantiated */
		}

		/*
		if((*freelist_head).prev != NULL){
			freelist_head = (*freelist_head).prev;
			if((unsigned long) freelist_head == (unsigned long) (*freelist_head).prev){
				(*freelist_head).prev = NULL;
			}
		}
		*/

		void* pointer = (void*) freelist_head;
		pointer += 8;
		void* footerPointer = pointer;
		footerPointer += ((*freelist_head).header.block_size << 4) - 16;
		pointer += -8;

		//printf("%p\n", pointer);
		
		/* Get the correct payload size. It must be divisible by 16 
		to align correctly */
		int payloadSize = size;
		if(size < 16){
			payloadSize = 16;
		} else {
			int i = 0;
			for(; i < 16; i++){
				if(payloadSize % 16 == 0){
					break;
				} else {
					payloadSize += 1;
				}
			}
		}

		//printf("%d\n", (*freelist_head).header.block_size << 4);

		/* First fit */
		bool fit = true;
		sf_free_header* fitHeader = freelist_head;
		sf_free_header* prevHolder = (*freelist_head).prev;
		sf_free_header* nextHolder = (*freelist_head).next;

		while(fit) {
			/* The requested memory will fit in the block */
			if(payloadSize <= ((*fitHeader).header.block_size << 4)){
				sf_header * header = (sf_header*) pointer;
				int sizeHolder = (*header).block_size << 4;
				(*header).alloc = 1;
				(*header).block_size = (payloadSize + 16) >> 4;
				(*header).requested_size = size;

				//printf("%d\n", sizeHolder);

				pointer += 8;

				void* blockFooter = pointer;
				blockFooter += ((*header).block_size << 4) - 16;
				sf_footer * bFooter = (sf_footer*) blockFooter;
				(*bFooter).alloc = 1;
				(*bFooter).block_size = (*header).block_size;

				//printf("%p\n", bFooter);


				/* Update the freelist*/

				void* realignHeader = (void*) blockFooter;
				realignHeader += 8;
				fitHeader = (sf_free_header*) realignHeader;
				(*fitHeader).header.alloc = 0;
				(*fitHeader).header.block_size = (sizeHolder >> 4) - ((*header).block_size);

				if((*fitHeader).header.block_size >= 32){
					void* footerRealign = (void*) fitHeader;
					footerRealign += 8;
					footerRealign += ((*fitHeader).header.block_size << 4) - 16;
					sf_footer* freeFooter = (sf_footer*) footerRealign;
					(*freeFooter).block_size = (*fitHeader).header.block_size;
					if((unsigned long) freelist_head == (unsigned long) header){
						freelist_head = fitHeader;
						(*freelist_head).prev = prevHolder;
						(*freelist_head).next = nextHolder;
					}
				}

				//printf("%d\n", (*freelist_head).header.block_size << 4);
				/*
				void* footerRealign = (void*) fitHeader;
				footerRealign += 8;
				footerRealign += ((*fitHeader).header.block_size << 4) - 16;
				sf_footer* freeFooter = (sf_footer*) footerRealign;
				(*freeFooter).block_size = (*fitHeader).header.block_size;
				sf_free_header* tempHeader = freelist_head;
				while((*tempHeader).next != NULL){
					tempHeader = (*tempHeader).next;
					*
					if((unsigned long) tempHeader == (unsigned long) (*tempHeader).next){
						(*tempHeader).next = NULL;
					}
					*
				}
				* tempHeader.next now equals NULL *
				(*tempHeader).next = fitHeader;

				bool adjustHeader = true;
				while(adjustHeader){
					if((*freelist_head).header.alloc == 1){
						freelist_head = (*freelist_head).next;
					} else {
						adjustHeader = false;
					}
				}

				* TODO Test thing, delete before submission *
				adjustHeader = true;
				sf_free_header* testHeader = freelist_head;
				while(adjustHeader){
					//printf("%p\n", testHeader);
					if((*testHeader).next != NULL) {
						testHeader = (*testHeader).next;
					} else {
						adjustHeader = false;
					}
				}
				*
				*/
				//printf("%p\n", freeFooter);

				fit = false;
			} else {
				/* Current block couldn't store it. Check next block */
				if((*fitHeader).next != NULL){
					fitHeader = (*fitHeader).next;
				} else {
					/* Request more memory to store value */
					int blocks = size / PAGE_SIZE;
					/* Need one more block to fulfill */
					if(!((blocks * PAGE_SIZE) > size)){
						blocks += 1;
					}

					//printf("%d\n%d\n", blocks, blocks * PAGE_SIZE);

					sf_free_header* newHeader = (sf_free_header*) sf_sbrk(blocks * PAGE_SIZE);
					(*newHeader).header.block_size = blocks * PAGE_SIZE >> 4;
					//(*newHeader).header.requested_size = size;
					(*newHeader).header.alloc = 1;

					void* fitFooterTemp = (void*) newHeader;
					fitFooterTemp += 8;
					fitFooterTemp += ((*newHeader).header.block_size << 4) - 16;
					sf_footer* fitFooter = (sf_footer*) fitFooterTemp;
					(*fitFooter).block_size = (*newHeader).header.block_size;
					(*fitFooter).alloc = 1;

					(*fitHeader).next = newHeader;
					(*newHeader).prev = fitHeader;

					//printf("%p\n%p\n%p\n", freelist_head, fitHeader, newHeader);
				}
			}
		}
		return pointer;
	}
	return NULL;
}

void sf_free(void *ptr) {
	if((ptr > sf_sbrk(0)) || (ptr < bottomOfHeap) || ((unsigned long) ptr % 16 != 0)){
    	/* Invalid pointer */
    } else {
    	sf_free_header* header = (sf_free_header*) (ptr - 8);
		(*header).header.alloc = 0;
		void* footerAlign = ptr;
		footerAlign += ((*header).header.block_size << 4) - 16;
		sf_footer* footer = (sf_footer*) footerAlign;
		(*footer).alloc = 0;

		/* Coallescing */
		sf_footer* prevFooter = (sf_footer*) (ptr - 16);
		sf_header* nextHeader = (sf_header*) (footerAlign + 8);
		/* Check for edge cases */
		if ((unsigned long) nextHeader > (unsigned long) sf_sbrk(0)){
			/* The block we freed was at the end of the heap */
			nextHeader = NULL;
		} else if ((unsigned long) prevFooter < (unsigned long) bottomOfHeap){
			/* The block we freed was at the start of the heap */
			prevFooter = NULL;
		}
		/* This is to avoid segfaults by going out of bounds of the heap */
		int nextAlloc;
		int prevAlloc;
		if(nextHeader != NULL && prevFooter != NULL){
			nextAlloc = (*nextHeader).alloc;
			prevAlloc = (*prevFooter).alloc;
		} else if(nextHeader == NULL){
			nextAlloc = 1;
			prevAlloc = (*prevFooter).alloc;
		} else {
			nextAlloc = (*nextHeader).alloc;
			prevAlloc = 1;
		}
		/* Figure out what case we have */
		if(nextAlloc == 1 && prevAlloc == 1){
			/* Case 1. Surrounding blocks are allocated. Do nothing. */
		} else if (nextAlloc == 0 && prevAlloc == 1){
			/* Case 2. Coallesce next block. */
			void* movingPtr = (void*) nextHeader;
			movingPtr += 8;
			movingPtr += ((*nextHeader).block_size << 4) - 16;
			sf_footer* nextFooter = (sf_footer*) movingPtr;
			int newSize = (*header).header.block_size + (*nextHeader).block_size;
			(*header).header.block_size = newSize;
			(*nextFooter).block_size = newSize;

			if((unsigned long) nextHeader == (unsigned long) freelist_head) {
				freelist_head = header;
			}
		} else if (nextAlloc == 1 && prevAlloc == 0){
			/* Case 3. Coallesce prev block. */
			int newSize = (*header).header.block_size + (*prevFooter).block_size;
			void* movingPtr = (void*) prevFooter;
			movingPtr -= 8;
			movingPtr -= ((*nextHeader).block_size << 4) - 16;
			bool isFreeHead = false;

			if((unsigned long) header == (unsigned long) freelist_head){
				isFreeHead = true;
			}

			header = (sf_free_header*) movingPtr;
			(*header).header.block_size = newSize;
			(*footer).block_size = newSize;

			if(isFreeHead){
				header = freelist_head;
			}
		} else {
			/* Case 4. Coallesce both blocks. */
			bool isFreeHead = false;
			int newSize = (*header).header.block_size + (*prevFooter).block_size + (*nextHeader).block_size;

			void* movingPtr = (void*) nextHeader;
			movingPtr += 8;
			movingPtr += ((*nextHeader).block_size << 4) - 16;
			sf_footer* nextFooter = (sf_footer*) movingPtr;

			movingPtr = (void*) prevFooter;
			movingPtr -= 8;
			movingPtr -= ((*nextHeader).block_size << 4) - 16;
			header = (sf_free_header*) movingPtr;

			(*header).header.block_size = newSize;
			(*nextFooter).block_size = newSize;

			if((unsigned long) nextHeader == (unsigned long) freelist_head || (unsigned long) header == (unsigned long) freelist_head){
				isFreeHead = true;
			}
			if(isFreeHead) {
				freelist_head = header;
			}
		}
		/* Done coallescing, update freelist if necessary */
		if((unsigned long) header != (unsigned long) freelist_head){
			(*header).next = NULL;
			(*header).prev = NULL;
			/* The new block and the freelist_head are two separate blocks. 
				They must be linked. */
			if((unsigned long) header > (unsigned long) freelist_head) {
				sf_free_header* headerLink = freelist_head;
				while((*headerLink).next != NULL){
					headerLink = (*headerLink).next;
				}
				(*headerLink).next = header;
				(*header).prev = headerLink;
			} else {
				sf_free_header* headerLink = freelist_head;
				while((*headerLink).prev != NULL){
					headerLink = (*headerLink).prev;
				}
				(*headerLink).prev = header;
				(*header).next = headerLink;
				freelist_head = header;
			}
		} 
    }
}

void* sf_realloc(void *ptr, size_t size) {
    if(size == 0) {
    	
    	if(ptr == NULL){
    		return sf_malloc(0);
    	} else {
    		return NULL;
    	}
    } else {
    	/* Size is non-zero */
    	/* No pointer given, return sf_malloc of given size */
    	if(ptr == NULL) {
    		return sf_malloc(size);
    	} else {
    		/* User wants given malloc to be resized. Let's check if we can. */
    		/* Is the address within the heap range? */
    		if(ptr > sf_sbrk(0) || ptr < bottomOfHeap){
    			return NULL;
    		} else if ((unsigned long) ptr % 16 != 0){
    			/* Address must be divisibile by 16. */
    			return NULL;
    		} else {
    			sf_header* header = (sf_header*) (ptr - 8);
    			if((*header).alloc == 0){
    				return NULL;
    			} else {
    				/* We reached this point. We can realloc. Let's do it. */

    				/* Check to see if growing bigger or smaller */
    				void* newBlock;
    				int oldPayloadSize = ((*header).block_size << 4) - 16;
    				if(size <= oldPayloadSize) {
    					/* Block growing smaller */
    					/* Get the size of the block that is going to be made and the leftover block 
    						avoid splinters */
    					int i = 0;
    					int payloadSize = size;
    					for(; i < 16; i++){
    						if(payloadSize % 16 != 0){
    							payloadSize++;
    						} else {
    							break;
    						}
    					}
    					/* If the payloadSize is the same as the current block, no change needed */
    					if(payloadSize == oldPayloadSize){
    						(*header).requested_size = size;
    						return ptr;
    					}
    					int newBlockSize = payloadSize + 16;
    					int oldBlockSize = (*header).block_size << 4;
    					int leftoverBlockSize = oldBlockSize - newBlockSize;
    					if(leftoverBlockSize < 32){
    						/* The new block will be a splinter. Can't do that. Need bigger block */
    						goto reallocBigger;
    					} else {
    						/* The new block will be a valid free block. We'll be fine, let's do it. */
    						(*header).alloc = 1;
    						(*header).requested_size = size;
    						(*header).block_size = (payloadSize + 16) >> 4;
    						void* footerRealign = ptr;
    						footerRealign += ((*header).block_size << 4) - 16;
    						sf_footer* footer = (sf_footer*) footerRealign;
    						(*footer).alloc = 1;
    						(*footer).block_size = (*header).block_size;

    						//printf("%p\n", footer);

    						/* After this point, footerRealign is being reused,
    							first as a pointer to the next header for the free
    							block and then as the block to free for the purposes
    							of coallescing */
    						footerRealign += 8;
    						sf_header* newHeader = (sf_header*) footerRealign;
    						(*newHeader).alloc = 1;
    						(*newHeader).block_size = newBlockSize >> 4;
    						void* newFooterRealign = footerRealign;
    						newFooterRealign += ((*newHeader).block_size << 4) - 16;
    						sf_footer* newFooter = (sf_footer*) newFooterRealign;

    						//printf("%p\n", newFooter);

    						(*newFooter).alloc = 1;
    						(*newFooter).block_size = (*newHeader).block_size;

    						footerRealign += 8;
    						sf_free(footerRealign);

    						return ptr;
    					}
    				} else {
				reallocBigger: 
    					newBlock = sf_calloc(1, size);
    					int i = 0;
    					char* holder = (char*) ptr;
    					char* newBlockHolder = (char*) newBlock;
    					char placer;
    					for(; i < size; i++){
    						placer = *holder;
    						holder += 1;
    						memset(newBlockHolder, placer, 1);
    						newBlockHolder += 1;
    					}
    					return newBlock;
    				}
    			}
    		}
    	}
    }
}

void* sf_calloc(size_t nmemb, size_t size) {
	/* Unless I'm mistaken, the size is just nmemb * size
		since it wants nmemb units of size size */
	void* mem = sf_malloc(size * nmemb);
    return memset(mem, 0, size * nmemb);
}
