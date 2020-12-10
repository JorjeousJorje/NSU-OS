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
    // status for waitpid()
    int status;
    // fork - create a child process
    pid_t child_pid = fork();
    // On failure, -1 is returned in the parent, no child process is created,
    // and errno is set appropriately.
    if (child_pid == -1) {
        perror("fork() error: ");
        return EXIT_FAILURE;
    }
    // On  success, the PID of the child process is returned in the parent, and 0 is returned in the child.
    // Here is success, child is created!
    // This process is child now!
    if (child_pid == 0) {
	// "/bin/cat" path to the filename associated with the file being executed
	//  path + args... (in our case it is "cat") "cat" will be executed by execl.
        execl("/bin/cat", "cat", argv[1], (char*)NULL);
        return EXIT_SUCCESS;
    }
    else { // Here is code executed by parent code
        do {
            // The waitpid() system call suspends execution of the calling process until one of
            // its children terminates or stopped by signal
            pid_t terminated_child_pid = waitpid(child_pid, &status, 0);
            if (terminated_child_pid == -1) {
                perror("waitpid() error: ");
                return EXIT_FAILURE;
            }

            if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return EXIT_SUCCESS;
}
