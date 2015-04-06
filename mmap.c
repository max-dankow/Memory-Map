#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void get_input(int argc, char** argv, char** file_name, 
               int* col, int* row)
{
    if (argc < 3)
    {
        fprintf(stderr, "Wrong parameters.\n");
        _exit(EXIT_FAILURE);
    }
    
    *file_name = argv[1];
    char* str_err;
    *col = strtol(argv[2], &str_err, 10);

    if (strcmp(str_err, "") != 0)
    {
        fprintf(stderr, "Wrong paraneter col.\n");
        _exit(EXIT_FAILURE);
    }

    *row = 0;

    //если указано число строк
    if (argc == 4)
    {
        char* str_err;
        *row = strtol(argv[3], &str_err, 10);

        if (strcmp(str_err, "") != 0)
        {
            fprintf(stderr, "Wrong paraneter row.\n");
            _exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Wrong parameters.\n");
        return 1;
    }

    int fd = open("buf.bin",
                  O_RDWR | O_CREAT,
                  S_IRWXU);

    int col = 0, row = 0;
    char* file_name;

    get_input(argc, argv, &file_name, &col, &row);
    printf("name:%s, col == %d, row == %d\n", file_name, col, row);

    return 0;
}
