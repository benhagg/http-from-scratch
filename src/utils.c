#include <stdlib.h>
#include <stdio.h>

void handle_error(int result, const char *message){
    if (result < 0){
        perror(message);
        exit(EXIT_FAILURE);
    }
}