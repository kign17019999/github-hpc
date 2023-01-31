#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#define MAX_CITIES 4
#define INFINITE INT_MAX
#define START_CITIES 0

int n;
int (*dist)[MAX_CITIES];
int *best_path;
int best_path_cost = INFINITE;

/*===================================================================*/
int (*init_path)[MAX_CITIES];
int *init_cost;
int (*init_visited)[MAX_CITIES];
int *init_path_rank;

int init_position=0;
int init_level=0;
int init_rank=0;
int num_init_path=1;

int best_rank;

int count_bb=0;
/*===================================================================*/

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);
void path_initiation(int *path_i, int path_cost, int *visited_i, int level, int size);
void level_initiation(int size);
int save_result(char* dist_file, double computing_time);

int main(int argc, char *argv[]) {
    printf("here\n");
    clock_t start = clock();

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
    printf("here2\n");
/*=============================[   2   ]======================================*/
    int size = 5;
    int *path_i = malloc(MAX_CITIES * sizeof(int));
    int *visited_i = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited_i[i]=0;
/*=============================[   2   ]======================================*/
    path_i[0] = START_CITIES;
    visited_i[START_CITIES] = 1;
    level_initiation(size);
    printf("here3 | n=%d | size=%d\n", n, size);
    //int (*dist)[MAX_CITIES];

    // int (*init_path)[MAX_CITIES]=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    // int *init_cost=malloc(num_init_path * sizeof(int));
    // int (*init_visited)[MAX_CITIES]=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    printf("here4 | num_init_patj=%d | init_level=%d\n", num_init_path, init_level);
    init_path=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    init_cost=malloc(num_init_path * sizeof(int));
    init_visited=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    init_path_rank=malloc(num_init_path * sizeof(int));
    path_initiation(path_i, 0, visited_i, 1, size);
    printf("here5\n");
/*=============================[   2   ]======================================*/

    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    //path[0] = START_CITIES;
    //visited[START_CITIES] = 1;
    //printf("here6\n");
    for(int i=0; i<num_init_path; i++){
        int rank=init_path_rank[i];
        //printf("here6-1\n");
        if(init_path_rank[i]==rank){
            //printf("here6-2\n");
            for(int j=0; j<n; j++){
                path[j] = init_path[i][j];
                visited[j] = init_visited[i][j];
            }
            //printf("here6-3\n");
            count_bb=0;
            branch_and_bound(path, init_cost[i], visited, init_level, rank);
            //printf("here6-4\n");
        }
        
    }
    //printf("here8\n");
    double computing_time = (double)(clock() - start) / CLOCKS_PER_SEC;

    printf("[System] Result: \n");
    printf("  | Best_path : ");
    for (int i = 0; i < n; i++) {
        printf("%d ", best_path[i]);
    }
    printf("\n");
    printf("  | Best_path_cost : %d\n", best_path_cost);
    //printf("  | Best_in_rank   : %d\n", best_rank);
    
    printf("[System] spent total : %f seconds\n", computing_time);
    
    //test//
    for(int i=0; i<size; i++){
        printf(">>>>>>>>rank:%d | cost=%d\n", i, best_path_cost[i]);
    }

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
    //test//
    if(count_bb==0) {
        printf("rank=%d | path=%d %d %d %d | visited=%d %d %d %d\n", rank, path[0], path[1], path[2], path[3], visited[0], visited[1], visited[2], visited[3]);
        //count_bb++;
    }
    //printf("here7-1\n");
    if (level == n) {
        //printf("here7-2\n");
        if(count_bb==1) printf("**rank=%d | path=%d %d %d %d | visited=%d %d %d %d\n", rank, path[0], path[1], path[2], path[3], visited[0], visited[1], visited[2], visited[3]);
        count_bb++;
        if (path_cost < best_path_cost) {
            best_path_cost = path_cost;
            for (int i = 0; i < n; i++) best_path[i] = path[i];
            best_rank = rank;
        }
    } else {
        //printf("here7-3\n");
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                //printf("here7-4\n");
                path[level] = i;
                visited[i] = 1;
                if(count_bb==0) printf("rank=%d | i=%d | path[3]=%d | visited[x]=%d\n", rank, i, path[level], visited[i]);
                //printf("here7-5\n");
                //printf("path[]: %d\n", path[level - 1]);
                //printf("i=%d level=%d\n", i, level);
                if(count_bb==0) {
                    printf("rank=%d | dist_p_c=%d\n", rank, dist[i][path[level - 1]]);
                    count_bb++;
                }
                int new_cost = path_cost + dist[i][path[level - 1]];
                //printf("here7-6\n");
                if (new_cost < best_path_cost) {
                    branch_and_bound(path, new_cost, visited, level + 1, rank);
                }
                visited[i] = 0;
            }
        }
    }
}

void level_initiation(int size){
    for(int level=1; level<n; level++){
        num_init_path = num_init_path*(n-level);
        if(num_init_path >= size || level==n-1){
            if(level==n-1){
                init_level=level;
            }else{
                init_level=level+1;
            }
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