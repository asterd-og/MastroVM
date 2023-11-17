import sys

movAddrVal = 0x0001
movAddrReg = 0x0002
movRegAddr = 0x0003
movRegVal  = 0x0004
movRegReg  = 0x0005

pushAddr   = 0x0006
pushVal    = 0x0007
pushReg    = 0x0008

popReg     = 0x0009

jmp        = 0x000a
je         = 0x000b
jne        = 0x000c
jle        = 0x000d
jl         = 0x000e
jge        = 0x000f
jg         = 0x0010

cmpAddrVal = 0x0011
cmpAddrReg = 0x0012
cmpRegAddr = 0x0013
cmpRegVal  = 0x0014
cmpRegReg  = 0x0015

addAddrVal = 0x0016
addAddrReg = 0x0017
addRegAddr = 0x0018
addRegVal  = 0x0019
addRegReg  = 0x001a

subAddrVal = 0x001b
subAddrReg = 0x001c
subRegAddr = 0x001d
subRegVal  = 0x001e
subRegReg  = 0x001f

mulAddrVal = 0x0020
mulAddrReg = 0x0021
mulRegAddr = 0x0022
mulRegVal  = 0x0023
mulRegReg  = 0x0024

divAddrVal = 0x0025
divAddrReg = 0x0026
divRegAddr = 0x0027
divRegVal  = 0x0028
divRegReg  = 0x0029

addrByReg  = 0x002a

inRegVal  = 0x002b
inRegReg  = 0x002c

outValVal  = 0x002d
outRegVal  = 0x002e
outValReg  = 0x002f
outRegReg  = 0x0030

#program = [cmpRegVal, 0x0, 0x0100, je, 0x817f + 9, addRegVal, 0x0, 0x0001, jmp, 0x817f, 0xffff]

program = []
ln = 1
mode = 0
labels = {}
labelsNeedChange = {}

def isReg(str_):
    if not str_.startswith('['):
        if 'sp' in str_.lower() or 'reg' in str_.lower():
            return True
    else:
        return False

def parseReg(reg):
    if 'reg' in reg:
        reg = reg.replace('reg', '').lower()
        if reg > 'c' or reg < 'a':
            print(f"Invalid register '{reg}' at line {ln}.")
            exit(1)
        if reg == 'a': return 0
        elif reg == 'b': return 1
        elif reg == 'c': return 2
    else:
        return 3

def parseHex(hex_):
    try:
        return int(hex_, 16)
    except:
        print(f"Invalid hex number at line {ln}.")
        exit(1)

def parseNum(num):
    try:
        return int(num, 16)
    except:
        try:
            return int(num, 10)
        except:
            print(f"Invalid decimal number at line {ln}.")
            exit(1)

def parseAddr(addr):
    addr = addr.replace('[', '').replace(']', '')
    addr = addr.lower()
    if isReg(addr):
        program.append(addrByReg)
        if 'reg' in addr:
            return parseReg(addr)
        if addr != 'sp':
            print(f"Invalid combination of operands at line {ln}.")
            exit(1)
        return 3
    return parseNum(addr)

def parseMov(line):
    val = 0
    addr = 0
    if line[1].startswith('['):
        mode = movAddrVal
        addr = parseAddr(line[1])

        if isReg(line[2]):
            mode = movAddrReg
            val = parseReg(line[2])
        else:
            val = parseNum(line[2])
    
    elif isReg(line[1]):
        mode = movRegVal
        addr = parseReg(line[1])
        if isReg(line[2]):
            mode = movRegReg
            val = parseReg(line[2])
        else:
            if line[2].startswith('['):
                mode = movRegAddr
                val = parseAddr(line[2])
            else: val = parseNum(line[2])
    
    program.append(mode)
    program.append(addr)
    program.append(val)

def parsePush(line):
    val = 0
    if line[1].startswith('['):
        mode = pushAddr
        val = parseAddr(line[1])
    elif isReg(line[1]):
        mode = pushReg
        val = parseReg(line[1])
    else:
        mode = pushVal
        val = parseNum(line[1])
    
    program.append(mode)
    program.append(val)

def parsePop(line):
    reg = 0
    if not isReg(line[1]):
        print(f"Invalid register at line {ln}.")
        exit(1)
    reg = parseReg(line[1])

    program.append(popReg)
    program.append(reg)

