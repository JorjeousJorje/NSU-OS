#include <stdlib.h> // exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>  // popen(), pclose(), fread(), fwrite(), stdout
#include <ctype.h>  // toupper()

#define BUFFER_SIZE 256

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



int main(int argc, char *argv[])
{
    // Opening pipe and execute command echo
    FILE *pipe = popen("echo Hello, another process!", "r");
    if (pipe == NULL)
    {
        perror("popen");
        return EXIT_FAILURE;
    }

    // Reading from stdout of writing process
    char buf[BUFFER_SIZE];
    size_t read_count = fread(buf, 1, BUFFER_SIZE, pipe);
    if (ferror(pipe)  == -1) {
        perror("fread");
        close_pipe(pipe);
        return EXIT_FAILURE;
    }
    //close pipe and wait for children to finally exit
    close_pipe(pipe);

    // Converting to upper, writing into stdout
    size_t i;
    for (i = 0; i < read_count; i++) {
        buf[i] = (char) toupper(buf[i]);
    }
    size_t written_count = fwrite(buf, read_count, 1, stdout);
    if(ferror(pipe)  == -1) {
        perror("fwrite");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
