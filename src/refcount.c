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
#include <string.h>
#include "refcount.h"

void *ref_alloc(size_t size)
{
  obj_t *obj;
  char *ptr;
  int result;

  obj = (obj_t *)calloc(sizeof(obj_t) + size, 1);
  if (obj == NULL) {
    return NULL;
  } else {
    obj->ref = 1;

    if ((result = pthread_mutex_init(&obj->mutex, NULL))) {
      free(obj);
      return NULL;
    }
    ptr = (char *)obj;
    ptr += sizeof(obj_t);
    obj->data = ptr;
    return (void *)ptr;
  }
}

void ref_retain(void *ptr)
{
  obj_t *obj;
  char *cptr;

  cptr = (char *)ptr;
  cptr -= sizeof(obj_t);
  obj = (obj_t *)cptr;
  pthread_mutex_lock(&obj->mutex);
  obj->ref++;
  pthread_mutex_unlock(&obj->mutex);
}

void ref_release(void *ptr)
{
  obj_t *obj;
  char *cptr;
  pthread_mutex_t mutex;
  int dofree = 0;

  cptr = (char *)ptr;
  cptr -= sizeof(obj_t);
  obj = (obj_t *)cptr;
  mutex = obj->mutex;

  pthread_mutex_lock(&obj->mutex);
  if (obj->ref-- == 0) {
    dofree = 1;
  }
  pthread_mutex_unlock(&obj->mutex);
  if (dofree) {
    pthread_mutex_destroy(&obj->mutex);
    free(obj);
  }
}

int ref_count(void *ptr)
{
  obj_t *obj;
  char *cptr;
  int value;

  cptr = (char *)ptr;
  cptr -= sizeof(obj_t);
  obj = (obj_t *)cptr;

  pthread_mutex_lock(&obj->mutex);
  value = obj->ref;
  pthread_mutex_unlock(&obj->mutex);
  return value;
}

int ref_pthread_mutex_lock(void *ptr)
{
  obj_t *obj;
  char *cptr;

  cptr = (char *)ptr;
  cptr -= sizeof(obj_t);
  obj = (obj_t *)cptr;
  return pthread_mutex_lock(&obj->mutex);
}

int ref_pthread_mutex_unlock(void *ptr)
{
  obj_t *obj;
  char *cptr;

  cptr = (char *)ptr;
  cptr -= sizeof(obj_t);
  obj = (obj_t *)cptr;
  return pthread_mutex_unlock(&obj->mutex);
}

int ref_pthread_mutex_trylock(void *ptr)
{
  obj_t *obj;
  char *cptr;

  cptr = (char *)ptr;
  cptr -= sizeof(obj_t);
  obj = (obj_t *)cptr;
  return pthread_mutex_trylock(&obj->mutex);
}

int ref_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                       void *(*start_routine)(void *), void *arg)
{
  int ret;

  ref_retain(arg);
  if ((ret = pthread_create(thread, attr, start_routine, arg))) {
    ref_release(arg);
  }
  return ret;
}
