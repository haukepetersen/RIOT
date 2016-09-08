

/* TODO: can we use something smarter here, e.g. main thread tcb ptr == NULL? */
static int init = 1;

/**
 *
 *
 * @note    This function will throw a kernel panic in case memory runs out!
 */
void *kinit_malloc(size_t len)
{
    /* make sure we are in the init phase */
    if (!init) {
        core_panic(PANIC_GENERAL_ERROR, "kinit_malloc called after init");
    }

    /* allocate the memory from the top of the heap */

    /* throw kernel panic on insufficient memory */
}

/**
 * @brief   This function closes the initialization phase. Can't be redone.
 */
void kinit_done(void)
{
    init = 0;
}
