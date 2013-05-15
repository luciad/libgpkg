/*
 * Copyright 2013 Luciad (http://www.luciad.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ALLOC_H
#define ALLOC_H

/**
 * \addtogroup alloc Memory management
 * @{
 */

/**
 * A memory allocator
 */
typedef struct {
    /**
     * Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
     * When the block of memory is no longer required, it should be release using the free function of this allocator.
     *
     * @param size the size of the memory block to allocate in bytes
     * @return A pointer to the allocated memory block or NULL if the allocation failed
     */
    void *(*malloc)(int size);
    /**
     * Changes the size of a previously allocated block of memory. Implementations may choose to either grow the existing
     * memory block or allocate a new memory block. The content of the previous memory block will be preserved.
     *
     * @param ptr A pointer to the block of memory to release
     * @param size the new size of the memory block in bytes
     * @return A pointer to the reallocated memory block or NULL if reallocation failed
     */
    void *(*realloc)(void* ptr,int size);
    /**
     * Releases a block of previously allocated memory.
     *
     * @param ptr A pointer to the block of memory to release
     */
    void (*free)(void* ptr);
} allocator_t;

/** @} */

#endif