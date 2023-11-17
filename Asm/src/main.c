#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv) {
    FILE *fptr;
    uint16_t program[0x7e80] = {};
    if ((fptr = fopen(argv[1], "rb")) == NULL){
       printf("Error! opening file");

       // Program exits if the file pointer returns NULL.
       exit(1);
    }

    fread(&program, sizeof(program), 1, fptr);
    for (int i = 0; i < 10; i++) {
        printf("0x%x ", program[i]);
    }
    fclose(fptr);
    return 0;

}