def parseJmp(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(jmp)
    program.append(addr)

def parseJe(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(je)
    program.append(addr)

def parseJne(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(jne)
    program.append(addr)

def parseJle(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(jle)
    program.append(addr)

def parseJl(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(jl)
    program.append(addr)

def parseJge(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(jge)
    program.append(addr)

def parseJg(line):
    addr = 0
    if not line[1].startswith('['):
        if line[1] not in labels:
            addr = 0x0000
            labelsNeedChange[len(program) + 1] = line[1]
        else:
            addr = labels[line[1]]
    else:
        addr = parseAddr(line[1])
    program.append(jg)
    program.append(addr)

def parseCmp(line):
    left = 0
    right = 0
    if line[1].startswith('['):
        mode = cmpAddrVal
        right = parseAddr(line[1])

        if isReg(line[2]):
            mode = cmpAddrReg
            left = parseReg(line[2])
        else:
            left = parseNum(line[2])
    
    elif isReg(line[1]):
        mode = cmpRegVal
        right = parseReg(line[1])
        if isReg(line[2]):
            mode = cmpRegReg
            left = parseReg(line[2])
        else:
            if line[2].startswith('['):
                mode = cmpRegAddr
                left = parseAddr(line[2])
            else: left = parseNum(line[2])
    
    program.append(mode)
    program.append(right)
    program.append(left)
    

def parseAdd(line):
    left = 0
    right = 0
    if line[1].startswith('['):
        mode = addAddrVal
        right = parseAddr(line[1])

        if isReg(line[2]):
            mode = addAddrReg
            left = parseReg(line[2])
        else:
            left = parseNum(line[2])
    
    elif isReg(line[1]):
        mode = addRegVal
        right = parseReg(line[1])
        if isReg(line[2]):
            mode = addRegReg
            left = parseReg(line[2])
        else:
            if line[2].startswith('['):
                mode = addRegAddr
                left = parseAddr(line[2])
            else: left = parseNum(line[2])
    
    program.append(mode)
    program.append(right)
    program.append(left)
    

def parseSub(line):
    left = 0
    right = 0
    if line[1].startswith('['):
        mode = subAddrVal
        right = parseAddr(line[1])

        if isReg(line[2]):
            mode = subAddrReg
            left = parseReg(line[2])
        else:
            left = parseNum(line[2])
    
    elif isReg(line[1]):
        mode = subRegVal
        right = parseReg(line[1])
        if isReg(line[2]):
            mode = subRegReg
            left = parseReg(line[2])
        else:
            if line[2].startswith('['):
                mode = subRegAddr
                left = parseAddr(line[2])
            else: left = parseNum(line[2])
    
    program.append(mode)
    program.append(right)
    program.append(left)

def parseMul(line):
    left = 0
    right = 0
    if line[1].startswith('['):
        mode = mulAddrVal
        right = parseAddr(line[1])

        if isReg(line[2]):
            mode = mulAddrReg
            left = parseReg(line[2])
        else:
            left = parseNum(line[2])
    
    elif isReg(line[1]):
        mode = mulRegVal
        right = parseReg(line[1])
        if isReg(line[2]):
            mode = mulRegReg
            left = parseReg(line[2])
        else:
            if line[2].startswith('['):
                mode = mulRegAddr
                left = parseAddr(line[2])
            else: left = parseNum(line[2])
    
    program.append(mode)
    program.append(right)
    program.append(left)

def parseDiv(line):
    left = 0
    right = 0
    if line[1].startswith('['):
        mode = mulAddrVal
        right = parseAddr(line[1])

        if isReg(line[2]):
            mode = mulAddrReg
            left = parseReg(line[2])
        else:
            left = parseNum(line[2])
    
    elif isReg(line[1]):
        mode = mulRegVal
        right = parseReg(line[1])
        if isReg(line[2]):
            mode = mulRegReg
            left = parseReg(line[2])
        else:
            if line[2].startswith('['):
                mode = mulRegAddr
                left = parseAddr(line[2])
            else: left = parseNum(line[2])
    
    program.append(mode)
    program.append(right)
    program.append(left)

def parseIn(line):
    left = 0
    right = 0
    if not isReg(line[1]):
        print(f"Invalid combination of operands at line {ln}.")
        exit(1)
    left = parseReg(line[1])
    if isReg(line[2]):
        mode = inRegReg
        right = parseReg(line[2])
    else:
        mode = inRegVal
        right = parseNum(line[2])
    
    program.append(mode)
    program.append(right)
    program.append(left)

def parseOut(line):
    left = 0
    right = 0

    if isReg(line[1]):
        left = parseReg(line[1])
        if isReg(line[2]):
            mode = outRegReg
            right = parseReg(line[2])
        else:
            mode = outRegVal
            right = parseNum(line[2])
    else:
        left = parseNum(line[1])
        if isReg(line[2]):
            mode = outValReg
            right = parseReg(line[2])
        else:
            mode = outValVal
            right = parseNum(line[2])

def parseLabel(line):
    labels[line[0].replace(':', '')] = len(program) + 0x817f

with open(sys.argv[1], 'r') as f:
    for line in f.readlines():
        line = line.replace('\n', '').split(' ')
        if line[0] == 'hlt':
            program.append(0xffff)
        elif line[0] == 'nop':
            program.append(0x0000)
        elif line[0] == 'mov':
            parseMov(line)
        elif line[0] == 'push':
            parsePush(line)
        elif line[0] == 'pop':
            parsePop(line)
        elif line[0] == 'jmp':
            parseJmp(line)
        elif line[0] == 'je':
            parseJe(line)
        elif line[0] == 'jne':
            parseJne(line)
        elif line[0] == 'jle':
            parseJle(line)
        elif line[0] == 'jl':
            parseJl(line)
        elif line[0] == 'jge':
            parseJge(line)
        elif line[0] == 'jg':
            parseJg(line)
        elif line[0] == 'cmp':
            parseCmp(line)
        elif line[0] == 'add':
            parseAdd(line)
        elif line[0] == 'dec':
            parseSub(line)
        elif line[0] == 'mul':
            parseMul(line)
        elif line[0] == 'div':
            parseDiv(line)
        elif line[0] == 'in':
            parseIn(line)
        elif line[0] == 'out':
            parseOut(line)
        elif line[0] == '':
            continue
        elif ':' in line[0]:
            parseLabel(line)
        else:
            print(f"tf is {line[0]}")
        ln += 1
                
program.append(0xffff)

for label in labelsNeedChange:
    if not labelsNeedChange[label] in labels:
        print(f"Error: Couldn't find label named {labelsNeedChange[label]}.")
        exit(1)
    else:
        program[label] = labels[labelsNeedChange[label]]

with open(sys.argv[2], 'wb') as file:
    for value in program:
        # Convert uint16 to bytes using little-endian byte order
        bytes_to_write = value.to_bytes(2, byteorder='little', signed=False)
        file.write(bytes_to_write)