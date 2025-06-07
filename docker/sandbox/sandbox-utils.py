#!/usr/bin/env python3

import os
import sys
import json
import time
import subprocess
from datetime import datetime

class SandboxUtils:
    def __init__(self):
        self.log_file = "/sandbox/logs/sandbox.log"
        self.results_file = "/sandbox/logs/results.json"
    
    def log(self, message):
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(self.log_file, "a") as f:
            f.write(f"[{timestamp}] {message}\n")
        print(f"[{timestamp}] {message}")
    
    def analyze_file(self, file_path):
        """Analyze file properties and potential threats"""
        if not os.path.exists(file_path):
            self.log(f"ERROR: File does not exist: {file_path}")
            return None
        
        results = {
            "timestamp": datetime.now().isoformat(),
            "file_path": file_path,
            "file_size": os.path.getsize(file_path),
            "analysis": {}
        }
        
        # File type analysis
        try:
            file_output = subprocess.check_output(["file", file_path], text=True)
            results["analysis"]["file_type"] = file_output.strip()
            self.log(f"File type: {file_output.strip()}")
        except Exception as e:
            self.log(f"Error analyzing file type: {e}")
        
        # Strings analysis
        try:
            strings_output = subprocess.check_output(["strings", file_path], text=True)
            suspicious_strings = []
            for line in strings_output.split('\n'):
                if any(keyword in line.lower() for keyword in ['password', 'admin', 'backdoor', 'virus', 'trojan']):
                    suspicious_strings.append(line.strip())
            
            results["analysis"]["suspicious_strings"] = suspicious_strings[:10]  # Limit to first 10
            self.log(f"Found {len(suspicious_strings)} suspicious strings")
        except Exception as e:
            self.log(f"Error analyzing strings: {e}")
        
        # Hash calculation
        try:
            md5_output = subprocess.check_output(["md5sum", file_path], text=True)
            results["analysis"]["md5"] = md5_output.split()[0]
            self.log(f"MD5: {results['analysis']['md5']}")
        except Exception as e:
            self.log(f"Error calculating MD5: {e}")
        
        # Save results
        with open(self.results_file, "w") as f:
            json.dump(results, f, indent=2)
        
        return results
    
    def monitor_execution(self, file_path, timeout=300):
        """Monitor file execution with timeout"""
        self.log(f"Starting execution monitoring for: {file_path}")
        
        try:
            # Start execution with monitoring
            cmd = ["/usr/local/bin/monitor.sh", "execute", file_path]
            process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            
            # Wait for completion with timeout
            try:
                stdout, stderr = process.communicate(timeout=timeout)
                self.log(f"Execution completed. Return code: {process.returncode}")
                
                if stdout:
                    self.log(f"STDOUT: {stdout}")
                if stderr:
                    self.log(f"STDERR: {stderr}")
                
                return process.returncode == 0
            except subprocess.TimeoutExpired:
                self.log("Execution timed out, terminating process")
                process.kill()
                return False
                
        except Exception as e:
            self.log(f"Error during execution monitoring: {e}")
            return False
    
    def get_logs(self):
        """Get all sandbox logs"""
        logs = {}
        
        # Execution log
        if os.path.exists("/sandbox/logs/execution.log"):
            with open("/sandbox/logs/execution.log", "r") as f:
                logs["execution"] = f.read()
        
        # Strace log
        if os.path.exists("/sandbox/logs/strace.log"):
            with open("/sandbox/logs/strace.log", "r") as f:
                logs["strace"] = f.read()
        
        # Results
        if os.path.exists(self.results_file):
            with open(self.results_file, "r") as f:
                logs["results"] = json.load(f)
        
        return logs

def main():
    if len(sys.argv) < 2:
        print("Usage: sandbox-utils.py {analyze|monitor|logs} [file_path]")
        sys.exit(1)
    
    utils = SandboxUtils()
    command = sys.argv[1]
    
    if command == "analyze" and len(sys.argv) > 2:
        file_path = sys.argv[2]
        results = utils.analyze_file(file_path)
        if results:
            print(json.dumps(results, indent=2))
    
    elif command == "monitor" and len(sys.argv) > 2:
        file_path = sys.argv[2]
        success = utils.monitor_execution(file_path)
        sys.exit(0 if success else 1)
    
    elif command == "logs":
        logs = utils.get_logs()
        print(json.dumps(logs, indent=2))
    
    else:
        print("Invalid command or missing arguments")
        sys.exit(1)

if __name__ == "__main__":
    main()
