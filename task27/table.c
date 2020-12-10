#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#define BUF_SIZE 256
void close_pipe(FILE *pipe)
{
    int status = pclose(pipe);
    if (status == -1) {
        perror("pclose");
        exit(EXIT_FAILURE);
    } else if (WEXITSTATUS(status) != 0) {
        perror("child process returned bad exit status");
    } else if (WIFSIGNALED(status) != 0) {
        perror("child process interrupted by signal");
    }
}

int main(int argc, char *argv[]) {

    FILE *pipe = popen("grep ^$ ./text | wc -l ", "r");

    if (pipe == NULL) {
        perror("popen");
        return EXIT_FAILURE;
    }

    char buf[BUF_SIZE];

    char*  str_number = fgets(buf, BUF_SIZE, pipe);
    if(str_number == NULL && !feof(pipe)) {
        perror("fgets");
        return EXIT_FAILURE;
    }


    close_pipe(pipe);
    char* end_ptr;
    printf("count of blank lines: %u\n", (unsigned)strtol(str_number, &end_ptr, 10));
    return EXIT_SUCCESS;
}
