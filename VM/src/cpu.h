#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Mastro is a 16-Bit CPU
// with 65536 memory locations
// and a basic screen (kinda like VGA, but it doesn't follows it's specs)

// Made by: Astrido >_<

// CPU specifications:
//  There are 16 registers:
//      - A
//      - B
//      - C
//      - ...
//
//  There are also the ports, which can be used to access peripherals
//  and an address bus, which is used to address the memory when writing/reading
//  from it.
//
//  Something that is also interesting to talk about, is the flags!
//  this CPU got 4 conditional flags and 2 control flags:
//   - Zero: is set if the subtraction from two operators is set to 0 (they are equal)
//   - Less: is set if the left operator is smaller than the right one
//   - Greater: is set if the left operator is bigger than the right one
//   - OpTp: sets the cmp/math op type
//   - AddrByReg: is set if the next addressing should be addressed by the value of the
//      register in the register bus.
//
//  And finally, the memory layout:
//   - Free Memory: from 0x0000 until 0x807f (32 kb)
//   - Stack: from 0x807f until 0x817f (256 entries of 16 bits data)
//   - ROM: from 0x817f until 0xffff (32 kb)
//   - TVO: 0x707f until 0x807f (80 * 25 chars)

#define freeStart  0x0000
#define freeEnd    0x807f

#define TVOStart   0x707f
#define TVOEnd     0x807f

#define stackStart 0x807f
#define stackEnd   0x817f

#define romStart   0x817f
#define romEnd     0xffff 

typedef struct {
    uint16_t memory[65536];
    uint16_t SP;
    uint16_t PC;
    
    //uint16_t regA;
    //uint16_t regB;
    //uint16_t regC;
    uint16_t registers[16];

    uint8_t status;

    uint8_t zero : 1;
    uint8_t less : 1;
    uint8_t greater : 1;
    uint8_t addrByReg : 1;
    uint8_t opTp;

    uint16_t addressBus;
    uint16_t dataBus;
    uint16_t registerBus;

    uint16_t ports[16];
} CPU;

int CPU_Init(CPU* cpu, uint16_t* memory);
uint16_t CPU_Fetch(CPU* cpu, uint8_t* cycles);
void CPU_WriteMem(CPU* cpu);
void CPU_ReadMem(CPU* cpu);
void CPU_WriteReg(CPU* cpu);
void CPU_ReadReg(CPU* cpu);
int CPU_Execute(CPU* cpu, uint8_t cycles);