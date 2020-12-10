#include <stdio.h>     // printf()
#include <string.h>    // strlen()
#include <stdlib.h>    // EXIT_SUCCESS, EXIT_FAILURE
#include <sys/types.h> // pid_t
#include <ctype.h>     // toupper()
#include <unistd.h>    // fork(), write(), read(), pipe()
#include <errno.h>

#define BUF_SIZE 100

int write_process(int pipe_fd[2]) {
    if (close(pipe_fd[0]) == -1) {
        fprintf(stderr, "close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    const char* message = "Hello, another process!";
    if (write(pipe_fd[1], message, strlen(message)) == -1) {
        fprintf(stderr, "write() error in %d:\n", getpid());
        if (close(pipe_fd[1]) == -1) {
            fprintf(stderr, "close() error in %d:\n", getpid());
        }
        return EXIT_FAILURE;
    }
    if (close(pipe_fd[1]) == -1) {
        fprintf(stderr, "close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int read_process(int pipe_fd[2]) {
    if (close(pipe_fd[1]) == -1) {
        fprintf(stderr, "close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    char buf[BUF_SIZE];
    ssize_t read_count = read(pipe_fd[0], buf, BUF_SIZE);
    if (read_count == -1 && errno != EINTR) {
        fprintf(stderr, "read() error in %d:\n", getpid());
        if (close(pipe_fd[0]) == -1) {
            fprintf(stderr, "close() error in %d:\n", getpid());
        }
        return EXIT_FAILURE;
    }

    size_t i;
    for (i = 0; i < read_count; i++) {
        buf[i] = (char) toupper(buf[i]);
    }
    char read_string[read_count + 1];
    strncpy(read_string, buf, read_count);
    read_string[read_count] = '\0';
    printf("Upper formatted text: %s\n", read_string);

    if (close(pipe_fd[0]) == -1) {
        fprintf(stderr, "close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int close_pipe(int pipe_fd[2]) {
    if (close(pipe_fd[0]) == -1) {
        fprintf(stderr, "close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    if (close(pipe_fd[1]) == -1) {
        fprintf(stderr, "close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int wait_for_pend(pid_t pid, int status){
    do {
        pid_t terminated_child_pid = waitpid(pid, &status, 0);
        if (terminated_child_pid == -1) {
            perror("waitpid() error: ");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            printf("exited, status=%d\nchild_pid=%d\n", WEXITSTATUS(status), (int)pid);
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\nchild_pid=%d\n", WTERMSIG(status), (int)pid);
        } else if (WIFSTOPPED(status)) {
            printf("stopped by signal %d\nchild_pid=%d\n", WSTOPSIG(status), (int)pid);
        } else if (WIFCONTINUED(status)) {
            printf("continued\n");
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int status1, status2;
    int pipe_fd[2];
    char buf[BUF_SIZE];

    if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "Error at pipe\n");
        return -1;
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        fprintf(stderr, "Error at fork() 1\n");
        close_pipe(pipe_fd);
        return EXIT_FAILURE;
    }
    if (pid1 == 0) {
        return write_process(pipe_fd);
    } else {
        pid_t pid2 = fork();
        if (pid2 == -1) {
            fprintf(stderr, "fork() error in %d:\n", getpid());
            close_pipe(pipe_fd);
            wait_for_pend(pid1, status1);
            return EXIT_FAILURE;
        }
        if (pid2 == 0) {
            return read_process(pipe_fd);
        } else {
            if(wait_for_pend(pid1, status1) == EXIT_FAILURE) {
                close_pipe(pipe_fd);
                return EXIT_FAILURE;
            }
            if(wait_for_pend(pid2, status2) == EXIT_FAILURE) {
                close_pipe(pipe_fd);
                return EXIT_FAILURE;
            }
        }
    }

    close_pipe(pipe_fd);
    return EXIT_SUCCESS;
}

