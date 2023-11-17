# MastroVM
MastroVM is a Virtual Machine (and fantasy CPU) that is made to be simple and easy to use.

# Specifications
- 128 KB of memory
- 3 Registers (a, b and c)
- I/O Ports (keyboard only for now)
- TVO (Text Video Output)
- 256 Entries for stack
- 17 Instructions

# Instructions
Mastro has 17 instructions, here's all of them:
- mov  [ADDR] VAL | [ADDR] REG | REG [ADDR] | REG VAL | REG REG
- push [ADDR] | VAL | REG
- pop  REG
- jmp  [ADDR]
- je   [ADDR]
- jne  [ADDR]
- jle  [ADDR]
- jl   [ADDR]
- jge  [ADDR]
- jg   [ADDR]
- cmp  [ADDR] VAL | [ADDR] REG | REG [ADDR] | REG VAL | REG REG
- add  [ADDR] VAL | [ADDR] REG | REG [ADDR] | REG VAL | REG REG
- dec  [ADDR] VAL | [ADDR] REG | REG [ADDR] | REG VAL | REG REG
- mul  [ADDR] VAL | [ADDR] REG | REG [ADDR] | REG VAL | REG REG
- div  [ADDR] VAL | [ADDR] REG | REG [ADDR] | REG VAL | REG REG
- in   REG VAL | REG REG
- out  VAL VAL | REG VAL | VAL REG| REG REG

# Why no moving address into address?
As you can see in the mov instruction, we can't mov an address directly into another, that's because you have to use an intermediate register to do that, for example:
```x86asm
mov regA [0x0042]
mov [0x0040] regA
```
Same thing applies for cmp, add, dec, mul and div

# Arithmetics while addressing?
In MastroVM you can't do something like
```x86asm
mov [sp - 4] 0x0042
```
This is because my CPU and Assembler doesn't support this type of arithmetic yet, here's what you can do instead:
```x86asm
dec sp 4
mov [sp] 0x0042
add sp 4
```

# Memory and ports layout
The memory in MastroVM works like this:
```
- Free Memory: from 0x0000 until 0x807f (32 kb)
- Stack: from 0x807f until 0x817f (256 entries of 16 bits data)
- ROM: from 0x817f until 0xffff (32 kb)
- TVO: 0x707f until 0x807f (80 * 25 chars)
```
And the current ports are:
```
port[0] = Keyboard status (set if key pressed)
port[1] = Key that is being pressed
```

# Known bugs
- port[1] (in this case the key of keyboard) doesn't support some keys, such as space. This happens because of the way that SDL handle keys, I still have to find a solution for that.