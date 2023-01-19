#include <stdio.h>
#include <mpi.h>
#include <string.h>

#define SIZE_R 10
#define INFINITE INT_MAX
int J=0;

//int (*best_path)[SIZE_R] = malloc(sizeof(int[SIZE_R][SIZE_R]));
//int (*best_J)[SIZE_R] = malloc(sizeof(int[SIZE_R]));
int best_J[SIZE_R];
int best_path[SIZE_R];

void function1(int input, int rank) {
    for (int i = 0; i < SIZE_R; i++) {
        if(i<rank*2){
            J+=input*i;
        }
        if(J>best_J[rank]){
            best_J[rank] = J;
            for(int j = 0; j<SIZE_R; j++){
                best_path[rank][j] = i*j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int input = rank*2;
    function1(input, rank);
    printf("Process %d | input = %d | last J= %d |\n", rank, input, J);

    if (rank == 0) {
        MPI_Gather(MPI_IN_PLACE, 1, MPI_INT, best_J, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Gather(&best_J[rank], 1, MPI_INT, best_J, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    printf("Process %d doing more work...\n", rank);
    if(rank==0){
        for(int i = 0; i < SIZE_R ; i++){
 //           printf("processor %d: \n", i);
 //           printf("  path: ");
            for(int j = 0 ; j < SIZE_R ; j++){
 //               printf("%d ", best_path[i][j]);
            }
           printf("\n  J: %d", best_J[i]);
        }
   }

    MPI_Finalize();
    return 0;
}
