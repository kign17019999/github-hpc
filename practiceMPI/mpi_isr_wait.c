#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size, message;
    MPI_Request request;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        message = 42;
        MPI_Isend(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
    } else if (rank == 1) {
        MPI_Irecv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);
        printf("Process 1 received message %d from Process 0\n", message);
    }

    MPI_Finalize();
    return 0;
}
