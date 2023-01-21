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
#define NUM_RESULT 7

// mode_send 0 = send Dist by Bcast
// mode_send 1 = send Dist by Ibcast
// mode_send 2 = send Dist by Send & Recv
// mode_send 3 = send Dist by Isend & Irecv
int mode_send = 3;

// mode_gather 0 = gather by Allgather
// mode_gather 1 = gather by Send & Recv
int mode_gather = 1;

int n;
int (*dist)[MAX_CITIES];
int (*best_path)[MAX_CITIES];
int *best_path_cost;

double (*result)[NUM_RESULT];
double count_bb=0;

int all_best_cost = INFINITE;

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank, int size);
int save_result(double index_time, int rank, char *dist_file, double total_computing_time, double sending_time, double BaB_computing_time, double gathering_time, double count_bab, double r_best_cost, double r_best_path);
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
        if (rank==ROOT) printf("[ROOT] The default file (%s) will be used if no input is provided  \n", df_file);
        file_path  = df_file;
    }
    

    if(rank==ROOT){
        get_cities_info(file_path);
        printf("[ROOT] number of cities = %d \n", n);
    }

    double start_time2 = MPI_Wtime();
    if(rank==ROOT){
        if(mode_send==0){
            // mode 0 = send Dist by Bcast
            printf("[ROOT] mode_send = 0 (send Dist by Bcast) \n");
            MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
            MPI_Bcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD);

        }else if(mode_send==1){
            // mode 1 = send Dist by Ibcast
            printf("[ROOT] mode_send = 1 (send Dist by Ibcast) \n");
            MPI_Ibcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD, &request1);
            MPI_Ibcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD, &request2);

        }else if(mode_send==2){
            // mode 2 = send Dist by Send & Recv
            printf("[ROOT] mode_send = 2 (send Dist by Send & Recv) \n");
            for (int i = 1; i < size; i++) {
                MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(dist, MAX_CITIES*MAX_CITIES, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

        }else if(mode_send==3){
            // mode 3 = send Dist by Isend & Irecv
            printf("[ROOT] mode_send = 3 (send Dist by Isend & Irecv) \n");
            for (int i = 1; i < size; i++) {
                    MPI_Isend(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request1);
                    MPI_Isend(dist, MAX_CITIES*MAX_CITIES, MPI_INT, i, 0, MPI_COMM_WORLD, &request2);
            }

        }
    }else {
        if(mode_send==0){
            // mode 0 = send Dist by Bcast
            MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
            MPI_Bcast(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, MPI_COMM_WORLD);
        }else if(mode_send==1){
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
        
        }else if(mode_send==2){
            // mode 2 = send Dist by Send & Recv
            MPI_Recv(&n, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        }else if(mode_send==3){
            // mode 3 = send Dist by Isend & Irecv
            MPI_Irecv(&n, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &request1);
            MPI_Irecv(dist, MAX_CITIES*MAX_CITIES, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &request2);
            MPI_Wait(&request1, MPI_STATUS_IGNORE);
            MPI_Wait(&request2, MPI_STATUS_IGNORE);

        }
    }
    double end_time2 = MPI_Wtime();

    double start_time3 = MPI_Wtime();

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
	    branch_and_bound(path, current_cost, visited, 2, rank, size);
    }

    double end_time3 = MPI_Wtime();

    //MPI_Barrier(MPI_COMM_WORLD);

    double start_time4 = MPI_Wtime();

    int send_buf_cost = best_path_cost[rank];
    int row_to_gather[MAX_CITIES];
    for (int i = 0; i < MAX_CITIES; i++) {
        row_to_gather[i] = best_path[rank][i];
    }

    if(mode_gather==0){
        // mode_gather 0 = gather by Allgather
        if(rank==ROOT) printf("[ROOT] mode_gather = 0 (gather by Allgather) \n");
        MPI_Allgather(&send_buf_cost, 1, MPI_INT, best_path_cost, 1, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather(&row_to_gather, MAX_CITIES, MPI_INT, best_path, MAX_CITIES, MPI_INT, MPI_COMM_WORLD);
    }else if(mode_gather==1){
        // mode_gather 1 = gather by Send & Recv
        if(rank==ROOT){
            printf("[ROOT] mode_gather = 1 (gather by Send & Recv) \n");
            for(int i=1; i<size; i++){
                MPI_Recv(&best_path_cost[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            for(int i=1; i<size; i++){
                MPI_Recv(&row_to_gather, MAX_CITIES, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for(int j=0; j<n; j++){
                    best_path[i][j]=row_to_gather[j];
                }
            }
        }else{
            MPI_Send(&send_buf_cost, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
            MPI_Send(&row_to_gather, MAX_CITIES, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
        }
    }
    double end_time4 = MPI_Wtime();

    //MPI_Barrier(MPI_COMM_WORLD);

    if(rank==ROOT){
        int min_dist=INFINITE;
        int index_best_path;
        for(int i=0; i<n; i++){
            if(best_path_cost[i]<min_dist){
                index_best_path = i;
                min_dist = best_path_cost[i];
            }
        }

        printf("[ROOT] best of the best is in rank %d, \n", index_best_path);
        printf("  | best_path: ");
        for(int i = 0; i < n ; i++){
            printf("%d ", best_path[index_best_path][i]);
        }
        printf("\n");
        printf("  | best_path_cost: %d \n", best_path_cost[index_best_path]);
   }

    double end_time1 = MPI_Wtime();

    //MPI_Barrier(MPI_COMM_WORLD);

    double total_computing_time = end_time1 - start_time1;
    double sending_time = end_time2 - start_time2;
    double BaB_computing_time = end_time3 - start_time3;
    double gathering_time = end_time4 - start_time4;
    if (rank ==ROOT){
        printf("[ROOT] spent total : %f seconds\n", total_computing_time);
        printf("[ROOT] spent Send  : %f seconds\n", sending_time);
        printf("[ROOT] spent BaB   : %f seconds\n", BaB_computing_time);
        printf("[ROOT] spent Gather: %f seconds\n", gathering_time);
    }
    
    result = malloc(sizeof(double[size][NUM_RESULT]));
    result[rank][0] = total_computing_time;
    result[rank][1] = sending_time;
    result[rank][2] = BaB_computing_time;
    result[rank][3] = gathering_time;
    result[rank][4] = count_bb;
    result[rank][5] = best_path_cost[rank];
    double double_path = power(10, 2*n)*404;
    for(int i=0; i<n; i++){
        double_path+=power(10, (n-i-1)*2)*best_path[rank][i];
    }
    result[rank][6] = double_path;
    
    double row_to_gather_result[NUM_RESULT];
    for (int i = 0; i < NUM_RESULT; i++) {
        row_to_gather_result[i] = result[rank][i];
    }

    MPI_Allgather(row_to_gather_result, NUM_RESULT, MPI_DOUBLE  , result, NUM_RESULT, MPI_DOUBLE  , MPI_COMM_WORLD);

    if(rank==ROOT){
        double index_time = MPI_Wtime();
        for(int i=0; i<size;i++){
            save_result(index_time, i, file_path, result[i][0], result[i][1], result[i][2], result[i][3], result[i][4], result[i][5], result[i][6]);
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

// void branch_and_bound_temp(int *path, int path_cost, int *visited, int level, int rank) {
//     count_bb+=1;
//     if (level == n) {
//         for(int i=0; i<MAX_CITIES; i++){
//             if(all_best_cost>best_path_cost[i]) all_best_cost=best_path_cost[i];
//         }
// 	    if (path_cost < all_best_cost) {
//             best_path_cost[rank] = path_cost;
//             for (int i = 0; i < n; i++) best_path[rank][i] = path[i];
//         }
//     } else {
//         for (int i = 0; i < n; i++) {
//             if (!visited[i]) {
//                     path[level] = i;
//                     visited[i] = 1;
//                     int new_cost = path_cost + dist[i][path[level - 1]];        
//             if (new_cost < all_best_cost) {
//                         branch_and_bound(path, new_cost, visited, level + 1, rank);
//                     }
//                     visited[i] = 0;
// 	        }
//         }
//     }
// }

void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank, int size) {
    count_bb+=1;
    if (level == n) {
        // for(int i=0; i<MAX_CITIES; i++){
        //     if(all_best_cost>best_path_cost[i]) all_best_cost=best_path_cost[i];
        // }
        if (path_cost < all_best_cost) {
            best_path_cost[rank] = path_cost;
            all_best_cost = path_cost;
            for (int i = 0; i < n; i++) best_path[rank][i] = path[i];

            for(int i = 0; i < size; i++) {
                if(i != rank) {
                    MPI_Request request;
                    MPI_Isend(&path_cost, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
                }
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                    path[level] = i;
                    visited[i] = 1;
                    int new_cost = path_cost + dist[i][path[level - 1]];
            
            if (new_cost < all_best_cost) {
                        branch_and_bound(path, new_cost, visited, level + 1, rank, size);
                    }
                    visited[i] = 0;
            }
        }
    }
    MPI_Status status;
    MPI_Request request;
    for(int i=0; i<size;i++){
        if(i!=rank){
            int incoming_cost;
            int flag;
            MPI_Irecv(&incoming_cost, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
            if(MPI_Test(&request, &flag, &status)) {
                if(incoming_cost < all_best_cost) {
                    all_best_cost = incoming_cost;
                }   
            }
        }
    }
}

int save_result(double index_time, int rank, char *dist_file, double total_computing_time, double sending_time, double BaB_computing_time, double gathering_time, double count_bab, double r_best_cost, double r_best_path) {
    FILE *file;
    char date[20];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char* fileName="result_parallel_RTcost.csv";
    file = fopen(fileName, "r"); // open the file in "read" mode
    if (file == NULL) {
        file = fopen(fileName, "w"); //create new file in "write" mode
        fprintf(file, "index_time, rank, date-time, dist file, total_computing_time (s), BaB_computing_time (s), gathering_time (s), count_BaB, best_cost, best_path, mode_send, mode_gather\n"); // add header to the file
    } else {
        fclose(file);
        file = fopen(fileName, "a"); // open the file in "append" mode
    }

    // Get the current date and time
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(file, "%f, %d, %s, %s, %f, %f, %f, %f, %f, %f, %f, %d, %d\n", index_time, rank, date, dist_file, total_computing_time, sending_time, BaB_computing_time, gathering_time, count_bab, r_best_cost, r_best_path, mode_send, mode_gather); // add new data to the file

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