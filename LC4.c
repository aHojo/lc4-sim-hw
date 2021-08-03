/*
 * LC4.c: Defines simulator functions for executing instructions
 */

#include "LC4.h"
#include <stdio.h>

// Handle the CONST opcode.
void SetConst(MachineState *CPU, unsigned short instr);

// Function to update our program controller
void UpdatePC(MachineState *CPU, unsigned short handler);
/*
MACROS TO GRAB THE PIECES OF THE INSTRUCTIONS
*/

#define OPCODE(I) ((I) >> 12)           // opcode
#define INSN_11_9(I) (((I) >> 9) & 0x7) // get rd
#define INSN_8_6(I) (((I) >> 6) & 0x7)  // get rs
#define INSN_2_0(I) ((I) & 0x7)           // get Rt
#define INSN_8_0(I) ((I) & 0x1FF)         // get IMM9
#define INSN_7_0(I) ((I) & 0xFF)          // get IMM 7
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
unsigned short instruction;
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
}

/*
 * This function should write out the current state of the CPU to the file output.
 */
void WriteOut(MachineState *CPU, FILE *output)
{
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
    
    case 1: // Arithmetic Operations
        ArithmeticOp(CPU, output);
        break;
    case 8: // THIS IS RTI.
        CPU->PC = CPU->R[7];
        CPU->PSR= CPU->PSR & 0x7; // set the MSB to 0;
        break;
    case 9: // Const OPCODE
        printf("got const");
        SetConst(CPU, instruction);
        UpdatePC(CPU, 0);
        break;
    case 15: // TRAP SIGNAL
        UpdatePC(CPU, 0); // set PC = PC + 1; 
        CPU->R[7] = CPU->PC;

        UpdatePC(CPU, 15); // TODO second part of trap

        CPU->PSR=CPU->PSR | 0x8000; // Set the MSB to 1
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

    switch (subopcode)
    {
    case 0: // Add
        printf("Got add: SUBOP: %d\n", subopcode);
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);
        rt = INSN_2_0(instruction);

        CPU->R[rd] = CPU->R[rs] + CPU->R[rt];
        
        UpdatePC(CPU, 0);
        break;
    case 1: // MUL
        printf("Got MUL: SUBOP: %d\n", subopcode);
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);
        rt = INSN_2_0(instruction);

        CPU->R[rd] = CPU->R[rs] * CPU->R[rt];
        UpdatePC(CPU, 0);
        break;
    case 2: //SUB
        printf("Got SUB: SUBOP: %d\n", subopcode);
        break;
    case 3: // DIV
        printf("Got DIV: SUBOP: %d\n", subopcode);
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
{
}

void SetConst(MachineState *CPU, unsigned short instr)
{
    unsigned short int reg = INSN_11_9(instr);
    unsigned short int constant = INSN_8_0(instr);

    CPU->R[reg] = constant;
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
