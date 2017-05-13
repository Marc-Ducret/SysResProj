#include "channel.h"

u8 static_data[NB_MAX_CHANNELS * CHANNEL_SIZE];
channel_t channels_table[NB_MAX_CHANNELS];

int check_channel(int chanid, channel_state_t *channels) {
    if (chanid < 0 || chanid >= NUM_CHANNELS_PROC || channels[chanid].chanid < 0) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int new_channel(channel_state_t *channels) {
    // Creates a new channel in the given process channel table.
    // Finds an empty place
    int chan_index = 0;
    for (; chan_index < NUM_CHANNELS_PROC; chan_index++) {
        if (channels[chan_index].chanid < 0) {
            break;
        }
    }
    if (chan_index == NUM_CHANNELS_PROC) {
        errno = EMCHAN;
        return -1;
    }
    
    for (int i = 1; i < NB_MAX_CHANNELS; i++) {
        if (channels_table[i].data == NULL) {
            // TODO malloc(data) ?
            channels_table[i].data = static_data + i * CHANNEL_SIZE;
            channels_table[i].len = 0;
            channels_table[i].size = CHANNEL_SIZE;
            channels_table[i].sender = -1;
            channels_table[i].receiver = -1;
            channels_table[i].nb_users = 1;
            channels[chan_index].chanid = i;
            channels[chan_index].read = 1;
            channels[chan_index].write = 1;
            return chan_index;
        }
    }
    errno = EMCHAN;
    return -1;
}

int free_channel(state *s, int chanid, channel_state_t *channels) {
    // Frees the corresponding channel for one user.
    if (check_channel(chanid, channels) == -1)
        return -1;
    
    int kchanid = channels[chanid].chanid;
    channels_table[kchanid].nb_users -= 1;
    if (channels_table[kchanid].nb_users == 1) {
        // Release with an error the receiver and sender.
        pid_t waiter = channels_table[kchanid].receiver;
        if (waiter >= 0) {
            // Release receiver
            s->processes[waiter].saved_context.regs.eax = -1;
            s->processes[waiter].saved_context.regs.ebx = EALONE;
            s->processes[waiter].state = RUNNABLE;
            channels_table[kchanid].receiver = -1;
        }
        waiter = channels_table[kchanid].sender;
        if (waiter >= 0) {
            // Release sender
            s->processes[waiter].saved_context.regs.eax = -1;
            s->processes[waiter].saved_context.regs.ebx = EALONE;
            s->processes[waiter].state = RUNNABLE;
            channels_table[kchanid].receiver = -1;
        }
    }
    
    if (channels_table[kchanid].nb_users <= 0) {
        channels_table[kchanid].data = NULL;
    }
    channels[chanid].chanid = -1;
    return 0;
}

ssize_t send(state *s) {
    // Decode the registers of state s to get args.
    // Tries to send a message of length len bytes into channel chanid.
    // Returns the number of transmitted bytes.
    
    int chanid = s->ctx->regs.ebx;
    u8 *buffer = (u8*) s->ctx->regs.ecx;
    if (check_address(buffer, 1, 1, s->processes[s->curr_pid].page_directory) == -1) {
        return -1;
    }
    size_t len = s->ctx->regs.esi;
    pid_t sender = s->curr_pid;
    channel_state_t *channels = s->processes[s->curr_pid].channels;
    
    if (check_channel(chanid, channels) == -1)
        return -1;
    if (!channels[chanid].write) {
        errno = EPERM; // No rights to write.
        return -1;
    }
    int kchanid = channels[chanid].chanid;
    size_t empty = channels_table[kchanid].size - channels_table[kchanid].len;
    len = umin(len, empty);
    
    if (len == 0)
        return 0; // Nothing to do.
    
    pid_t old_sender = channels_table[kchanid].sender;
    if (old_sender >=0 && old_sender != sender) {
        errno = EOCCUPIED;
        return -1;
    }
    
    // Writes the message in the channel buffer.
    size_t start = channels_table[kchanid].write;
    u8 *data = channels_table[kchanid].data;
    size_t first_step = channels_table[kchanid].size - start;
    int overflow = len >= first_step;
    memcpy(data + start, buffer, overflow ? first_step : len);
    if (overflow)
        memcpy(data, buffer + first_step, len - first_step);
    
    channels_table[kchanid].len += len;
    channels_table[kchanid].write = overflow ? (len - first_step) : (start + len);

    if (channels_table[kchanid].receiver >= 0) {
        // Someone is waiting for receiving.
        pid_t receiver = channels_table[kchanid].receiver;
        // Sets the return values in the registers of receiving process
        s->processes[receiver].saved_context.regs.eax = len;
        // Unblocks receiver
        s->processes[receiver].state = RUNNABLE;
        channels_table[kchanid].receiver = -1; // RESET, NO LOCK (TODO ?)
    }
    return len;
}

ssize_t receive(state *s) {
    // Tries to read at most len bytes in the buffer.
    // Returns the size of the read message.
    
    int chanid = s->ctx->regs.ebx;
    u8 *buffer = (u8*) s->ctx->regs.ecx;
    if (check_address(buffer, 1, 1, s->processes[s->curr_pid].page_directory) == -1) {
        return -1;
    }
    size_t len = s->ctx->regs.esi;
    pid_t receiver = s->curr_pid;
    channel_state_t *channels = s->processes[s->curr_pid].channels;
    
    if (check_channel(chanid, channels) == -1)
        return -1;
    
    if (!channels[chanid].read) {
        errno = EPERM; // No rights to read.
        return -1;
    }
    
    int kchanid = channels[chanid].chanid;
    len = umin(len, channels_table[kchanid].len);
    
    if (len == 0)
        return 0; // Nothing to do.
    
    pid_t old_receiver = channels_table[kchanid].receiver;
    if (old_receiver >= 0 && old_receiver != receiver) {
        errno = EOCCUPIED;
        return 1;
    }
    
    // Reads len bytes of the message.
    size_t start = channels_table[kchanid].read;
    u8 *data = channels_table[kchanid].data;
    size_t first_step = channels_table[kchanid].size - start;
    int overflow = len >= first_step;
    memcpy(buffer, data + start, overflow ? first_step : len);
    if (overflow)
        memcpy(buffer + first_step, data, len - first_step);
    
    channels_table[kchanid].len -= len;
    channels_table[kchanid].read = overflow ? (len - first_step) : (start + len);

    if (channels_table[kchanid].sender >= 0) {
        // Someone is waiting for writing.
        pid_t sender = channels_table[kchanid].sender;
        // Sets the return values in the registers of waiting process
        s->processes[sender].saved_context.regs.eax = len;
        // Unblocks receiver
        s->processes[sender].state = RUNNABLE;
        channels_table[kchanid].sender = -1; // RESET, NO LOCK (TODO ?)
    }
    return len;
}

ssize_t wait_channel(state *s) {
    // Waits for a message or some place on channel chanid. 
    // When the wanted event occurs, it returns without doing anything. 
    // To get the message or send some, it uses send / receive syscalls.
    // Return the size of the event.
    
    int chanid = s->ctx->regs.ebx;
    int write = s->ctx->regs.ecx;
    pid_t waiter = s->curr_pid;
    channel_state_t *channels = s->processes[s->curr_pid].channels;
    
    if (check_channel(chanid, channels) == -1) 
        return -1;
    if (write ? (!channels[chanid].write) : (!channels[chanid].read)) {
        errno = EPERM;
        return -1;
    }
    int kchanid = channels[chanid].chanid;
    
    if (write ? channels_table[kchanid].sender >= 0 : channels_table[kchanid].receiver >= 0) {
        errno = EOCCUPIED;
        return -1;
    }
    if (write) {
        // Looks for some place.
        size_t empty = channels_table[kchanid].size - channels_table[kchanid].len;
        if (empty == 0) {
            if (channels_table[kchanid].nb_users <= 1) {
                errno = EALONE;
                return -1;
            }
            // Blocks the waiter.
            channels_table[kchanid].sender = waiter;
            s->processes[waiter].state = BLOCKEDWRITING;
        }
        errno = 0;
        return empty;
    }
    else {
        // Looks for some data.
        size_t len = channels_table[kchanid].len;
        if (len == 0) {
            if (channels_table[kchanid].nb_users <= 1) {
                errno = EALONE;
                return -1;
            }
            // Blocks the waiter.
            channels_table[kchanid].receiver = waiter;
            s->processes[waiter].state = BLOCKEDREADING;
        }
        errno = 0;
        return len;
    }
}
