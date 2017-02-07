
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include "printInternalReg.h"

#define ERROR_RETURN -1
#define SUCCESS 0


// the instruction length in bytes map with the array index corresponds to icode
int instructionLengths[12] = {1, 1, 2, 10, 10, 10, 2, 9, 9, 1, 2, 2};
char* instructionNames[12] = {"halt", "nop", "rrmovq", "irmovq", "rmmovq", "mrmovq", "OPq", "jmp", "call", "ret", "pushq", "popq"};

int main(int argc, char **argv) {

  int machineCodeFD = -1;       // File descriptor of file with object code
  uint64_t PC = 0;              // The program counter
  struct fetchRegisters fReg;

  // Verify that the command line has an appropriate number
  // of arguments

  if (argc < 2 || argc > 3) {
    printf("Usage: %s InputFilename [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to open, attempt to open it
  // for reading and verify that the open did occur.
  machineCodeFD = open(argv[1], O_RDONLY);

  if (machineCodeFD < 0) {
    printf("Failed to open: %s\n", argv[1]);
    return ERROR_RETURN;
  }

  // If there is a 2nd argument present, it is an offset so
  // convert it to a value. This offset is the initial value the
  // program counter is to have. The program will seek to that location
  // in the object file and begin fetching instructions from there.
  if (3 == argc) {
    // See man page for strtol() as to why
    // we check for errors by examining errno
    errno = 0;
    PC = strtol(argv[2], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      return ERROR_RETURN;
    }
  }

  printf("Opened %s, starting offset 0x%016llX\n", argv[1], PC);

  // Start adding your code here and comment out the line the #define EXAMPLESON line
  // start reading from the initial PC
  lseek(machineCodeFD, PC, SEEK_SET);

  // reading buffers
  uint8_t icode_ifun_buf[1];
  uint8_t rA_rB_buf[1];
  uint8_t valC_buf[8];

  struct fetchRegisters registers;

  // read the whole file until EOF
  while (read(machineCodeFD, icode_ifun_buf, sizeof(icode_ifun_buf)/sizeof(uint8_t)) > 0) {
    unsigned int icode = icode_ifun_buf[0] / 16;
    unsigned int ifun = icode_ifun_buf[0] % 16;

    int instructionLength = (icode <= 12) ? instructionLengths[icode] : 1;

    // read the whole instruction
    if (instructionLength == 2) {
      read(machineCodeFD, rA_rB_buf, sizeof(rA_rB_buf)/sizeof(uint8_t));
    } else if (instructionLength == 9) {
      read(machineCodeFD, valC_buf, sizeof(valC_buf)/sizeof(uint8_t));
    } else if (instructionLength == 10) {
      read(machineCodeFD, rA_rB_buf, sizeof(rA_rB_buf)/sizeof(uint8_t));
      read(machineCodeFD, valC_buf, sizeof(valC_buf)/sizeof(uint8_t));
    }

    // print the instruction's icode and ifun
    registers.PC = PC;
    registers.icode = icode;
    registers.ifun = ifun;
    registers.regsValid = 0;
    registers.valCValid = 0;
    registers.valP = PC + instructionLength;
    registers.instr = instructionNames[icode];
    printRegS(&registers);

    // advance PC
    PC = registers.valP;
  }

#define EXAMPLESON 1
#ifdef  EXAMPLESON



  // The next few lines are examples of various types of output. In the comments is
  // an instruction, the address it is at and the associated binary code that would
  // be found in the object code file at that address (offset). Your program
  // will read that binary data and then pull it appart just like the fetch stage.
  // Once everything has been pulled apart then a call to printRegS is made to
  // have the output printed. Read the comments in printInternalReg.h for what
  // the various fields of the structure represent. Note: Do not fill in fields
  // int the structure that aren't used by that instruction.


  /*************************************************
     irmovq $1, %rsi   0x008: 30f60100000000000000
  ***************************************************/

  fReg.PC = 8; fReg.icode = 3; fReg.ifun = 0;
  fReg.regsValid = 1; fReg.rA = 15;  fReg.rB = 6;
  fReg.valCValid = 1; fReg.valC = 1;
  fReg.byte0 = 1;  fReg.byte1 = 0; fReg.byte2 = 0; fReg.byte3 = 0;
  fReg.byte4 = 0;  fReg.byte5 = 0; fReg.byte6 = 0; fReg.byte7 = 0;
  fReg.valP = 8 + 10;  fReg.instr = "irmovq";

  printRegS(&fReg);


    /*************************************************
     je target   x034: 733f00000000000000     Note target is a label

     ***************************************************/


  fReg.PC = 0x34; fReg.icode = 7; fReg.ifun = 3;
  fReg.regsValid = 0;
  fReg.valCValid = 1; fReg.valC = 0x3f;
  fReg.byte0 = 0x3f;  fReg.byte1 = 0; fReg.byte2 = 0; fReg.byte3 = 0;
  fReg.byte4 = 0;  fReg.byte5 = 0; fReg.byte6 = 0; fReg.byte7 = 0;
  fReg.valP = 0x34 + 9;  fReg.instr = "je";

  printRegS(&fReg);
    /*************************************************
     nop  x03d: 10

     ***************************************************/

  fReg.PC = 0x3d; fReg.icode = 1; fReg.ifun = 0;
  fReg.regsValid = 0;
  fReg.valCValid = 0;
  fReg.valP = 0x3d + 1;  fReg.instr = "nop";

  printRegS(&fReg);

    /*************************************************
     addq %rsi,%rdx  0x03f: 6062

     ***************************************************/

  fReg.PC = 0x3f; fReg.icode = 6; fReg.ifun = 0;
  fReg.regsValid = 1; fReg.rA = 6; fReg.rB = 2;
  fReg.valCValid = 0;
  fReg.valP = 0x3f + 2;  fReg.instr = "add";

  printRegS(&fReg);
#endif


  return SUCCESS;

}
