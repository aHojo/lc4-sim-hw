/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>
#include <string.h>

// Handle the CONST opcode.
void SetConst(MachineState *CPU, FILE *output);

// Function to update our program controller
void UpdatePC(MachineState *CPU, unsigned short handler);

// Fills string with binary representation.
void intToB(char *); // Always gives my the revers order

/*
MACROS TO GRAB THE PIECES OF THE INSTRUCTIONS
*/

#define OPCODE(I) ((I) >> 12)           // opcode
#define INSN_11_9(I) (((I) >> 9) & 0x7) // get rd
#define INSN_8_6(I) (((I) >> 6) & 0x7)  // get rs
#define INSN_2_0(I) ((I)&0x7)           // get Rt
#define INSN_8_0(I) ((I)&0x1FF)         // get IMM9
#define INSN_7_0(I) ((I)&0xFF)          // get IMM 7
#define INSN_5_3(I) (((I) >> 3) & 0x7)
#define INSN_8_7(I) (((I) >> 7) & 0x3)
#define INSN_5_0(I) ((I)&0x3F)
#define INSN_11(I) (((I) >> 11) & 0x1)
#define INSN_5_4(I) (((I) >> 4) & 0x3)
#define INSN_4_0(I) ((I)&0x1F)
#define INSN_3_0(I) ((I)&0xF)
#define INSN_6_0(I) ((I)&0x7F)
#define INSN_10_0(I) ((I)&0x7FF)
#define INSN_15(I) ((I) >> 15)
#define INSN_5(I) (((I) >> 5) & 0x1)

//  Instruction we are currently executing.
unsigned short int instruction;
unsigned short int lastRegWritten;
/*
 * Reset the machine state as Pennsim would do
 */
void Reset(MachineState *CPU)
{
    // Values of PennSIM when before load
    CPU->PSR = 0x8002;
    CPU->PC = 0x8200;

    for (int i = 0; i < 8; i++)
    {
        CPU->R[i] = 0;
    }
    for (int j = 0; j < 65535; j++)
    {
        CPU->memory[j] = 0;
    }
}

/*
 * Clear all of the control signals (set to 0)
 */
void ClearSignals(MachineState *CPU)
{
    CPU->rsMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;

    CPU->regInputVal = 0;
    CPU->NZPVal = 0;
    CPU->dmemAddr = 0;
    CPU->dmemValue = 0;
}

/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState *CPU, FILE *output)
{

    char binary[16];
    
    /*
    1.the current PC
    2.the current instruction (written in binary)
    3.the register file WE
    4.if regFileWE is high, which register is being written to
    5.if regFileWEis high, what value is being written to the register file
    6.the NZP WE
    7.if NZP WE is high, what value is being written to the NZP register
    8.the data WE
    9.if data WE is high, the data memory address
    10.if data WE is high, what value is being loaded or stored into memory
    */

    intToB(binary);

    // printf(output, "%04x %s", CPU->PC, instruction);
    fprintf(output, "%04x ", CPU->PC);

    // Print the binary representation of the instruction to the file.
    for (int i = 15; i >= 0; i--)
    {
        if ((unsigned short int)binary[i] == 48)
        {
            fprintf(output, "%d", 0);
        }
        else
            fprintf(output, "%d", 1);
        {
        }
    }
    fputs(" ", output);

    /*
    REG FILE WE SECTION
    */

    fprintf(output, "%d ", CPU->regFile_WE);

    if (CPU->regFile_WE == 1)
    {
        fprintf(output, "%d ", lastRegWritten);
        fprintf(output, "%04x ", CPU->regInputVal);
    }
    else
    {
        fprintf(output, "%d ", 0);
        fprintf(output, "%04x ", 0);
    }

    /*
    NZP SECTION
    */
    fprintf(output, "%d ", CPU->NZP_WE);
    if(CPU->NZP_WE == 1)
    {
        fprintf(output, "%d ", CPU->NZPVal);
    } else
    {
        fprintf(output, "%d ", 0);
    }
    
    /* DATA MEMORY SECTION */
    fprintf(output, "%d ", CPU->DATA_WE);
    if (CPU->DATA_WE == 1)
    {
        fprintf(output, "%04X ", CPU->dmemAddr);
        fprintf(output, "%04X ", CPU->dmemValue);
    }
    else
    {
        fprintf(output, "%04X ", 0);
        fprintf(output, "%04X ", 0);
    }
    
    fputs("\n", output);
}

