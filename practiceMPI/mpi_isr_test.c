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
        printf("Process 0: Sent message %d\n", message);
        printf("Process 0: Request handle %d\n", request);
    } else if (rank == 1) {
        MPI_Irecv(&message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        int flag = 0;
        while (!flag) {
            MPI_Test(&request, &flag, &status);
        }
        printf("Process 1: Received message %d from Process 0\n", message);
        printf("Process 1: Request handle %d\n", request);
        printf("Process 1: Status handle %d\n", status);
    }

    MPI_Finalize();
    return 0;
}
