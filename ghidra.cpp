#include <stdio.h>
int add(int a, int b) {
    return a + b;
}
int main() {
    int secret = 1234;
    int result = add(secret, 4321);
    if (result == 5555)
        printf("correct\n");
    return 0;
}