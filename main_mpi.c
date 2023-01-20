#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
#define START_CITIES 0

int n;
int (*dist)[MAX_CITIES];
int (*best_path)[MAX_CITIES];
int *best_path_cost;

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);


int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);

    double start_time = MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    dist = malloc(sizeof(int[MAX_CITIES][MAX_CITIES]));
    best_path = malloc(sizeof(int[MAX_CITIES][MAX_CITIES]));
    best_path_cost = malloc(sizeof(int[MAX_CITIES]));

    char* file_path;
    if(argc >=3 && strcmp("-i", argv[1]) == 0){
        char* myArg = argv[2];
        while (myArg[0] == '\'') myArg++;
        while (myArg[strlen(myArg)-1] == '\'') myArg[strlen(myArg)-1] = '\0';;
        file_path = myArg;
    }else{
        char *df_file = "input/dist4";
        printf("[System] The default file (%s) will be used if no input is provided  \n", df_file);
        file_path  = df_file;
    }

    if(rank==0){
        get_cities_info(file_path);
        printf("n= %d \n", n);

        // Broadcast the array from the root processor
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&dist, MAX_CITIES*MAX_CITIES, MPI_INT, 0, MPI_COMM_WORLD);
    }else {
        // Receive the array on all other processors
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&dist, MAX_CITIES*MAX_CITIES, MPI_INT, 0, MPI_COMM_WORLD);
    }

    for(int i=0; i<MAX_CITIES; i++){
        best_path_cost[i]=INFINITE;
    }

    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited[i]=0;

    // set starter city to city 1 (or 0 in array order) in path array at position 0
    path[0] = START_CITIES;

    // set visited city to 1 by setting visited array position 0 to 1
    visited[START_CITIES] = 1;
    
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
            min_dist = best_path_cost[i];
        }
    }

    printf("best of the best is in rank %d, \n", index_best_path);
	printf("  best_path: ");
    for(int i = 0; i < n ; i++){
        printf("%d ", best_path[index_best_path][i]);
    }
    printf("\n  best_path_cost: %d \n", best_path_cost[index_best_path]);
   }

    double end_time = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    double computing_time = end_time - start_time;
    printf("rank=%d spent: %f seconds\n", rank, computing_time);


    MPI_Finalize();

    //save_result(file_path, computing_time);

    return 0;
}

int get_cities_info(char* file_path) {
    FILE* file = fopen(file_path, "r");
    
    //int number_of_city = 0;
    char line[256];
    fscanf(file, "%d", &n); 

    int row=1;
    int col=0;

    dist[row-1][col]=0;

    while (fgets(line, sizeof(line), file)) {
        col=0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            if(atoi(token)>0){
                dist[row-1][col] = atoi(token);
                dist[col][row-1] = atoi(token);
            }
            token = strtok(NULL, " ");
            col++;
        }
        dist[row][col]=0;
        row++;
    }
    fclose(file);
}

void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank) {
    if (level == n) {
	if (path_cost < best_path_cost[rank]) {
            best_path_cost[rank] = path_cost;
            for (int i = 0; i < n; i++) best_path[rank][i] = path[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                    path[level] = i;
                    visited[i] = 1;
                    int new_cost = path_cost + dist[i][path[level - 1]];
            if (new_cost < best_path_cost[rank]) {
                        branch_and_bound(path, new_cost, visited, level + 1, rank);
                    }
                    visited[i] = 0;
	        }
        }
    }
}

int save_result(int rank, char *dist_file, double computing_time) {
    FILE *file;
    char date[20];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char* fileName="result_serial.csv";
    file = fopen(fileName, "r"); // open the file in "read" mode
    if (file == NULL) {
        file = fopen(fileName, "w"); //create new file in "write" mode
        fprintf(file, "date-time, dist file, computing time (s)\n"); // add header to the file
    } else {
        fclose(file);
        file = fopen(fileName, "a"); // open the file in "append" mode
    }

    // Get the current date and time
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(file, "%s,%s,%f\n", date, dist_file, computing_time); // add new data to the file

    fclose(file); // close the file
    return 0;
}