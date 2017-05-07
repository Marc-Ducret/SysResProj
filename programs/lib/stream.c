#include "stream.h"

stream_t stream_table[NUM_STREAMS];
u8 static_buffers[NUM_STREAMS * STREAM_BUFFER_SIZE];

int check_sid(sid_t sid) {
    if (sid < 0 || sid >= NUM_STREAMS || stream_table[sid].status == S_UNUSED) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

sid_t create_stream(char *path) {
    // Creates a stream writing at specified file location.
    sid_t sid = NUM_CHANNELS_PROC;
    while (sid < NUM_CHANNELS_PROC && stream_table[sid].status != S_UNUSED) {
        sid++;
    }
    if (sid == NUM_CHANNELS_PROC) {
        errno = EMSTREAM;
        return -1;
    }
    oflags_t flags = O_WRONLY | O_CREAT | O_APPEND;
    fd_t file = fopen(path, flags);
    if (file < 0) {
        // Error while opening file.
        return -1;
    }
    //u8 *buffer = (u8*) malloc(STREAM_BUFFER_SIZE); TODO
    u8 *buffer = &(static_buffers[sid * STREAM_BUFFER_SIZE]);
    if (buffer == NULL) {
        close(file);
        return -1;
    }
    stream_t *stream = &(stream_table[sid]);

    stream->status = S_FILE;
    stream->buffer = buffer;
    stream->file = file;
    stream->size = STREAM_BUFFER_SIZE;
    stream->index = 0;    
    return sid;
}

sid_t create_channel_stream(int chanid) {
    // Creates a stream writing at specified channel.
    sid_t sid = chanid;
    
    //u8 *buffer = (u8*) malloc(STREAM_BUFFER_SIZE); TODO 
    u8 *buffer = &(static_buffers[sid * STREAM_BUFFER_SIZE]);

    if (buffer == NULL) {
        return -1;
    }
    stream_t *stream = &(stream_table[sid]);

    stream->status = S_CHANNEL;
    stream->buffer = buffer;
    stream->chanid = chanid;
    stream->size = STREAM_BUFFER_SIZE;
    stream->index = 0;
    return sid;
}

int stream_putchar(char c, sid_t sid) {
    if (check_sid(sid) == -1)
        return -1;
    
    stream_t *stream = &(stream_table[sid]);
    if (stream->index == stream->size || c == '\n') {
        int res = flush(sid);
        if (res == -1) {
            // Error while flush
            return -1;
        }
    }
    stream->buffer[stream->index] = c;
    stream->index ++;
    return 0;
}

int flush(sid_t sid) {
    // Writes content of stream buffer to the disk.
    if (check_sid(sid) == -1)
        return -1;
    
    stream_t *stream = &(stream_table[sid]);
    u32 remaining = stream->index;
    int written = 0;
    u8 *index = stream->buffer;
    int is_channel = stream->status == S_CHANNEL;
    while (remaining > 0) {
        if (is_channel) {
            written = send(stream->chanid, index, remaining);
        }
        else {
            written = write(stream->file, index, remaining);
        }
        if (written == -1) {
            // Error while writing
            return -1;
        }
        remaining -= written;
        index += written;
    }
    stream->index = 0;
    return written;
}

int close_stream(sid_t sid) {
    // Closes the stream after flushing it a last time.
    if (flush(sid) == -1) {
        // Error while flush
        return -1;
    }
    stream_t *stream = &(stream_table[sid]);
    if (stream->status == S_FILE) {
        close(stream->file);
    }
    //free(stream->buffer);
    stream->status = S_UNUSED;
    // TODO close stdout / stderr ?
    return 0;
}
