#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <string.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
#define START_CITIES 3

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
/*===================================================================*/

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);
void path_initiation(int *path_i, int path_cost, int *visited_i, int level, int size);
void level_initiation(int size);
int save_result(char* dist_file, double computing_time);

int main(int argc, char *argv[]) {
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
        char *df_file = "input/dist5";
        printf("[System] The default file (%s) will be used if no input is provided  \n", df_file);
        file_path  = df_file;
    }
    get_cities_info(file_path);
    
/*=============================[   2   ]======================================*/
    int size = 5;
    int *path_i = malloc(MAX_CITIES * sizeof(int));
    int *visited_i = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited_i[i]=0;
/*=============================[   2   ]======================================*/
    path_i[0] = START_CITIES;
    visited_i[START_CITIES] = 1;
    level_initiation(size);

    int (*dist)[MAX_CITIES];

    // int (*init_path)[MAX_CITIES]=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    // int *init_cost=malloc(num_init_path * sizeof(int));
    // int (*init_visited)[MAX_CITIES]=malloc(sizeof(int[num_init_path][MAX_CITIES]));

    init_path=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    init_cost=malloc(num_init_path * sizeof(int));
    init_visited=malloc(sizeof(int[num_init_path][MAX_CITIES]));
    init_path_rank=malloc(num_init_path * sizeof(int));
    path_initiation(path_i, 0, visited_i, 1, size);

/*=============================[   2   ]======================================*/

    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    //path[0] = START_CITIES;
    //visited[START_CITIES] = 1;
    for(int i=0; i<num_init_path; i++){
        int rank=init_path_rank[i];
        if(init_path_rank[i]==rank){
            for(int j=0; j<n; j++){
                path[j] = init_path[i][j];
                visited[j] = init_visited[i][j];
            }
            branch_and_bound(path, init_cost[i], visited, init_level, rank);
        }
        
    }
    
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