#include <stdio.h>
#include <mpi.h>
#include <string.h>

#define SIZE_R 10
#define INFINITE INT_MAX
int J=0;

//int (*best_path)[SIZE_R] = malloc(sizeof(int[SIZE_R][SIZE_R]));
//int (*best_J)[SIZE_R] = malloc(sizeof(int[SIZE_R]));
int best_J[SIZE_R];

void function1(int input, int rank) {
    for (int i = 0; i < SIZE_R; i++) {
        if(i<rank*2){
            J+=input*i;
        }
        if(J>best_J[rank]){
            best_J[rank] = J;
      //      for(int j = 0; j<SIZE_R; j++){
      //          //best_path[rank][j] = i*j;
       //     }
        }

    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //memset(best_J, 0, sizeof(int[SIZE_R]));
    //

    int input = rank*2;
    function1(input, rank);
    printf("Process %d | input = %d | last J= %d |\n", rank, input, J);
    //MPI_Bcast(&best_J[rank], 1, MPI_INT, &rank, MPI_COMM_WORLD);
    if(rank == 0) {
        for (int i = 1; i < size; i++) {
                MPI_Recv(&best_J[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(&best_J[rank], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }


        for(int i=0 ; i<SIZE_R ; i++){
        //MPI_Bcast(&best_path[rank][i], 1, MPI_INT, rank, MPI_COMM_WORLD);
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
