#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <limits.h>
#include "save_mat2.h"
#include <stdlib.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX

int n;
int dist[MAX_CITIES][MAX_CITIES];
int best_path[MAX_CITIES][MAX_CITIES];
int best_path_cost[MAX_CITIES];

void copy_path(int current_path[], int rank) {
    for (int i = 0; i < n; i++) {
        best_path[rank][i] = current_path[i];
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
    char* check_i = argv[1];
    char* file_path;
    if(check_i == "-i"){
        char* myArg = argv[2];
        while (myArg[0] == '\'') myArg++;
        while (myArg[strlen(myArg)-1] == '\'') myArg[strlen(myArg)-1] = '\0';;
        printf("Argument: %s\n", myArg);
        file_path = myArg;
    }else{
        file_path  = "input/dist4";
    }
    
    n = save_mat_size(file_path);
    if(rank==0) printf("n= %d \n", n);
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



    for(int i=0; i<MAX_CITIES; i++){
        best_path_cost[i]=INFINITE;
    }   
    int path[MAX_CITIES];
    int visited[MAX_CITIES] = {0};
    path[0] = 0;
    visited[0] = 1;
    
    int second_city;
    int num_procs_cities;
    int current_cost;

    int min_city_per_rank = (n-1)/size;
    int max_city_per_rank = min_city_per_rank+1;
    int number_of_cities_process_max = (n-1)%size;

    if(rank < number_of_cities_process_max){
        second_city = rank*max_city_per_rank+1;
        num_procs_cities = max_city_per_rank;
    }else if(rank >= number_of_cities_process_max){
        second_city = number_of_cities_process_max*max_city_per_rank+((rank-number_of_cities_process_max)*min_city_per_rank)+1;
        num_procs_cities = min_city_per_rank;
    }

    for(int i = second_city ; i < second_city+num_procs_cities; i++){
        path[1] = i;
        for(int i=1; i<MAX_CITIES; i++){
	    visited[i]=0;
	}
	visited[i] = 1;
        current_cost = dist[path[0]][path[1]];
	branch_and_bound(path, current_cost, visited, 2, rank);
    }


    MPI_Barrier(MPI_COMM_WORLD);
    
    int send_buf_cost = best_path_cost[rank];
    MPI_Allgather(&send_buf_cost, 1, MPI_INT, best_path_cost, 1, MPI_INT, MPI_COMM_WORLD);

    int row_to_gather[MAX_CITIES];
    for (int i = 0; i < MAX_CITIES; i++) {
        row_to_gather[i] = best_path[rank][i];
    }

    MPI_Allgather(row_to_gather, MAX_CITIES, MPI_INT, best_path, MAX_CITIES, MPI_INT, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if(rank==0){
    int min_dist=INFINITE;
    int index_best_path;
    for(int i=0; i<n; i++){
        if(best_path_cost[i]<min_dist){
            index_best_path = i;
        }
    }

    printf("best of the best is in rank %d, \n", index_best_path);
	printf("  best_path: ");
    for(int i = 0; i < n ; i++){
        printf("%d ", best_path[index_best_path][i]);
    }
    printf("\n  best_path_cost: %d \n", best_path_cost[index_best_path]);
   }

    MPI_Finalize();
    return 0;
}
