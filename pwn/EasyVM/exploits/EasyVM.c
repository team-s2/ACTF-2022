#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <io.h>
#include <immintrin.h>

#define CODE_SIZE		0x1000
#define STACK_SIZE		0x1000
#define DATA_SIZE		0x1000
#define LONGEST_INSN	10

/* 
	 0000       0          00		 000      000       000         ...
	opcode   reg_op     bitwidth   dst_reg  src_reg1  src_reg2      imm
*/

enum Opcode
{
	OP_HALT,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_LD,
	OP_ST,
	OP_PUSH,
	OP_POP,
	OP_JMP,
	OP_JE,
	OP_JNE,
	OP_CALL,
	OP_RET,
	OP_SYSCALL,
};

enum Bitwidth
{
	BW_BYTE,
	BW_WORD,
	BW_DWORD,
	BW_QWORD
};

enum Reg
{
	RG_R0,
	RG_R1,
	RG_R2,
	RG_R3,
	RG_R4,
	RG_R5,
	RG_R6,
	RG_R7,
	RG_PC,
	RG_SP
};

enum ExcetionCode
{
	EC_STACK_UNDERFLOW,
	EC_STACK_OVERFLOW,
	EC_ILLEGAL_INSTRUCTION,
	EC_CODE_BUFFER_VIOLATION,
	EC_DATA_BUFFER_VIOLATION
};

enum SyscallNumber
{
	SN_READ,
	SN_WRITE,
	SN_OPEN,
	SN_CLOSE
};

typedef struct File
{
	char filename[0x10];
	uint64_t length;
	char* buffer;
} File, *File_t;

typedef struct VMState
{
	int64_t regs[10];
	char *code;
	uint64_t code_len;
	uint64_t *stack;
	uint64_t stack_len;
	char* data;
	uint64_t data_len;
	void (*ExceptionTable[5])(struct VMState *);
	void (*SyscallTable[4])(struct VMState *);
	File_t files[0x10];
} VMState, *VMState_t;

HANDLE StateHeap;
HANDLE FileHeap;
HANDLE BufferHeap;

