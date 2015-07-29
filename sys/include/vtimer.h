#ifndef VTIMER_H
#define VTIMER_H

/**
 * @brief IPC message type for vtimer msg callback
 */
#define MSG_TIMER 12345

#include "msg.h"
#include "timex.h"
#include "xtimer.h"

#include <time.h>
#include <sys/time.h>

typedef struct vtimer {
    xtimer_t timer;
    msg_t msg;
} vtimer_t;

void vtimer_now(timex_t *out);
int vtimer_sleep(timex_t time);
int vtimer_msg_receive_timeout(msg_t *m, timex_t timeout);
void vtimer_get_localtime(struct tm *localt);
void vtimer_set_msg(vtimer_t *t, timex_t interval, kernel_pid_t pid, uint16_t type, void *ptr);
void vtimer_gettimeofday(struct timeval *tp);
void vtimer_remove(vtimer_t *t);

#endif /* VTIMER_H */
