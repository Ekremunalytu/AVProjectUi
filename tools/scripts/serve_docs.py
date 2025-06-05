#!/usr/bin/env python3
"""
Documentation Server for AVProjectUi
Serves the Doxygen-generated documentation via HTTP and opens it in the default browser.
"""

import http.server
import socketserver
import webbrowser
import os
import sys
import signal
import threading
import time
from pathlib import Path

class DocumentationServer:
    def __init__(self, docs_dir=None, port=8080):
        """
        Initialize the documentation server.
        
        Args:
            docs_dir (str): Path to the documentation directory
            port (int): Port number to serve on
        """
        self.port = port
        self.server = None
        self.server_thread = None
        
        # Determine documentation directory
        if docs_dir:
            self.docs_dir = Path(docs_dir)
        else:
            # Default to docs/html relative to script location
            script_dir = Path(__file__).parent
            self.docs_dir = script_dir.parent / "docs" / "html"
        
        self.docs_dir = self.docs_dir.resolve()
        
        # Validate documentation directory
        if not self.docs_dir.exists():
            raise FileNotFoundError(f"Documentation directory not found: {self.docs_dir}")
        
        index_file = self.docs_dir / "index.html"
        if not index_file.exists():
            raise FileNotFoundError(f"index.html not found in: {self.docs_dir}")
    
    def find_available_port(self, start_port=8080, max_attempts=10):
        """Find an available port starting from start_port."""
        import socket
        
        for port in range(start_port, start_port + max_attempts):
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.bind(('localhost', port))
                    return port
            except OSError:
                continue
        
        raise RuntimeError(f"Could not find available port in range {start_port}-{start_port + max_attempts}")
    
    def start_server(self, open_browser=True):
        """Start the HTTP server and optionally open browser."""
        # Find available port
        try:
            self.port = self.find_available_port(self.port)
        except RuntimeError as e:
            print(f"Error: {e}")
            return False
        
        try:
            # Create custom handler that serves from docs directory
            docs_dir_str = str(self.docs_dir)
            
            class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
                def __init__(self, *args, **kwargs):
                    super().__init__(*args, directory=docs_dir_str, **kwargs)
            
            # Create server
            self.server = socketserver.TCPServer(("localhost", self.port), CustomHTTPRequestHandler)
            
            print(f"🚀 Starting documentation server...")
            print(f"📁 Serving directory: {self.docs_dir}")
            print(f"🌐 Server URL: http://localhost:{self.port}")
            print(f"📖 Documentation: http://localhost:{self.port}/index.html")
            print()
            print("Press Ctrl+C to stop the server")
            print("-" * 50)
            
            # Start server in a separate thread
            self.server_thread = threading.Thread(target=self.server.serve_forever)
            self.server_thread.daemon = True
            self.server_thread.start()
            
            # Wait a moment for server to start
            time.sleep(0.5)
            
            # Open browser
            if open_browser:
                url = f"http://localhost:{self.port}/index.html"
                print(f"🔗 Opening browser to: {url}")
                webbrowser.open(url)
            
            return True
            
        except Exception as e:
            print(f"❌ Error starting server: {e}")
            return False
    
    def stop_server(self):
        """Stop the HTTP server."""
        if self.server:
            print("\n🛑 Stopping documentation server...")
            self.server.shutdown()
            self.server.server_close()
            if self.server_thread:
                self.server_thread.join(timeout=1)
            print("✅ Server stopped")
    
    def run(self, open_browser=True):
        """Run the server with graceful shutdown handling."""
        # Setup signal handlers for graceful shutdown
        def signal_handler(signum, frame):
            self.stop_server()
            sys.exit(0)
        
        signal.signal(signal.SIGINT, signal_handler)
        signal.signal(signal.SIGTERM, signal_handler)
        
        # Start server
        if self.start_server(open_browser):
            try:
                # Keep main thread alive
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                pass
            finally:
                self.stop_server()
        else:
            sys.exit(1)

def main():
    """Main entry point."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Serve AVProjectUi documentation via HTTP",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                          # Serve docs and open browser
  %(prog)s --port 9090              # Use custom port
  %(prog)s --no-browser             # Don't open browser
  %(prog)s --docs-dir /path/to/docs # Custom docs directory
        """
    )
    
    parser.add_argument(
        "--port", "-p",
        type=int,
        default=8080,
        help="Port number to serve on (default: 8080)"
    )
    
    parser.add_argument(
        "--docs-dir", "-d",
        type=str,
        help="Path to documentation directory (default: ../docs/html)"
    )
    
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Don't open browser automatically"
    )
    
    parser.add_argument(
        "--version", "-v",
        action="version",
        version="AVProjectUi Documentation Server 1.0"
    )
    
    args = parser.parse_args()
    
    try:
        server = DocumentationServer(docs_dir=args.docs_dir, port=args.port)
        server.run(open_browser=not args.no_browser)
    except FileNotFoundError as e:
        print(f"❌ Error: {e}")
        print("💡 Tip: Make sure to generate documentation first with 'doxygen Doxyfile'")
        sys.exit(1)
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
