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
    printf("cur cost= %d | lvl=%d\n", path_cost, level);
    if (level == n) {
        printf("path: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", path[i]);
        }
        printf("| path_cost: %d\n", path_cost);        
        if (path_cost < best_path_cost) {
            best_path_cost = path_cost;
            copy_path(best_path, path);   
        }
    } else {
        for (int i = 0; i < n; i++) {
            printf(" [visit[%d]= %d\n", i, visited[i]);
            if (!visited[i]) {
                printf("lvl = %d \n", level);
                path[level] = i;
                visited[i] = 1;
                int new_cost = path_cost + dist[path[level - 1]][i];
                printf("[i=%d] new cost = %d/%d (jsut add: %d {dist[%d][%d]}) \n", i, new_cost, best_path_cost,dist[path[level - 1]][i],path[level - 1],i);
                printf("visited before BB: [%d %d %d %d] \n", visited[0],visited[1],visited[2],visited[3]);
                if (new_cost < best_path_cost) {
                //if (1==1) {
                    branch_and_bound(path, new_cost, visited, level + 1);
                }
                
                visited[i] = 0;
                printf("visited after BB: [%d %d %d %d] \n", visited[0],visited[1],visited[2],visited[3]);
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
    /////////
    for(int i=0; i<n ; i++){
        for(int j=0 ; j<n ; j++){
            printf("%d ", dist[i][j]);
        }
        printf("\n");
    }     
    ////////

    for (int start = 0; start < n; start++) {
        int path[MAX_CITIES];
        int visited[MAX_CITIES] = {0};
        path[0] = start;
        visited[start] = 1;
        branch_and_bound(path, 0, visited, 1);
        printf("-----------------------------------\n");
    }

    printf("Best path: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", best_path[i]);
    }
    printf("\nCost: %d", best_path_cost);


    
    //free(raw_dist);
    //free(dist);
    return 0;
}
