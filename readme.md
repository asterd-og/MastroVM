# MastroVM
MastroVM is a Virtual Machine (and fantasy CPU) that is made to be simple and easy to use.

![Image](https://github.com/asterd-og/MastroVM/blob/master/image.png?raw=true)


# Specifications
- 128 KB of memory
- 16 Registers (a, b, c and so on)
- I/O Ports (keyboard only for now)
- TVO (Text Video Output)
- 256 Entries for stack
- 19 Instructions

# Instructions
Mastro has 19 instructions, here's all of them:
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
- call [ADDR]
- ret

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

# Call and subroutines
Whenever you call a subroutine in MastroVM, it will do the following things:
- Push current address to the stack
- Jmp to the address
So remember to ALWAYS return a subroutine, or else, it will never... return!
```x86asm
call printChar
mov regA 0x42

.:
 jmp .

printChar:
 mov [0x3a2f] 0x64
 ret
```

# Rodata
Let's say you want to define a string to use it later, in Mastro, you would do it in the following way:
```x86asm
str wd "Hello World!"

mov regB str            ; Moves the address of str into regB
mov regC [regB]         ; Moves the first letter of str into regC
mov [0x3a2f] regC       ; Moves the contents of regC to TVO address
; This will print 'H' to the screen
```
Remember to ALWAYS define the string BEFORE you use it!

# Memory and ports layout
The memory layout in Mastro looks like this:
```
- Free Memory: from 0x0000 until 0x41ff (16 kb)
- TVO: 0x3a2f until 0x41ff (80 * 25 chars)
- Stack: from 0x41ff until 0x42ff (256 entries of 16 bits data)
- Rodata: from 0x42ff until 0x817f (16KB of rodata)
- ROM: from 0x817f until 0xffff (32 kb)
```
And the current ports are:
```
port[0] = Keyboard status (set if key pressed)
port[1] = Key that is being pressed
port[2] = cursor X
port[3] = cursor Y
```

# Known bugs
- Going to put known bugs here.