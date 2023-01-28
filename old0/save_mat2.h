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
    // printf("finish create array malloc\n");


    while (fgets(line, sizeof(line), file)) {
        //printf("row %d \n", row);
        col=0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            if(atoi(token)>0){
                //printf("token = %s \n", token);
                dist[row-1][col] = atoi(token);
                //printf("elecment in array = %d | %d:%d \n", dist[row-1][col], row, col);
            }
            token = strtok(NULL, " ");
            col++;
        }
        //printf(" >> row=%d | col=%d \n", row, col-1);
        row++;
    }
    // printf("finish while\n");
    fclose(file);
    //free(file);
    //free(line);
    //printf("elecment in array = %d \n", dist[0][0]);
    //printf("...done... \n");
    // printf("finish save mat\n");
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
    //printf("...done... \n");
    //free(file);
    //free(line);
    // printf("finish save size\n");
    return number_of_city;
}
