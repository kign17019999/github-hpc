#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
#define START_CITIES 0

int n;
int (*dist)[MAX_CITIES];
int *best_path;
int best_path_cost = INFINITE;

/*===================================================================*/
#define MAX_P 4
#define START_CITIES 0
int init_path[MAX_P][MAX_CITIES];
int init_cost[MAX_P];
int init_visited[MAX_P][MAX_CITIES];
int init_level;
int init_last_rank=0;
int init_rank=0;
int init_path_rank=0;
int fork=1;

int best_rank;
/*===================================================================*/

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);
void branch_and_bound_path(int *path0, int path_cost, int *visited0, int level, int size);
int save_result(char* dist_file, double computing_time);

int main(int argc, char *argv[]) {
    int tt=12;
    int bb=23;
    printf("xxxxxx= %d\n", 23-(tt/bb)*12);
    time_t start_t = time(NULL);

    dist = malloc(sizeof(int[MAX_CITIES][MAX_CITIES]));
    best_path = malloc(sizeof(int[MAX_CITIES]));

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
    get_cities_info(file_path);
    
/*=============================[   2   ]======================================*/
    int size = 5;
    int *path0 = malloc(MAX_CITIES * sizeof(int));
    int *visited0 = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited0[i]=0;
/*=============================[   2   ]======================================*/
    path0[0] = START_CITIES;
    visited0[START_CITIES] = 1;
    cal_init_level(size);
    branch_and_bound_path(path0, 0, visited0, 1, size);
    
    printf("init_level: %d\n", init_level);
    printf("init_last_rank: %d\n", init_last_rank);

/*=============================[   2   ]======================================*/

    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited[i]=0;

    path[0] = START_CITIES;
    visited[START_CITIES] = 1;

    for(int i=0; i<=init_last_rank; i++){
        for(int j=1; j<=init_level; j++){
            path[j] = init_path[i][j];
            visited[START_CITIES] = 1;
        }
        // printf("loop: ");
        // printf("%d ", i);
        // printf("\n");
        branch_and_bound(path, init_cost[i], visited, init_level, i);
    }
    
    time_t end_t = time(NULL);
    double computing_time = difftime(end_t, start_t);

    printf("[System] Result: \n");
    printf("  | Best_path : ");
    for (int i = 0; i < n; i++) {
        printf("%d ", best_path[i]);
    }
    printf("\n");
    printf("  | Best_path_cost : %d\n", best_path_cost);
    printf("  | Best_in_rank   : %d\n", best_rank);
    
    printf("[System] spent total : %f seconds\n", computing_time);
    
    free(dist);

    save_result(file_path, computing_time);

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
        if (path_cost < best_path_cost) {
            best_path_cost = path_cost;
            for (int i = 0; i < n; i++) best_path[i] = path[i];
            best_rank = rank;
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                path[level] = i;
                visited[i] = 1;
                int new_cost = path_cost + dist[i][path[level - 1]];
                if (new_cost < best_path_cost) {
                    branch_and_bound(path, new_cost, visited, level + 1, rank);
                }
                visited[i] = 0;
            }
        }
    }
}

int cal_init_level(int size){
    for(int level=1; level<n; level++){
        fork = fork*(n-level);
        if(fork >= size || level==n-1){
            init_level=level;
            break;
        }
    }
}

void branch_and_bound_path(int *path0, int path_cost, int *visited0, int level, int size) {
    //printf("r: %d, visit[1]: %d\n", rank, visited0[1]);
    if (level == init_level+1) {
        printf("rank %d : ", init_rank);
        for(int i=0; i<=init_level; i++){
            init_path[init_rank][i] = path0[i];
            init_visited[init_rank][i] = visited0[i];
            printf("%d ", init_path[init_rank][i]);
        }
        printf("\n");
        init_cost[init_rank]=path_cost;

        printf("init_last_rank %d\n", init_last_rank);

        if(init_rank>=init_last_rank){
            init_last_rank = init_rank;
            printf("change to %d \n", init_last_rank);
        }

        if(init_rank==size-1){
            init_rank=0;
            printf("rank new: %d\n", init_rank);
        }else{
            init_rank++;
            printf("rank new: %d\n", init_rank);
        }
    } else {
        for (int i = 0; i < n; i++) {
            //printf("lvl %d | iiii %d | vvvv: %d\n", level, i, visited0[i]);
            if (!visited0[i]) {
                // if(init_rank==size-1){
                //     init_rank==0;
                // }else{
                //     init_rank++;
                // }
                //printf("rank new: %d\n", init_rank);
                path0[level] = i;
                visited0[i] = 1;
                int new_cost = path_cost + dist[i][path0[level - 1]];
                branch_and_bound_path(path0, new_cost, visited0, level + 1, size);
                visited0[i] = 0;
                //printf("rank now: %d\n", rank);

                
            }
        }
    }
}

int save_result(char* dist_file, double computing_time) {
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

int factorial(int num) {
    int i, result = 1;
    for (i = 1; i <= num; i++) {
        result *= i;
    }
    return result;
}