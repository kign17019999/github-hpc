/////////////////////   test.c      //////////////////////////
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int A = 0;

    if (rank == 0) {
        MPI_Request request;
        int sendd = A;
        MPI_Ialltoall(&sendd, 1, MPI_INT, &A, 1, MPI_INT, MPI_COMM_WORLD, &request);
        //sleep(1);
        A = 1;
        sendd = A;
        MPI_Ialltoall(&sendd, 1, MPI_INT, &A, 1, MPI_INT, MPI_COMM_WORLD, &request);
    } else {
        MPI_Request request;
        //MPI_Irecv(&A, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        if (rank == 1) sleep(2);
        MPI_Irecv(&A, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        int flag = 0;
        MPI_Test(&request, &flag, MPI_STATUS_IGNORE);

        flag =0;
        MPI_Irecv(&A, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);

    }

    printf("Process %d has A = %d\n", rank, A);
    MPI_Finalize();
    return 0;
}




/////////////       test2.c     /////////////


#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int A = 0;

    if (rank == 0) {
        MPI_Request request;
        int sendd = A;
        int sendcounts[size], recvcounts[size];
        int sdispls[size], rdispls[size];
        for (int i = 0; i < size; i++) {
                sendcounts[i] = 1;
                recvcounts[i] = 1;
                sdispls[i] = i;
                rdispls[i] = i;
        }


        //MPI_Ialltoallv(&sendd, sendcounts, sdispls, MPI_INT, &A, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD, &request);
        MPI_Ialltoallv(&sendd, sendcounts, sdispls, MPI_INT, A, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD, &request);

        //MPI_Ialltoall(&sendd, 1, MPI_INT, &A, 1, MPI_INT, MPI_COMM_WORLD, &request);
        //sleep(1);
        A = 1;
        sendd = A;
        //MPI_Ialltoallv(&sendd, sendcounts, sdispls, MPI_INT, &A, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD, &request);
        MPI_Ialltoallv(&sendd, sendcounts, sdispls, MPI_INT, A, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD, &request);
        //MPI_Ialltoall(&sendd, 1, MPI_INT, &A, 1, MPI_INT, MPI_COMM_WORLD, &request);
    } else {
        MPI_Request request;
        //MPI_Irecv(&A, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        if (rank == 1) sleep(2);
        MPI_Irecv(&A, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        int flag = 0;
        MPI_Test(&request, &flag, MPI_STATUS_IGNORE);

        flag =0;
        MPI_Irecv(&A, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);

    }

    printf("Process %d has A = %d\n", rank, A);
    MPI_Finalize();
    return 0;
}


/////////////////       mpi2.c      ///////////////////

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
   int send_buf = best_J[rank];
   MPI_Allgather(&send_buf, 1, MPI_INT, best_J, 1, MPI_INT, MPI_COMM_WORLD);

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
