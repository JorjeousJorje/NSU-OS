#include <stdio.h>     // printf()
#include <string.h>    // strlen()
#include <stdlib.h>    // EXIT_SUCCESS, EXIT_FAILURE
#include <sys/types.h> // pid_t
#include <ctype.h>     // toupper()
#include <unistd.h>    // fork(), write(), read(), pipe()
#include <errno.h>

#define BUF_SIZE 100
#define PROC_FAILURE -2
#define PROC_SUCCESS 2

int write_process(int pipe_fd[2])
{
    if (close(pipe_fd[0]) == -1) {
        fprintf(stderr, "write close() error in %d:\n", getpid());
        return PROC_FAILURE;
    }
    const char* message = "Hello, another process!";
    if (write(pipe_fd[1], message, strlen(message)) == -1) {
        fprintf(stderr, "write() error in %d:\n", getpid());
        if (close(pipe_fd[1]) == -1) {
            fprintf(stderr, "write close() error in %d:\n", getpid());
        }
        return PROC_FAILURE;
    }
    if (close(pipe_fd[1]) == -1) {
        fprintf(stderr, "write close() error in %d:\n", getpid());
        return PROC_FAILURE;
    }
    return PROC_SUCCESS;
}

int read_process(int pipe_fd[2])
{
    if (close(pipe_fd[1]) == -1) {
        fprintf(stderr, "read close() error in %d:\n", getpid());
        return PROC_FAILURE;
    }
    char buf[BUF_SIZE];
    ssize_t read_count = read(pipe_fd[0], buf, BUF_SIZE);
    if (read_count == -1 && errno != EINTR) {
        fprintf(stderr, "read() error in %d:\n", getpid());
        if (close(pipe_fd[0]) == -1) {
            fprintf(stderr, "read close() error in %d:\n", getpid());
        }
        return PROC_FAILURE;
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
        fprintf(stderr, "read close() error in %d:\n", getpid());
        return PROC_FAILURE;
    }
    return PROC_SUCCESS;
}

int close_pipe(int pipe_fd[2])
{
    if (close(pipe_fd[0]) == -1) {
        fprintf(stderr, "main close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    if (close(pipe_fd[1]) == -1) {
        fprintf(stderr, "main close() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int wait_for_pend(pid_t pid, int status)
{
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


int create_process(int pipe_fd[2], int (*process_event)(int pipe_fd[2]))
{
    int status = 0;
    pid_t pid = fork();
    if(pid == -1) {
        fprintf(stderr, "fork() error in %d:\n", getpid());
        close_pipe(pipe_fd);
        return EXIT_FAILURE;
    }
    if(pid == 0) {
        return (*process_event)(pipe_fd);
    } else {
        if(wait_for_pend(pid, status) == EXIT_FAILURE) {
            close_pipe(pipe_fd);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}



int main(int argc, char *argv[]) {
    int pipe_fd[2];
    int status = 0;

    if (pipe(pipe_fd) == -1) {
        fprintf(stderr, "pipe() error in %d:\n", getpid());
        return EXIT_FAILURE;
    }

    int post_code = create_process(pipe_fd, &write_process);
    if(post_code == PROC_FAILURE || post_code == EXIT_FAILURE) {
        return EXIT_FAILURE;
    } else if (post_code == PROC_SUCCESS) {
        return PROC_SUCCESS;
    }

    post_code = create_process(pipe_fd, &read_process);
    if(post_code == PROC_FAILURE || post_code == EXIT_FAILURE) {
        return EXIT_FAILURE;
    } else if (post_code == PROC_SUCCESS) {
        return PROC_SUCCESS;
    }

    close_pipe(pipe_fd);
    return EXIT_SUCCESS;
}
