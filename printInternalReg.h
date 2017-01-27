

/* This file contiains the prototypes and 
   constants needed to use the routines 
   defined in printRoutines.c
*/

#ifndef PRINTINTERNALREG
#define PRINTINTERNALREG

#include <stdint.h>

#define PRINTERROR -1
#define PRINTSUCCESS 0

typedef unsigned char nibble;

struct fetchRegisters {
  uint64_t PC;          // Address of instruction       
  nibble icode;         // Instruction Code
  nibble ifun;          // Function code
  int regsValid;        // Does this instruction specify the registers?
     nibble rA;         // Register rA 
     nibble rB;         // register rB   
  int  valCValid;       // Does this instruction set valC?
     uint64_t valC;     // valC in proper big endian format
     uint8_t byte0;     // The 0th byte (least significant byte of valC 
     uint8_t byte1;
     uint8_t byte2;
     uint8_t byte3;  
     uint8_t byte4;
     uint8_t byte5;
     uint8_t byte6;
     uint8_t byte7;     // The most significatnt byte of valC
  int64_t valP;        // valP at the end of the Fetch stage 
  char * instr;        // The instruction mnemonic
};


void printRegS(struct fetchRegisters *);

#endif /* PRINTROUTINES */
