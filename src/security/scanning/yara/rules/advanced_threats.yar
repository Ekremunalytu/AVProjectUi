/*
    Advanced Threat Detection Rules
    More sophisticated YARA rules for modern threats
*/

import "pe"
import "math"
import "hash"

rule Advanced_Code_Injection
{
    meta:
        description = "Detects advanced code injection techniques"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "high"
        reference = "https://attack.mitre.org/techniques/T1055/"

    strings:
        $api1 = "NtCreateSection" ascii
        $api2 = "NtMapViewOfSection" ascii
        $api3 = "NtUnmapViewOfSection" ascii
        $api4 = "RtlCreateUserThread" ascii
        $api5 = "NtQueueApcThread" ascii
        
        // Process hollowing indicators
        $hollow1 = "ZwUnmapViewOfSection" ascii
        $hollow2 = "NtUnmapViewOfSection" ascii
        $hollow3 = "CREATE_SUSPENDED" ascii

    condition:
        uint16(0) == 0x5A4D and
        pe.is_pe and
        (
            (3 of ($api*)) or
            (any of ($hollow*) and any of ($api*))
        )
}

rule Fileless_Malware_Indicators
{
    meta:
        description = "Detects fileless malware techniques"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "critical"

    strings:
        $wmi1 = "Win32_Process" ascii wide
        $wmi2 = "CREATE" ascii wide
        $ps1 = "powershell.exe" ascii wide nocase
        $ps2 = "System.Management.Automation" ascii wide
        $mem1 = "VirtualAlloc" ascii
        $mem2 = "VirtualProtect" ascii
        
        // Base64 PowerShell indicators
        $b64_ps1 = "cG93ZXJzaGVsbA==" // "powershell" base64
        $b64_ps2 = "SW52b2tlLUV4cHJlc3Npb24=" // "Invoke-Expression" base64

    condition:
        (
            (any of ($wmi*) and any of ($ps*)) or
            (any of ($mem*) and any of ($ps*)) or
            any of ($b64_ps*)
        )
}

rule Living_Off_The_Land_Binaries
{
    meta:
        description = "Detects abuse of legitimate Windows binaries (LOLBins)"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "medium"
        reference = "https://lolbas-project.github.io/"

    strings:
        // Common LOLBins
        $lol1 = "regsvr32.exe" ascii wide nocase
        $lol2 = "rundll32.exe" ascii wide nocase
        $lol3 = "mshta.exe" ascii wide nocase
        $lol4 = "certutil.exe" ascii wide nocase
        $lol5 = "bitsadmin.exe" ascii wide nocase
        $lol6 = "wmic.exe" ascii wide nocase
        
        // Suspicious parameters
        $param1 = "-decode" ascii wide
        $param2 = "-urlcache" ascii wide
        $param3 = "-split" ascii wide
        $param4 = "/c start" ascii wide
        $param5 = "scrobj.dll" ascii wide
        $param6 = "javascript:" ascii wide

    condition:
        any of ($lol*) and any of ($param*)
}

rule Entropy_Based_Detection
{
    meta:
        description = "High entropy sections indicating packed/encrypted content"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "medium"

    condition:
        uint16(0) == 0x5A4D and
        pe.is_pe and
        for any section in pe.sections : (
            math.entropy(section.raw_data_offset, section.raw_data_size) >= 7.0 and
            section.raw_data_size > 0x1000
        )
}

rule Suspicious_Import_Hashing
{
    meta:
        description = "Detects suspicious import hashing techniques"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "high"

    strings:
        $hash1 = { 6A 40 68 00 30 00 00 } // Common shellcode pattern
        $hash2 = { 64 8B 25 30 00 00 00 } // PEB access
        $hash3 = { 8B 40 0C 8B 70 14 } // LDR access
        
    condition:
        uint16(0) == 0x5A4D and
        pe.is_pe and
        pe.imports("kernel32.dll", "GetProcAddress") and
        pe.imports("kernel32.dll", "LoadLibraryA") and
        any of ($hash*)
}

rule Advanced_Persistence_Mechanisms
{
    meta:
        description = "Detects advanced persistence techniques"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "high"

    strings:
        // COM hijacking
        $com1 = "HKEY_CURRENT_USER\\Software\\Classes\\CLSID" ascii wide
        $com2 = "InprocServer32" ascii wide
        
        // Service persistence
        $svc1 = "CreateServiceA" ascii
        $svc2 = "CreateServiceW" ascii
        $svc3 = "ChangeServiceConfigA" ascii
        
        // DLL hijacking
        $dll1 = "SetDllDirectoryA" ascii
        $dll2 = "SetCurrentDirectoryA" ascii
        
        // Scheduled tasks
        $task1 = "schtasks" ascii wide
        $task2 = "TaskScheduler" ascii wide
        $task3 = "ITaskService" ascii wide

    condition:
        uint16(0) == 0x5A4D and
        (
            (any of ($com*)) or
            (any of ($svc*)) or
            (any of ($dll*)) or
            (any of ($task*))
        )
}
