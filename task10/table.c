#include <stdio.h>     // srderr
#include <errno.h>     // EINVAL
#include <stdlib.h>    // EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>    // fork(), execl()
#include <wait.h>      // wait()
#include <sys/types.h> // pid_t


int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [executable] <args> error\n", argv[0]);
        return EINVAL;
    }

    int status;
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork() error: ");
        return EXIT_FAILURE;
    }
    if (child_pid == 0) {
        execvp(argv[1], &argv[1]);
        return EXIT_SUCCESS;
    }
    else {
        do {
            pid_t terminated_child_pid = waitpid(child_pid, &status, 0);
            if (terminated_child_pid == -1) {
                perror("waitpid() error: ");
                return EXIT_FAILURE;
            }

            if (WIFEXITED(status)) {
                printf("exited, status=%d\nchild_pid=%d\n", WEXITSTATUS(status), (int)child_pid);
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\nchild_pid=%d\n", WTERMSIG(status), (int)child_pid);
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\nchild_pid=%d\n", WSTOPSIG(status), (int)child_pid);
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return EXIT_SUCCESS;
}
