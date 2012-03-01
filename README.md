# refcount

A simple c program that demonstrates one style of reference counting in c.

## Building

    $ make

If all goes well a sample program in `src/refcount` will be built. Run the
sample program to test things out.

## Reference Counting

To allocate memory with reference counting use the `ref_alloc` function defined
in refcount.h. The returned pointer will point to the allocated memory and
have the initial reference count set to 1. To increase the reference count
call the `ref_retain` function. To decrease the reference count use the
`ref_release` function. When the reference count reaches zero by calling
`ref_release` the allocated memory will be freed.

If the reference counted object is passed to another thread, the parent
thread must call `ref_retain` on behalf of the new thread before the object is
passed. The new thread must call `ref_release` before terminating.

See `refcount.h` for details.

## Other Functions

* `ref_count` - gets the current reference count
* `ref_pthread_mutex_lock` - locks the object mutex
* `ref_pthread_mutex_unlock` - unlocks the object mutex
* `ref_pthread_try_lock` - tries the mutex lock for the object
* `ref_pthread_create` - creates a new thread incrementing the ref count before
    passing the object to the new thread

The `ref_pthread` functions are analogous to the pthread functions of the same
name. Instead of specifying a mutex to use for lock operations you pass a
pointer to the reference counted object and its mutex is used.

See `refcount.h` for full details

## How it Works

When memory is allocated with `ref_alloc` additional memory is also
allocated which is used to store the reference count and a mutex. Calls to
`ref_retain` and `ref_release` increment and decrement the retain count.
Access to the counter are protected by the mutex. The mutex can also be used
to protect the data by use the `ref_pthread` functions.

See `refcount.h` and `refcount.c` for details.
