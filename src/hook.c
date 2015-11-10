/*****************************************************************************
 * Copyright (c) 2014 Ted John
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifdef _WIN32
	#include <windows.h>
#else
	#include <sys/mman.h>
#endif // _WIN32
#include "hook.h"
#include "platform/platform.h"

void* g_hooktableaddress = 0;
int g_hooktableoffset = 0;
int g_maxhooks = 1000;

void hookfunc(int address, int newaddress, int stacksize, int registerargs[], int registersreturned, int eaxDestinationRegister)
{
	int i = 0;
	char data[100];

	registersreturned |= eaxDestinationRegister;

	int registerssaved = 7;
	int n = registersreturned;
	for (; n; registerssaved--) {
		n &= n - 1;
	}
	int numrargs = 0;
	for (int j = 0; ; j++) {
		if (registerargs[j] != END) {
			numrargs++;
		} else {
			break;
		}
	}

	int rargssize = numrargs * 4;

	data[i++] = 0x50; // push eax

	// move stack down for possible existing arguments
	for (int j = 0; j < stacksize; j++) {
		data[i++] = 0x8B; // mov eax, [esp+x]
		data[i++] = 0x44;
		data[i++] = 0xE4;
		data[i++] = (signed char)((4 * (stacksize - j)) + 4);

		data[i++] = 0x89; // mov [esp+x], eax
		data[i++] = 0x44;
		data[i++] = 0xE4;
		data[i++] = (signed char)((4 * (stacksize - j)) - ((registerssaved + stacksize) * 4));
	}

	if (numrargs > 0) {
		// push the registers to be on the stack to access as arguments
		data[i++] = 0x83; // add esp, x
		data[i++] = 0xC4;
		data[i++] = -((registerssaved + stacksize) * 4) + 4;

		for (signed int j = numrargs - 1; j >= 0; j--) {
			switch (registerargs[j]) {
				case EAX: data[i++] = 0x50; break;
				case EBX: data[i++] = 0x53; break;
				case ECX: data[i++] = 0x51; break;
				case EDX: data[i++] = 0x52; break;
				case ESI: data[i++] = 0x56; break;
				case EDI: data[i++] = 0x57; break;
				case EBP: data[i++] = 0x55; break;
			}
		}

		data[i++] = 0x83; // add esp, x
		data[i++] = 0xC4;
		data[i++] = rargssize + ((registerssaved + stacksize) * 4) - 4;
	}


	data[i++] = 0xE8; // call
	data[i++] = 0x00;
	data[i++] = 0x00;
	data[i++] = 0x00;
	data[i++] = 0x00;

	int sizec = i;

	data[i++] = 0x8B; // push eax, [esp]  - puts eip in eax
	data[i++] = 0x04;
	data[i++] = 0xE4;

	data[i++] = 0x83; // add eax, x
	data[i++] = 0xC0;
	int sizeoffset = i;
	data[i++] = 0; // set to returnlocation offset later

	data[i++] = 0x89; // mov [esp-20h], eax  - put return address on stack
	data[i++] = 0x44;
	data[i++] = 0xE4;
	data[i++] = (signed char)(-(registerssaved * 4) - rargssize - (stacksize * 4)) + 4;

	data[i++] = 0x83; // add esp, x
	data[i++] = 0xC4;
	data[i++] = 4;

	data[i++] = 0x58; // pop eax

	if (!(registersreturned & EAX)) {
		data[i++] = 0x50; // push eax
	}
	if (!(registersreturned & EBX)) {
		data[i++] = 0x53; // push ebx
	}
	if (!(registersreturned & ECX)) {
		data[i++] = 0x51; // push ecx
	}
	if (!(registersreturned & EDX)) {
		data[i++] = 0x52; // push edx
	}
	if (!(registersreturned & EBP)) {
		data[i++] = 0x55; // push ebp
	}
	if (!(registersreturned & ESI)) {
		data[i++] = 0x56; // push esi
	}
	if (!(registersreturned & EDI)) {
		data[i++] = 0x57; // push edi
	}

	data[i++] = 0x83; // sub esp, x
	data[i++] = 0xEC;
	data[i++] = 4 + (stacksize * 4) + rargssize;

	data[i++] = 0xE9; // jmp
	*((int *)&data[i]) = (newaddress - address - i - 4); i += 4;

	data[sizeoffset] = i - sizec;

	// returnlocation:

	switch (eaxDestinationRegister) {
	case EBX:
		// mov ebx, eax
		data[i++] = 0x8B;
		data[i++] = 0xD8;
		break;
	case ECX:
		// mov ecx, eax
		data[i++] = 0x8B;
		data[i++] = 0xC8;
		break;
	case EDX:
		// mov ecx, eax
		data[i++] = 0x8B;
		data[i++] = 0xD0;
		break;
	case ESI:
		// mov ecx, eax
		data[i++] = 0x8B;
		data[i++] = 0xF0;
		break;
	case EDI:
		// mov ecx, eax
		data[i++] = 0x8B;
		data[i++] = 0xF8;
		break;
	case EBP:
		// mov ecx, eax
		data[i++] = 0x8B;
		data[i++] = 0xE8;
		break;
	}

	data[i++] = 0x83; // sub esp, x
	data[i++] = 0xEC;
	data[i++] = (signed char)(stacksize * -4) - rargssize;

	if (!(registersreturned & EDI)) {
		data[i++] = 0x5F; // pop edi
	}
	if (!(registersreturned & ESI)) {
		data[i++] = 0x5E; // pop esi
	}
	if (!(registersreturned & EBP)) {
		data[i++] = 0x5D; // pop ebp
	}
	if (!(registersreturned & EDX)) {
		data[i++] = 0x5A; // pop edx
	}
	if (!(registersreturned & ECX)) {
		data[i++] = 0x59; // pop ecx
	}
	if (!(registersreturned & EBX)) {
		data[i++] = 0x5B; // pop ebx
	}
	if (!(registersreturned & EAX)) {
		data[i++] = 0x58; // pop eax
	}

	data[i++] = 0xC3; // retn

#ifdef _WIN32
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, data, i, 0);
#else
	// We own the pages with PROT_WRITE | PROT_EXEC, we can simply just memcpy the data
	memcpy((void *)address, data, i);
#endif // _WIN32
}

void addhook(int address, int newaddress, int stacksize, int registerargs[], int registersreturned, int eaxDestinationRegister)
{
	if (!g_hooktableaddress) {
		size_t size = g_maxhooks * 100;
#ifdef _WIN32
		g_hooktableaddress = VirtualAllocEx(GetCurrentProcess(), NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
#else
		g_hooktableaddress = mmap(NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (g_hooktableaddress == MAP_FAILED)
		{
			perror("mmap");
			exit(1);
		}
#endif // _WIN32
	}
	if (g_hooktableoffset > g_maxhooks) {
		return;
	}
	unsigned int hookaddress = (unsigned int)g_hooktableaddress + (g_hooktableoffset * 100);
	char data[9];
	int i = 0;
	data[i++] = 0xE9; // jmp
	*((int *)&data[i]) = hookaddress - address - i - 4; i += 4;
	data[i++] = 0xC3; // retn
#ifdef _WIN32
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)address, data, i, 0);
#else
	// We own the pages with PROT_WRITE | PROT_EXEC, we can simply just memcpy the data
	memcpy((void *)address, data, i);
#endif // _WIN32
	hookfunc(hookaddress, newaddress, stacksize, registerargs, registersreturned, eaxDestinationRegister);
	g_hooktableoffset++;
}
