#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "save_mat2.h"

int factorial(int n) {
    if (n == 0) {
        return 1;
    }
    return n * factorial(n-1);
}

int main(){
    char path[] = "input/dist4";
    int number_of_city = save_mat_size(path);
    int (*dist)[20*20] = save_mat(path);
    
    int size_d = sizeof(*dist)/ sizeof(int);
    printf("number of city = %d \n", number_of_city);
    printf("size of matrix = %d \n", size_d);
    
    for(int i=0; i<number_of_city-1 ; i++){
        for(int j=0 ; j<=i ; j++){
            //printf("%d ", *(*(dist+i)+j));
            printf("%d ", dist[i][j]);
        }
        printf("\n");
    }


    // int min_dist = 0;
    // int start = 1;
    // int stop = 4;
    // int round = factorial(number_of_city-1);
    // int visited[number_of_city-1] = {0};

    // for(int i=0; i<round ; i++){
        
    // }
    
    // int result = factorial(4-1);
    // printf("%d \n", result);


    free(dist);
    return 0;
}