/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char* filename, MachineState* CPU)
{
  unsigned short bytes;
  unsigned short header;
  unsigned short address;
  unsigned short length;
  FILE *file;

  file = fopen(filename, "rb"); // open the object file for reading.

  if (file == NULL) {
    printf("Error: Could not open file -> %s\n", filename);
    return 3;
  }

  while(fread(&bytes, sizeof(unsigned short), 1, file) == 1) {
    //printf("%04x: \n", swap_two_bytes(bytes));

    switch(swap_two_bytes(bytes)) {
      case 0xCADE:
        printf("%04x: \n", swap_two_bytes(bytes));
      break;
      case 0xDADA:
        printf("%04x: \n", swap_two_bytes(bytes));
      break;
      case 0xC3B7:
        printf("%04x: \n", swap_two_bytes(bytes));
      break;
      case 0xF17E:
        printf("%04x: \n", swap_two_bytes(bytes));
      break;
      case 0x715E:
        printf("%04x: \n", swap_two_bytes(bytes));
      break;


    }
  }

  return 0;
}




unsigned short int swap_two_bytes(unsigned short int num)
{
    unsigned short int endianSwitch = (((num & 0x00FF) << 8) | ((num & 0xFF00) >> 8));
    return endianSwitch;
}