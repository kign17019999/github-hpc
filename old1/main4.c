#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_CITIES 20
#define INFINITE INT_MAX
#define START_CITIES 0

int *n;
int (*dist)[MAX_CITIES];
int *best_path;
int *best_path_cost = INFINITE;

int get_cities_info(char* file_path);
void branch_and_bound(int *path, int path_cost, int *visited, int level);

int main() {
    //set default array to the size of MAX_CITIES
    dist = malloc(sizeof(int[MAX_CITIES][MAX_CITIES]));
    best_path = malloc(sizeof(int[MAX_CITIES]));

    char *file_path = "input/dist6";
    get_cities_info(file_path);

    for(int i=0; i<n ; i++){
        for(int j=0 ; j<n ; j++) printf("%d ", dist[i][j]);
        printf("\n");
    }  
    
    int *path = malloc(MAX_CITIES * sizeof(int));
    int *visited = malloc(MAX_CITIES * sizeof(int));
    for(int i=0; i<MAX_CITIES; i++) visited[i]=0;
    
    // set starter city to city 1 (or 0 in array order) in path array at position 0
    path[0] = START_CITIES;

    // set visited city to 1 by setting visited array position 0 to 1
    visited[START_CITIES] = 1;

    branch_and_bound(path, 0, visited, 1);

    printf(">> Best path: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", best_path[i]);
    }
    printf("\n");
    printf(">> Cost: %d\n", best_path_cost);
    
    free(dist);
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

void branch_and_bound(int *path, int path_cost, int *visited, int level) {
    if (level == n) {
        if (path_cost < best_path_cost) {
            best_path_cost = path_cost;
            for (int i = 0; i < n; i++) best_path[i] = path[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (!visited[i]) {
                path[level] = i;
                visited[i] = 1;
                //int new_cost = path_cost + dist[path[level - 1]][i];
                int new_cost = path_cost + dist[i][path[level - 1]];
                if (new_cost < best_path_cost) {
                    branch_and_bound(path, new_cost, visited, level + 1);
                }
                visited[i] = 0;
            }
        }
    }
}