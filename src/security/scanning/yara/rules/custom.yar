/*
    Custom Detection Rules
    Customizable YARA rules for specific threat detection
*/

rule Suspicious_PowerShell_Commands
{
    meta:
        description = "Detects suspicious PowerShell command patterns"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "medium"

    strings:
        $ps1 = "powershell" ascii wide nocase
        $ps2 = "pwsh" ascii wide nocase
        $cmd1 = "-EncodedCommand" ascii wide nocase
        $cmd2 = "-WindowStyle Hidden" ascii wide nocase
        $cmd3 = "-ExecutionPolicy Bypass" ascii wide nocase
        $cmd4 = "Invoke-Expression" ascii wide nocase
        $cmd5 = "DownloadString" ascii wide nocase
        $cmd6 = "WebClient" ascii wide nocase

    condition:
        any of ($ps*) and 2 of ($cmd*)
}

rule Packed_Executable
{
    meta:
        description = "Detects potentially packed executables"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "medium"

    strings:
        $upx1 = "UPX!" ascii
        $upx2 = "UPX0" ascii
        $upx3 = "UPX1" ascii
        $aspack = "aPLib" ascii
        $pecompact = "PECompact" ascii
        $themida = "Themida" ascii
        $vmprotect = "VMProtect" ascii

    condition:
        uint16(0) == 0x5A4D and // PE signature
        any of them
}

rule Suspicious_File_Operations
{
    meta:
        description = "Detects suspicious file operations that might indicate malware"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "medium"

    strings:
        $file1 = "DeleteFile" ascii
        $file2 = "MoveFile" ascii
        $file3 = "CopyFile" ascii
        $sys1 = "System32" ascii wide nocase
        $sys2 = "SysWOW64" ascii wide nocase
        $startup1 = "Startup" ascii wide nocase
        $startup2 = "Start Menu" ascii wide nocase
        $temp1 = "TEMP" ascii wide nocase
        $temp2 = "TMP" ascii wide nocase

    condition:
        uint16(0) == 0x5A4D and // PE signature
        2 of ($file*) and
        (any of ($sys*) or any of ($startup*) or any of ($temp*))
}

rule Anti_Analysis_Techniques
{
    meta:
        description = "Detects anti-analysis and evasion techniques"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "high"

    strings:
        $debug1 = "IsDebuggerPresent" ascii
        $debug2 = "CheckRemoteDebuggerPresent" ascii
        $debug3 = "FindWindow" ascii
        $vm1 = "VMware" ascii wide nocase
        $vm2 = "VirtualBox" ascii wide nocase
        $vm3 = "VBox" ascii wide nocase
        $vm4 = "QEMU" ascii wide nocase
        $sandbox1 = "sample" ascii wide nocase
        $sandbox2 = "sandbox" ascii wide nocase
        $sandbox3 = "cuckoo" ascii wide nocase

    condition:
        uint16(0) == 0x5A4D and // PE signature
        (any of ($debug*) or any of ($vm*) or any of ($sandbox*))
}
