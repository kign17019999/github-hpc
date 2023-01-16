#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start_time = MPI_Wtime();
    sleep(rank);
    end_time = MPI_Wtime();

    printf("Process %d/%d: Elapsed time: %f\n", rank, size, end_time - start_time);
    MPI_Finalize();
    return 0;
}
