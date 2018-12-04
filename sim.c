#include <stdio.h>
#include <stdint.h>
#include "shell.h"

/* process_instruction() is implemented at the bottom of the file as it calls files that are not defined yet*/
/* all following functions will be called by process_instruction() */ 

/* this function is used to create a mask of bits in order to isolate the different bits inside of things
   like the instruction to be processed.  */ 
uint32_t mask(uint32_t x, uint32_t y){
  uint32_t shift = 0;
  uint32_t i;
  for (i = x; i <= y; i++){
    shift |= 1 << i; 
  }
  return shift;
}

/* processes the special type instruction */
void special(uint32_t instruction, uint32_t convert){
  uint32_t temp = mask(0, 5);
  uint32_t instrName = temp & instruction;
  uint32_t sa, rd, rt, rs, lowOrder;
  switch(instrName){
  case 24: //SYSCALL
    if (CURRENT_STATE.REGS[2] == 10){
      RUN_BIT = 0;
    }
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break; 
  case 0:
    temp = mask(6, 10);
    sa = temp & instruction;
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction; 
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << sa;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 2:
    temp = mask(6, 10);
    sa = temp & instruction;
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction; 
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 3:
    temp = mask(6, 10);
    sa = temp & instruction;
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction; 
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
    NEXT_STATE.REGS[rd] = (int32_t)CURRENT_STATE.REGS[rd]; // sign extends the high-order bits
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 4:
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    temp = mask(0, 4);
    lowOrder = temp & CURRENT_STATE.REGS[rs];
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << lowOrder; // shifted left by the low-order 5 bits in rs
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 6:
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    temp = mask(0, 4);
    lowOrder = temp & CURRENT_STATE.REGS[rs];
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> lowOrder; // shifted right by the low-order 5 bits rs
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 7:
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    temp = mask(0, 4);
    lowOrder = temp & CURRENT_STATE.REGS[rs];
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> lowOrder; // shifted left by the low-order 5 bits in rs
    NEXT_STATE.REGS[rd] = (int32_t)CURRENT_STATE.REGS[rd]; // sign extends the high order bits
    NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 8:// JR
    temp = mask(21, 25);
    rs = temp & instruction;
    NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
    break;
  case 9:
    temp = mask(21, 25);
    rs = temp & instruction;
    NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
    NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
    break;
  case 32: // ADD
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] + CURRENT_STATE.REGS[rs];
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break;
  case 33: // ADDU, no different than ADD, since exceptions are not cared about in this project.
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] + CURRENT_STATE.REGS[rs];
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
  case 34: // SUB
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break;
  case 35: //SUBU
    temp = mask(11, 15);
    rd = temp & instruction;
    temp = mask(16, 20);
    rt = temp & instruction;
    temp = mask(21, 25);
    rs = temp & instruction;
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break; 
  }
  return;
}

/* processes the register Immediate instructions, BLTZ, BGEZ, BLTZAL, BGEZAL */
void regImm(uint32_t instruction, uint32_t convert){
  uint32_t temp = mask(16, 20);
  uint32_t instructionDec = temp & instruction; // Decode the instruction  
  temp = mask(0, 15);
  uint32_t offset = temp & instruction;
  offset = offset << 2;  // every instruction calls for the same operations to be done on the offset 
  offset = (int32_t)offset;
  temp = mask(21, 25);
  uint32_t rs = temp & instruction;
  switch(instruction){ /* every instruction description implemented here is described to use the sum of the
			  offset and the address of the instruction in the delay slot, since this project is
			  not using delay slots, the new target address is simply the offset that was sign
			  extended and shifted to the left 2 bits */ 
  case 1: //BGEZ 
    if (CURRENT_STATE.REGS[rs] == 0)
      NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break; 
  case 34: //BGEZAL
    NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
    if (CURRENT_STATE.REGS[rs] == 0)
      NEXT_STATE.PC = CURRENT_STATE.PC + offset; 
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
    break;
  case 0: // BLTZ
    if (CURRENT_STATE.REGS[rs] < 0)
      NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break;
  case 32: //BLTZAL
    NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
    if (CURRENT_STATE.REGS[rs] < 0)
      NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    else
      NEXT_STATE.PC = CURRENT_STATE.PC = 4;
    break; 
  }
  return;
}

