/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

/*
 * Read an object file and modify the machine state as described in the writeup
 */
int ReadObjectFile(char *filename, MachineState *CPU)
{
  unsigned short swappedBytes;
  unsigned short bytes;
  unsigned short header;
  unsigned short address;
  unsigned short length;
  FILE *file;

  file = fopen(filename, "rb"); // open the object file for reading.

  if (file == NULL)
  {
    printf("Error: Could not open file -> %s\n", filename);
    return 3;
  }

  while (fread(&bytes, sizeof(unsigned short), 1, file) == 1)
  {
    //printf("%04x: \n", swap_two_bytes(bytes));
    swappedBytes = swap_two_bytes(bytes);
    switch (swappedBytes)
    {
    // Code Header
    case 0xCADE:
      parse_header(&address, &length, file);
      load_memory(address, length, CPU, file);
      printf("%04x: Address: %04x, WordBody: %04x\n", swappedBytes, address, length);
      break;
    // Data Header
    case 0xDADA:
      parse_header(&address, &length, file);
      load_memory(address, length, CPU, file);
      printf("%04x: Address: %04x, WordBody: %04x\n", swappedBytes, address, length);
      break;
    // Symbol Header
    case 0xC3B7:
      parse_header(&address, &length, file);
      printf("%04x: Address: %04x, WordBody: %04x\n", swappedBytes, address, length);
      break;
    // File Name Header
    case 0xF17E:
      parse_header(&address, &length, file);
      printf("%04x: Address: %04x, WordBody: %04x\n", swappedBytes, address, length);
      break;
    // Line Number Header
    case 0x715E:
      parse_header(&address, &length, file);
      printf("%04x: Address: %04x, WordBody: %04x\n", swappedBytes, address, length);
      break;
    }
  }
  fclose(file);
  return 0;
}

int parse_header(unsigned short *address, unsigned short *length, FILE *file)
{
  int exit_status = 0;

  exit_status = fread(address, sizeof(unsigned short), 1, file);
  *address = swap_two_bytes(*address);

  exit_status = fread(length, sizeof(unsigned short), 1, file);

  *length = swap_two_bytes(*length);

  return exit_status;
}

void load_memory(unsigned short address, unsigned short length, MachineState *machine, FILE *file)
{
  unsigned short unswappedBytes;
  for (int i = 0; i < length; i++)
  {
    fread(&unswappedBytes, sizeof(unsigned short), 1, file);
    machine->memory[address + i] = swap_two_bytes(unswappedBytes);
  }
}

unsigned short int swap_two_bytes(unsigned short int num)
{
  unsigned short int endianSwitch = (((num & 0x00FF) << 8) | ((num & 0xFF00) >> 8));
  return endianSwitch;
}