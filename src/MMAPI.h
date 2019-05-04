#include <Windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <TlHelp32.h>
#include <stdio.h>
#include <algorithm>


namespace MMAPI
{

	DWORD GetProcessIdByName(char* name)
	{
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);
		HANDLE SnapShot32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if(SnapShot32 == INVALID_HANDLE_VALUE)
		{
			return -1;
		}
		if (Process32First(SnapShot32, &entry))
		{
			while (Process32Next(SnapShot32, &entry) == TRUE)
			{
				wchar_t* entryname = entry.szExeFile;
				std::wstring ws(entryname);
				std::string ProcessName(ws.begin(), ws.end());
				if (std::string(ProcessName) == std::string(name))
				{
					return entry.th32ProcessID;
				}
			}
		}
		return -1;
	}


	// protection memory functions
	template<typename T>
	DWORD protectMemory(HANDLE proc, LPVOID adr, DWORD prot) {
		DWORD oldProt;
		VirtualProtectEx(proc, adr, sizeof(T), prot, &oldProt);
		return oldProt;
	}


	// Read Memory function
	template<typename T>
	T readMemory(LPVOID adr) 
	{
		return *((T*)adr);
	}

	// Write Memory Function
	template<typename T>
	void writeMemory(LPVOID adr, T val) {
		*((T*)adr) = val;
	}


	// Write Nop in address
	template<int SIZE>
	void writeNop(DWORD address)
	{
		auto oldProtection =
			protectMemory<BYTE[SIZE]>(address, PAGE_EXECUTE_READWRITE);
		for (int i = 0; i < SIZE; i++)
			writeMemory<BYTE>(address + i, 0x90);
	}


	// asm functions
	DWORD ExtractAddress(int CurrentAddress, int DestinationAddress)
	{
		return DestinationAddress - CurrentAddress - 5;
	}

	// calcule correct jump offset
	DWORD CalculateJump(DWORD OriginalFunctionAddress, DWORD JMPOffset)
	{
		return OriginalFunctionAddress + (JMPOffset + 5);
	}


	// find in memory byte array
	DWORD FindPattern(DWORD StartAddres, DWORD EndAddres, char Bytes_to_find[], int Bytes_len )
	{
		DWORD PTR = StartAddres;
		char* buffer = new char[Bytes_len];
		while (PTR <= EndAddres)
		{
			memcpy(buffer, Bytes_to_find, Bytes_len);
			if (!buffer) break;
			if (std::strcmp(buffer, Bytes_to_find))
			{
				delete buffer;
				return PTR;
			}
		}
		delete buffer;
		return PTR;
	}


	struct MyHook
	{
		DWORD FunctionAddress;
		DWORD CallOffset;
		DWORD CallPointerFunction;
		DWORD* MyDllFunctionAddress;
	};



	DWORD DoHook(DWORD* FunctionAddress, DWORD* HookAddress)
	{
		std::cout << "FunctionAddress: " << FunctionAddress << std::endl;
		std::cout << "HookAddress: " << HookAddress << std::endl;
		DWORD OldProtection;
		VirtualProtect(FunctionAddress, 5, PAGE_EXECUTE_READWRITE, &OldProtection);
		DWORD HookOffset = (DWORD)HookAddress - (DWORD)FunctionAddress - 5;
		std::cout << "MyHookOffset: " << (DWORD*)HookOffset << std::endl;
		
		char* CharPointer = (char*)FunctionAddress;
		CharPointer++;
		FunctionAddress = (DWORD*)CharPointer;
		DWORD OriginalOffset = readMemory<DWORD>(FunctionAddress);
		writeMemory<DWORD>(FunctionAddress, HookOffset);
		std::cout << "Call is overwrited!" << std::endl;
		VirtualProtect(FunctionAddress, 5, OldProtection, &OldProtection);
		return OriginalOffset + (DWORD)FunctionAddress + 4;
	}


	void PlaceJMP(BYTE *pAddress, DWORD dwJumpTo, DWORD dwLen) {

		std::cout << "FunctionAddress: " << (DWORD*)pAddress << std::endl;
		std::cout << "MyFunctionAddres: " << (DWORD*)dwJumpTo << std::endl;
		DWORD dwOldProtect, dwBkup, dwRelAddr;

		VirtualProtect(pAddress, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		dwRelAddr = (DWORD)dwJumpTo - (DWORD)pAddress - 5;
		std::cout << "JMP Offset: " << (DWORD*)dwRelAddr << std::endl;


		// remove this

		*pAddress = 0xE9;

		*((DWORD *)(pAddress + 0x1)) = dwRelAddr;

		for (DWORD x = 0x5; x < dwLen; x++)
			*(pAddress + x) = 0x90;

		VirtualProtect(pAddress, dwLen, dwOldProtect, &dwBkup);

	}



}


