#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <limits.h>
#include "save_mat2.h"
#include <stdlib.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
//int J=0;

//int (*best_path)[SIZE_R] = malloc(sizeof(int[SIZE_R][SIZE_R]));
//int (*best_J)[SIZE_R] = malloc(sizeof(int[SIZE_R]));
//int best_J[SIZE_R];
//int best_path[SIZE_R];

int n;
int dist[MAX_CITIES][MAX_CITIES];
int best_path[MAX_CITIES][MAX_CITIES];
int best_path_cost[MAX_CITIES] = {INFINITE};

//int (*raw_dist)[MAX_CITIES];
//int raw_dist[MAX_CITIES][MAX_CITIES];

void copy_path(int src[], int rank) {
    for (int i = 0; i < n; i++) {
        best_path[rank][i] = src[i];
    }
}

void branch_and_bound(int path[], int path_cost, int visited[], int level, int rank) {
    if (level == n) {
        if (path_cost < best_path_cost[rank]) {
            best_path_cost[rank] = path_cost;
            copy_path(path, rank);
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                path[level] = i;
                visited[i] = 1;
                int new_cost = path_cost + dist[path[level - 1]][i];
                if (new_cost < best_path_cost[rank]) {
                    branch_and_bound(path, new_cost, visited, level + 1, rank);
                }
                visited[i] = 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char file_path[] = "input/dist4";
    n = save_mat_size(file_path);

    //int (*raw_dist)[MAX_CITIES] = save_mat(file_path);
    int (*raw_dist)[MAX_CITIES] = save_mat(file_path);
    
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            if (i==j){
                dist[i][j] = 0;
            }else if(i==n-1 && j==n-1){
                dist[i][j] = 0;
            }
            else{
                dist[i][j] = raw_dist[j-1][i];
                dist[j][i] = dist[i][j];
            }
        }
    }
    
    int path[MAX_CITIES];
    int visited[MAX_CITIES] = {0};
    path[0] = 0;
    visited[0] = 1;

    for(int i=0; i<MAX_CITIES; i++){
        best_path_cost[i]=INFINITE;
    }
    
    int second_city;
    int num_procs_cities;
    int current_cost;

    int min_city_per_rank = (n-1)/size;
    int max_city_per_rank = min_city_per_rank+1;
    int number_of_cities_process_max = (n-1)%size;

    if(rank < number_of_cities_process_max){
        second_city = rank*max_city_per_rank;
        num_procs_cities = max_city_per_rank;
    }else if(rank >= number_of_cities_process_max){
        second_city = number_of_cities_process_max*max_city_per_rank+((rank-number_of_cities_process_max)*min_city_per_rank);
        num_procs_cities = min_city_per_rank;
    }

    for(int i = second_city ; i < num_procs_cities; i++){
        path[1] = second_city;
        visited[1] = 1;
        current_cost = dist[path[0]][path[1]];
        branch_and_bound(path, current_cost, visited, 2, rank);
    }

    int send_buf_cost = best_path_cost[rank];
    MPI_Allgather(&send_buf_cost, 1, MPI_INT, best_path_cost, 1, MPI_INT, MPI_COMM_WORLD);

    for(int i=0; i<MAX_CITIES; i++){
        int send_buf_path = best_path[rank][i];
        MPI_Allgather(&send_buf_path, 1, MPI_INT, best_path, 1, MPI_INT, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(rank==0){
        printf("Process %d going to show a result...\n", rank);
        for(int i = 0; i < size ; i++){
            printf("processor %d: \n", i);
            printf("  path: ");
            for(int j = 0 ; j < MAX_CITIES ; j++){
                printf("%d ", best_path[i][j]);
            }
           printf("\n  best_path_cost: %d \n", best_path_cost[i]);
        }
   }

    MPI_Finalize();
    return 0;
}
