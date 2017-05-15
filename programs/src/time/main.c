#include "parsing.h"
#include "lib.h"
#include "int.h"

void print_time_diff(rtc_time_t *start, rtc_time_t *end) {
    int min = 0;
    if (end->minutes - start->minutes > 0) {
        printf("%dmin ", end->minutes - start->minutes);
        min = 1;
    }
    if (end->seconds - start->seconds > 0)
        printf("%ds ", end->seconds - start->seconds);
    if (!min && (end->mseconds - start->mseconds > 0))
        printf("%dms", end->mseconds - start->mseconds);
}

int main(char *a) {
    if (!*a) {
        // Prints time and exit.
        rtc_time_t time;
        int res = gettimeofday(&time);
        if (res == -1) {
            fprintf(STDERR, "time: Couldn't get current time: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        print_time(&time);
        exit(EXIT_SUCCESS);
    }
    
    // Otherwise, executes the arg command, and measures its duration.
    rtc_time_t start;
    rtc_time_t end;
    int res = gettimeofday(&start);
    if (res == -1) {
        fprintf(STDERR, "time: Couldn't get current time: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    pid_t pid = exec("/shell.bin", a, STDIN, STDOUT);
    if (pid == -1) {
        fprintf(STDERR, "time: Couldn't exec a shell: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    wait(NULL);
    res = gettimeofday(&end);
    if (res == -1) {
        fprintf(STDERR, "time: Couldn't get current time: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("Execution time of %s: ", a);
    printf("%bg", BLUE);
    print_time_diff(&start, &end);
    printf("%pbg");
    printf("\n");
    exit(EXIT_SUCCESS);
}