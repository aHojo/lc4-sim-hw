/*
 * loader.c : Defines loader functions for opening and loading object files
 */

#include "loader.h"

// memory array location
unsigned short memoryAddress;

// Function signatures
int parse_header(unsigned short* address, unsigned short *length, unsigned short *line, unsigned short *index, FILE *file); // function to get address and number of lines.
void load_memory(unsigned short address, unsigned short length, MachineState *machine ,FILE *file); // load memory into machine state.
unsigned short int swap_two_bytes(unsigned short int num); // Swap the endianness
int read_byte(unsigned short length, FILE *file); // read a byte from the file passed in.

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
  unsigned short line;
  unsigned short index;
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
      parse_header(&address, &length, NULL, NULL, file);
      load_memory(address, length, CPU, file);
      break;
    // Data Header
    case 0xDADA:
      parse_header(&address, &length, NULL, NULL, file);
      load_memory(address, length, CPU, file);
      break;
    // Symbol Header
    case 0xC3B7:
      parse_header(&address, &length, NULL, NULL, file);
      read_byte(length, file);
      break;
    // File Name Header
    case 0xF17E:
      parse_header(NULL, &length, NULL, NULL, file);
      read_byte(length, file);
      break;
    // Line Number Header
    case 0x715E:
      parse_header(&address, &length, &line, &index, file);
      break;
    }
  }
  fclose(file);
  return 0;
}

int parse_header(unsigned short *address, unsigned short *length, unsigned short *line, unsigned short *index ,FILE *file)
{
  int exit_status = 0;

  if (address!= NULL){
    exit_status = fread(address, sizeof(unsigned short), 1, file);
    *address = swap_two_bytes(*address);
  }
  // line and index are optional and only used for the line number header
  if (line != NULL){
    exit_status = fread(line, sizeof(unsigned short), 1, file);
    *line = swap_two_bytes(*line);
  }

  if (index != NULL){
    exit_status = fread(index, sizeof(unsigned short), 1, file);
    *index = swap_two_bytes(*index);
  }
  

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

int read_byte(unsigned short length, FILE *file)
{
  unsigned char byte;
  printf("Byte: ");
  for (int i = 0; i < length; i++)
  {
    byte = fgetc(file);
    printf("%c", byte);
  }
  printf("\n");

  
  return 0;
}