#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "sql.h"

typedef struct test_row_t {
    int col_count;
    value_t *values;
    struct test_row_t *next;
} test_row_t;

typedef struct {
    char* sql;
    test_row_t *expected;
} test_t;

static int test_init(test_t *test) {
    test->sql = NULL;
    test->expected = NULL;
    return 0;
}

static int test_add_row(test_t *test, test_row_t *row) {
    test_row_t *tail = test->expected;
    if (tail == NULL) {
        test->expected = row;
    } else {
        while(tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = row;
    }
    row->next = NULL;

    return 0;
}

static int test_destroy(test_t *test) {
    if (test) {
        printf("Freeing sql\n");
        free(test->sql);
        test->sql = NULL;
        test_row_t* row = test->expected;
        while(row != NULL) {
            printf("Freeing row\n");
            test_row_t* next = row->next;
            free(row);
            row = next;
        }
        test->expected = NULL;
    }
    return 0;
}

typedef struct {
    char* data;
} linereader_t;

static int reader_init_from_file(linereader_t *reader, char *file_name) {
    struct stat stat_buf;
    stat(file_name, &stat_buf);

    size_t script_length = (size_t)stat_buf.st_size;
    char *script = malloc(script_length);
    
    FILE *f = fopen(file_name, "r");
    size_t to_read = script_length;
    while(to_read > 0) {
        size_t read = fread(script, 1, to_read, f);
        if (read > 0) {
            to_read -= read;
        } else {
            return -1;
        }
    }

    reader->data = script;
    return 0;
}

static int reader_destroy(linereader_t *reader) {
    if (reader) {
        free(reader->data);
        reader->data = NULL;
    }
    return 0;
}

static int reader_next_line(linereader_t *reader, char** line) {
    *line = strtok(reader->data, "\n");
    if (*line != NULL) {
        size_t len = strlen(*line);
        reader->data += len + 1;
        if (len > 0) {
            if (*(reader->data - 1) == '\r') {
                *(reader->data - 1) = 0;
            }
        }
    }

    return *line != NULL ? 0 : 1;
}

static test_row_t *parse_row(char *line, size_t col_count) {
    void *data = malloc(sizeof(test_row_t) + col_count * sizeof(value_t));
    test_row_t *row = data;
    row->values = data + sizeof(test_row_t);
    memset(row->values, 0, col_count * sizeof(value_t));
    row->col_count = col_count;

    printf("'%s'\n", line);

    return row;
}

static int run_script(char *script_name) {
    linereader_t reader;
    reader_init_from_file(&reader, script_name);

    test_t test;
    do {
        test_init(&test);

        reader_next_line(&reader, &test.sql);

        char *col_count_text;
        reader_next_line(&reader, &col_count_text);
        size_t col_count = (size_t)atoi(col_count_text);

        char *result_line;
        while(1) {
            reader_next_line(&reader, &result_line);
            if (result_line == NULL || strlen(result_line) == 0) {
                break;
            } else {
                test_row_t *row = parse_row(result_line, col_count);
                test_add_row(&test, row);
            }
        }

        test_destroy(&test);
    } while(test.sql != NULL);

    reader_destroy(&reader);

    return 0;
}

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        run_script(argv[i]);
    }
    
    return 0;
}