/* processes the 3 store instructions, sign extends and adds the 16 bit offset and stores a certain size of
   data based on what the instruction specifies.  */
void store(uint32_t instruction, uint32_t convert){
  uint32_t result; 
  uint32_t temp = mask(0, 15);
  uint32_t offset = temp & instruction;
  offset = (int32_t)offset;
  temp = mask(21, 25);
  uint32_t base = temp & instruction;
  uint32_t address = CURRENT_STATE.REGS[base] + offset;
  temp = mask(16, 20);
  uint32_t regT = temp & instruction;
  /* once the instruction is decoded into variables, pick how much of the regT to store in the effective 
     address based on the instruction. */ 
  switch(convert){
  case 40:
    temp = mask(24, 31);
    break;
  case 41:
    temp = mask(16, 31);
    break; 
  case 43:
    temp = mask(0,31);
    break;
  }
  result = temp & CURRENT_STATE.REGS[regT];
  mem_write_32(address, result);
  NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
  return;
}

/* processes the various load instructions */
void load(uint32_t instruction, uint32_t convert){
  /* add the offset to the contents of the base register and the contents in memory location address are 
     loaded into the target register */
  uint32_t temp = mask(0, 15);
  uint32_t offset = temp & instruction;
  offset = (int32_t)offset;
  temp = mask(21, 25);
  uint32_t base = temp & instruction;
  uint32_t address = CURRENT_STATE.REGS[base] + offset;
  uint32_t memory = mem_read_32(address);
  temp = mask(16, 20);
  uint32_t regT = temp & instruction;
  uint32_t result;
  /* loads the correct length of the memory location into the target register */ 
  switch(convert){
  case 32:
    temp = mask(24, 31);
    break;
  case 33:
    temp = mask(16, 31);
    break;
  case 35:
    temp = mask(0, 31);
    break;
  case 36:
    temp = mask(24, 31);
    result = temp & memory;
    NEXT_STATE.REGS[regT] = result;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    return; 
  case 37:
    temp = mask(16, 31);
    result = temp & memory;
    NEXT_STATE.REGS[regT] = result;
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    return; 
  }
  result = temp & memory;
  result = (int32_t)result;
  NEXT_STATE.REGS[regT] = result;
  NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
  return;
}

/* processes the I-type instructions */ 
void immediate(uint32_t instruction, uint32_t convert){
  uint32_t temp = mask(0, 15);
  uint32_t value  = temp & instruction;
  uint32_t regS, regT, temp2, result;
  /* sign extend the immediate value */
  value = (int32_t)value;
  /* gets the registers to be operated on */
  temp = mask(16, 20);
  temp2 = mask(21, 25);
  regS = temp2 & instruction;
  regT = temp & instruction; 
  /* executes the appropriate instruction based on what was read from the instruction memory */ 
  switch(convert){
  case 8:
    result = CURRENT_STATE.REGS[regS] + value;
    break;
    /* 8 and 9 are addi and addiu, they are the same in this project since we are ignoring oveflows */ 
  case 9:
    result = CURRENT_STATE.REGS[regS] + value; 
    break;
  case 10:
    result = CURRENT_STATE.REGS[regS] - value;
    if (result < 0){
      result = 1;
    }
    if (result >= 0){
      result = 0;
    }
    break;
    /* SLTI and SLTIU are the same for the same reason as ADDIU and ADDI */ 
  case 11:
    result = CURRENT_STATE.REGS[regS] - value;
    if (result < 0) {
      result = 1;
    }
    if (result >= 0){
      result = 0;
    }
    break;
  case 12:
    result = CURRENT_STATE.REGS[regS] & value;
    break;
  case 13:
    result = CURRENT_STATE.REGS[regS] | value;
    break;
  case 14:
    result = CURRENT_STATE.REGS[regS] ^ value;
    break;
  case 15:
    /* value is shifted left by 14, already was shifted by 2 before. making it shifted by 16, already was 
       given 16 bits of 0's when it was sign extended before the switch. so just store the result. */
    result = value << 14;
    break;
  }
  NEXT_STATE.REGS[regT] = result; 
  NEXT_STATE.PC = CURRENT_STATE.PC + 4; 
  return;
}

