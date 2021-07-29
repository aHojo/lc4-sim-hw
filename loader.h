/*
 * loader.h: Declares loader functions for opening and loading object files
 */

#include <stdio.h>
#include "LC4.h"

// Read an object file and modify the machine state as described in the writeup
int ReadObjectFile(char* filename, MachineState* CPU);
int parse_header(unsigned short* address, unsigned short *length, unsigned short *line, unsigned short *index, FILE *file); // function to get address and number of lines.
void load_memory(unsigned short address, unsigned short length, MachineState *machine ,FILE *file); // load memory into machine state.
unsigned short int swap_two_bytes(unsigned short int num); // Swap the endianness
int read_byte(unsigned short length, FILE *file); // read a byte from the file passed in.