#include <stdio.h>
#include <stdlib.h>
#include "error_handling.h"

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}