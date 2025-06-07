/*
    Debug Test Rules - Simple patterns to verify YARA is working
*/

rule Test_Simple_Pattern
{
    meta:
        description = "Simple test rule to verify YARA detection is working"
        author = "AVProjectUi Debug"
        date = "2025-06-07"
        severity = "low"

    strings:
        $test1 = "MALICIOUS_PATTERN" ascii nocase
        $test2 = "dangerous" ascii nocase
        $test3 = "suspicious" ascii nocase
        $test4 = "powershell" ascii nocase

    condition:
        any of them
}

rule Test_EICAR_Fixed
{
    meta:
        description = "Fixed EICAR test string detection"
        author = "AVProjectUi Debug"
        date = "2025-06-07"
        severity = "high"

    strings:
        $eicar = "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*" ascii
        $eicar_fixed = { 58 35 4F 21 50 25 40 41 50 5B 34 5C 50 5A 58 35 34 28 50 5E 29 37 43 43 29 37 7D 24 45 49 43 41 52 2D 53 54 41 4E 44 41 52 44 2D 41 4E 54 49 56 49 52 55 53 2D 54 45 53 54 2D 46 49 4C 45 21 24 48 2B 48 2A }

    condition:
        any of them
}

rule Test_PowerShell_Detection
{
    meta:
        description = "Test PowerShell suspicious command detection"
        author = "AVProjectUi Debug"
        date = "2025-06-07"
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
        any of ($ps*) and any of ($cmd*)
}
