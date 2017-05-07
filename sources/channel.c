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
            channels[chan_index].chanid = i;
            channels[chan_index].read = 1;
            channels[chan_index].write = 1;
            return chan_index;
        }
    }
    errno = EMCHAN;
    return -1;
}

int free_channel(int chanid, channel_state_t *channels) {
    // Frees the corresponding channel
    if (check_channel(chanid, channels) == -1)
        return -1;
    
    int kchanid = channels[chanid].chanid;
    if (channels_table[kchanid].len != 0 || channels_table[kchanid].receiver >= 0) {
        errno = EINPROGRESS;
        return -1;
    }
    // TODO free(channels_table[chanid].data);
    channels_table[chanid].data = NULL;
    channels[chanid].chanid = -1;
    return 0;
}

ssize_t send(state *s) {
    // Decode the registers of state s to get args.
    // Tries to send a message of length len bytes into channel chanid.
    // Returns the number of transmitted bytes.
    // If no process is waiting for receiving enough bytes, this blocks.
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
    len = umin(len, channels_table[kchanid].size);
    
    if (len == 0) {
        errno = EINVAL;
        return -1;
    }
    
    if (channels_table[kchanid].sender >= 0) {
        // Someone is already sending a message on it.
        errno = EALREADY;
        return -1;
    }
    
    // Saves the message in the channel buffer.
    memcpy(channels_table[kchanid].data, buffer, len);
    channels_table[kchanid].len = len;
    channels_table[kchanid].sender = sender;
    
    // TODO Set sender registers !
    if (channels_table[kchanid].receiver >= 0) {
        // Someone is waiting for receiving.
        pid_t receiver = channels_table[kchanid].receiver;
        // Sets the return values in the registers of receiving process
        s->processes[receiver].saved_context.regs.eax = sender;
        // Unblocks receiver
        s->processes[receiver].state = RUNNABLE;
    }
    s->processes[sender].state = BLOCKEDWRITING;
    return len;
}

ssize_t receive(state *s) {
    // Receive the message contained in channel chanid, and unblocks the sender.
    // Returns the size of the read message.
    // If there isn't any message, returns immediately with value -1 and sets errno.
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
    pid_t sender = channels_table[kchanid].sender;
    if (sender == -1) {
        errno = EEMPTY;
        return -1;
    }
    
    if (channels_table[kchanid].receiver != receiver) {
        if (channels_table[kchanid].receiver >= 0) {
            errno = EOCCUPIED;
            return 1;
        }
        else
            channels_table[kchanid].receiver = receiver;
    }
    size_t msg_len = channels_table[kchanid].len;
    if (msg_len > len) {
        errno = EMSGSIZE; // Not enough space in the buffer
        return -1;
    }
    
    // Reads the message and then unblocks the sender.
    memcpy(buffer, channels_table[kchanid].data, msg_len);
    s->processes[sender].state = RUNNABLE;
    
    // The message was read, both sender and receivers are unblocked.
    // Reset of the channel.
    channels_table[kchanid].receiver = -1;
    channels_table[kchanid].sender = -1;
    channels_table[kchanid].len = 0;
    return msg_len;
}

pid_t wait_channel(state *s) {
    // Waits for a message on channel chanid. When a message arrives, it returns
    // without reading it. To get the content, it uses receive syscall.
    // Return the pid of the sender.
    int chanid = s->ctx->regs.ebx;
    pid_t receiver = s->curr_pid;
    channel_state_t *channels = s->processes[s->curr_pid].channels;
    
    if (check_channel(chanid, channels) == -1) 
        return -1;
    if (!channels[chanid].read) {
        errno = EPERM;
        return -1;
    }
    int kchanid = channels[chanid].chanid;
    if (channels_table[kchanid].receiver >= 0) {
        errno = EOCCUPIED;
        return -1;
    }
    channels_table[kchanid].receiver = receiver;
    pid_t sender = channels_table[kchanid].sender;
    
    
    if (sender == -1) {
        // Blocks the receiver.
        s->processes[receiver].state = BLOCKEDREADING;
    }
    errno = 0;
    return sender;
}
