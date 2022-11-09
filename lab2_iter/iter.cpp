#include <iostream>
#include <math.h>
#include <ctime>

using namespace std;

#define N_SIZE 2
#define eps 0.0000001
#define t 0.1/N_SIZE

double A[N_SIZE][N_SIZE];
double B[N_SIZE];
int status = 0;

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
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void PrintX(double* x){
    cout << "Answer: " << x[0];
    for(int i = 1; i < N_SIZE; i++){
        cout << ", " << x[i] ;
    }
    cout << endl;
}

void CheckAnswer(double* x) {
    for(int i = 0; i < N_SIZE; i ++) {
        if(x[i] != 1) {
            status = 1;
            return;
        }
    }
    return;
}

int main(int argc, char *argv[]) {
    FillMatrix(A);
    
    for(int i = 0; i < N_SIZE; i++){
        B[i] = N_SIZE + 1;
    }
    
    double* x = new double[N_SIZE];

    for(int i = 0; i < N_SIZE; i++){
        x[i] = 0;
    }
    
    clock_t start;
    
    int k = 0;
    while (true){
        cout << "______________________k = " << k << "______________________" << endl;
        double res[N_SIZE];
        for(int i = 0; i < N_SIZE; i++){
            res[i] = 0;
        }

        PrintX(res);
        
        for(int i = 0; i < N_SIZE; i++){
            for(int j = 0; j < N_SIZE; j++){
                res[i] += A[i][j] * x[j];
            }
        }
        PrintX(res);

        for(int i = 0; i < N_SIZE; i++){
            res[i] -= B[i];
        }
        PrintX(res);
        
        double sum1 = 0;
        for(int i = 0; i < N_SIZE; i++){
            sum1 += pow(res[i], 2);
        }
        sum1 = sqrt(sum1);
        cout << "sum1  " << sum1 << endl;
        
        double sum2 = 0;
        for(int i = 0; i < N_SIZE; i++){
            sum2 += pow(B[i], 2);
        }
        sum2 = sqrt(sum2);
        cout << "sum1/sum2  " << sum1/sum2 << endl;

        if(sum1/sum2 < eps){
            break;
        }
        else {
            for(int i = 0; i < N_SIZE; i++){
                x[i] -=  t * res[i];
            }
        }
        cout << "MAYBE ANS: " << endl;
        PrintX(x);
        k++;
    }
    
    double duration = (clock() - start) / (double) CLOCKS_PER_SEC;
    
    CheckAnswer(x);
    if(status == 1){
        cout << "Wrong Answer!!!" << endl;
        //PrintX(x);
    }
    else{
        cout << "Right Answer!!!" << endl;
        PrintX(x);
    }
    cout << "Time: " << duration << endl;
    return 0;
}
