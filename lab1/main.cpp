#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>
#include <ctime>

#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define CYN   "\x1B[36m"

#define M_SIZE 6000
#define NOT_ENOUGH_PROCESSES_NUM_ERROR 1

MPI_Status status;

int a[M_SIZE][M_SIZE];
int b[M_SIZE][M_SIZE];
int c1[M_SIZE][M_SIZE];
int c2[M_SIZE][M_SIZE];
int c3[M_SIZE][M_SIZE];

int GenerateRandomNumber() {
    return std::rand() % 9 + 1;
}

template<int rows, int cols>
void FillMatrix(int (&matrix)[rows][cols]){
    for(int i = 0; i < cols; i ++){
        for(int j = 0; j < rows; j ++){
          matrix[i][j] = GenerateRandomNumber();
        }
    }
}

template<int rows, int cols>
void PrintMatrix(int (&matrix)[rows][cols]) {
    for(int i = 0; i < rows; i ++) {
        for(int j = 0; j < cols; j ++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

template<int rows, int cols>
void CheckMatrix(int (&matrix1)[rows][cols], int (&matrix2)[rows][cols], int &status) {
    for(int i = 0; i < M_SIZE; i ++) {
        for(int j = 0; j < M_SIZE; j ++) {
            if (matrix1[i][j] != matrix2[i][j]) {
                printf("\n ERROR - MATRIX ARE NOT EQUAL");
                status = 1;
                return;
            }
        }
    }
    status = 0;
}

int main(int argc, char *argv[]) {
    int communicator_size;
    int process_rank, process_id;
    int offset;
    int rows_num;
    int workers_num;
    int remainder;
    int whole_part;
    int message_tag;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &communicator_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    if(communicator_size < 2) {
        MPI_Abort(MPI_COMM_WORLD, NOT_ENOUGH_PROCESSES_NUM_ERROR);
    }

    if(process_rank == 0) {
        printf("%s------------------Generating matrixes------------------%s\n", CYN, RESET);
        
        printf("%sGenerating matrix %sA%s with size %s%dx%d\n",CYN, GRN, CYN, RESET, M_SIZE, M_SIZE);
        FillMatrix(a);
        // PrintMatrix(a);

        printf("%sGenerating matrix %sB%s with size %s%dx%d\n",CYN, GRN, CYN, RESET, M_SIZE, M_SIZE);
        FillMatrix(b);
        // PrintMatrix(b);

        printf("\n%s----------------Starting Multiplication----------------%s\n", CYN, RESET);
        
        clock_t start1 = clock();
        for(int i = 0; i < M_SIZE; i++) {
            for(int j = 0; j < M_SIZE; j++) {
                c1[i][j] = 0;
                for(int k = 0; k < M_SIZE; k++) {
                    c1[i][j] += (a[i][k] * b[k][j]);
                }
            }
        }
        clock_t finish1 = clock();
        double elapsed1 = (double) (finish1 - start1) / CLOCKS_PER_SEC;
        printf("%sStandart Method by rows:%s %f \n", CYN, RESET, elapsed1);
        // PrintMatrix(c1);
        
        clock_t start2 = clock();
        for(int i = 0; i < M_SIZE; i++) {
            for(int j = 0; j < M_SIZE; j++) {
                c2[i][j] = 0;
                for(int k = 0; k < M_SIZE; k++) {
                    c2[i][j] += (a[i][k] * b[k][j]);
                }
            }
        }
        clock_t finish2 = clock();
        double elapsed2 = (double) (finish2 - start2) / CLOCKS_PER_SEC;
        printf("%sStandart Method by columns:%s %f \n", CYN, RESET, elapsed2);
        // PrintMatrix(c2);
     
        clock_t start3 = clock();
        workers_num = communicator_size - 1;
        whole_part = M_SIZE / workers_num;
        remainder = M_SIZE % workers_num;
        offset = 0;

        message_tag = 1;
        for(process_id = 1; process_id <= workers_num; process_id++ ) {
            rows_num = process_id <= remainder ? whole_part + 1 : whole_part;
            MPI_Send(&offset, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&rows_num, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&a[offset][0], rows_num * M_SIZE, MPI_INT, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&b, M_SIZE * M_SIZE, MPI_INT, process_id, message_tag, MPI_COMM_WORLD);

            offset += rows_num;
        }

        message_tag = 2;
        for(process_id = 1; process_id <= workers_num; process_id++) {
            MPI_Recv(&offset, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows_num, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&c3[offset][0], rows_num * M_SIZE, MPI_INT, process_id, message_tag, MPI_COMM_WORLD, &status);
        }
        clock_t finish3 = clock();
        double elapsed3 = (double)((finish3 - start3) / (1.0 * 1000000));
        printf("%sParallel Method:%s %f \n", CYN, RESET, elapsed3);
        // PrintMatrix(c3);
        int status;
        CheckMatrix(c1, c3, status);
        if(status == 1) {
            printf("\n ERROR - MATRICES ARE NOT EQUAL");
        }
        else {
            printf("\n%sMATRICES ARE EQUAL%s\n", CYN, RESET);
        }
    }
    
    if(process_rank != 0) {
        message_tag = 1;
        MPI_Recv(&offset, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows_num, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&a, rows_num * M_SIZE, MPI_INT, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&b, M_SIZE * M_SIZE, MPI_INT, 0, message_tag, MPI_COMM_WORLD, &status);

        for(int k = 0; k < M_SIZE; k ++) {
            for(int i = 0; i < rows_num; i ++) {
                c3[i][k] = 0;
                for(int j = 0; j < M_SIZE; j ++) {
                    c3[i][k] += a[i][j] * b[j][k];
                }
            }
        }

        message_tag = 2;
        MPI_Send(&offset, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD);
        MPI_Send(&rows_num, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD);
        MPI_Send(&c3, rows_num * M_SIZE, MPI_INT, 0, message_tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
