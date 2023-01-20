#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
#define START_CITIES 0
#define ROOT 0

// mode 0 = send Dist by Bcast
// mode 1 = send Dist by Ibcast
// mode 2 = send Dist by Send & Recv
// mode 3 = send Dist by Isend & Irecv
int mode = 0;

int n;
int (*dist)[MAX_CITIES];
int (*best_path)[MAX_CITIES];
int *best_path_cost;

double (*result)[5];
double count_bb=0;

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);
int save_result(double index_time, int rank, char *dist_file, double total_computing_time, double BaB_computing_time, double count_bab, double r_best_cost, double r_best_path);
double power(double base, int exponent);

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);

    double start_time1 = MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request request1, request2;

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
        if (rank==ROOT) printf("[System] The default file (%s) will be used if no input is provided  \n", df_file);
        file_path  = df_file;
    }
    
    if(rank==ROOT){
        get_cities_info(file_path);
        printf("n= %d \n", n);

        if(mode==0){
            // mode 0 = send Dist by Bcast
            MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
            MPI_Bcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD);

        }else if(mode==1){
            // mode 1 = send Dist by Ibcast
            MPI_Ibcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD, &request1);
            MPI_Ibcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD, &request2);

        }else if(mode==2){
            // mode 2 = send Dist by Send & Recv
            for (int i = 0; i < size; i++) {
                MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(dist, MAX_CITIES*MAX_CITIES, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

        }else if(mode==3){
            // mode 3 = send Dist by Isend & Irecv
            for (int i = 0; i < size; i++) {
                    MPI_Isend(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request1);
                    MPI_Isend(dist, MAX_CITIES*MAX_CITIES, MPI_INT, i, 0, MPI_COMM_WORLD, &request2);
            }

        }
    }else {
        if(mode==0){
            // mode 0 = send Dist by Bcast
            MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
            MPI_Bcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD);
        }else if(mode==1){
            // mode 1 = send Dist by Ibcast
            int flag;
            MPI_Ibcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD, &request1);
            MPI_Ibcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD, &request2);
            do {
                MPI_Test(&request1, &flag, MPI_STATUS_IGNORE);
            } while (!flag);
            
            do {
                MPI_Test(&request2, &flag, MPI_STATUS_IGNORE);
            } while (!flag);
        
        }else if(mode==2){
            // mode 2 = send Dist by Send & Recv
            MPI_Recv(&n, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        }else if(mode==3){
            // mode 3 = send Dist by Isend & Irecv
            MPI_Irecv(&n, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &request1);
            MPI_Irecv(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &request2);
            MPI_Wait(&request1, MPI_STATUS_IGNORE);
            MPI_Wait(&request2, MPI_STATUS_IGNORE);

        }
    }

    double start_time2 = MPI_Wtime();

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

    double end_time2 = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);
    
    int send_buf_cost = best_path_cost[rank];
    MPI_Allgather(&send_buf_cost, 1, MPI_INT, best_path_cost, 1, MPI_INT, MPI_COMM_WORLD);

    int row_to_gather[MAX_CITIES];
    for (int i = 0; i < MAX_CITIES; i++) {
        row_to_gather[i] = best_path[rank][i];
    }

    //MPI_Allgather(row_to_gather, MAX_CITIES, MPI_INT, best_path, MAX_CITIES, MPI_INT, MPI_COMM_WORLD);
    MPI_Allgather(&row_to_gather, MAX_CITIES, MPI_INT, best_path, MAX_CITIES, MPI_INT, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if(rank==ROOT){
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

    double end_time1 = MPI_Wtime();

    MPI_Barrier(MPI_COMM_WORLD);

    double total_computing_time = end_time1 - start_time1;
    double BaB_computing_time = end_time2 - start_time2;
    if (rank ==ROOT){
        printf("rank=%d spent total : %f seconds\n", rank, total_computing_time);
        printf("rank=%d spent BaB   : %f seconds\n", rank, BaB_computing_time);
    }
    
    result = malloc(sizeof(double[size][5]));
    result[rank][0] = total_computing_time;
    result[rank][1] = BaB_computing_time;
    result[rank][2] = count_bb;
    result[rank][3] = best_path_cost[rank];
    double double_path = power(10, 2*n)*404;
    for(int i=0; i<n; i++){
        double_path+=power(10, (n-i-1)*2)*best_path[rank][i];
    }
    result[rank][4] = double_path;
    
    double row_to_gather_result[5];
    for (int i = 0; i < 5; i++) {
        row_to_gather_result[i] = result[rank][i];
    }

    MPI_Allgather(row_to_gather_result, 5, MPI_DOUBLE  , result, 5, MPI_DOUBLE  , MPI_COMM_WORLD);

    if(rank==ROOT){
        double index_time = MPI_Wtime();
        for(int i=0; i<size;i++){
            save_result(index_time, i, file_path, result[i][0], result[i][1], result[i][2], result[i][3], result[i][4]);
        }
        
    }
    
    MPI_Finalize();
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
    count_bb+=1;
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

int save_result(double index_time, int rank, char *dist_file, double total_computing_time, double BaB_computing_time, double count_bab, double r_best_cost, double r_best_path) {
    FILE *file;
    char date[20];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char* fileName="result_parallel.csv";
    file = fopen(fileName, "r"); // open the file in "read" mode
    if (file == NULL) {
        file = fopen(fileName, "w"); //create new file in "write" mode
        fprintf(file, "index_time, rank, date-time, dist file, total_computing_time (s), BaB_computing_time (s), count_BaB, best_cost, best_path\n"); // add header to the file
    } else {
        fclose(file);
        file = fopen(fileName, "a"); // open the file in "append" mode
    }

    // Get the current date and time
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(file, "%f, %d, %s,%s,%f, %f, %f, %f, %f\n", index_time, rank, date, dist_file, total_computing_time, BaB_computing_time, count_bab, r_best_cost, r_best_path); // add new data to the file

    fclose(file); // close the file
    return 0;
}

double power(double base, int exponent) {
    double result = 1;
    while (exponent > 0) {
        if (exponent & 1) { 
            result *= base;
        }
        base *= base;
        exponent >>= 1;
    }
    return result;
}