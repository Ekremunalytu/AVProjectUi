/*
    Trojan Detection Rules
    YARA rules for detecting common trojan patterns
*/

rule Generic_Trojan_Dropper
{
    meta:
        description = "Detects generic trojan dropper patterns"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "high"

    strings:
        $drop1 = "CreateFile" ascii
        $drop2 = "WriteFile" ascii
        $drop3 = "GetTempPath" ascii
        $drop4 = "GetSystemDirectory" ascii
        $sus1 = "CreateProcess" ascii
        $sus2 = "ShellExecute" ascii
        $hide1 = "SetFileAttributes" ascii
        $hide2 = "FILE_ATTRIBUTE_HIDDEN" ascii

    condition:
        uint16(0) == 0x5A4D and // PE signature
        2 of ($drop*) and
        any of ($sus*) and
        any of ($hide*)
}

rule RAT_Behavior
{
    meta:
        description = "Detects Remote Access Trojan behavior patterns"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "high"

    strings:
        $rat1 = "GetCursorPos" ascii
        $rat2 = "GetDC" ascii
        $rat3 = "BitBlt" ascii
        $rat4 = "StretchBlt" ascii
        $screen1 = "screenshot" ascii wide nocase
        $screen2 = "desktop" ascii wide nocase
        $remote1 = "VNC" ascii wide nocase
        $remote2 = "remote" ascii wide nocase
        $remote3 = "TeamViewer" ascii wide nocase

    condition:
        uint16(0) == 0x5A4D and // PE signature
        (2 of ($rat*) or any of ($screen*) or any of ($remote*))
}

rule Banking_Trojan_Indicators
{
    meta:
        description = "Detects potential banking trojan indicators"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "critical"

    strings:
        $bank1 = "banking" ascii wide nocase
        $bank2 = "paypal" ascii wide nocase
        $bank3 = "amazon" ascii wide nocase
        $bank4 = "ebay" ascii wide nocase
        $inject1 = "SetWindowsHookEx" ascii
        $inject2 = "CallNextHookEx" ascii
        $inject3 = "UnhookWindowsHookEx" ascii
        $browser1 = "firefox" ascii wide nocase
        $browser2 = "chrome" ascii wide nocase
        $browser3 = "iexplore" ascii wide nocase

    condition:
        uint16(0) == 0x5A4D and // PE signature
        any of ($bank*) and
        2 of ($inject*) and
        any of ($browser*)
}

rule Cryptolocker_Patterns
{
    meta:
        description = "Detects potential ransomware/cryptolocker patterns"
        author = "AVProjectUi Team"
        date = "2025-06-05"
        severity = "critical"

    strings:
        $crypto1 = "CryptEncrypt" ascii
        $crypto2 = "CryptDecrypt" ascii
        $crypto3 = "CryptGenKey" ascii
        $crypto4 = "CryptAcquireContext" ascii
        $ransom1 = "ransom" ascii wide nocase
        $ransom2 = "bitcoin" ascii wide nocase
        $ransom3 = "decrypt" ascii wide nocase
        $ransom4 = "payment" ascii wide nocase
        $ext1 = ".encrypted" ascii wide nocase
        $ext2 = ".locked" ascii wide nocase

    condition:
        uint16(0) == 0x5A4D and // PE signature
        2 of ($crypto*) and
        (any of ($ransom*) or any of ($ext*))
}
