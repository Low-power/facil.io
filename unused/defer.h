/*
Copyright: Boaz Segev, 2016-2017
License: MIT

Feel free to copy, use and enjoy according to the license provided.
*/
#ifndef H_DEFER_H
/**
A library for deferring execution of code.

Deferred execution can be multi-threaded, although threads aren't managed by the
library.

All deferred execution is shared among the same process and inherited by any
forked process.
*/
#define H_DEFER_H

#ifdef __cplusplus
extern "C" {
#endif
/* *****************************************************************************
Core API
***************************************************************************** */

/** Defer an execution of a function for later. Returns -1 on error.*/
int defer(void (*func)(void *), void *arg);

/** Performs all deferred functions until the queue had been depleted. */
void defer_perform(void);

/** returns true if there are deferred functions waiting for execution. */
int defer_has_queue(void);

/* *****************************************************************************
Thread Pool support
***************************************************************************** */

/** an opaque thread pool type */
typedef struct defer_pool *pool_pt;

/** Starts a thread pool that will run deferred tasks in the background. */
pool_pt defer_pool_start(unsigned int thread_count);
/** Signals a running thread pool to stop. Returns immediately. */
void defer_pool_stop(pool_pt pool);
/** Waits for a running thread pool, joining threads and finishing all tasks. */
void defer_pool_wait(pool_pt pool);
/** Returns TRUE (1) if the pool is hadn't been signaled to finish up. */
int defer_pool_is_active(pool_pt pool);

/**
OVERRIDE THIS to replace the default pthread implementation.

Accepts a pointer to a function and a single argument that should be executed
within a new thread.

The function should allocate memory for the thread object and return a pointer
to the allocated memory that identifies the thread.

On error NULL should be returned.
*/
void *defer_new_thread(void *(*thread_func)(void *), void *arg);

/**
OVERRIDE THIS to replace the default pthread implementation.

Accepts a pointer returned from `defer_new_thread` (should also free any
allocated memory) and joins the associated thread.

Return value is ignored.
*/
int defer_join_thread(void *p_thr);

/* *****************************************************************************
Child Process support (`fork`)
***************************************************************************** */

/**
 * Forks the process, starts up a thread pool and waits for all tasks to run.
 * All existing tasks will run in all processes (multiple times).
 *
 * It's possible to synchronize workload across processes by using a pipe (or
 * pipes) and a self-scheduling event that reads instructions from the pipe.
 *
 * This function will use SIGINT to signal all the children processes to finish
 * up and exit. It will also setup a child process reaper (which will remain
 * active for the application's lifetime).
 *
 * The `on_finish` callback (and it's argument) will be called by the children
 * before they exit.
 *
 * Returns 0 on success and -1 on error.
 */
int defer_perform_in_fork(unsigned int process_count, unsigned int thread_count,
                          void (*on_finish)(void *), void *arg);
/** Returns TRUE (1) if the forked thread pool hadn't been signaled to finish
 * up. */
int defer_fork_is_active(void);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif