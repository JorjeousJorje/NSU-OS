#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 3
#define BUFFER_SIZE 4

typedef struct {
    size_t size;
    size_t capacity;
    size_t* array;

} dynamic_array;

void* array_create()
{
    dynamic_array* array = (dynamic_array*)malloc(sizeof(dynamic_array));
    if (array == NULL) {
        return NULL;
    }

    array->capacity = INITIAL_CAPACITY;
    array->size = 0;

    array->array = (size_t*)malloc(INITIAL_CAPACITY * sizeof(size_t));
    if (array->array == NULL) {
        return NULL;
    }

    return array;
}

bool array_resize(dynamic_array* array)
{
    const size_t new_capacity = array->capacity * 2;
    size_t* new_array = (size_t*)realloc(array->array, sizeof(size_t) * new_capacity);
    if (new_array == NULL) {
        return false;
    }
    array->array = new_array;
    array->capacity = new_capacity;
    return true;
}

bool array_insert(dynamic_array* array, size_t value)
{
    if (array->size == array->capacity)
    {
        if (!array_resize(array)) {
            return false;
        }
    }
    array->array[array->size++] = value;
    return true;
}

size_t array_item(const dynamic_array* array, size_t index)
{
    if (array->size <= index) {
        return ~(size_t)0;
    }
    return array->array[index];
}

void array_destroy(dynamic_array* array)
{
    if (array == NULL) {
        return;
    }

    dynamic_array* destroyed = array;

    if (destroyed->array != NULL) {

        free(destroyed->array);
        destroyed->array = NULL;
    }
    array->size = 0;
    array->capacity = 0;
    free(destroyed);
    destroyed = NULL;
}


bool fill_table(dynamic_array* table, int fd)
{
    char buffer[BUFFER_SIZE + 1];
    size_t current_pos = 0;
    ssize_t read_count = 0;

    while ((read_count = read(fd, buffer, BUFFER_SIZE)) != 0) {
        if (read_count == -1 && errno != EINTR) {
            perror("read() error: ");
            return false;
        }
        buffer[read_count] = '\0';//strchr needs to understand end of buffer
        char* n_pos = buffer;

        while ((n_pos = strchr(n_pos, '\n')) != NULL) {
            const int end_line_pos = n_pos - buffer;
            if (!array_insert(table, current_pos + end_line_pos)) {
                return false;
            }
            ++n_pos; // skip detected '\n'
        }
        current_pos += read_count;
    }
    return true;
}

char* allocate_string(const size_t len)
{
    char* line = (char*)calloc(len + 1, sizeof(char));

    if (!line) {
        return NULL;
    }
    return line;
}

int print_line(int file_des, dynamic_array* table, const size_t line_number)
{
    const size_t pos = line_number == 0 ? 0 : array_item(table, line_number - 1) + 1;

    const size_t len = table->array[line_number] - pos + 1;
    char* line = allocate_string(len);
    if(!line) {
        return -1;
    }

    lseek(file_des, pos, SEEK_SET); //set pos to desired '\n'

    ssize_t read_count = read(file_des, line, len);
    if (read_count == -1) {
        perror("read() error:");
        return -1;
    }

    printf("%s\n", line);
    free(line);
    return 0;
}

void scan_line(unsigned int* value, size_t table_size)
{
    char* p, s[100];

    while (fgets(s, sizeof(s), stdin)) {
	*value = strtol(s, &p, 10);
        if (p == s || *p != '\n') {
            printf("Please, enter an integer: ");
        }
        else {
            if(*value > table_size) {
                printf("Incorrect line number. Try again: ");
		continue;
            }
            break;
        }
    }
}

int is_instream_empty(const long interval, const int fd, fd_set* set, struct timeval* tv)
{
    FD_ZERO(set); // clear fd_set
    FD_SET(fd, set); // set fd in fd_set to stdin
    // FD_ISSET
    tv->tv_sec = interval;
    tv->tv_usec = 0;

    int max_fd = fd;
    int number_of_fd = select(max_fd + 1, set, NULL, NULL, tv);
    if(FD_ISSET(fd, set) == 0) {
	return -2;
    }
    if(number_of_fd == -1) {  //wait until person input smth in stand-in
        return -1;
    }
    return number_of_fd;
}


int main(int argc, char** argv)
{
    fd_set set;
    struct timeval tv;

    dynamic_array* table = (dynamic_array*)array_create();

    const int fd = open("text.txt", O_RDONLY);
    if (fd == -1) {
        perror("open() error: ");
        return EXIT_FAILURE;
    }

    if(!fill_table(table, fd)) {
	printf("error to fill the table\n");
	close(fd);
        array_destroy(table);
        return EXIT_FAILURE;
    }

    printf("Program will be closed in 5 second. Input line number or 0 to exit:\n");

    const int is_empty = is_instream_empty(5, 0, &set, &tv);
    if (is_empty == -2) {
	perror("fd is not set : ");
	close(fd);
	array_destroy(table);
	return EXIT_FAILURE;
    }
    if (is_empty == -1) {
        perror("select() error : ");
        close(fd);
        array_destroy(table);
        return EXIT_FAILURE;
    }

    size_t line_num = 0;
    if(is_empty) {
        while (true) {
            scan_line(&line_num, table->size);
	    if(line_num == 0) {
		break;
	    }

            printf("Line number is: %zu\n", line_num);
            if (print_line(fd, table, line_num - 1) == -1) {
                close(fd);
                array_destroy(table);
                return EXIT_FAILURE;
            }
        }
    }
    else {
        printf("Time is out\n");
    }
    close(fd);
    array_destroy(table);
    return EXIT_SUCCESS;
}

