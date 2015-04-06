#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <assert.h>

typedef struct
{
    int col;
    int row;
    int new_file_flag;
    int file_size;
    int fd;
    double* shared_mem_ptr;
} Matrix;

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

void open_matrix(char* file_name, Matrix* matrix)
{
    matrix->fd = open(file_name, O_RDWR);

    if (matrix->fd == -1)
    {
         matrix->fd = open(file_name,
                              O_RDWR | O_CREAT,
                              S_IRWXU);

         if (matrix->fd == -1)
         {
             perror("Can't open file.\n");
             _exit(EXIT_FAILURE);
         }

         matrix->new_file_flag = 1;
    }

    //если не указано число строк, то определим его
    if (matrix->row == -1)
    {
        if (matrix->new_file_flag == 1)
        {
            //если файла не существовало
            fprintf(stderr, "Can't calculate matrix size.\n");
            _exit(EXIT_FAILURE);;
        }
        else
        {
            //определим количество строк по размеру файла
            struct stat info;

            if (fstat(matrix->fd, &info) == -1)
            {
                perror("Can't get file size.\n");
                _exit(EXIT_FAILURE);
            }

            matrix->file_size = info.st_size;
            matrix->row = matrix->file_size / (sizeof(double) * matrix->col);
            printf("row == %d\n", matrix->row);
        }
    }
    else
    {
        //если задано, то изменим размер файла
        matrix->file_size = matrix->col * matrix->row * sizeof(double);
        ftruncate(matrix->fd, matrix->file_size);
        printf("New size is: %d\n", matrix->file_size);
    }

    matrix->shared_mem_ptr = mmap(NULL, matrix->file_size,
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED, matrix->fd, 0);

    if (matrix->shared_mem_ptr == MAP_FAILED)
    {
        perror("mmap error\n");
    }

    close(matrix->fd);
}

int get_index(int x, int y, Matrix matrix)
{
    if (x < 0 || x >= matrix.row || y < 0 || y >= matrix.col)
        return -1;

    return x * matrix.row + y;
}

void run(Matrix* matrix)
{
    char* command = (char*) malloc(20);

    while (1)
    {
        int code = scanf("%s", command);

        if (code == EOF)
            break;

        if (strcmp(command, "exit") == 0)
        {
            break;
        }

        if (strcmp(command, "getinfo") == 0)
        {
            printf("col == %d, row == %d\n", matrix->col, matrix->row);
            continue;
        }

        if (strcmp(command, "get") == 0)
        {
            int x, y;
            code = scanf("%d%d", &x, &y);

            if (code != 2)
            {
                fprintf(stderr, "Wrong parametrs.\n");
                continue;
            }

            int index = get_index(x, y, *matrix);

            if (index == -1)
            {
                fprintf(stderr, "Wrong parametrs.\n");
            }
            else
            {
                printf("matrix[%d][%d] == %lf\n", x, y, *(((double*) matrix->shared_mem_ptr) + index));
            }

            continue;
        }

        if (strcmp(command, "set") == 0)
        {
            int x, y;
            double new_value;
            code = scanf("%d%d%lf", &x, &y, &new_value);

            if (code != 3)
            {
                fprintf(stderr, "Wrong parametrs.\n");
                continue;
            }

            int index = get_index(x, y, *matrix);

            if (index == -1)
            {
                fprintf(stderr, "Wrong parametrs.\n");
            }
            else
            {
                *(((double*) matrix->shared_mem_ptr) + index) = new_value;
                printf("set matrix[%d][%d] = %lf\n", x, y, new_value);
            }

            continue;
        }

        if (strcmp(command, "sum") == 0)
        {
            int i;
            char mode[10];
            code = scanf("%s%d", mode, &i);

            if (code != 2)
            {
                fprintf(stderr, "Wrong parametrs.\n");
                continue;
            }

            //int index = get_index(x, y, *matrix);

            if (strcmp(mode, "col") == 0)
            {
                if (i >= matrix->col || i < 0)
                {
                    fprintf(stderr, "Wrong parametrs.\n");
                    continue;
                }

                double sum = 0;

                for (int k = 0; k < matrix->row; ++k)
                {
                    int index = get_index(k, i, *matrix);
                    assert(index >= 0);
                    sum += matrix->shared_mem_ptr[index];
                }

                printf("Col %d: sum == %lf\n", i, sum);
                continue;
            }

            if (strcmp(mode, "row") == 0)
            {
                if (i >= matrix->row || i < 0)
                {
                    fprintf(stderr, "Wrong parametrs.\n");
                    continue;
                }

                double sum = 0;

                for (int k = 0; k < matrix->col; ++k)
                {
                    int index = get_index(i, k, *matrix);
                    assert(index >= 0);
                    sum += matrix->shared_mem_ptr[index];
                }

                printf("Row %d: sum == %lf\n", i, sum);
                continue;
            }

            fprintf(stderr, "Wrong input.\n");
            continue;
        }

        fprintf(stderr, "Wrong input.\n");
    }

    free(command);
}

int main(int argc, char** argv)
{
    Matrix matrix;
    char* file_name;

    get_input(argc, argv, &file_name, &matrix.col, &matrix.row);
    open_matrix(file_name, &matrix);
    run(&matrix);

    if (munmap(matrix.shared_mem_ptr, matrix.file_size) == -1)
    {
        perror("Faild to unmap\n");
    }

    return 0;
}
