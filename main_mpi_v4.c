#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
#define ROOT 0
#define NUM_RESULT 7

#define LOOP_ALL_FOR_1ST_CITY_is_true 1
int START_CITIES=0;

// mode_send 0 = send Dist by Bcast
// mode_send 1 = send Dist by Ibcast
// mode_send 2 = send Dist by Send & Recv
// mode_send 3 = send Dist by Isend & Irecv
int mode_send = 3;

// mode_gather 0 = gather by Allgather
// mode_gather 1 = gather by Send & Recv
int mode_gather = 1;

//path variable
int n;
int (*dist)[MAX_CITIES];
int (*best_path)[MAX_CITIES];
int *best_path_cost;

//city initiation variable
int (*init_path)[MAX_CITIES];
int *init_cost;
int (*init_visited)[MAX_CITIES];
int *init_path_rank;
int init_position=0;
int init_level=0;
int init_rank=0;
int num_init_path=1;

//other variable
double (*result)[NUM_RESULT];
double count_bb=0;

int get_cities_info(char* file_path);
void send_data_to_worker(int rank, int size);
void path_initiation(int *path_i, int path_cost, int *visited_i, int level, int size);
void level_initiation(int size);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);
int save_result(double index_time, int rank, char *dist_file, double total_computing_time, double sending_time, double BaB_computing_time, double gathering_time, double count_bab, double r_best_cost, double r_best_path);
double power(double base, int exponent);

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);

    double start_time1 = MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request request1, request2;

    //allocation path variable
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
    send_data_to_worker(rank, size);
    double end_time2 = MPI_Wtime();

    double start_time3 = MPI_Wtime();

    for(int i=0; i<MAX_CITIES; i++){
        best_path_cost[i]=INFINITE;
    }

    //path and visit for initiation
    int *path_i = malloc(MAX_CITIES * sizeof(int));
    int *visited_i = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited_i[i]=0;
    path_i[0] = START_CITIES;
    visited_i[START_CITIES] = 1;
    level_initiation(size);

    //path initiation
    init_path=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    init_cost=malloc(num_init_path * sizeof(int));
    init_visited=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    init_path_rank=malloc(num_init_path * sizeof(int));
    path_initiation(path_i, 0, visited_i, 1, size);    

    //path and visit for branch_and_bound
    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    path[0] = START_CITIES;
    visited[START_CITIES] = 1;
    for(int i=0; i<num_init_path; i++){
        if(init_path_rank[i]==rank){
            for(int j=1; j<n; j++){
                path[j] = init_path[i][j];
                visited[j] = init_visited[i][j];
            }
            branch_and_bound(path, init_cost[i], visited, init_level, rank);
        }
        
    }

    double end_time3 = MPI_Wtime();

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

void send_data_to_worker(int rank, int size){
    MPI_Request request1, request2;
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
}

void level_initiation(int size){
    for(int level=1; level<n; level++){
        num_init_path = num_init_path*(n-level);
        if(num_init_path >= size || level==n-1){
            init_level=level+1;
            break;
        }
    }
}

void path_initiation(int *path_i, int path_cost, int *visited_i, int level, int size) {
    if (level == init_level) {
        for(int i=0; i<n; i++){
            init_path[init_position][i] = path_i[i];
            init_visited[init_position][i] = visited_i[i];
        }
        init_cost[init_position]=path_cost;
        init_path_rank[init_position]=init_rank;
        init_position++;

        if(init_rank==size-1){
            init_rank=0;
        }else{
            init_rank++;
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited_i[i]) {
                path_i[level] = i;
                visited_i[i] = 1;
                int new_cost = path_cost + dist[i][path_i[level - 1]];
                path_initiation(path_i, new_cost, visited_i, level + 1, size);
                visited_i[i] = 0;
            }
        }
    }
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

int save_result(double index_time, int rank, char *dist_file, double total_computing_time, double sending_time, double BaB_computing_time, double gathering_time, double count_bab, double r_best_cost, double r_best_path) {
    FILE *file;
    char date[20];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char* fileName="result_parallel.csv";
    file = fopen(fileName, "r"); // open the file in "read" mode
    if (file == NULL) {
        file = fopen(fileName, "w"); //create new file in "write" mode
        fprintf(file, "index_time, rank, date-time, dist file, total_computing_time (s), sending_time (s), BaB_computing_time (s), gathering_time (s), count_BaB, best_cost, best_path, mode_send, mode_gather\n"); // add header to the file
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