/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState *CPU, FILE *output)
{
    unsigned short opcode;

    if (CPU->PC == 0x80FF)
    {
        printf("Hit 80FF, Exiting..\n");
        return 1;
    }

    instruction = CPU->memory[CPU->PC];

    printf("Current instruction is %x\n", instruction);
    opcode = OPCODE(instruction);
    printf("Current Opcode is %x\n", opcode);
    switch (opcode)
    {
        case 0:
        ClearSignals(CPU);

        BranchOp(CPU, output);
        break;

    case 1: // Arithmetic Operations
        ClearSignals(CPU);

        ArithmeticOp(CPU, output);
        break;
    case 8: // THIS IS RTI.
        ClearSignals(CPU);
        WriteOut(CPU, output);

        CPU->PC = CPU->R[7];
        CPU->PSR = CPU->PSR & 0x7; // set the MSB to 0;

        
        break;
    case 9: // Const OPCODE
        printf("got const\n");
        ClearSignals(CPU);

        SetConst(CPU, output);
        UpdatePC(CPU, 0);
        break;
    case 15: // TRAP SIGNAL
        ClearSignals(CPU);
        
        CPU->rdMux_CTL = 0;
        CPU->rtMux_CTL = 0;
        CPU->rsMux_CTL = 0;

        CPU->regFile_WE = 1;
        CPU->NZP_WE = 1;
        CPU->DATA_WE = 0;

        CPU->R[7] = CPU->PC + 1;

        lastRegWritten = 7;
        CPU->regInputVal = CPU->R[7];

        if (CPU->regInputVal < 0)
        {
            SetNZP(CPU, 4);
        }
        else if (CPU->regInputVal == 0)
        {
            SetNZP(CPU, 2);
        }
        else {
            SetNZP(CPU, 1);
        }
        WriteOut(CPU, output);

        // set PC = PC + 1;
        UpdatePC(CPU, 0);
        UpdatePC(CPU, 15); // TODO second part of trap

        CPU->PSR = CPU->PSR | 0x8000; // Set the MSB to 1
        
        break;
    default:
        printf("Unidentified operation\n");
        return 2;
    }

    return 0;
}

//////////////// PARSING HELPER FUNCTIONS ///////////////////////////

/*
 * Parses rest of branch operation and updates state of machine.
 */
void BranchOp(MachineState *CPU, FILE *output)
{
}

/*
 * Parses rest of arithmetic operation and prints out.
 */
