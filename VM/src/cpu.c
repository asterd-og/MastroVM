#include "cpu.h"

enum {
    STARTING,
    OK,
    ERROR,
    HALT,
    ADDR_OUT_OF_BOUNDS,
    STACK_OUT_OF_BOUNDS,
    UNKNOWN_REGISTER,
    INVALID_JMP_ADDR,
    DIVIDE_BY_ZERO
} CPU_Status;

enum {
    ADDR_VAL,
    ADDR_REG,
    REG_ADDR,
    REG_VAL,
    REG_REG
} CMP_Types;

enum {
    ADD,
    SUB,
    MUL,
    DIV
} CPU_Operation;

int CPU_Init(CPU* cpu, uint16_t* rom) {
    if (cpu == NULL) {
        printf("Could not initialise CPU!\n");
        return 1;
    }

    for (uint16_t i = 0x0; i < 0x7e80; i++) {
        cpu->memory[i + 0x817f] = rom[i];
    }
    
    cpu->SP = 0;
    cpu->PC = 0x817f;

    cpu->regA = 0;
    cpu->regB = 0;
    cpu->regC = 0;

    cpu->status = STARTING;
    cpu->zero = 0;
    cpu->less = 0;
    cpu->greater = 0;
    cpu->opTp = 0;
    cpu->addrByReg = 0;

    cpu->addressBus = 0x0000;
    cpu->dataBus = 0x0000;
    cpu->registerBus = 0x0000;

    return 0;
}

uint16_t CPU_Fetch(CPU* cpu, uint8_t* cycles) {
    uint16_t data = cpu->memory[cpu->PC];
    cpu->PC++;
    (*cycles)--;
    return data;
}

void CPU_WriteMem(CPU* cpu) {
    if (cpu->addrByReg) {
        uint16_t tmpDBus = cpu->dataBus;
        uint16_t tmpRBus = cpu->registerBus;
        cpu->registerBus = cpu->addressBus;
        CPU_ReadReg(cpu);
        cpu->addressBus = cpu->dataBus;

        if (cpu->addressBus > 0x817f) {
            cpu->status = ADDR_OUT_OF_BOUNDS;
            return;
        }

        cpu->memory[cpu->addressBus] = tmpDBus;
        cpu->addrByReg = 0;
        cpu->registerBus = tmpRBus;
        cpu->addressBus = tmpDBus;

        return;
    }
    if (cpu->addressBus > 0x817f) {
        cpu->status = ADDR_OUT_OF_BOUNDS;
        return;
    }
    cpu->memory[cpu->addressBus] = cpu->dataBus;
}

void CPU_ReadMem(CPU* cpu) {
    if (cpu->addrByReg) {
        uint16_t tmpDBus = cpu->dataBus;
        uint16_t tmpRBus = cpu->registerBus;
        cpu->registerBus = cpu->addressBus;
        CPU_ReadReg(cpu);
        cpu->addressBus = cpu->dataBus;

        if (cpu->addressBus > 0x817f) {
            cpu->status = ADDR_OUT_OF_BOUNDS;
            return;
        }

        cpu->dataBus = cpu->memory[cpu->addressBus];
        cpu->addrByReg = 0;
        cpu->registerBus = tmpRBus;
        cpu->addressBus = tmpDBus;

        return;
    }
    if (cpu->addressBus > 0x817f) {
        cpu->status = ADDR_OUT_OF_BOUNDS;
    }
    cpu->dataBus = cpu->memory[cpu->addressBus];
}

void CPU_WriteReg(CPU* cpu) {
    switch (cpu->registerBus) {
        case 0x0:
            cpu->regA = cpu->dataBus;
            break;
        case 0x1:
            cpu->regB = cpu->dataBus;
            break;
        case 0x2:
            cpu->regC = cpu->dataBus;
            break;
        case 0x3:
            cpu->SP = cpu->dataBus;
            break;
        default:
            cpu->status = UNKNOWN_REGISTER;
            break;
    }
}

void CPU_ReadReg(CPU* cpu) {
    switch (cpu->registerBus) {
        case 0x0:
            cpu->dataBus = cpu->regA;
            break;
        case 0x1:
            cpu->dataBus = cpu->regB;
            break;
        case 0x2:
            cpu->dataBus = cpu->regC;
            break;
        case 0x3:
            cpu->dataBus = cpu->SP;
            break;
        default:
            cpu->status = UNKNOWN_REGISTER;
            break;
    }
}

void CPU_WritePort(CPU* cpu) {
    if (cpu->addressBus > 15) {
        cpu->status = ADDR_OUT_OF_BOUNDS;
        return;
    }

    cpu->ports[cpu->addressBus] = cpu->dataBus;
}

void CPU_ReadPort(CPU* cpu) {
    if (cpu->addressBus > 15) {
        cpu->status = ADDR_OUT_OF_BOUNDS;
        return;
    }

    cpu->dataBus = cpu->ports[cpu->addressBus];
}

