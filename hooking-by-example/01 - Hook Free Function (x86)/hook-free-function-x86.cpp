//This file shows how to install a (destructive) function hook
//into a free function that is part of the same program. "Destructive" 
//in this case just means that the hook payload does not contain a trampoline, 
//so after the hook is installed, the original function is no longer callable.

//This example is only valid when built in 32 bits (it will likely work in 64 bits, but isn't
//guaranteed to), and though it will work with incremental linking enabled, 
//stepping through code makes more sense with that turned off
#include <stdio.h>
#include <Windows.h>
#include <memoryapi.h>
#include <stdint.h>

#if !defined(_WIN64)

#define check(expr) if (!(expr)){ DebugBreak(); exit(-1); }

//the target and palyoad functions in this example are so small/trivial that
//enabling optimizations for them breaks this tiny example program, since we need
//at least 5 bytes of instructions
#pragma optimize( "", off )
int GetNum()
{
	return 99;
}

int HookPayload()
{
	return 1;
}
#pragma optimize( "", on )

int main()
{
	printf("Before Hook, GetNum() returns %i\n", GetNum());

	DWORD oldProtect;
	BOOL success = VirtualProtect(GetNum, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	check(success);

	//32 bit relative jump opcode is E9, takes 1 32 bit operand for jump offset
	uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };

	//to fill out the last 4 bytes of jmpInstruction, we need the offset between 
	//the payload function and the instruction immediately AFTER the jmp instruction
	const uint32_t relAddr = (uint32_t)HookPayload - ((uint32_t)GetNum + sizeof(jmpInstruction));
	memcpy(jmpInstruction + 1, &relAddr, 4);
	memcpy(GetNum, jmpInstruction, sizeof(jmpInstruction));

	printf("After Hook, getNum() returns %i\n", GetNum());

	return 0;
}

#else
int main() { printf("This program is only valid when compiled as a 32 bit executable\n"); return -1; }
#endif