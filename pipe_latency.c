#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <stdlib.h>

#define NUM_MSG_SIZES 10
#define SAMPLES 100000

int write_properly(int fd, const void *buf, size_t count) {

    int bytes_left = count, bytes_written = 0;
    while (bytes_left > 0) {
//        printf("[%d] writing...\n", getpid());
        bytes_written = write(fd, buf + count - bytes_left, bytes_left);
        if (bytes_written == -1) {
            printf("Error writing to pipe!\n");
            return -1;
        }
        bytes_left -= bytes_written;
//        printf("[%d][w] bytes left: %d\n", getpid(), bytes_left);
    }
    return 0;
}

int read_properly(int fd, void *buf, size_t count) {

    int bytes_left = count, bytes_read = 0;
    while (bytes_left > 0) {
//        printf("[%d] reading...\n", getpid());
        bytes_read = read(fd, buf + count - bytes_left, bytes_left);
        if (bytes_read == -1) {
            printf("Error writing to pipe!\n");
            return -1;
        }
        bytes_left -= bytes_read;
//        printf("[%d][r] bytes left: %d\n", getpid(), bytes_left);
    }
//    printf("[%d][r] after while\n", getpid());
    return 0;
}

// Send message to child and receive the same message back and time it
int parent(int socket, char message[], int size) {

//    char *read_buf = malloc(size * sizeof(char));
    char read_buf[size * sizeof(char)];

    int bytes_read, bytes_written;
    uint64_t diff, cycles_low, cycles_low1, cycles_high, cycles_high1;

    // ------- start time ---------
    asm volatile ("CPUID\n\t"
    "RDTSC\n\t"
    "mov %%rdx, %0\n\t"
    "mov %%rax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
    "%rax", "%rbx", "%rcx", "%rdx");

    bytes_written = write_properly(socket, message, size);
    if (bytes_written == -1) {
        printf("Error!\n");
        exit(1);
    }

    bytes_read = read_properly(socket, read_buf, size * sizeof(char));
    if (bytes_read == -1) {
        printf("Error!\n");
        exit(1);
    }

    // -------- end time --------
    asm volatile ("RDTSCP\n\t"
                "mov %%rdx, %0\n\t"
                "mov %%rax, %1\n\t"
                "CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
                "%rax", "%rbx", "%rcx", "%rdx");

    diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );
    return diff;
}

// Simply receive message from parent and send it back
int child(int socket, char *buf, int size) {
    int bytes_read = read_properly(socket, buf, size * sizeof(char));
    if (bytes_read == -1) {
        printf("Error!\n");
        exit(1);
    }
    //printf("read returned: %d\n", n);
    int bytes_written = write_properly(socket, buf, size * sizeof(char));
    if (bytes_written == -1) {
        printf("Error!\n");
        exit(1);
    }
}

int main(void) {
    printf("\n\n============== pipe latency measurement ===========\n");
    int i, j;

    int msg_size[] = {4, 16, 64, 256, 1*1024, 4*1024, 16*1024, 64*1024, 256*1024, 512*1024};
    char msg[msg_size[NUM_MSG_SIZES-1]];
    char buf[msg_size[NUM_MSG_SIZES-1]];

    int fd[2], one = 1;
    static const int parentsocket = 0;
    static const int childsocket = 1;
    pid_t pid;
    uint64_t min_diff;

    socketpair(PF_LOCAL, SOCK_STREAM, 0, fd);
    setsockopt(fd[0], SOL_TCP, TCP_NODELAY, &one, sizeof(one));

    pid = fork();

    // Test for each sample size
    for (j = 0; j < NUM_MSG_SIZES; j++) {

        if (pid != 0) {
            // printf("Test for size = %d bytes\n", msg_size[j]);
            min_diff = 0;
        }

        // Get lots of times for a single message size
        for (i = 0; i < SAMPLES; i++) {
            if (pid == 0) { /* you are the child */
                close(fd[parentsocket]); /* Close the parent file descriptor */
                child(fd[childsocket], buf, msg_size[j]);
            } else { /* you are the parent */
                close(fd[childsocket]); /* Close the child file descriptor */
                int diff = parent(fd[parentsocket], msg, msg_size[j]);

                if (min_diff == 0 || min_diff > diff) {
                    min_diff = diff;
                }
            }
        }

        if (pid != 0) {
            printf("Latency for packet size %d bytes = %.3f us\n", msg_size[j], min_diff / (3.2 * 1000));
        }
    }

    return 0;
}
