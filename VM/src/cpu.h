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
//   - Free Memory: from 0x0000 until 0x41ff (16 kb)
//   - TVO: 0x3a2f until 0x41ff (80 * 25 chars)
//   - Stack: from 0x41ff until 0x42ff (256 entries of 16 bits data)
//   - Rodata: from 0x42ff until 0x817f (16KB of rodata)
//   - ROM: from 0x817f until 0xffff (32 kb)

#define freeStart  0x0000
#define freeEnd    0x41ff

#define TVOStart   freeEnd - (80 * 25)
#define TVOEnd     freeEnd

#define stackStart TVOEnd
#define stackEnd   TVOEnd + 0x0100

#define rdataStart stackEnd
#define rdataEnd   stackEnd + 0x3e80

#define romStart   rdataEnd
#define romEnd     0xffff

#define registersCount 15
#define portsCount 15

typedef struct {
    uint16_t memory[65536];
    uint16_t SP;
    uint16_t PC;
    
    uint16_t registers[registersCount + 1];

    uint8_t status;

    uint8_t zero : 1;
    uint8_t less : 1;
    uint8_t greater : 1;
    uint8_t addrByReg : 1;
    uint8_t opTp;

    uint16_t addressBus;
    uint16_t dataBus;
    uint16_t registerBus;

    uint16_t ports[portsCount + 1];
} CPU;

int CPU_Init(CPU* cpu, uint16_t* memory);
uint16_t CPU_Fetch(CPU* cpu, uint8_t* cycles);
void CPU_WriteMem(CPU* cpu);
void CPU_ReadMem(CPU* cpu);
void CPU_WriteReg(CPU* cpu);
void CPU_ReadReg(CPU* cpu);
int CPU_Execute(CPU* cpu, uint8_t cycles);