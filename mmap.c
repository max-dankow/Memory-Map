#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

void get_input(int argc, char** argv, char** file_name, 
               int* col, int* row)
{
    if (argc < 3)
    {
        fprintf(stderr, "Wrong parameters.\n");
        _exit(EXIT_FAILURE);
    }
    
    //читаем имя файла
    *file_name = argv[1];

    //читаем число столбцов
    char* str_err;
    *col = strtol(argv[2], &str_err, 10);

    if (strcmp(str_err, "") != 0)
    {
        fprintf(stderr, "Wrong paraneter col.\n");
        _exit(EXIT_FAILURE);
    }

    *row = -1;

    //если указано число строк, то читаем
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
    int col = 0, row = 0;
    int new_file_flag = 0;
    int file_size;
    char* file_name;

    get_input(argc, argv, &file_name, &col, &row);

    int matrix_fd = open(file_name, O_RDWR);

    if (matrix_fd == -1)
    {
         int matrix_fd = open(file_name,
                              O_RDWR | O_CREAT,
                              S_IRWXU);

         if (matrix_fd == -1) 
         {
             perror("Can't open file.\n");
             return 1;
         }

         new_file_flag = 1;
    }

    //если не указано число строк, то определим его
    if (row == -1)
    {
        if (new_file_flag == 1)
        {
            //если файла не существовало
            fprintf(stderr, "Can't calculate matrix size.\n");
            return 1;
        }
        else
        {
            //определим количество строк по размеру файла
            struct stat info;

            if (fstat(matrix_fd, &info) == -1)
            {
                perror("Can't get file size.\n");
                return 1;
            }

            file_size = info.st_size;
            row = file_size / (sizeof(double) * col);
            printf("row == %d\n", row);
        }    
    }
    else
    {
        //если задано, то изменим размер файла
        file_size = col * row * sizeof(double); 
        ftruncate(matrix_fd, file_size);
        printf("New size is: %d\n", file_size);
    }

    void* shared_mem = mmap(NULL, file_size, 
                            PROT_READ | PROT_WRITE, 
                            MAP_SHARED, matrix_fd, 0);

    if (shared_mem == MAP_FAILED)
    {
        perror("mmap error\n");
    }

    close(matrix_fd);
    char* ch = (char*) shared_mem;
    *ch = 'A';
    ch[1] = '\n';

    if (munmap(shared_mem, file_size) == -1)
    {
        perror("Faild to unmap\n");
    }

    return 0;
}
