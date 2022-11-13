#include <mpi.h>
#include <iostream>
#include <math.h>

#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define CYN   "\x1B[36m"

#define N_SIZE 16384
#define eps 0.0000001
#define t 0.1/N_SIZE
#define NOT_ENOUGH_PROCESSES_NUM_ERROR 1

using namespace std;

MPI_Status status;

void FillMatrix(double* matrix) {
    for (int i = 0; i < N_SIZE; i++) {
        for (int j = 0; j < N_SIZE; j++) {
            if (i == j) matrix[i * N_SIZE + j] = 2.0;
            else matrix[i * N_SIZE + j] = 1.0;
        }
    }
}

void FillB(double* B) {
    for (int i = 0; i < N_SIZE; i++) {
        B[i] = N_SIZE + 1;
    }
}

void FillX(double* x) {
    for (int i = 0; i < N_SIZE; i++) {
        x[i] = 0;
    }
}

void PrintMatrix(double* matrix) {
    for (int i = 0; i < N_SIZE; i++) {
        for (int j = 0; j < N_SIZE; j++) {
            printf("%f ", matrix[i * N_SIZE + j]);
        }
        printf("\n");
    }
}

void PrintX(double* x, int size) {
    cout << "Answer: " << x[0];
    for (int i = 1; i < size; i++) {
        cout << ", " << x[i];
    }
    cout << endl;
}

bool CheckAnswer(double* x) {
    for (int i = 0; i < N_SIZE; i++) {
        if (abs(x[i] - 1.0) < 0.00001) {
            continue;
        }
        else return false;
    }
    return true;
}

double GetNorm(double* v) {
    double s = 0;
    for (int i = 0; i < N_SIZE; i++) s += pow(v[i], 2);
    s = sqrt(s);
    return s;
}

int main(int argc, char* argv[]) {
    double* A = new double[N_SIZE * N_SIZE];
    double* B = new double[N_SIZE];
    double* x = new double[N_SIZE];
    double* y = new double[N_SIZE];
    
    int num_of_processes;
    int process_rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    if (num_of_processes <= 1) {
        MPI_Abort(MPI_COMM_WORLD, NOT_ENOUGH_PROCESSES_NUM_ERROR);
    }
    
    if (process_rank == 0) {
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
    }
    
    double start = MPI_Wtime();
    MPI_Bcast(x, N_SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double condition = eps;
    int k = 0;
    while (true) {
        //cout << "_________________________iteration = " << k << "____________________________" << endl;
        k++;
        
        int buf_size = N_SIZE * N_SIZE / num_of_processes;
        int rows_num = N_SIZE / num_of_processes;

        double* partOfA = new double[buf_size];

        MPI_Scatter(A, buf_size, MPI_DOUBLE, partOfA, buf_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        double* res = new double[rows_num];
        for (int i = 0; i < rows_num; i++) {
            res[i] = 0;
            for (int j = 0; j < N_SIZE; j++) res[i] += partOfA[i * rows_num + j] * x[j];
        }

        MPI_Gather(res, rows_num, MPI_DOUBLE, y, rows_num, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        delete[] partOfA;
        delete[] res;
        
        if (process_rank == 0) {
            for (int i = 0; i < N_SIZE; i++) {
                y[i] -= B[i];
            }
            condition = GetNorm(y) / GetNorm(B);
        }
        MPI_Bcast(&condition, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        if (condition < eps) {
            break;
        }
        MPI_Bcast(y, N_SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        if (process_rank == 0) {
            for (int i = 0; i < N_SIZE; i++) {
                x[i] -= t * y[i];
            }
        }
        //cout << "probably ";
        //PrintX(x, N_SIZE);
        MPI_Bcast(x, N_SIZE, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();
    if (process_rank == 0) {
        printf("\n%s----------------!!DONE!!----------------%s\n", CYN, RESET);
        cout << "duration is " << end - start << " seconds for " << num_of_processes  << " threads" << endl;
        // PrintX(x, N_SIZE);
        if (!CheckAnswer(x)) {
            printf("\n!!ERROR - THE WRONG ANSWER!!\n");
        }
        else {
            printf("\n!!THE RIGHT ANSWER!!\n");
        }
    }
    MPI_Finalize();
    
    delete[] A;
    delete[] B;
    delete[] x;
    delete[] y;

    return 0;
}
