/*
 * Copyright 2012 Jeremy Raymond
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _REFCOUNT_H
#define _REFCOUNT_H

#include <pthread.h>
#include <rpc/types.h>

typedef struct {
  void *data;

  pthread_mutex_t mutex;
  int ref;
} obj_t;

/**
 * Allocates memory of the specified size and sets reference count to 1 for the
 * newly allocated object. Memory must be freed by calling ref_release(void *)
 * on the returned pointer.
 *
 * The pointer returned points to newly allocated memory of the size specified
 * by the user and is to be used normally as if the pointer was created with
 * malloc or calloc except for freeing of the memory. Additional memory, hidden
 * from the user, is also allocated for the use of reference counting and
 * locking (which is why releasing the object via ref_release(void *) is
 * required.
 *
 * @param size the size in bytes of memory to allocate
 * @see ref_release(void *)
 * @return a pointer to the allocated memory or NULL on error
 */
void *ref_alloc(size_t size);

/**
 * Increases the retain count of an object.
 *
 * Call this function on a pointer (obtained from ref_alloc(size_t)) to
 * increase the retain count on the object to keep it from being freed. Ensure
 * ref_release(void *) is called on the object when it is no longer needed.
 *
 * A parent process must retain a reference on behalf of any child process
 * prior to passing along the pointer to the object to the new thread.
 * Processes are only allowed to retain reference for objects they already
 * have a reference to.
 *
 * @param ptr the pointer to the object.
 * @see ref_alloc(size_t)
 * @see ref_release(void *)
 */
void ref_retain(void *ptr);

/**
 * Decreases the retain count of an object by 1 freeing the object when it is no
 * longer referenced (when the reference count reaches 0).
 *
 * Call this function on a pointer (obtained from ref_alloc(size_t)) to
 * decrease the retain count when finished with it. Only call this function on
 * an object created with ref_alloc(size_t) or retained with ref_retain(void *).
 * ref_release(void *) must be called the same number of times
 * ref_alloc(size_t) and ref_retain(void *) have been called on the object
 * for the object's memory to be freed.
 *
 * @param ptr the pointer to the object.
 * @see ref_alloc(size_t)
 * @see ref_retain(void *)
 */
void ref_release(void *ptr);

/**
 * Get the current reference count for the object.
 *
 * @param ptr the pointer to the object.
 */
int ref_count(void *ptr);

/**
 * Locks the mutex for the object. The object must have been created via
 * ref_alloc(size_t).
 *
 * @param ptr pointer to the object to lock
 * @see pthread_mutex_lock
 * @return standard pthread_mutex_lock return values
 */
int ref_pthread_mutex_lock(void *ptr);

/**
 * Unlocks the mutex for the object. The object must have been created via
 * ref_alloc(size_t).
 *
 * @param ptr pointer to the object to unlock
 * @see pthread_mutex_unlock
 * @return standard pthread_mutex_unlock return values
 */
int ref_pthread_mutex_unlock(void *ptr);

/**
 * Tries to lock the mutex for the object. The object must have been created via
 * ref_alloc(size_t).
 *
 * @param ptr pointer to the object to try to lock
 * @see pthread_mutex_trylock
 * @return standard pthread_mutex_trylock return values
 */
int ref_pthread_mutex_trylock(void *ptr);

/**
 * Creates a new thread of execution. Increments retain count before passing
 * object off to new thread. Releases the incremented retain count if thread
 * creation fails.
 *
 * @return standard pthread_create return values
 * @see pthread_create
 */
int ref_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		       void *(*start_routine)(void *), void *arg);

#endif
