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

void GetHighConst(MachineState* CPU, FILE* output);

int StrOp(MachineState *CPU, FILE *output);
int LoadOp(MachineState *CPU, FILE *output);

/*
MACROS TO GRAB THE PIECES OF THE INSTRUCTIONS
*/

#define OPCODE(I) ((I) >> 12)           // opcode
#define INSN_11_9(I) (((I) >> 9) & 0x7) // get rd
#define INSN_8_6(I) (((I) >> 6) & 0x7)  // get rs
#define INSN_2_0(I) ((I)&0x7)           // get Rt
#define INSN_8_0(I) ((I)&0x1FF)         // get IMM9
#define INSN_7_0(I) ((I)&0xFF)          // get IMM 8
#define INSN_10_0(I) ((I)&0x7FF) // Get IMM11
#define INSN_15_9(I) (((I) >> 9) & 0x7F); // Get Branch OPCODE
#define INSN_4_0(I) ((I)&0x1F)           // for add IMMM
#define CHECK_MSB(I) ((I) >> 15) // Get MSB
#define CHECK_MSB_9(I) (((I) >> 8) & (1)) // SEXT(IMM9)
#define INSN_15_11(I) (((I) >> 11) & 0x1F) // Get Jump OPCODE
#define INSN_5_3(I) (((I) >> 3) & 0x7) // Get the sub-opcode
#define INSN_5_0(I) ((I)&0x3F) // GET IMM5


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
        fprintf(output, "%04X ", CPU->regInputVal);
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
        fprintf(output, "%04X", CPU->dmemValue);
    }
    else
    {
        fprintf(output, "%04X ", 0);
        fprintf(output, "%04X", 0);
    }
    
    fputs("\n", output);
}

/*
 * This function should execute one LC4 datapath cycle.
 */
