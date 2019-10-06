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
void parent(int socket, char message[], int size) {

    char *read_buf = malloc(size * sizeof(char));

    int bytes_read, bytes_written;

    int i;
    for (i = 0; i < SAMPLES; i++) {
//        printf("parent sends sample %d\n", i);
        message[0] = 'N';
        bytes_written = write_properly(socket, message, size * sizeof(char));
        if (bytes_written == -1) {
            printf("Error!\n");
            exit(1);
        }
    }

//    printf("parent waiting for ack\n");
    bytes_read = read_properly(socket, read_buf, 1);
//    printf("parent receives ack\n");
    if (bytes_read == -1) {
        printf("Error!\n");
        exit(1);
    }
}

// Simply receive message from parent and send it back
void child(int socket, char *buf, int size) {
    
    int bytes_read, bytes_written;
    int i;
    for (i = 0; i < SAMPLES; i++) {
//        printf("child waiting for sample %d\n", i);
        bytes_read = read_properly(socket, buf, size);
//        printf("child receives sample %d with value %c\n", i, buf[0]);
        if (bytes_read == -1) {
            printf("Error!\n");
            exit(1);
        }
    }

//    printf("child sends ack\n");
    bytes_written = write_properly(socket, buf, 1);
    if (bytes_written == -1) {
        printf("Error!\n");
        exit(1);
    }
    //printf("read returned: %d\n", n);
}

int main(void) {
    printf("\n\n============== pipe throughput measurement ===========\n");
    int i, j;

    char *msg[NUM_MSG_SIZES];
    char *buf[NUM_MSG_SIZES];
    int msg_size[] = {4, 16, 64, 256, 1*1024, 4*1024, 16*1024, 64*1024, 256*1024, 512*1024};
    
    for (i = 0; i < NUM_MSG_SIZES; i++) {
        msg[i] = malloc(msg_size[i] * sizeof(char));
        if (msg[i] == 0) {
            printf("Error allocating memory of size %d for msg[%d]!\n", msg_size[i], i);
            exit(1);
        }
    }
    for (i = 0; i < NUM_MSG_SIZES; i++) {
        buf[i] = malloc(msg_size[i] * sizeof(char));
        if (buf[i] == 0) {
            printf("Error allocating memory of size %d for buf[%d]!\n", msg_size[i], i);
            exit(1);
        }
    }

    int fd[2], one = 1;
    static const int parentsocket = 0;
    static const int childsocket = 1;
    pid_t pid;
    uint64_t diff, cycles_low, cycles_low1, cycles_high, cycles_high1;
    long double throughput;

    socketpair(PF_LOCAL, SOCK_STREAM, 0, fd);
    setsockopt(fd[0], SOL_TCP, TCP_NODELAY, &one, sizeof(one));

    pid = fork();

    // Test for each sample size
    for (j = 0; j < NUM_MSG_SIZES; j++) {

        if (pid != 0) {
            // printf("Test for size = %d bytes\n", msg_size[j]);
            // ------- start time ---------
            asm volatile ("CPUID\n\t"
                        "RDTSC\n\t"
                        "mov %%rdx, %0\n\t"
                        "mov %%rax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
                        "%rax", "%rbx", "%rcx", "%rdx");
        }

        if (pid == 0) { /* you are the child */
            close(fd[parentsocket]); /* Close the parent file descriptor */
            child(fd[childsocket], buf[j], msg_size[j]);
        } else { /* you are the parent */
            close(fd[childsocket]); /* Close the child file descriptor */
            parent(fd[parentsocket], msg[j], msg_size[j]);
        }

        if (pid != 0) {
            // -------- end time --------
            asm volatile ("RDTSCP\n\t"
                        "mov %%rdx, %0\n\t"
                        "mov %%rax, %1\n\t"
                        "CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
                        "%rax", "%rbx", "%rcx", "%rdx");
            // Calculate number of cycles
            diff = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 ) - ( ((uint64_t)cycles_high << 32) | cycles_low );

            // Calculate throughput
            throughput = (((uint64_t)msg_size[j]) * SAMPLES * 3.2 * 1000) / diff;
            // printf("cycles = %lu, throughput = %Lf Gbytes per sec\n", diff, throughput);
            printf("Throughput for packet size %d bytes = %.3Lf MBps\n", msg_size[j], throughput);
        }
    }

    return 0;
}
