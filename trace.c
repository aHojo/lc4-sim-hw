/*
 * trace.c: location of main() to start the simulator
 */

#include "loader.h"
#include <unistd.h>

int main(int argc, char **argv)
{
    // this assignment does NOT require dynamic memory - do not use 'malloc() or free()'
    // create a local varible in main to represent the CPU and then pass it around
    // to subsequent functions using a pointer!

    /* notice: there are test cases for part 1 & part 2 here on codio
    they are located under folders called: p1_test_cases & p2_test_cases

    once you code main() to open up files, you can access the test cases by typing:
    ./trace p1_test_cases/divide.obj   - divide.obj is just an example

    please note part 1 test case files have an OBJ and a TXT file
    the .TXT shows you what's in the .OBJ files

    part 2 test case files have an OBJ, an ASM, and a TXT files
    the .OBJ is what you must read in
    the .ASM is the original assembly file that the OBJ was produced from
    the .TXT file is the expected output of your simulator for the given OBJ file
    */
    char* outfile; // File to be written to
    FILE *outputfp; // File pointer
    MachineState *machine_state=malloc(sizeof(MachineState));

    if (!(argc >= 3)) {
        printf("usage: ./trace <filename> <...objectfiles>\n");
    }

    // Check if all of the object files exist
    for (int i =2; i < argc; i++) {
        if (access(argv[i], F_OK) == -1)
        {
            printf("Error 1: %s does not exist\n", argv[i]);
            return -1;
        }
    }
    outfile = argv[1];

    
    for (int i =2; i < argc; i++) {
        int rC = ReadObjectFile(argv[i], machine_state);
        if (rC != 0) {
            printf("Error 3: Failed to parse object file: %s\n", argv[i]);
        }
        for (size_t i = 0; i < 65536; i++)
        {
            printf("Address: %ld Contents: %04x \n",i,machine_state->memory[i]);
        }
        
    }

    free(machine_state);

    return 0;
}