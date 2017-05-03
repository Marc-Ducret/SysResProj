#include "channel.h"

u8 static_data[NB_MAX_CHANNELS * CHANNEL_SIZE];
channel_t channels_table[NB_MAX_CHANNELS];

int check_channel(int chanid) {
    if (chanid < 0 || chanid >= NB_MAX_CHANNELS) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int new_channel() {
    // Creates a new channel
    for (int i = 0; i < NB_MAX_CHANNELS; i++) {
        if (channels_table[i].data == NULL) {
            // TODO malloc(data) !
            channels_table[i].data = static_data + i * CHANNEL_SIZE;
            channels_table[i].len = 0;
            channels_table[i].size = CHANNEL_SIZE;
            channels_table[i].sender = -1;
            channels_table[i].receiver = -1;
            return i;
        }
    }
    errno = EMCHAN;
    return -1;
}

int free_channel(int chanid) {
    // Frees the corresponding channel
    if (check_channel(chanid) == -1)
        return -1;
    
    if (channels_table[chanid].len != 0) {
        errno = EINPROGRESS;
        return -1;
    }
    // TODO free(channels_table[chanid].data);
    channels_table[chanid].data = NULL;
    return 0;
}

ssize_t send(state *s) {
    // Decode the registers of state s to get args.
    // Tries to send a message of length len bytes into channel chanid.
    // Returns the number of transmitted bytes.
    // If no process is waiting for receiving enough bytes, this blocks.
    int chanid = s->ctx->regs.ebx;
    u8 *buffer = (u8*) s->ctx->regs.ecx;
    size_t len = s->ctx->regs.esi;
    pid_t sender = s->curr_pid;
    
    if (check_channel(chanid) == -1)
        return -1;
    
    len = umin(len, channels_table[chanid].size);
    
    if (channels_table[chanid].sender >= 0) {
        // Someone is already sending a message on it.
        errno = EALREADY;
        return -1;
    }
    
    // Saves the message in the channel buffer.
    memcpy(channels_table[chanid].data, buffer, len);
    channels_table[chanid].len = len;
    channels_table[chanid].sender = sender;
    
    // TODO Set sender registers !
    if (channels_table[chanid].receiver >= 0) {
        // Someone is waiting for receiving.
        pid_t receiver = channels_table[chanid].receiver;
        // Sets the return values in the registers of receiving process
        s->processes[receiver].saved_context.regs.eax = sender;
        // Unblocks receiver
        s->processes[receiver].state.state = RUNNABLE;
    }
    s->processes[sender].state.state = BLOCKEDWRITING;
    return len;
}

ssize_t receive(state *s) {
    // Receive the message contained in channel chanid, and unblocks the sender.
    // Returns the size of the read message.
    // If there isn't any message, returns immediately with value -1 and sets errno.
    int chanid = s->ctx->regs.ebx;
    u8 *buffer = (u8*) s->ctx->regs.ecx;
    size_t len = s->ctx->regs.esi;
    pid_t receiver = s->curr_pid;
    if (check_channel(chanid) == -1)
        return -1;
    
    pid_t sender = channels_table[chanid].sender;
    if (sender == -1) {
        errno = EEMPTY;
        return -1;
    }
    
    if (channels_table[chanid].receiver != receiver) {
        if (channels_table[chanid].receiver >= 0) {
            errno = EOCCUPIED;
            return 1;
        }
        else
            channels_table[chanid].receiver = receiver;
    }
    size_t msg_len = channels_table[chanid].len;
    if (msg_len > len) {
        errno = EMSGSIZE; // Not enough space in the buffer
        return -1;
    }
    
    // Reads the message and then unblocks the sender.
    memcpy(buffer, channels_table[chanid].data, msg_len);
    s->processes[sender].state.state = RUNNABLE;
    
    // The message was read, both sender and receivers are unblocked.
    // Reset of the channel.
    channels_table[chanid].receiver = -1;
    channels_table[chanid].sender = -1;
    channels_table[chanid].len = 0;
    return msg_len;
}

pid_t wait_channel(state *s) {
    // Waits for a message on channel chanid. When a message arrives, it returns
    // without reading it. To get the content, it uses receive syscall.
    // Return the pid of the sender.
    int chanid = s->ctx->regs.ebx;
    pid_t receiver = s->curr_pid;
    
    if (check_channel(chanid) == -1) 
        return -1;
    
    if (channels_table[chanid].receiver >= 0) {
        errno = EOCCUPIED;
        return -1;
    }
    channels_table[chanid].receiver = receiver;
    pid_t sender = channels_table[chanid].sender;
    
    
    if (sender == -1) {
        // Blocks the receiver.
        s->processes[receiver].state.state = BLOCKEDREADING;
    }
    errno = 0;
    return sender;
}