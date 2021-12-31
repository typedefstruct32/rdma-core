#include <stdio.h>

int main() {
    int *p;
    p = (int*)malloc(sizeof(int) * 1);
    free(p);
    free(p);
}