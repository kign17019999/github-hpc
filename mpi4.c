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
int best_path_cost[MAX_CITIES];

//int (*raw_dist)[MAX_CITIES];
//int raw_dist[MAX_CITIES][MAX_CITIES];

void copy_path(int current_path[], int rank) {
    for (int i = 0; i < n; i++) {
        best_path[rank][i] = current_path[i];
    }
}

void branch_and_bound(int path[], int path_cost, int visited[], int level, int rank) {
    //printf("cur cost= %d | lvl=%d\n", path_cost, level);
    if (level == n) {
	//printf("lvl = %d \n", level);
	printf("[END Route] p=%d | path %d %d %d %d \n", rank, path[0], path[1], path[2], path[3]);
        //printf("previous best path cost = %d\n",best_path_cost[rank]);
	if (path_cost < best_path_cost[rank]) {
            best_path_cost[rank] = path_cost;
            copy_path(path, rank);
        }
	printf("current best path cost = %d\n",best_path_cost[rank]);
    } else {
        for (int i = 0; i < n; i++) {
            //printf(" [visit[%d]= %d\n", i, visited[i]);
	    if (!visited[i]) {
		//printf("lvl = %d \n", level);
                path[level] = i;
                visited[i] = 1;
                int new_cost = path_cost + dist[path[level - 1]][i];
		//printf("[i=%d] new cost = %d (jsut add: %d {dist[%d][%d]}) \n", i, new_cost, dist[path[level - 1]][i],path[level - 1],i);
                //printf("visited before BB: [%d %d %d %d] \n", visited[0],visited[1],visited[2],visited[3]);
		if (new_cost < best_path_cost[rank]) {
                    branch_and_bound(path, new_cost, visited, level + 1, rank);
                }
                visited[i] = 0;
            	//printf("visited after BB: [%d %d %d %d] \n", visited[0],visited[1],visited[2],visited[3]);	
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
    if(rank==0) printf("n= %d \n", n);
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
    
    //printf("p%d min= %d | max= %d | num_max= %d \n", rank, min_city_per_rank, max_city_per_rank, number_of_cities_process_max);
    //printf("p%d 2ndCity= %d | num= %d \n", rank, second_city, num_procs_cities);

    for(int i = second_city ; i < second_city+num_procs_cities; i++){
        path[1] = i;
        for(int i=1; i<MAX_CITIES; i++){
	    visited[i]=0;
	}
	visited[i] = 1;
        current_cost = dist[path[0]][path[1]];
        //printf("i= %d | p%d got lastest cost = %d | path[1]=%d | visit[0 1 2 3]=[%d %d %d %d] | cur_cost= %d \n", i, rank, best_path_cost[rank], path[1] , visited[0], visited[1] , visited[2] , visited[3] , current_cost );
	branch_and_bound(path, current_cost, visited, 2, rank);
	//printf("i= %d | p%d got lastest cost = %d | path[1]=%d | visit[0 1 2 3]=[%d %d %d %d] | cur_cost= %d \n", i, rank, best_path_cost[rank], path[1] , visited[0], visited[1] , visited[2] , visited[3] , current_cost );
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
   	printf("Process %d going to show a result...\n", rank);
	for(int i = 0; i < size ; i++){
            printf("processor %d: \n", i);
            printf("  path: ");
            for(int j = 0 ; j < n ; j++){
                printf("%d ", best_path[i][j]);
            }
           printf("\n  best_path_cost: %d \n", best_path_cost[i]);
        }

    int min_dist=INFINITE;
    int index_best_path;
    for(int i=0; i<n; i++){
        if(best_path_cost[i]<min_dist){
            index_best_path = i;
        }
    }

    printf("best of the best is in rank %d, \n", index_best_path);
	for(int i = 0; i < n ; i++){
        printf("  best_path: ");
        for(int j = 0 ; j < n ; j++){
            printf("%d ", best_path[index_best_path][i]);
        }
    }
    printf("\n  best_path_cost: %d \n", best_path_cost[index_best_path]);
   }

    MPI_Finalize();
    return 0;
}