void CPU_Push(CPU* cpu) {
    if (cpu->SP + stackStart > stackEnd) {
        cpu->status = STACK_OUT_OF_BOUNDS;
        return;
    }
    cpu->memory[cpu->SP + stackStart] = cpu->dataBus;
    cpu->SP++;
}

void CPU_Pop(CPU* cpu) {
    cpu->SP--;
    cpu->dataBus = cpu->memory[cpu->SP + stackStart];
    CPU_WriteReg(cpu);
    cpu->memory[cpu->SP + stackStart] = 0;
}

void CPU_Jmp(CPU* cpu) {
    if (cpu->addressBus < romStart) {
        cpu->status = INVALID_JMP_ADDR;
        return;
    }
    cpu->PC = cpu->addressBus;
    return;
}

void CPU_Cmp(CPU* cpu) {
    uint16_t right = 0;
    uint16_t left = 0;

    switch (cpu->opTp) {
        case ADDR_VAL:
            right = cpu->dataBus;
            CPU_ReadMem(cpu);
            left = cpu->dataBus;
            break;
        case ADDR_REG:
            CPU_ReadReg(cpu);
            right = cpu->dataBus;
            CPU_ReadMem(cpu);
            left = cpu->dataBus;
            break;
        case REG_ADDR:
            CPU_ReadReg(cpu);
            left = cpu->dataBus;
            CPU_ReadMem(cpu);
            right = cpu->dataBus;
            break;
        case REG_VAL:
            right = cpu->dataBus;
            CPU_ReadReg(cpu);
            left = cpu->dataBus;
            break;
        case REG_REG:
            left = cpu->registerBus;
            right = cpu->dataBus;
            cpu->registerBus = right;
            CPU_ReadReg(cpu);
            right = cpu->dataBus;
            cpu->registerBus = left;
            CPU_ReadReg(cpu);
            left = cpu->dataBus;
            break;
    }

    cpu->zero = ((left - right) == 0);
    cpu->less = (left < right);
    cpu->greater = (left > right);
}

void CPU_DoMath(CPU* cpu, uint8_t type) {
    uint16_t right = 0;
    uint16_t left = 0;

    switch (cpu->opTp) {
        case ADDR_VAL:
            right = cpu->dataBus;
            CPU_ReadMem(cpu);
            left = cpu->dataBus;
            break;
        case ADDR_REG:
            CPU_ReadReg(cpu);
            right = cpu->dataBus;
            CPU_ReadMem(cpu);
            left = cpu->dataBus;
            break;
        case REG_ADDR:
            CPU_ReadReg(cpu);
            left = cpu->dataBus;
            CPU_ReadMem(cpu);
            right = cpu->dataBus;
            break;
        case REG_VAL:
            right = cpu->dataBus;
            CPU_ReadReg(cpu);
            left = cpu->dataBus;
            break;
        case REG_REG:
            left = cpu->registerBus;
            right = cpu->dataBus;
            cpu->registerBus = right;
            CPU_ReadReg(cpu);
            right = cpu->dataBus;
            cpu->registerBus = left;
            CPU_ReadReg(cpu);
            left = cpu->dataBus;
            break;
    }

    switch (type) {
        case ADD:
            cpu->dataBus = left + right;
            break;
        case SUB:
            cpu->dataBus = left - right;
            break;
        case MUL:
            cpu->dataBus = left * right;
            break;
        case DIV:
            if (left == 0 || right == 0) {
                cpu->status = DIVIDE_BY_ZERO;
                return;
            }
            cpu->dataBus = left / right;
            break;
    }
}

uint16_t opcode = 0;
uint8_t tmpReg = 0;

