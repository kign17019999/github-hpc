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
int (*level_from)[MAX_CITIES];
int (*level_to)[MAX_CITIES];
/*===================================================================*/

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level, int rank);
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
        char *df_file = "input/dist10";
        printf("[System] The default file (%s) will be used if no input is provided  \n", df_file);
        file_path  = df_file;
    }
    get_cities_info(file_path);
    
/*=============================[   1   ]======================================*/
    int size = 4;
/*=============================[   1   ]======================================*/

    level_from= malloc(sizeof(int[MAX_P][MAX_CITIES]));
    level_to = malloc(sizeof(int[MAX_P][MAX_CITIES]));

    // Initialize level_from and level_to to -1
    for (int i = 0; i < MAX_P; i++) {
        for (int j = 0; j < MAX_CITIES; j++) {
            level_from[i][j] = -1;
            level_to[i][j] = -1;
        }
    }

    // Divide the cities as evenly as possible among the processors
    int cities_per_processor = (n-1) / size;
    int remainder = (n-1) % size;
    int start = 0;
    int end = 0;
    int processor_counter = 0;
    for (int i = 0; i < n; i++) {
        if(start != START_CITIES){
            for (int j = 0; j < size; j++) {
                if(processor_counter >= size) {
                    processor_counter = 0;
                }
                level_from[processor_counter][i] = start;
                if (remainder > 0) {
                    end = start + cities_per_processor;
                    remainder--;
                } else {
                    end = start + cities_per_processor - 1;
                }
                level_to[processor_counter][i] = end;
                start = end + 1;
                processor_counter++;
            }
        }else{
            start++;
        }
    }

    printf("level_from...\n");
    for (int i=0; i<size; i++){
        for(int j=0; j<n; j++){
            printf("%d ", level_from[i][j]);
        }
        printf("\n");
    }
    printf("level_to...\n");
    for (int i=0; i<size; i++){
        for(int j=0; j<n; j++){
            printf("%d ", level_to[i][j]);
        }
        printf("\n");
    }
/*=============================[   1   ]======================================*/
/*=============================[   2   ]======================================*/

int way=1;
for(int i=1; i<n ;i++){
    way = (n-i)*way;
    if(i==n){
        for(int k=0; k<way; k++){
            level_from[k][i] = ;
            level_to[k][i] = n-1;
        }
        break;
    }
    if(way<size){
        for(int j=0; j<size;j++){
            level_from[j][i] = 0;
            level_to[j][i] = n-1;
        }
    }else if(way>=size){
        for(int j=0; j<size;j++){
            
            level_to[j][i] = n-1;
        }
        break;
    }
}


/*=============================[   2   ]======================================*/

    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited[i]=0;

    path[0] = START_CITIES;
    visited[START_CITIES] = 1;

    for(int i=0; i<size; i++){
        branch_and_bound(path, 0, visited, 1, i);
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
        }
    } else {
        for (int i = level_from[rank][level-1]; i < level_to[rank][level-1]; i++) {
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

int init_path[MAX_P][MAX_CITIES];
int init_cost[MAX_P];
int init_visited[MAX_P][MAX_CITIES];

int SIZE=4;
int init_level=1;
int cal_apth(){
    int fork=1;
    for(int level=1; level<n; level++){
        fork = fork*(n-level);
        if(fork >= SIZE){
            init_level=level;
        }
    }
}

void branch_and_bound_path(int *path, int path_cost, int *visited, int level, int rank) {
    if (level == init_level) {
        for(int i=0; i<init_level; i++){
            init_path[rank][i] = path[i];
        }
        init_cost[rank]=path_cost;
    } else {
        for (int i = 0; i < init_level; i++) {
            if (!visited[i]) {
                path[level] = i;
                visited[i] = 1;
                int new_cost = path_cost + dist[i][path[level - 1]];
                branch_and_bound_path(path, new_cost, visited, level + 1, rank);
                visited[i] = 0;
                rank++;
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