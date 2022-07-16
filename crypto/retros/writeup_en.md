There are two steps to solve this challenge: AES Padding Oracle and VM Shellcode Construction. 

(1) AES Padding Oracle 

Here you are asked to input a ticket and fortune which actually are cipher and IV respectively. The length of ticket are no more than 32, which means you can decrypt at most two plaintext blocks(32 bytes). After decryption, the plaintext will be checked for the padding part. Here the expected padding format is PKCS7(e.g., if you need to pad two bytes, then the padding part is '\x02\x02'). When the padding part is not correct, the program will throw the statement, saying "Nonono, invaild ticket. ". However, the connection is not closed. It's a typical attack scenario of AES Padding Oracle. Using AES Padding Oracle Attack, we can construct abitrary plaintext. Just like this:

```python
code = bytes.fromhex('51f822566810501123323414244137598654209222815011191125f460420001')

from pwn import *

iv = b""
cipher2 = b'\x00' * 15 + b'\x00'
useless = b'\x00' * 16

io = remote(xxxxxx)

for i in range(16):
    for j in range(256):
        if j == 10:
            continue
        cur_iv = (15-i) * b'\x00' + bytes([j]) + strxor(iv, bytes([i+1]*(i)))
        assert len(cur_iv) == 16
        io.recvuntil("ticket: ")
        io.sendline(cur_iv + cipher2)
        io.recvuntil("fortune: ")
        io.sendline(useless)
        resp = io.recvuntil(".\n")
        if b"invaild ticket" in resp:
            continue
        else:
            print("find it", cur_iv + cipher2)
            iv = bytes([j]) + strxor(iv, bytes([i+1]*(i)))
            break
    if len(iv) != i+1:
        raise "not found"
    iv = strxor(iv, bytes([i+1]*(i+1)))
    print("round {}: {}".format(i+1, iv))

cipher1 = strxor(iv, code[16:])

iv = b''
for i in range(16):
    for j in range(256):
        if j == 10:
            continue
        cur_iv = (15-i) * b'\x00' + bytes([j]) + strxor(iv, bytes([i+1]*(i)))
        assert len(cur_iv) == 16
        io.recvuntil("ticket: ")
        io.sendline(cur_iv + cipher1)
        io.recvuntil("fortune: ")
        io.sendline(useless)
        resp = io.recvuntil(".\n")
        if b"invaild ticket" in resp:
            continue
        else:
            print("find it", cur_iv + cipher1)
            iv = bytes([j]) + strxor(iv, bytes([i+1]*(i)))
            break
    if len(iv) != i+1:
        raise "not found"
    iv = strxor(iv, bytes([i+1]*(i+1)))
    print("round {}: {}".format(i+1, iv))

cipher = cipher1 + cipher2
iv = strxor(code[:16], iv)

print("cipher:", cipher)
print("iv:", iv)
```

It is worth mentioning that in the logic of the program receiving the input, when encountering the char '\n', it will return. So in your arbitrary plaintext construction, you should ensure your input is absent of '\n', although you may introduce it unintentionally. So you can understand in the above script, there are two seemingly inexplicable code lines:

```python
        if j == 10:
            continue
```

(2) VM Shellcode Construction

In this step, you are going to implement a simply sort algorithm(not qsort, just bubble sort) using the prescribed instructions. At the same time, you can only use at most 31 bytes and the last byte must be '\x00'.

In this VM, there are 7 register, named as PC, IDX1, IDX2, TMP1, TMP2, IMM and FLAG. Here are simply instruction description.

| op   | operand | desp                               | size |
| ---- | ------- | ---------------------------------- | ---- |
| 0    |         | check                              | 4    |
| 1    | reg     | add reg, reg_imm                   | 8    |
| 2    | reg     | sub reg, reg_imm                   | 8    |
| 3    | reg_idx | load reg_t, list[reg_idx]          | 12   |
| 4    | reg_idx | store list[reg_idx], reg_t         | 12   |
| 5    | imm     | mov reg_imm, imm                   | 12   |
| 6    | imm     | movf reg_imm, imm                  | 12   |
| 7    |         | cmp list[reg_idx1], list[reg_idx2] | 4    |
| 8    | imm     | cmpi reg_idx, reg_imm              | 8    |
| 9    | reg_idx | movr reg_imm, reg_idx              | 8    |

In order to avoid unexpected solutions, I have tried my best to do a lot of bounds checking on the registers, while restricting certain instructions to the specified registers.

When I attempt to use these instructions to implement the sort algorithm, it actually spent 30.5 bytes(the $code​$ in the above script). That's why I set a 31-bytes bound and I believe you can use less bytes. Not surprisingly, both 0ops and zer0pts(https://furutsuki.hatenablog.com/entry/2022/06/28/001931) use 30 bytes to complete this shellcode.