int CPU_Execute(CPU* cpu, uint8_t cycles) {
    cpu->status = OK;
    if (cpu->PC < 0xffff) {
        opcode = CPU_Fetch(cpu, &cycles);
        switch (opcode) {
            case 0xFFFF:
                // Halt
                cpu->status = HALT;
                return 0;
                break;
            case 0x0000:
                // NOP
                cycles = 3;
                break;
            
            // =======================================
            // =========== MOV INSTRUCTION ===========
            // =======================================

            case 0x0001:
                // MOV [ADDR], VAL
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0002:
                // MOV [ADDR], REG
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0003:
                // MOV REG, [ADDR]
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0004:
                // MOV REG, VAL
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0005:
                // MOV REG, REG
                tmpReg = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                cpu->registerBus = tmpReg;
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            
            // ========================================
            // =========== PUSH INSTRUCTION ===========
            // ========================================

            case 0x0006:
                // PUSH [ADDR]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadMem(cpu);
                CPU_Push(cpu);
                break;
            case 0x0007:
                // PUSH VAL
                cycles--;
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_Push(cpu);
                break;
            case 0x0008:
                // PUSH REG
                cycles--;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                CPU_Push(cpu);
                break;
            case 0x0009:
                // POP REG
                cycles--;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_Pop(cpu);
                break;
            
            // =======================================
            // =========== JMP INSTRUCTION ===========
            // =======================================

            case 0x000A:
                // JMP [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_Jmp(cpu);
                break;
            case 0x000B:
                // JE [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                if (cpu->zero) {
                    CPU_Jmp(cpu);
                }
                break;
            case 0x000C:
                // JNE [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                if (!cpu->zero) {
                    CPU_Jmp(cpu);
                }
                break;
            case 0x000D:
                // JLE [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                if (cpu->zero || cpu->less) {
                    CPU_Jmp(cpu);
                }
                break;
            case 0x000E:
                // JL [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                if (cpu->less) {
                    CPU_Jmp(cpu);
                }
                break;
            case 0x000F:
                // JGE [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                if (cpu->zero || cpu->greater) {
                    CPU_Jmp(cpu);
                }
                break;
            case 0x0010:
                // JG [addr]
                cycles--;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                if (cpu->greater) {
                    CPU_Jmp(cpu);
                }
                break;
            
            // =======================================
            // =========== CMP INSTRUCTION ===========
            // =======================================

            case 0x0011:
                // CMP [addr], VAL
                cpu->opTp = ADDR_VAL;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_Cmp(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0012:
                // CMP [addr], REG
                cpu->opTp = ADDR_REG;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_Cmp(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0013:
                // CMP REG, [addr]
                cpu->opTp = REG_ADDR;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_Cmp(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0014:
                // CMP REG, VAL
                cpu->opTp = REG_VAL;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_Cmp(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0015:
                // CMP REG, REG
                cpu->opTp = REG_REG;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_Cmp(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            // =====================================
            // =========== ADD OPERATION ===========
            // =====================================
            case 0x0016:
                // ADD [addr], VAL
                cpu->opTp = ADDR_VAL;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, ADD);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0017:
                // ADD [addr], REG
                cpu->opTp = ADDR_REG;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, ADD);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0018:
                // ADD REG, [ADDR]
                cpu->opTp = REG_ADDR;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, ADD);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0019:
                // ADD REG, VAL
                cpu->opTp = REG_VAL;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, ADD);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x001a:
                // ADD REG, REG
                cpu->opTp = REG_REG;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, ADD);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;

            // =====================================
            // =========== SUB OPERATION ===========
            // =====================================

            case 0x001b:
                // SUB [addr], VAL
                cpu->opTp = ADDR_VAL;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, SUB);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x001c:
                // SUB [addr], REG
                cpu->opTp = ADDR_REG;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, SUB);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x001d:
                // SUB REG, [ADDR]
                cpu->opTp = REG_ADDR;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, SUB);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x001e:
                // SUB REG, VAL
                cpu->opTp = REG_VAL;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, SUB);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x001f:
                // SUB REG, REG
                cpu->opTp = REG_REG;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, SUB);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            
            // =====================================
            // =========== MUL OPERATION ===========
            // =====================================

            case 0x0020:
                // MUL [addr], VAL
                cpu->opTp = ADDR_VAL;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, MUL);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0021:
                // MUL [addr], REG
                cpu->opTp = ADDR_REG;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, MUL);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0022:
                // MUL REG, [ADDR]
                cpu->opTp = REG_ADDR;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, MUL);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0023:
                // MUL REG, VAL
                cpu->opTp = REG_VAL;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, MUL);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0024:
                // MUL REG, REG
                cpu->opTp = REG_REG;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, MUL);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            
            // =====================================
            // =========== DIV OPERATION ===========
            // =====================================

            case 0x0025:
                // DIV [addr], VAL
                cpu->opTp = ADDR_VAL;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, DIV);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0026:
                // DIV [addr], REG
                cpu->opTp = ADDR_REG;
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, DIV);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteMem(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0027:
                // DIV REG, [ADDR]
                cpu->opTp = REG_ADDR;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, DIV);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0028:
                // DIV REG, VAL
                cpu->opTp = REG_VAL;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, DIV);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0029:
                // DIV REG, REG
                cpu->opTp = REG_REG;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_DoMath(cpu, DIV);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            
            // =============================================
            // =========== ADDRBYREG INSTRUCTION ===========
            // =============================================
            case 0x002a:
                // ADDRBYREG
                cycles -= 2;
                cpu->addrByReg = 1;
                break;
            
            // ======================================================
            // =========== INPORT AND OUTPORT INSTRUCTION ===========
            // ======================================================
            case 0x002b:
                // IN REG VAL
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadPort(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x002c:
                // IN REG REG
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                cpu->registerBus = cpu->addressBus;
                cpu->addressBus = cpu->dataBus;
                CPU_ReadPort(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WriteReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x002d:
                // OUT VAL VAL
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_WritePort(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x002e:
                // OUT REG VAL
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                cpu->addressBus = cpu->dataBus;
                cpu->dataBus = CPU_Fetch(cpu, &cycles);
                CPU_WritePort(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x002f:
                // OUT VAL REG
                cpu->addressBus = CPU_Fetch(cpu, &cycles);
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WritePort(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
            case 0x0030:
                // OUT REG REG
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                cpu->addressBus = cpu->dataBus;
                cpu->registerBus = CPU_Fetch(cpu, &cycles);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                CPU_WritePort(cpu);
                CPU_ReadReg(cpu);
                if (cpu->status != OK) {
                    return 1;
                }
                break;
        }
    }
    return 0;
}