int UpdateMachineState(MachineState *CPU, FILE *output)
{
    unsigned short opcode;
    unsigned short check;;

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
    case 6: // Load
        ClearSignals(CPU);
        check = LoadOp(CPU, output);
        if (check != 0)
        {
            return 1;
        }
        WriteOut(CPU, output);
        UpdatePC(CPU,0);
        break;
    case 7: //Store operations
        ClearSignals(CPU);
        check = StrOp(CPU, output);
        if (check != 0)
        {
            return 1;
        }
        WriteOut(CPU, output);
        UpdatePC(CPU, 0);
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

    case 12: // Jump
        ClearSignals(CPU);

        JumpOp(CPU, output);
        break;
    case 13: // High Const
        ClearSignals(CPU);

        GetHighConst(CPU, output);
        WriteOut(CPU, output);
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

        unsigned short int opcode = INSN_15_9(instruction);
        unsigned short int offset = INSN_8_0(instruction);
        // Set Branch codes
        CPU->regFile_WE = 0;
        CPU->NZP_WE = 0;
        CPU->DATA_WE = 0;
        CPU->regInputVal = 0;
        
        unsigned short int psr = CPU->PSR & 0x7;
        /* 
        1 = result > 0
        2 = 0
        4 = result < 0;
        */
        switch (opcode){
            case 0: // NOP
                UpdatePC(CPU, 0);
            break;
            case 4: // BRn
                WriteOut(CPU, output);
                if(psr == 4)
                {
                    CPU->PC = CPU->PC + offset;
                }
                CPU->PC = CPU->PC + 1;
                
            break;

            case 6: // BRnz
                WriteOut(CPU, output);
                if ((psr == 4) || psr == 2)
                {
                    CPU->PC = CPU->PC +  offset;
                }
                CPU->PC = CPU->PC + 1;
                
            break;

            case 5: //BRnp

                WriteOut(CPU, output);
                if ((psr == 4) || psr == 1)
                {
                    CPU->PC = CPU->PC + offset;
                } 
                CPU->PC = CPU->PC + 1;
                
            break;

            case 2: //BRz
                WriteOut(CPU, output);
                if (psr == 2)
                {
                    CPU->PC = CPU->PC + offset;
                }
                CPU->PC = CPU->PC + 1;
            break;

            case 3: //BRzp
                WriteOut(CPU, output);
                if((psr == 1) || (psr == 2))
                {
                    CPU->PC = CPU->PC + offset;
                }
                CPU->PC = CPU->PC + 1;
            break;

            case 1: //BRp
                WriteOut(CPU, output);
                if (psr == 1){
                    CPU->PC = CPU->PC + offset;
                }
                CPU->PC = CPU->PC + 1;
            break;

            case 7: // BRnzp
                WriteOut(CPU, output);
                CPU->PC = CPU->PC + 1+ offset; // Always do this.
            break;
        }
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
    signed short int neg;

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


        neg = CPU->R[rd] & 0x8000; // check if negative

        if (neg < 0)
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
        neg = CPU->R[rd] & 0x8000; // check if negative

        if (neg < 0)
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

        neg = CPU->R[rd] & 0x8000; // check if negative

        if (neg < 0)
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

        neg = CPU->R[rd] & 0x8000; // check if negative

        if (neg < 0)
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
        rd = INSN_11_9(instruction);
        rs = INSN_8_6(instruction);

        unsigned  short imm = INSN_4_0(instruction);
        signed short int magic = imm >> 4; // check for sign bit 0 positive, 1 negative

        if (magic == 1)
        {
            imm = imm + 0xFFE0; // 65536 - 7 = 65529
        }



        CPU->R[rd] = CPU->R[rs] + imm;
        CPU->regInputVal = CPU->R[rd];
        lastRegWritten = rd;

        if (magic == 1)
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
    unsigned short opcode = INSN_15_11(instruction);
    unsigned short rs;
    unsigned short imm11;

    CPU->regFile_WE = 0;
    CPU->NZP_WE = 0;
    CPU->DATA_WE = 0;
    CPU->regInputVal = 0;

    switch (opcode)
    {
    case 24:
        rs =  INSN_8_6(instruction);
        WriteOut(CPU, output);
        CPU->PC = rs;
        break;
    case 25:
        imm11 = INSN_10_0(instruction);
        WriteOut(CPU, output);
        CPU->PC = CPU->PC + imm11 + 1;
    break;
    default:
        break;
    }
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

int LoadOp(MachineState *CPU, FILE *output)
{
    unsigned short int rd;
    unsigned short int rs;
    unsigned short int imm5;
    signed short extendSign;
    unsigned short psr;
    int x;

    rd = INSN_11_9(instruction);
    rs = INSN_8_6(instruction);
    imm5 = INSN_5_0(instruction);

    CPU->NZP_WE = 1;
    CPU->DATA_WE = 1;
    CPU->regFile_WE = 1;

    extendSign = imm5;
	if (((imm5 >> 5) & 1) == 1) {
		extendSign = imm5 | 0xffc0;
	}

    // Check for invalid memory address
    if (CPU->R[rs] + extendSign >= 0x0000 && CPU->R[rs] + extendSign <= 0x1fff)
    {
        printf("Trying to execute in user code land");
        return 3;
    }
    if (CPU->R[rs] + extendSign >= 0x8000 && CPU->R[rs] + extendSign <= 0x9FFF)
    {
        printf("Trying to execute in os code land");
        return 3;
    }

    // Get psr for priveleges check
    psr = CHECK_MSB(CPU->PSR);
    if (psr == 0 &&
        CPU->R[rs] + extendSign >= 0xA000 && 
        CPU->R[rs] + extendSign <= 0xFFFF
    )
    {
        printf("Trying to execute in os data land");
        return 4;
    }

    CPU->dmemAddr = CPU->R[rs] + imm5;
    CPU->dmemValue = CPU->memory[CPU->dmemAddr];
    CPU->R[rd] = CPU->dmemValue;
    CPU->regInputVal = CPU->dmemValue;
    lastRegWritten = rd;
    x = CHECK_MSB(CPU->R[rd]);

    if (x == 1)
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
    return 0;

}

int StrOp(MachineState *CPU, FILE *output)
{
    unsigned short int rt;
    unsigned short int rs;
    short int imm5;
    signed short extendSign;
    unsigned short int psr;
	

    rt = INSN_11_9(instruction);
    rs = INSN_8_6(instruction);
    imm5 = INSN_5_0(instruction);

    
    extendSign = imm5;
	if (((imm5 >> 5) & 1) == 1) {
		extendSign = imm5 | 0xffc0;
	}

    // Check for invalid memory address
    if (CPU->R[rs] + extendSign >= 0x0000 && CPU->R[rs] + extendSign <= 0x1fff)
    {
        printf("Trying to execute in user code land");
        return 3;
    }
    if (CPU->R[rs] + extendSign >= 0x8000 && CPU->R[rs] + extendSign <= 0x9FFF)
    {
        printf("Trying to execute in os code land");
        return 3;
    }

    // Get psr for priveleges check
    psr = CHECK_MSB(CPU->PSR);
    if (psr == 0 &&
        CPU->R[rs] + extendSign >= 0xA000 && 
        CPU->R[rs] + extendSign <= 0xFFFF
    )
    {
        printf("Trying to execute in os data land");
        return 4;
    }


    CPU->DATA_WE = 1;

    CPU->dmemAddr = CPU->R[rs] + extendSign;
    CPU->dmemValue = CPU->R[rt];
    CPU->memory[CPU->dmemAddr] = CPU->R[rt];

    return 0;
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
    int checkMSB = CHECK_MSB(CPU->PSR);

    CPU->NZPVal = result;

    switch (result)
    {
    case 1:// Positive
        if (checkMSB == 1)
        {
            CPU->PSR = 0x8001;
        } else 
        {
            CPU->PSR = 1;
        }
        break;
    case 2:// zero
        if (checkMSB == 1)
        {
            CPU->PSR = 0x8002;
        } else 
        {
            CPU->PSR = 2;
        }
        break;
    case 4:// Negative
        if (checkMSB == 1)
        {
            CPU->PSR = 0x8004;
        } else 
        {
            CPU->PSR = 4;
        }
        break;
    
    default:
        break;
    }
}

void SetConst(MachineState *CPU, FILE *output)
{   
    // TODO Check for negative numbers... 

    unsigned short int reg = INSN_11_9(instruction);
    unsigned short int constant = INSN_8_0(instruction);
    signed short int extendSign;


    extendSign = constant;
    if(CHECK_MSB_9(extendSign) == 1){
        extendSign = extendSign | 0xFE00;
    }
    lastRegWritten = reg;
    CPU->R[reg] = extendSign;

    CPU->rdMux_CTL = 0;
    CPU->regFile_WE = 1;
    CPU->NZP_WE = 1;
    CPU->DATA_WE = 0;
    CPU->regInputVal = CPU->R[reg];

    

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

void GetHighConst(MachineState* CPU, FILE* output)
{
    unsigned short int rd = INSN_11_9(instruction);
    unsigned short int imm8 = INSN_7_0(instruction);
    short int neg;


    CPU->NZP_WE = 1;
    CPU->regFile_WE = 1;
    CPU->rdMux_CTL = 0;
    CPU->DATA_WE = 0;

    CPU->R[rd] =  (CPU->R[rd] & 0xFF) | (imm8 << 8);
    CPU->regInputVal = CPU->R[rd];
    neg = CPU->R[rd] & 0x8000; // check if negative

        if (neg < 0)
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



}