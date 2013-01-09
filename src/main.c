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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include "refcount.h"

#define ITERS 100000

/**
 * This function increments the integer, locking the object when doing so
 */
void *incr(void *arg)
{
  int i;
  int *an_int;
  int lock_ret;

  an_int = (int *)arg;
  printf("\tincr in ref count: %d\n", ref_count(an_int));

  for (i = 0; i < ITERS; ++i) {
    if ((lock_ret = ref_pthread_mutex_lock(an_int))) {
      char error_message[BUFSIZ];
      int strerror_error;

      strerror_error = strerror_r(lock_ret, error_message, BUFSIZ);
      if (strerror_error)
        printf("incr could not lock (%d), could not get error message (%d)\n",
               lock_ret, strerror_error);
      else
        fprintf(stderr, "incr could not lock (%d): %s\n", lock_ret,
                error_message);
      exit(1);
    }
    (*an_int)++;
    ref_pthread_mutex_unlock(an_int);
    sched_yield();
  }

  /*
   * ref_pthread_create retained for this thread; the calling thread needs
   * to increment the retain count before starting the thread
   */
  ref_release(an_int);
  printf("\tincr ref count after release: %d\n", ref_count(an_int));
  return 0;
}

/**
 * This function decrements the integer, locking the object when doing so
 */
void *decr(void *arg)
{
  int i;
  int *an_int;
  int lock_ret;

  an_int = (int *)arg;
  printf("\tdecr in ref count: %d\n", ref_count(an_int));
  for (i = ITERS - 1; i >= 0; --i) {
    if((lock_ret = ref_pthread_mutex_lock(an_int))) {
      char error_message[BUFSIZ];
      int strerror_error;

      strerror_error = strerror_r(lock_ret, error_message, BUFSIZ);
      if (strerror_error)
        printf("decr could not lock (%d), could not get error message (%d)\n",
               lock_ret, strerror_error);
      else
        fprintf(stderr, "decr could not lock (%d): %s\n", lock_ret,
                error_message);
      exit(2);
    }
    (*an_int)--;
    sched_yield();
    ref_pthread_mutex_unlock(an_int);
  }
  ref_release(an_int);
  printf("\tdecr ref count after release: %d\n", ref_count(an_int));
  return 0;
}

int main(void)
{
  int *an_int;
  pthread_t incr_thr;
  pthread_t decr_thr;

  printf("A simple reference counting example in C.\n"
         "Spawn an increment and decrement thread for a shared integer.\n"
         "Each thread iterates %d times.\n"
         "If all goes well the start and end int will have the same value.\n",
         ITERS);

  /* allocates the object, ref count is 1 */
  an_int = ref_alloc(sizeof(*an_int));
  if (an_int == NULL) {
    fprintf(stderr, "Could not allocate int\n");
    exit(1);
  }
  printf("alloc ref count: %d\n", ref_count(an_int));

  /* use the object normally, no need to lock as no other threads have access */
  *an_int = 100;

  printf("start: %d\n", *an_int);

  /* create incrementing thread */
  if (ref_pthread_create(&incr_thr, NULL, &incr, an_int)) {
    ref_release(an_int);
    fprintf(stderr, "Could not create thread\n");
    exit(1);
  }

  /* create decrementing thread */
  if (ref_pthread_create(&decr_thr, NULL, &decr, an_int)) {
    ref_release(an_int);
    fprintf(stderr, "Could not create thread\n");
    exit(1);
  }

  /* wait for threads to finish */
  if (pthread_join(incr_thr, NULL)) {
    fprintf(stderr, "Could not join\n");
    exit(1);
  }
  if (pthread_join(decr_thr, NULL)) {
    fprintf(stderr, "Could not join\n");
    exit(1);
  }

  /* count should be the same as the original */
  printf("end: %d\n", *an_int);
  printf("\tref count before release: %d\n", ref_count(an_int));
  ref_release(an_int);

  return 0;
}