/* branch target address is determined by the instruction, then shifted 2 bits to the left and sign-extended
   if the given condition is true, then the program branches to that target address */ 
void branch(uint32_t instruction, uint32_t convert){
  uint32_t temp = mask(0, 15);
  uint32_t temp2, regS, regT;
  uint32_t target = temp & instruction;
  /* shifts the target to the left twice */
  target = target << 2;
  /* sign extends the target */ 
  target = (int32_t)target; 
  /* gets the bits that represent each register in the branch instruction */
  temp = mask(16, 20);
  temp2 = mask(21, 25);
  regS = temp & instruction;
  regT = temp2 & instruction;
  /* performs the appropriate comparison based on the instruction */
  switch (convert){
  case 4:
    if (regS == regT){
      NEXT_STATE.PC = CURRENT_STATE.PC + target;
      return; 
    }
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
  case 5:
    if (regS != regT){
      NEXT_STATE.PC = CURRENT_STATE.PC + target;
      return;
    }
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
  case 6:
    if (regT <= 0){
      NEXT_STATE.PC = CURRENT_STATE.PC + target;
      return;
    }
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
  case 7:
    if (regT >= 0){
      NEXT_STATE.PC = CURRENT_STATE.PC + target;
      return;
    }
    else
      NEXT_STATE.PC = CURRENT_STATE.PC + 4;
  }
}

/* 26-bit target is shifted left by 2 bits. the program unconditionally jumps to this calculated address */ 
void jump(uint32_t instruction, uint32_t convert){
  uint32_t temp = mask(0, 25);
  uint32_t target = temp & instruction;
  target = target << 2;
  NEXT_STATE.PC = CURRENT_STATE.PC + target;
  if (convert == 3){
    NEXT_STATE.REGS[31] = CURRENT_STATE.PC + target;
  }
  /* RUN_BIT = 0; */ 
  return;
}

/* main function that uses the above defined functions. */ 
void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
  
  /* fetch the instruction using the current PC value */
  uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

  /* gets bits 26-31  of the instruction */
  uint32_t temp = mask(26, 31);
  uint32_t convert = temp & instruction;
  
  /* calls a certain function based on what instruction type it is */ 
  switch (convert){
  case 0:
    special(instruction, convert);
    break;
  case 1:
    regImm(instruction, convert);
    break;
  case 2:
    jump(instruction, convert);
    break;
  case 3:
    jump(instruction, convert);
    break;
  case 4:
    branch(instruction, convert);
    break;
  case 5:
    branch(instruction, convert);
    break;
  case 6:
    branch(instruction, convert);
    break;
  case 7:
    branch(instruction, convert);
    break;
  case 8:
    immediate(instruction, convert);
    break;
  case 9:
    immediate(instruction, convert);
    break;
  case 10:
    immediate(instruction, convert);
    break;
  case 11:
    immediate(instruction, convert);
    break;
  case 12:
    immediate(instruction, convert);
    break;
  case 13:
    immediate(instruction, convert);
    break;
  case 14:
    immediate(instruction, convert);
    break;
  case 15:
    immediate(instruction, convert);
    break;
  case 32:
    load(instruction, convert);
    break;
  case 33:
    load(instruction, convert);
    break;
  case 35:
    load(instruction, convert);
    break;
  case 36:
    load(instruction, convert);
    break;
  case 37:
    load(instruction, convert);
    break;
  case 40:
    store(instruction, convert);
    break;
  case 41:
    store(instruction, convert);
    break;
  case 43:
    store(instruction, convert);
    break; 
  }
  return;
}
