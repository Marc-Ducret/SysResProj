#ifndef CHANNEL_H
#define CHANNEL_H
#include "int.h"
#include "kernel.h"
#include <stddef.h>
#include "error.h"

#define CHANNEL_SIZE 512
#define NB_MAX_CHANNELS 100

typedef struct {
    u8 *data;
    size_t size;
    size_t len;
    pid_t sender;
    pid_t receiver;
} channel_t;

int new_channel(channel_state_t *channels);
ssize_t send(state *s);
ssize_t receive(state *s);
int free_channel(int chanid, channel_state_t *channels);
pid_t wait_channel(state *s);


#endif /* CHANNEL_H */

