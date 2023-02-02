#include <graphics.h>
#include <stdio.h>
#include <conio.h>

void main() {
    int gd = DETECT, gm;
    int i, num_points;
    int x[100], y[100];
    
    printf("Enter the number of points: ");
    scanf("%d", &num_points);

    for (i = 0; i < num_points; i++) {
        printf("Enter x[%d] and y[%d]: ", i, i);
        scanf("%d%d", &x[i], &y[i]);
    }

    initgraph(&gd, &gm, "C:\\TC\\BGI");

    for (i = 0; i < num_points - 1; i++) {
        line(x[i], y[i], x[i + 1], y[i + 1]);
    }

    getch();
    closegraph();
}
