#include "error.h"

stream_t *stderr = NULL;
u8 stderr_buffer[STREAM_BUFFER_SIZE];

void init_error_msg() {
    // Fills the array error_msg
    error_msg[0] = "No error";
    error_msg[1] = "Argument list too long";
    error_msg[2] = "Permission denied";
    error_msg[3] = "Resource temporarily unavailable; try again";
    error_msg[4] = "Bad file descriptor";
    error_msg[5] = "Resource unavailable";
    error_msg[6] = "No child process";
    error_msg[7] = "Resource deadlock would occur";
    error_msg[8] = "Domain error for math functions, etc.";
    error_msg[9] = "File exists";
    error_msg[10] = "Bad address";
    error_msg[11] = "File too large";
    error_msg[12] = "Function interrupted by signal";
    error_msg[13] = "Invalid argument";
    error_msg[14] = "Hardware I/O error";
    error_msg[15] = "Is a directory";
    error_msg[16] = "Too many open files by the process";
    error_msg[17] = "Too many links";
    error_msg[18] = "Filename too long";
    error_msg[19] = "Too many open files in the system";
    error_msg[20] = "No such device";
    error_msg[21] = "No such file or directory";
    error_msg[22] = "Not an executable file";
    error_msg[23] = "No locks available";
    error_msg[24] = "Not enough memory";
    error_msg[25] = "No space left on device";
    error_msg[26] = "Function not supported";
    error_msg[27] = "Not a directory";
    error_msg[28] = "Directory not empty";
    error_msg[29] = "Inappropriate I/O control operation";
    error_msg[30] = "No such device or address";
    error_msg[31] = "Operation not permitted";
    error_msg[32] = "Broken pipe";
    error_msg[33] = "Result too large";
    error_msg[34] = "Read-only file system";
    error_msg[35] = "Invalid seek e.g. on a pipe";
    error_msg[36] = "No such process";
    error_msg[37] = "Invalid link";
    error_msg[38] = "Operation would block";
    error_msg[39] = "Operation now in progress";
    error_msg[40] = "Operation already in progress";
    error_msg[41] = "Socket operation on non-socket";
    error_msg[42] = "Destination address required";
    error_msg[43] = "Message too long";
    error_msg[44] = "Protocol wrong type for socket";
    error_msg[45] = "Protocol not available";
    error_msg[46] = "Protocol not supported";
    error_msg[47] = "Socket type not supported";
    error_msg[48] = "Operation not supported on socket";
    error_msg[49] = "Protocol family not supported";
    error_msg[50] = "Address family not supported by protocol family";
    error_msg[51] = "Address already in use";
    error_msg[52] = "Can't assign requested address";
    error_msg[53] = "Network is down";
    error_msg[54] = "Network is unreachable";
    error_msg[55] = "Network dropped connection on reset";
    error_msg[56] = "Software caused connection abort";
    error_msg[57] = "Connection reset by peer";
    error_msg[58] = "No buffer space available";
    error_msg[59] = "Socket is already connected";
    error_msg[60] = "Socket is not connected";
    error_msg[61] = "Can't send after socket shutdown";
    error_msg[62] = "Too many references: can't splice";
    error_msg[63] = "Connection timed out";
    error_msg[64] = "Connection refused";
    error_msg[65] = "Host is down";
    error_msg[66] = "No route to host";
    error_msg[67] = "Too many levels of symbolic links";
    error_msg[68] = "File size or position not representable";
    error_msg[69] = "Corrupted or not consistant file";
    error_msg[70] = "Unknown error";
}

int init_stderr(char *path) { // TODO Really place this here ?
    // Creates the error stream writing at specified location.
    // If path is NULL, chooses default path.
    init_error_msg();
    if (path == NULL) {
        u8 mode = 0;
        
        if (mkdir(DEFAULT_STDERR_DIR, mode) == -1) {
            // Failed to create such directory. (maybe handle with finddir ?)
            if (errno != EEXIST) {
                // There wasn't already such a directory -> Nope !
                return -1;
            }
        }
        stderr = create_stream(DEFAULT_STDERR_PATH, stderr_buffer);
        return (stderr == NULL) ? -1 : 0;
    }
    else {
        stderr = create_stream(path, stderr_buffer);
        return (stderr == NULL) ? -1 : 0;
    }
}

void perror(char *data) {
    // Prints data followed by the error message corresponding to errno.
    if (data != NULL && strlen(data) > 0) {
        fprintf(stderr, "%s: %s\n", data, strerror(errno));
    }
}

char *static_unknown_error = "Unknown error";
char *strerror(error_t errnum) {
    // Returns the message ecorresponding to error number errnum.
    // The returned string must not be modified.
    if (errnum < NB_ERR)
        return error_msg[errnum];
    
    return static_unknown_error;
}
