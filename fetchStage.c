
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
// #include <math.h>
#include "printInternalReg.h"

#define ERROR_RETURN -1
#define SUCCESS 0


// the instruction length in bytes map with the array index corresponds to icode
int instructionLengths[12] = {1, 1, 2, 10, 10, 10, 2, 9, 9, 1, 2, 2};

// instruction names
char* instructionNames[12] = {"halt", "nop", "rrmovq", "irmovq", "rmmovq", "mrmovq", "OPq", "jmp", "call", "ret", "pushq", "popq"};
char* instruction_op_names[7] = {"addq", "subq", "andq", "xorq", "mulq", "divq", "modq"};
char* instruction_jmp_names[7] = {"jmp", "jle", "jl", "je", "jne", "jge", "jg"};
char* instruction_rrmov_names[7] = {"rrmovq", "cmovle", "cmovl", "cmove", "cmovne", "cmovge", "cmovg"};

// parse the input and set rA and rB
int parse_rA_rB(struct fetchRegisters *registers, const uint8_t *rA_rB_buf, int bytesRead);

// parse the input and set valC
int parse_valC(struct fetchRegisters *registers, uint8_t *valC_buf, int bytesRead);

// check if icode is valid
int valid_icode(unsigned int icode);

// check if ifun is valid
int valid_ifun(unsigned int icode, unsigned int ifun);

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

  int bytesRead;
  int sufficientBytes;

  struct fetchRegisters registers;

  // read the file
  while ((bytesRead = read(machineCodeFD, icode_ifun_buf, sizeof(icode_ifun_buf)/sizeof(uint8_t))) >= 0) {
    // exit if EOF
    if (bytesRead == 0) {
      printf("Normal termination\n");
      return SUCCESS;
    }

    unsigned int icode = icode_ifun_buf[0] / 16;
    unsigned int ifun = icode_ifun_buf[0] % 16;

    // exit if the opcode is invalid
    if (!valid_icode(icode)) {
      printf("Invalid opcode %X at %08llX\n", icode, PC);
      return ERROR_RETURN;
    }

    // exit if the function code is invalid
    if (!valid_ifun(icode, ifun)) {
      printf("Invalid function code %X at %08llX.\n", ifun, PC);
      return ERROR_RETURN;
    }

    int instructionLength = (icode <= 12) ? instructionLengths[icode] : 1;

    // set the basic registers
    registers.PC = PC;
    registers.icode = icode;
    registers.ifun = ifun;
    registers.regsValid = 0;
    registers.valCValid = 0;
    registers.valP = PC + instructionLength;
    if (icode == 6) {
      registers.instr = instruction_op_names[ifun];
    } else if (icode == 7) {
      registers.instr = instruction_jmp_names[ifun];
    } else if (icode == 2) {
      registers.instr = instruction_rrmov_names[ifun];
    } else {
      registers.instr = instructionNames[icode];
    }

    // read the whole instruction
    if (instructionLength == 2) {
      bytesRead = read(machineCodeFD, rA_rB_buf, sizeof(rA_rB_buf)/sizeof(uint8_t));
      sufficientBytes = parse_rA_rB(&registers, rA_rB_buf, bytesRead);
    } else if (instructionLength == 9) {
      bytesRead = read(machineCodeFD, valC_buf, sizeof(valC_buf)/sizeof(uint8_t));
      sufficientBytes = parse_valC(&registers, valC_buf, bytesRead);
    } else if (instructionLength == 10) {
      bytesRead = read(machineCodeFD, rA_rB_buf, sizeof(rA_rB_buf)/sizeof(uint8_t));
      sufficientBytes = parse_rA_rB(&registers, rA_rB_buf, bytesRead);

      if (sufficientBytes) {
        bytesRead = read(machineCodeFD, valC_buf, sizeof(valC_buf)/sizeof(uint8_t));
        sufficientBytes = parse_valC(&registers, valC_buf, bytesRead);
      }
    }

    // print registers
    printRegS(&registers);

    // exit if there are insufficient bytes
    if (!sufficientBytes) {
      printf("Memory access error at %08llX, required %d bytes, read %d bytes.\n", PC, instructionLength, bytesRead+1);
      return ERROR_RETURN;
    }

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

// parse the input and set rA and rB
int parse_rA_rB(struct fetchRegisters *registers, const uint8_t *rA_rB_buf, int bytesRead) {
  // return if there are insufficient bytes
  if (bytesRead < 1) {
    registers->regsValid = 0;

    return 0;
  }

  registers->regsValid = 1;

  registers->rA = rA_rB_buf[0] / 16;
  registers->rB = rA_rB_buf[0] % 16;

  return 1;
}

// parse the input and set valC
int parse_valC(struct fetchRegisters *registers, uint8_t *valC_buf, int bytesRead) {
  // return if no bytes are read
  if (bytesRead <= 0) {
    registers->valCValid = 0;

    return 0;
  }

  registers->valCValid = 1;

  // set the unread bytes as 0 if there are insufficient bytes
  if (bytesRead < 8) {
    for (int i = bytesRead; i < 8; i++) {
      valC_buf[i] = 0;
    }
  }

  registers->byte0 = valC_buf[0];
  registers->byte1 = valC_buf[1];
  registers->byte2 = valC_buf[2];
  registers->byte3 = valC_buf[3];
  registers->byte4 = valC_buf[4];
  registers->byte5 = valC_buf[5];
  registers->byte6 = valC_buf[6];
  registers->byte7 = valC_buf[7];

  registers->valC = 0;
  for (int i = 0; i < 8; i++) {
    uint64_t power = 1;
    for (int j = 0; j < i; j++) {
      power *= 256;
    }
    registers->valC += valC_buf[i] * power;
  }

  return (bytesRead == 8);
}

// check if icode is valid
int valid_icode(unsigned int icode) {
  if (icode >= 0 && icode <= 11) {
    return 1;
  }

  return 0;
}

// check if ifun is valid
int valid_ifun(unsigned int icode, unsigned int ifun) {
  if ((icode >= 0 && icode <= 1) || (icode >= 3 && icode <= 5) || (icode >= 8 && icode <= 11)) {
    return (ifun == 0);
  } else {
    return (ifun >= 0 && ifun <= 6);
  }
}
