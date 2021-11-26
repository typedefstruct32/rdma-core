#include <stdio.h>
#include <math.h>
#include <stdlib.h>
 
char* itoa(int num, char* buffer, int base) {
    int curr = 0;
 
    if (num == 0) {
        // Base case
        buffer[curr++] = '0';
        buffer[curr] = '\0';
        return buffer;
    }
 
    int num_digits = 0;
 
    if (num < 0) {
        if (base == 10) {
            num_digits ++;
            buffer[curr] = '-';
            curr ++;
            // Make it positive and finally add the minus sign
            num *= -1;
        }
        else
            // Unsupported base. Return NULL
            return NULL;
    }
 
    num_digits += (int)floor(log(num) / log(base)) + 1;
 
    // Go through the digits one by one
    // from left to right
    while (curr < num_digits) {
        // Get the base value. For example, 10^2 = 1000, for the third digit
        int base_val = (int) pow(base, num_digits-1-curr);
 
        // Get the numerical value
        int num_val = num / base_val;
 
        char value = num_val + '0';
        buffer[curr] = value;
 
        curr ++;
        num -= base_val * num_val;
    }
    buffer[curr] = '\0';
    return buffer;
}
 
int main() {
    int a = 1234;
    char buffer[256];
    char* s = "1234";
    if (itoa(a, buffer, 10) != NULL) {
        printf("Input = %d, base = %d, Buffer = %s\n", a, 10, buffer);
    }
    int b = -231;
    if (itoa(b, buffer, 10) != NULL) {
        printf("Input = %d, base = %d, Buffer = %s\n", b, 10, buffer);
    }
    int c = 10;
    if (itoa(c, buffer, 2) != NULL) {
        printf("Input = %d, base = %d, Buffer = %s\n", c, 2, buffer);
    }
    return 0;
}