#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "save_mat2.h"

#define MAX_CITIES 20
#define INFINITE INT_MAX

int n;
int dist[MAX_CITIES][MAX_CITIES];
int best_path[MAX_CITIES], best_path_cost = INFINITE;

void copy_path(int dest[], int src[]) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

void branch_and_bound(int path[], int path_cost, int visited[], int level) {
    if (level == n) {
        if (path_cost < best_path_cost) {
            best_path_cost = path_cost;
            copy_path(best_path, path);
            ////////////////////////////////////////////////////////////////////////
            printf("{END}  END Journy at last level: %d , it will be previouse level (%d) \n", level, level-1);
            ////////////////////////////////////////////////////////////////////////
        }
    } else {
        for (int i = 0; i < n; i++) {
                ////////////////////////////////////////////////////////////////////////
                printf("level: %d \n", level);
                printf("path: ");
                for (int i = 0; i < n; i++) {
                    printf("%d ", path[i]);
                }
                printf("\n");
                    
                printf("visited: ");
                for (int i = 0; i < n; i++) {
                    printf("%d ", visited[i]);
                }
                printf("\n");

                printf("path_cost: ");
                for (int i = 0; i < n; i++) {
                    printf("%d ", visited[i]);
                }
                printf("\n");
                ////////////////////////////////////////////////////////////////////////
            printf("@@@-0 visited[i]= %d \n",visited[i]);
            if (!visited[i]) {
                path[level] = i;
                visited[i] = 1;
                int new_cost = path_cost + dist[path[level - 1]][i];
                ////////////////////////////////////////////////////////////////////////
                printf("{%d} new_cost: %d | best cost: %d \n", i, new_cost, best_path_cost);
                ////////////////////////////////////////////////////////////////////////
                if (new_cost < best_path_cost) {
                    printf(">>> {%d} going to %d from %d\n", i, level+1, level);
                    branch_and_bound(path, new_cost, visited, level + 1);
                }
                ////////////////////////////////////////////////////////////////////////
                // print test for check whay will be happened if new_cost > best_path_cost
                else{
                    printf("!!! {%d} cost not good \n", i);
                }
                printf("<<< {%d} heading back from %d to %d\n", i, level, level-1);
                printf("@@@-1 visited[i]= %d \n",visited[i]);
                ////////////////////////////////////////////////////////////////////////
                visited[i] = 0;
            }
        }
    }
}

int main() {
    printf(">>>start 1 \n");
    char file_path[] = "input/dist4";
    n = save_mat_size(file_path);
    int (*raw_dist)[MAX_CITIES] = save_mat(file_path);
    printf(">>>finish read array from file \n");
    printf("------------------- \n");

    ////////////////////////////////////////////////////////////////////////
    for(int i=0; i<n-1 ; i++){
        for(int j=0 ; j<=i ; j++){
            printf("%d ", raw_dist[i][j]);
        }
        printf("\n");
    }    
    ////////////////////////////////////////////////////////////////////////
    
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

    for (int start = 0; start < n; start++) {
        int path[MAX_CITIES];
        int visited[MAX_CITIES] = {0};
        path[0] = start;
        visited[start] = 1;

        // Call branch_and_bound with the new starting point
        branch_and_bound(path, 0, visited, 1);

        ////////////////////////////////////////////////////////////////////////
        printf("path: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", path[i]);
        }
        printf("\n");
        
        printf("visited: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", visited[i]);
        }
        printf("\n");
        ////////////////////////////////////////////////////////////////////////
    }

    printf("Best path: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", best_path[i]);
    }
    printf("\nCost: %d", best_path_cost);


    
    free(raw_dist);
    free(dist);
    return 0;
}
