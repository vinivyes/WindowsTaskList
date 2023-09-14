#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <winternl.h>
#include <psapi.h>
#include <algorithm>
#include <string>
#pragma comment(lib, "psapi.lib")

// Needed because Windows doesn't define this function in headers
extern "C" NTSTATUS NTAPI NtQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);

int main(int argc, char* argv[]) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    // Get the search query from command line argument
    std::wstring searchQuery;
    if (argc > 1) {
        // Convert argument to a wide string and to lower case for case insensitive comparison
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, argv[1], strlen(argv[1]), NULL, 0);
        wchar_t* wstr = new wchar_t[size_needed + 1];
        MultiByteToWideChar(CP_UTF8, 0, argv[1], strlen(argv[1]), wstr, size_needed);
        wstr[size_needed] = L'\0';
        searchQuery = std::wstring(wstr);
        delete[] wstr;
        std::transform(searchQuery.begin(), searchQuery.end(), searchQuery.begin(), towlower);
    }

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateToolhelp32Snapshot failed." << std::endl;
        return 1;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        std::cerr << "Process32First failed." << std::endl;
        return 1;
    }

    do {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess) {
            PROCESS_BASIC_INFORMATION pbi;
            ULONG returnLength;

            if (NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &returnLength) == 0) {
                PEB peb;
                RTL_USER_PROCESS_PARAMETERS upp;

                if (ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), NULL)) {
                    if (ReadProcessMemory(hProcess, peb.ProcessParameters, &upp, sizeof(upp), NULL)) {
                        wchar_t* cmdLine = new wchar_t[upp.CommandLine.Length / sizeof(wchar_t) + 1];
                        if (ReadProcessMemory(hProcess, upp.CommandLine.Buffer, cmdLine, upp.CommandLine.Length, NULL)) {
                            cmdLine[upp.CommandLine.Length / sizeof(wchar_t)] = L'\0';  // Null-terminate the string

                            std::wstring exeName = pe32.szExeFile;
                            std::transform(exeName.begin(), exeName.end(), exeName.begin(), towlower);

                            // If a search query was provided, only print processes that match the query
                            if (!searchQuery.empty()) {
                                if (exeName.find(searchQuery) != std::wstring::npos) {
                                    // Print only matching process details
                                    std::wcout << pe32.th32ProcessID << ": " << cmdLine << std::endl;
                                }
                            }
                            else {
                                // If no search query was provided, print all process details
                                std::wcout << pe32.th32ProcessID << ": " << cmdLine << std::endl;
                            }
                        }
                        delete[] cmdLine;
                    }
                }
            }

            CloseHandle(hProcess);
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return 0;
}
