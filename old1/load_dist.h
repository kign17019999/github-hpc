//load_dist.h
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE_R 20

int (*save_mat(char* path))[SIZE_R] {
    FILE* file = fopen(path, "r");
    
    int number_of_city = 0;
    char line[256];
    fscanf(file, "%d", &number_of_city); 

    int row=0;
    int col=0;

    int (*dist)[SIZE_R] = malloc(sizeof(int[SIZE_R][SIZE_R]));


    while (fgets(line, sizeof(line), file)) {
        col=0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            if(atoi(token)>0){
                dist[row-1][col] = atoi(token);
            }
            token = strtok(NULL, " ");
            col++;
        }
        row++;
    }
    fclose(file);
    return dist;
}

int save_mat_size(char* path) {
    FILE* file = fopen(path, "r");
    char line[256];
    int row = 0;
    int number_of_city = 0;
    int col=0;
    fscanf(file, "%d", &number_of_city); 
    fclose(file);
    return number_of_city;
}
