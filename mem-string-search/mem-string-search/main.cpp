#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <TlHelp32.h>


typedef unsigned __int64 QWORD;

template <typename T>
void fatal(T error) {
	std::cout << error << std::endl;
	exit(1);
}

MODULEENTRY32 me32;
uintptr_t begin = 0;
uintptr_t end = 0;

template <typename T>
void search_string(HANDLE procHandle, T *search_string) {
	uintptr_t address = 0;
	SIZE_T read;
	while (address < end) {
		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQueryEx(procHandle, (LPCVOID)address, &mbi, sizeof(mbi)))
			fatal("virtual query error");
		T *buffer = (T*)calloc(mbi.RegionSize, sizeof(T));
		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS) {
			if (!ReadProcessMemory(procHandle, mbi.BaseAddress, buffer, mbi.RegionSize, &read)) {
				free(buffer);
				address += 4096;
				continue;
			}
			find_string(search_string, buffer, read);
		}
		address = address + mbi.RegionSize;
		free(buffer);
	}
}

void find_string(wchar_t *search_string, wchar_t *buffer, SIZE_T read) {
	DWORD i = 0;
	for (DWORD x = 0; x < read; x++) {
		if (buffer[x] != search_string[i]) {
			i = 0;
			continue;
		}
		i++;
		if (i == wcslen(search_string)) {
			std::cout << "found" << std::endl;
			size_t outputSize = 100 + wcslen(search_string);
			wchar_t *output = (wchar_t*)calloc(outputSize + 8, sizeof(wchar_t));
			wcsncpy_s(output, outputSize + 1, &buffer[x - i] + 1, outputSize);
			printf("%ls\n", output);
			free(output);
			i = 0;
		}
	}
}

void find_string(unsigned char *search_string, unsigned char *buffer, SIZE_T read) {
	DWORD i = 0;
	for (DWORD x = 0; x < read; x++) {
		if (buffer[x] != search_string[i]) {
			i = 0;
			continue;
		}
		i++;
		if (i == strlen((const char*)search_string)) {
			std::cout << "found" << std::endl;
			size_t outputSize = 100 + strlen((const char*)search_string);
			char *output = (char*)calloc(outputSize + 8, sizeof(char));
			strncpy_s(output, outputSize + 1, (const char*)&buffer[x - i] + 1, outputSize);
			printf("%s\n", output);
			free(output);
			i = 0;
		}
	}
}

int main(int argc, char **argv) {
	unsigned char SEARCH_STRING[] = "CardNumber=";
	wchar_t SEARCH_WSTRING[] = L"CardNumber=";
	HANDLE snap_shot = INVALID_HANDLE_VALUE;
	DWORD processID;
	std::cin >> processID;
	HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	if (!procHandle) {
		fatal("couldn't get handle");
	}
	snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
	if (snap_shot == INVALID_HANDLE_VALUE)
		fatal("snapshot error");
	me32.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(snap_shot, &me32))
		fatal("module32first error");
	std::cout << me32.szExePath << std::endl;
	begin = (uintptr_t)me32.modBaseAddr;
	end = begin + me32.modBaseSize;
	std::cout << "string search" << std::endl;
	search_string(procHandle, SEARCH_WSTRING);
	std::cout << "wstring search" << std::endl;
	search_string(procHandle, SEARCH_STRING);
	system("pause");
	return 0;
}