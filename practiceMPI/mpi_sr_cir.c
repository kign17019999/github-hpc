#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size, message;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    message = rank;
    MPI_Send(&message, 1, MPI_INT, (rank + 1) % size, 0, MPI_COMM_WORLD);
    MPI_Recv(&message, 1, MPI_INT, (rank + size - 1) % size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received message %d from Process %d\n", rank, message, (rank + size - 1) % size);

    MPI_Finalize();
    return 0;
}
