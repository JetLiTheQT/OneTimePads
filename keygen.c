#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_KEY_LEN 100000

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <key_len>\n", argv[0]);
        return 1;
    }
    int key_len = atoi(argv[1]);
    if (key_len > MAX_KEY_LEN) {
        printf("Error: key length cannot exceed %d\n", MAX_KEY_LEN);
        return 1;
    }
    srand(time(NULL));
    for (int i = 0; i < key_len; i++) {
        putchar('A' + (rand() % 26));
    }
    putchar('\n');
    return 0;
}