void Error(const char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

void InitPrivateHeap()
{
	StateHeap = HeapCreate(0, 0x1000, 0);
	FileHeap = HeapCreate(0, 0x1000, 0);
	BufferHeap = HeapCreate(0, 0x1000, 0);

	if (StateHeap == NULL || FileHeap == NULL || BufferHeap == NULL)
		Error("Create heap failed!");
}

void InitBuf()
{
	setvbuf(stdin, 0, _IONBF, 0);
	setvbuf(stdout, 0, _IONBF, 0);
	setvbuf(stderr, 0, _IONBF, 0);
}

void InitRegs(VMState_t state)
{
	state->regs[RG_R0] = 0;
	state->regs[RG_R1] = 0;
	state->regs[RG_R2] = 0;
	state->regs[RG_R3] = 0;
	state->regs[RG_R4] = 0;
	state->regs[RG_R5] = 0;
	state->regs[RG_R6] = 0;
	state->regs[RG_R7] = 0;

	state->regs[RG_PC] = 0;
	state->regs[RG_SP] = STACK_SIZE / 8;
}

void DumpVMState(VMState_t state)
{
	int64_t i;

	printf("R0: 0x%016llx, R1: 0x%016llx\n", state->regs[RG_R0], state->regs[RG_R1]);
	printf("R2: 0x%016llx, R3: 0x%016llx\n", state->regs[RG_R2], state->regs[RG_R3]);
	printf("R4: 0x%016llx, R5: 0x%016llx\n", state->regs[RG_R4], state->regs[RG_R5]);
	printf("R6: 0x%016llx, R7: 0x%016llx\n", state->regs[RG_R6], state->regs[RG_R7]);
	printf("PC: 0x%016llx, SP: 0x%016llx\n", state->regs[RG_PC], state->regs[RG_SP]);

	printf("Stack Buffer:\n");
	for (i = state->regs[RG_SP] - 3; i <= state->regs[RG_SP]; i++)
	{
		printf("\t0x%04llx: 0x%016llx\n", i, state->stack[i]);
	}

	printf("Code Buffer:\n");
	printf("\t");
	for (i = state->regs[RG_PC];  i < state->regs[RG_PC] + 0x10; i++)
	{
		printf("0x%02x ", (unsigned char)state->code[i]);
	}
	printf("\n");

	printf("Opened file:\n");
	for (i = 3; i < 0x10; i++)
	{
		if (state->files[i] != NULL)
			printf("\tfileno: %lld filename: %s filesize: %llu\n", i, state->files[i]->filename, state->files[i]->length);
	}
	printf("\n");
}

void stack_underflow(VMState_t state)
{
	printf("[!]Error: Stack Underflow\n");
	DumpVMState(state);
	exit(1);
}

void stack_overflow(VMState_t state)
{
	printf("[!]Error: Stack Underflow\n");
	DumpVMState(state);
	exit(1);
}

void illegal_instruction(VMState_t state)
{
	printf("[!]Error: Illegal Out of Bounds\n");
	DumpVMState(state);
	exit(1);
}

void code_buffer_violation(VMState_t state)
{
	printf("[!]Error: Code Buffer Out of Bounds\n");
	DumpVMState(state);
	/* Not exit here, to provide information leak bug */
}

void data_buffer_violation(VMState_t state)
{
	printf("[!]Error: Data Buffer Out of Bounds\n");
	DumpVMState(state);
	exit(1);
}

void do_sys_read(VMState_t state)
{
	int64_t arg0 = state->regs[RG_R1];
	int64_t arg1 = state->regs[RG_R2]; 
	int64_t arg2 = state->regs[RG_R3];
	int64_t len;

	if (arg1 < 0 || (uint64_t)arg1 >= state->data_len || (uint64_t)arg2 > state->data_len - arg1)
		state->ExceptionTable[EC_DATA_BUFFER_VIOLATION](state);

	if (arg0 < 3)
	{
		state->regs[RG_R0] = _read(arg0, state->data + arg1, arg2);
	}
	else if(arg0 < 0x10)
	{
		if (state->files[arg0] != NULL && state->files[arg0]->buffer != NULL)
		{	
			len = (uint64_t)arg2 <= state->files[arg0]->length ? arg2 : state->files[arg0]->length;
			memcpy(state->data + arg1, state->files[arg0]->buffer, len);
			state->regs[RG_R0] = len;
		}
		else
		{
			state->regs[RG_R0] = -1;
		}
	}
}

void do_sys_write_vul(VMState_t state)
{
	int64_t arg0 = state->regs[RG_R1];
	int64_t arg1 = state->regs[RG_R2];
	int64_t arg2 = state->regs[RG_R3];

	if (arg1 < 0 || (uint64_t)arg1 >= state->data_len || (uint64_t)arg2 > state->data_len - arg1)
		state->ExceptionTable[EC_DATA_BUFFER_VIOLATION](state);

	if (arg0 < 3)
	{
		state->regs[RG_R0] = _write(arg0, state->data + arg1, arg2);
	}
	else if (arg0 < 0x10)
	{
		if (state->files[arg0] != NULL)
		{
			if (state->files[arg0]->length < (uint64_t)arg2)
			{
				HeapFree(BufferHeap, 0, state->files[arg0]->buffer);
				state->files[arg0]->buffer = (char*)HeapAlloc(BufferHeap, 0, state->files[arg0]->length);
				if (state->files[arg0]->buffer == NULL)
					Error("Unexpected error!");
				/* Not change file->length here to avoid OOB read */
			}
			/* OOB write here to do unlink attack */
			memcpy(state->files[arg0]->buffer, state->data + arg1, arg2);

			state->regs[RG_R0] = arg2;
		}
	}
}

void do_sys_write(VMState_t state)
{
	int64_t arg0 = state->regs[RG_R1];
	int64_t arg1 = state->regs[RG_R2];
	int64_t arg2 = state->regs[RG_R3];

	if (arg1 < 0 || (uint64_t)arg1 >= state->data_len || (uint64_t)arg2 > state->data_len - arg1)
		state->ExceptionTable[EC_DATA_BUFFER_VIOLATION](state);

	if (arg0 < 3)
	{
		state->regs[RG_R0] = _write(arg0, state->data + arg1, arg2);
	}
	else if (arg0 < 0x10)
	{
		if (state->files[arg0] != NULL)
		{
			if (state->files[arg0]->length < (uint64_t)arg2)
			{
				HeapFree(BufferHeap, 0, state->files[arg0]->buffer);
				state->files[arg0]->buffer = (char *)HeapAlloc(BufferHeap, 0, arg2);
				if (state->files[arg0]->buffer == NULL)
					Error("Unexpected error!");
				state->files[arg0]->length = arg2;
			}
			memcpy(state->files[arg0]->buffer, state->data + arg1, arg2);

			state->regs[RG_R0] = arg2;
		}
	}
}

void do_sys_open(VMState_t state)
{
	int64_t arg0 = state->regs[RG_R1];
	int64_t arg1 = state->regs[RG_R2];
	int filename_len;

	if ((uint64_t)arg0 >= state->data_len)
		state->ExceptionTable[EC_DATA_BUFFER_VIOLATION](state);

	filename_len = strlen(state->data + arg0) > state->data_len - arg0 ? state->data_len - arg0 : strlen(state->data + arg0);

	if (arg1 != 0 && arg1 != 1)
	{
		state->regs[RG_R0] = -1;
		return;
	}

	int i;
	for (i = 3; i < 0x10; i++)
	{
		if (state->files[i] == NULL)
			break;
	}

	if (i == 0x10)
	{
		state->regs[RG_R0] = -1;
		return;
	}
	
	state->regs[RG_R0] = i;
	state->files[i] = (File_t)HeapAlloc(FileHeap, 0, sizeof(File));
	if (state->files[i] == NULL)
		Error("Unexpected error!");
	state->files[i]->buffer = NULL;
	state->files[i]->length = 0;
	state->files[i]->filename[0] = 0;
	memcpy(state->files[i]->filename, state->data + arg0, filename_len > 0xF ? 0xF : filename_len);
	state->files[i]->filename[0xF] = 0;

}

void do_sys_close(VMState_t state)
{
	int64_t arg0 = state->regs[RG_R1];

	if (arg0 < 3)
	{
		state->regs[RG_R0] = _close((int)arg0);
	}
	else if (arg0 < 0x10)
	{
		if (state->files[arg0]->buffer != 0)
			HeapFree(BufferHeap, 0, state->files[arg0]->buffer);
		HeapFree(FileHeap, 0, state->files[arg0]);
		state->files[arg0] = NULL;

		state->regs[RG_R0] = 0;
	}
	else
	{
		state->regs[RG_R0] = -1;
	}
}

void InitFunctionTables(VMState_t state)
{
	/* To be done */
	state->ExceptionTable[EC_STACK_UNDERFLOW] = stack_underflow;
	state->ExceptionTable[EC_STACK_OVERFLOW] = stack_overflow;
	state->ExceptionTable[EC_ILLEGAL_INSTRUCTION] = illegal_instruction;
	state->ExceptionTable[EC_CODE_BUFFER_VIOLATION] = code_buffer_violation;
	state->ExceptionTable[EC_DATA_BUFFER_VIOLATION] = data_buffer_violation;

	state->SyscallTable[SN_READ] = do_sys_read;
	state->SyscallTable[SN_WRITE] = do_sys_write;
	state->SyscallTable[SN_OPEN] = do_sys_open;
	state->SyscallTable[SN_CLOSE] = do_sys_close;
}

void ClearFiles(VMState_t state)
{
	int i;
	for (i = 0; i < 0x10; i++)
	{
		state->files[i] = NULL;
	}
}

VMState_t InitVM()
{
	VMState_t state = (VMState_t)HeapAlloc(StateHeap, HEAP_ZERO_MEMORY, sizeof(VMState));
	if (state == NULL)
		Error("Create VM failed!");

	InitRegs(state);

	state->code = (char *)HeapAlloc(BufferHeap, HEAP_ZERO_MEMORY, CODE_SIZE);
	state->code_len = CODE_SIZE;
	state->stack = (uint64_t *)HeapAlloc(BufferHeap, HEAP_ZERO_MEMORY, STACK_SIZE);
	state->stack_len = STACK_SIZE / 8;
	state->data = (char *)HeapAlloc(BufferHeap, HEAP_ZERO_MEMORY, DATA_SIZE);
	state->data_len = DATA_SIZE;
	if (state->code == NULL || state->stack == NULL || state->data == NULL)
		Error("Init VM code buffer or stack buffer or data buffer failed!");

	InitFunctionTables(state);
	ClearFiles(state);

	return state;
}

void InputCode(VMState_t state)
{
	int i;

	printf("Please input your code: \n");
	for (i = 0; i < 0x1000; i++)
		if (_read(0, state->code + i, 1) < 0)
			Error("Read code failed!");

}

#define OPCODE	            (state->code[state->regs[RG_PC]] & 0xF)
#define BITWIDTH            ((state->code[state->regs[RG_PC]] >> 5) & 0x03)
#define MASK(val)			(val & _bzhi_u64(-1LL, 8ULL << BITWIDTH))
#define REG_OP              (state->code[state->regs[RG_PC]] & 0x10)
#define DST_REG             ((state->code[state->regs[RG_PC]] >> 7) & 0x1 | ((state->code[state->regs[RG_PC] + 1] & 0x03) << 1))
#define SRC_REG_0           ((state->code[state->regs[RG_PC] + 1] >> 2) & 0x7)
#define SRC_REG_1           ((state->code[state->regs[RG_PC] + 1] >> 5) & 0x7)
#define IMM_8               (*(int16_t *)(&state->code[state->regs[RG_PC] + 2]))
#define IMM_16              (*(int16_t *)(&state->code[state->regs[RG_PC] + 2]))
#define IMM_32              (*(int32_t *)(&state->code[state->regs[RG_PC] + 2]))
#define IMM_64              (*(int64_t *)(&state->code[state->regs[RG_PC] + 2]))


void OperAdd(VMState_t state)
{
	int64_t src_reg_0_val, src_reg_1_val, src_imm;

	if (REG_OP)
	{
		src_reg_0_val = MASK(state->regs[SRC_REG_0]);
	    src_reg_1_val = MASK(state->regs[SRC_REG_1]);

		state->regs[DST_REG] = MASK(src_reg_0_val + src_reg_1_val);

		state->regs[RG_PC] += 2;
	}
	else
	{
		switch (BITWIDTH)
		{
			case BW_BYTE:
				src_imm = IMM_8;
				break;
			case BW_WORD:
				src_imm = IMM_16;
				break;
			case BW_DWORD:
				src_imm = IMM_32;
				break;
			case BW_QWORD:
				src_imm = IMM_64;
				break;
			default:
				src_imm = 0;
				break;
		}

		src_reg_0_val = MASK(state->regs[SRC_REG_0]);

		state->regs[DST_REG] = MASK(src_reg_0_val + src_imm);

		state->regs[RG_PC] += 2 + (1ULL << BITWIDTH);
	}
}

void OperSub(VMState_t state)
{
	int64_t src_reg_0_val, src_reg_1_val, src_imm;

	if (REG_OP)
	{
		src_reg_0_val = MASK(state->regs[SRC_REG_0]);
		src_reg_1_val = MASK(state->regs[SRC_REG_1]);

		state->regs[DST_REG] = MASK(src_reg_0_val - src_reg_1_val);

		state->regs[RG_PC] += 2;
	}
	else
	{
		switch (BITWIDTH)
		{
			case BW_BYTE:
				src_imm = IMM_8;
				break;
			case BW_WORD:
				src_imm = IMM_16;
				break;
			case BW_DWORD:
				src_imm = IMM_32;
				break;
			case BW_QWORD:
				src_imm = IMM_64;
				break;
			default:
				src_imm = 0;
				break;
		}

		src_reg_0_val = MASK(state->regs[SRC_REG_0]);

		state->regs[DST_REG] = MASK(src_reg_0_val - src_imm);

		state->regs[RG_PC] += 2 + (1ULL << BITWIDTH);
	}
}

void OperMul(VMState_t state)
{
	int64_t src_reg_0_val, src_reg_1_val, src_imm;

	if (REG_OP)
	{
		src_reg_0_val = MASK(state->regs[SRC_REG_0]);
		src_reg_1_val = MASK(state->regs[SRC_REG_1]);

		state->regs[DST_REG] = MASK(src_reg_0_val * src_reg_1_val);

		state->regs[RG_PC] += 2;
	}
	else
	{
		switch (BITWIDTH)
		{
			case BW_BYTE:
				src_imm = IMM_8;
				break;
			case BW_WORD:
				src_imm = IMM_16;
				break;
			case BW_DWORD:
				src_imm = IMM_32;
				break;
			case BW_QWORD:
				src_imm = IMM_64;
				break;
			default:
				src_imm = 0;
				break;
		}

		src_reg_0_val = MASK(state->regs[SRC_REG_0]);

		state->regs[DST_REG] = MASK(src_reg_0_val * src_imm);

		state->regs[RG_PC] += 2 + (1ULL << BITWIDTH);
	}
}

void OperDiv(VMState_t state)
{
	int64_t src_reg_0_val, src_reg_1_val, src_imm;

	if (REG_OP)
	{
		if (state->regs[SRC_REG_1] == 0)
		{
			puts("Warning: Divide by zero, skipped\n");

			state->regs[RG_PC] += 2;

			return;
		}

		src_reg_0_val = MASK(state->regs[SRC_REG_0]);
		src_reg_1_val = MASK(state->regs[SRC_REG_1]);

		state->regs[DST_REG] = MASK(src_reg_0_val / src_reg_1_val);

		state->regs[RG_PC] += 2;
	}
	else
	{
		if (IMM_8 == 0)
		{
			puts("Warning: Divide by zero, skipped\n");

			state->regs[RG_PC] += 2 + (1ULL << BITWIDTH);

			return;
		}

		switch (BITWIDTH)
		{
			case BW_BYTE:
				src_imm = IMM_8;
				break;
			case BW_WORD:
				src_imm = IMM_16;
				break;
			case BW_DWORD:
				src_imm = IMM_32;
				break;
			case BW_QWORD:
				src_imm = IMM_64;
				break;
			default:
				src_imm = 0;
				break;
		}

		src_reg_0_val = MASK(state->regs[SRC_REG_0]);

		state->regs[DST_REG] = MASK(src_reg_0_val / src_imm);

		state->regs[RG_PC] += 2 + (1ULL << BITWIDTH);
	}
}

void OperLd(VMState_t state)
{
	int64_t offset;
	int64_t val;

	offset = IMM_16;

	if(offset < 0 || offset + (1LL << BITWIDTH) > 0x1000)
		state->ExceptionTable[EC_DATA_BUFFER_VIOLATION](state);

	switch (BITWIDTH)
	{
		case BW_BYTE:
			val = *(int8_t *)(state->data + offset);
			break;
		case BW_WORD:
			val = *(int16_t *)(state->data + offset);
			break;
		case BW_DWORD:
			val = *(int32_t *)(state->data + offset);
			break;
		case BW_QWORD:
			val = *(int64_t *)(state->data + offset);
			break;
	}

	state->regs[DST_REG] = MASK(val);

	state->regs[RG_PC] += 4;
}

void OperSt(VMState_t state)
{
	int64_t offset;
	int64_t val;

	offset = IMM_16;

	if (offset < 0 || offset + (1LL << BITWIDTH) > 0x1000)
		state->ExceptionTable[EC_DATA_BUFFER_VIOLATION](state);

	val = MASK(state->regs[DST_REG]);

	switch (BITWIDTH)
	{
		case BW_BYTE:
			*(int8_t *)(state->data + offset) = val;
			break;
		case BW_WORD:
			*(int16_t *)(state->data + offset) = val;
			break;
		case BW_DWORD:
			*(int32_t *)(state->data + offset) = val;
			break;
		case BW_QWORD:
			*(int64_t *)(state->data + offset) = val;
			break;
	}

	state->regs[RG_PC] += 4;
}

void OperPush(VMState_t state)
{
	if (state->regs[RG_SP] <= 0)
		state->ExceptionTable[EC_STACK_UNDERFLOW](state);

	if (REG_OP)
	{
		state->stack[--state->regs[RG_SP]] = state->regs[DST_REG];

		state->regs[RG_PC] += 2;
	}
	else
	{
		state->stack[--state->regs[RG_SP]] = IMM_64;

		state->regs[RG_PC] += 10;
	}
}

void OperPop(VMState_t state)
{
	if ((uint64_t)state->regs[RG_SP] > state->stack_len)
		state->ExceptionTable[EC_STACK_OVERFLOW](state);

	state->regs[DST_REG] = state->stack[state->regs[RG_SP]++];

	state->regs[RG_PC] += 2;
}

void OperJmp(VMState_t state)
{
	state->regs[RG_PC] += 4LL + IMM_16;
}

void OperJe(VMState_t state)
{
	int64_t src_reg_0_val, src_reg_1_val, src_imm;

	src_reg_0_val = MASK(state->regs[SRC_REG_0]);
	src_reg_1_val = MASK(state->regs[SRC_REG_1]);

	src_imm = IMM_16;

	state->regs[RG_PC] += 4;
	state->regs[RG_PC] += src_reg_0_val == src_reg_1_val ? src_imm : 0;
}

void OperJne(VMState_t state)
{
	int64_t src_reg_0_val, src_reg_1_val, src_imm;

	src_reg_0_val = MASK(state->regs[SRC_REG_0]);
	src_reg_1_val = MASK(state->regs[SRC_REG_1]);

	src_imm = IMM_16;

	state->regs[RG_PC] += 4;
	state->regs[RG_PC] += src_reg_0_val != src_reg_1_val ? src_imm : 0;
}

void OperCall(VMState_t state)
{
	if (state->regs[RG_SP] <= 0)
		state->ExceptionTable[EC_STACK_UNDERFLOW](state);

	state->stack[--state->regs[RG_SP]] = state->regs[RG_PC] + 4;
	state->regs[RG_PC] = IMM_16;
}

void OperRet(VMState_t state)
{
	if ((uint64_t)state->regs[RG_SP] > state->stack_len)
		state->ExceptionTable[EC_STACK_OVERFLOW](state);

	state->regs[RG_PC] = state->stack[state->regs[RG_SP]++];
}

void OperSyscall(VMState_t state)
{
	if (state->regs[RG_R0] >= 0 && state->regs[RG_R0] <= 3)
		state->SyscallTable[state->regs[RG_R0]](state);
	else
		state->regs[RG_R0] = -1;

	state->regs[RG_PC] += 2;
}

void RunVM(VMState_t state)
{
	printf("Running...\n");

	while (1)
	{
		if (state->regs[RG_PC] < 0 || (uint64_t)state->regs[RG_PC] > state->code_len - LONGEST_INSN)
			state->ExceptionTable[EC_CODE_BUFFER_VIOLATION](state);

		switch (OPCODE)
		{
			case OP_HALT:
				return;
			case OP_ADD:
				OperAdd(state);
				break;
			case OP_SUB:
				OperSub(state);
				break;
			case OP_MUL:
				OperMul(state);
				break;
			case OP_DIV:
				__try
				{
					OperDiv(state);
				}
				__except (1)
				{
					puts("Warning: Divide by zero, skipped\n");
					state->regs[RG_PC] += 2;
					/* Backdoor, to provide OOB write bug */
					state->SyscallTable[SN_WRITE] = do_sys_write_vul;
				}
				break;
			case OP_LD:
				OperLd(state);
				break;
			case OP_ST:
				OperSt(state);
				break;
			case OP_PUSH:
				OperPush(state);
				break;
			case OP_POP:
				OperPop(state);
				break;
			case OP_JMP:
				OperJmp(state);
				break;
			case OP_JE:
				OperJe(state);
				break;
			case OP_JNE:
				OperJne(state);
				break;
			case OP_CALL:
				OperCall(state);
				break;
			case OP_RET:
				OperRet(state);
				break;
			case OP_SYSCALL:
				OperSyscall(state);
				break;
			default:
				return;
		}
	}
}

int main(void)
{
	VMState_t state;

	InitBuf();
	InitPrivateHeap();
	state = InitVM();
	InputCode(state);
	RunVM(state);

	return 0;
}