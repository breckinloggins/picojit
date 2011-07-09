#include <stdio.h>

/*
 * A little playground for VM and JIT testing.  The basic idea is to have an instruction set that is
 * EXTREMELY EASY to decode: we prefer more instructions versus complex decoding logic.  For example, 
 * most opcodes have a W version that lets the last argument (usually a word) take up an entire
 * wordsize.  Also, we don't have operand selector bits.  So OP_LOADI would load the address from
 * the register given by the second arg and OP_LOADM would use INS+1 to load the contents of a full
 * memory address. 
 */

typedef enum { 
	OP_HALT = 0, 
	OP_ADD, 		// Arg1 = REG, Arg2 = word
	OP_LOAD, 		// Arg1 = REG, Arg2 = word
	OP_LOADW,		// Arg1 = REG, Arg2 = <unused>, INS+1 = word
	OP_COPY,		// Arg1 = REG[dst], Arg2 = REG[src]
	OP_CNDPC,		// Arg1 = word
	OP_JMP,			// Arg1 = word
	OP_JMPEQ,		// Arg1 = REG, Arg2 = word
	OP_JMPEQW,		// Arg1 = REG, Arg2 = <unused>, INS+1 = word
	OP_PRINT		// Arg1 = REG, Arg2 = <unused> 
} opcode;

typedef enum { REG_0 = 0, REG_1, REG_2, REG_3, REG_COUNT} reg;

#define INS(op, arg1, arg2)	(((op) << 8) | ((arg1) << 4) | (arg2))
#define OP(ins)				(((ins) & 0xF00) >> 8)
#define ARG1(ins)			(((ins) & 0xF0) >> 4)
#define ARG2(ins)			((ins) & 0xF)

void native()	{
	int i = 0;
	
	for (i = 0; i < 1000000; i++)	{
		i = i + i;
	}
	
	fprintf(stdout, "i=%d\n", i);
	
}

void print_ins(int ins)	{
	fprintf(stdout, "INS=0x%x, OP=0x%x, ARG1=0x%x, ARG2=0x%x\n", 
		ins, OP(ins), ARG1(ins), ARG2(ins));
}

void vm()	{
	int regs[REG_COUNT] = {};
	int code[1024] = {};
	
	int pc, condpc;
	int ins;
	
	code[0] = INS(OP_CNDPC, 7, 0);
	code[1] = INS(OP_LOADW, REG_0, 0);
	code[2] = 0;
	code[3] = INS(OP_ADD, REG_0, 1);
	code[4] = INS(OP_JMPEQW, REG_0, 0);
	code[5] = 1000000;
	code[6] = INS(OP_JMP, 3, 0);
	code[7] = INS(OP_PRINT, REG_0, 0);
	code[8] = INS(OP_HALT, 0, 0);
	 
	pc = condpc = 0;
	
	while(1)	{
		ins = code[pc];
		
//		fprintf(stdout, "PC = 0x%x, ", pc);
//		print_ins(ins);
		
		switch (OP(ins))	{
		case OP_HALT:	return;
		case OP_LOAD:
			regs[ARG1(ins)] = ARG2(ins);
			break;
		case OP_LOADW:
			regs[ARG1(ins)] = code[++pc];
			break;
		case OP_ADD:
			regs[ARG1(ins)] += ARG2(ins);
			break;
		case OP_CNDPC:
			condpc = ARG1(ins);
			break;
		case OP_JMP:
			pc = ARG1(ins);
			continue;
		case OP_JMPEQ:
			if (regs[ARG1(ins)] == ARG2(ins))	{
				pc = condpc;
				continue;
			}
			break;
		case OP_JMPEQW:
			if (regs[ARG1(ins)] == code[++pc])	{
				pc = condpc;
				continue;
			}
			break;
		case OP_PRINT:
			fprintf(stdout, "%d\n", regs[ARG1(ins)]);
			break;
		default:
			fprintf(stderr, "Unrecognized opcode at 0x%x\n", pc);
			return;
		}
		
		pc++;
	}
	
}

int main(int argc, char** argv)	{
	int ins;
	//native();
	vm();
	return 0;
}