void ArithmeticOp(MachineState *CPU, FILE *output)
{
    unsigned short subopcode = INSN_5_3(instruction);
    unsigned short rd = 0;
    unsigned short rt = 0;
    unsigned short rs = 0;

    CPU->rdMux_CTL = 0;
    CPU->rtMux_CTL = 0;
    CPU->rsMux_CTL = 0;

    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;

    switch (subopcode)
    {
    case 0: // Add
        printf("Got add: SUBOP: %d\n", subopcode);
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);
        rt = INSN_2_0(instruction);
        lastRegWritten = rd;

        CPU->R[rd] = CPU->R[rs] + CPU->R[rt];
        CPU->regInputVal = CPU->R[rd];

        if (CPU->regInputVal < 0)
        {
            SetNZP(CPU, 4);
        }
        else if (CPU->regInputVal == 0)
        {
            SetNZP(CPU, 2);
        }
        else {
            SetNZP(CPU, 1);
        }

        WriteOut(CPU, output);
        UpdatePC(CPU, 0);

        break;
    case 1: // MUL
        printf("Got MUL: SUBOP: %d\n", subopcode);
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);
        rt = INSN_2_0(instruction);
        lastRegWritten = rd;

        CPU->R[rd] = CPU->R[rs] * CPU->R[rt];
        CPU->regInputVal = CPU->R[rd];

        if (CPU->regInputVal < 0)
        {
            SetNZP(CPU, 4);
        }
        else if (CPU->regInputVal == 0)
        {
            SetNZP(CPU, 2);
        }
        else {
            SetNZP(CPU, 1);
        }

        WriteOut(CPU, output);
        UpdatePC(CPU, 0);
        break;
    case 2: //SUB
        printf("Got SUB: SUBOP: %d\n", subopcode);
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);
        rt = INSN_2_0(instruction);
        lastRegWritten = rd;

        CPU->R[rd] = CPU->R[rs] - CPU->R[rt];
        CPU->regInputVal = CPU->R[rd];

        if (CPU->regInputVal < 0)
        {
            SetNZP(CPU, 4);
        }
        else if (CPU->regInputVal == 0)
        {
            SetNZP(CPU, 2);
        }
        else {
            SetNZP(CPU, 1);
        }

        WriteOut(CPU, output);
        UpdatePC(CPU, 0);

        break;
    case 3: // DIV
        printf("Got DIV: SUBOP: %d\n", subopcode);
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);
        rt = INSN_2_0(instruction);
        lastRegWritten = rd;

        CPU->R[rd] = CPU->R[rs] / CPU->R[rt];
        CPU->regInputVal = CPU->R[rd];

        if (CPU->regInputVal < 0)
        {
            SetNZP(CPU, 4);
        }
        else if (CPU->regInputVal == 0)
        {
            SetNZP(CPU, 2);
        }
        else {
            SetNZP(CPU, 1);
        }

        WriteOut(CPU, output);
        UpdatePC(CPU, 0);
        break;

    default: // ADD IMM
        printf("GOT ADD IMM: SUBOP: %d\n", subopcode);
        break;
    }
}

/*
 * Parses rest of comparative operation and prints out.
 */
void ComparativeOp(MachineState *CPU, FILE *output)
{
}

/*
 * Parses rest of logical operation and prints out.
 */
void LogicalOp(MachineState *CPU, FILE *output)
{
}

/*
 * Parses rest of jump operation and prints out.
 */
void JumpOp(MachineState *CPU, FILE *output)
{
}

/*
 * Parses rest of JSR operation and prints out.
 */
void JSROp(MachineState *CPU, FILE *output)
{
}

/*
 * Parses rest of shift/mod operations and prints out.
 */
void ShiftModOp(MachineState *CPU, FILE *output)
{
}

/*
 * Set the NZP bits in the PSR.
 */
void SetNZP(MachineState *CPU, short result)
{   /* 
        1 = result > 0
        2 = 0
        4 = result < 0;
    */
    CPU->NZPVal = result;
    
    
}

void SetConst(MachineState *CPU, FILE *output)
{
    unsigned short int reg = INSN_11_9(instruction);
    unsigned short int constant = INSN_8_0(instruction);

    lastRegWritten = reg;
    CPU->R[reg] = constant;

    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 1;
    CPU->regInputVal = CPU->R[reg];

    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;

    if(constant < 0)
    {
        SetNZP(CPU,4);
    }
    else if (constant == 0)
    {
        SetNZP(CPU,2);
    }
    else 
    {
        SetNZP(CPU,1);
    }
    

    WriteOut(CPU, output);
}

void UpdatePC(MachineState *CPU, unsigned short int handler)
{
    unsigned short int imm = INSN_7_0(instruction);
    switch (handler)
    {

    case 0:
        CPU->PC += 1;
        break;

    case 15:

        CPU->PC = (0x8000 | imm);
        break;

    default:
        printf("TODO");
        break;
        // todo implement failure.
    }
}

// TODO  figure out why it's in memory backwards.
void intToB(char *binary)
{

    int i;
    for (i = 15; i >= 0; i--)
    {

        if ((instruction & (1 << i)) == 0)
        {
            binary[i] = '0';
        }
        else
        {
            binary[i] = '1';
        }
    }

    binary[i] = '\0';
    printf("\n");
}
