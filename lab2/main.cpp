#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <math.h>

#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define CYN   "\x1B[36m"

#define N_SIZE 2
#define eps 0.0000001
#define t 0.1/N_SIZE
#define NOT_ENOUGH_PROCESSES_NUM_ERROR 1

using namespace std;

MPI_Status status;
int flag = 0;

double A[N_SIZE][N_SIZE];
double B[N_SIZE];
double x[N_SIZE];

template<int rows, int cols>
void FillMatrix(double (&matrix)[rows][cols]){
    for(int i = 0; i < cols; i ++){
        for(int j = 0; j < rows; j ++){
            if(i == j) matrix[i][j] = 2.0;
            else matrix[i][j] = 1.0;
        }
    }
}

template<int rows, int cols>
void PrintMatrix(double (&matrix)[rows][cols]){
    for(int i = 0; i < rows; i ++){
        for(int j = 0; j < cols; j ++){
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void FillB(double* B){
    for(int i = 0; i < N_SIZE; i++){
        B[i] = N_SIZE + 1;
    }
}

void FillX(double* x){
    for(int i = 0; i < N_SIZE; i++){
        x[i] = 0;
    }
}

void PrintX(double* x, int size){
    cout << "Answer: " << x[0];
    for(int i = 1; i < size; i++){
        cout << ", " << x[i] ;
    }
    cout << endl;
}

void CheckAnswer(double* x) {
    for(int i = 0; i < N_SIZE; i++) {
        if(x[i] != 0) {
            flag = 1;
            return;
        }
    }
    return;
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
        printf("%sGenerating matrix %sA%s with size %s%dx%d\n",CYN, GRN, CYN, RESET, N_SIZE, N_SIZE);
        FillMatrix(A);
        // PrintMatrix(A);

        printf("%sGenerating vector %sB%s with size %s%d\n",CYN, GRN, CYN, RESET, N_SIZE);
        FillB(B);
        // PrintMatrix(B);
        
        printf("%sGenerating vector %sx%s with size %s%d\n",CYN, GRN, CYN, RESET, N_SIZE);
        FillX(x);
        // PrintMatrix(x);

        printf("\n%s----------------Starting Calculations----------------%s\n", CYN, RESET);

        clock_t start3 = clock();
        
        
        
        workers_num = communicator_size - 1;
        whole_part = N_SIZE / workers_num;
        offset = 0;

        message_tag = 1;
        for(process_id = 1; process_id <= workers_num; process_id++ ) {
            rows_num = process_id < workers_num ? whole_part : N_SIZE - (process_id - 1) * whole_part;
            MPI_Send(&offset, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&rows_num, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&A[offset][0], rows_num * N_SIZE, MPI_DOUBLE, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&B, N_SIZE, MPI_DOUBLE, process_id, message_tag, MPI_COMM_WORLD);
            MPI_Send(&x, N_SIZE, MPI_DOUBLE, process_id, message_tag, MPI_COMM_WORLD);
            offset += rows_num;
        }

        message_tag = 2;
        for(process_id = 1; process_id <= workers_num; process_id++) {
            MPI_Recv(&offset, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows_num, 1, MPI_INT, process_id, message_tag, MPI_COMM_WORLD, &status);
            MPI_Recv(&x[offset], rows_num * N_SIZE, MPI_DOUBLE, process_id, message_tag, MPI_COMM_WORLD, &status);
        }
        clock_t finish3 = clock();
        double elapsed3 = (double)((finish3 - start3) / (1.0 * 1000000));
        PrintX(x, N_SIZE);
        printf("%sParallel Method 1:%s %f \n", CYN, RESET, elapsed3);
        CheckAnswer(x);
        if(flag == 1) {
            printf("\n ERROR - THE ANSWER IS WRONG");
        }
        else {
            printf("\n%sTHE ANSWER IR RIGHT%s\n", CYN, RESET);
        }
    }
    
    if(process_rank != 0) {
        message_tag = 1;
        MPI_Recv(&offset, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows_num, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&A, rows_num * N_SIZE, MPI_DOUBLE, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&B, N_SIZE, MPI_DOUBLE, 0, message_tag, MPI_COMM_WORLD, &status);
        MPI_Recv(&x, N_SIZE, MPI_DOUBLE, 0, message_tag, MPI_COMM_WORLD, &status);
        
        double res[rows_num];
        
        double sum2 = 0;
        for(int i = 0; i < N_SIZE; i++){
            sum2 += pow(B[i], 2);
        }
        sum2 = sqrt(sum2);
        
        
        int k = 0;
        while (true){
            if (process_rank == 1) {
                cout << "_________________________k = " << k << "____________________________" << endl;
            }
            
            double res[rows_num];
            for(int i = 0; i < rows_num; i++){
                res[i] = 0;
            }
            if (process_rank == 1) {
                PrintX(res, rows_num);
            }
            
            for(int i = 0; i < rows_num; i++){
                for(int j = 0; j < N_SIZE; j++){
                    res[i] += A[i][j] * x[offset + j];
                }
            }
            if (process_rank == 1) {
                PrintX(res, rows_num);
            }
            
            for(int i = 0; i < rows_num; i++){
                res[i] -= B[i];
            }
            if(process_rank == 1) {
                PrintX(res, rows_num);
            }
            
            double sum1 = 0;
            for(int i = 0; i < rows_num; i++){
                sum1 += pow(res[i], 2);
            }
            sum1 = sqrt(sum1);
            if (process_rank == 1) {
                cout << "SUM 1 " << sum1 << endl;
                cout << "SUM 1 / SUM 2 " << sum1/sum2 << endl;
            }
            
            if(sum1/sum2 < eps){
                break;
            }
            else {
                for(int i = 0; i < rows_num; i++){
                    x[offset + i] -=  t * res[i];
                }
            }
            if (process_rank == 1) {
                cout << "MAYBE ANS : " << endl;
                PrintX(x, N_SIZE);
            }

            k++;
        }
        
        message_tag = 2;
        MPI_Send(&offset, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD);
        MPI_Send(&rows_num, 1, MPI_INT, 0, message_tag, MPI_COMM_WORLD);
        MPI_Send(&x[offset], N_SIZE, MPI_DOUBLE, 0, message_tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
