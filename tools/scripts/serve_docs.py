#!/usr/bin/env python3
"""
Documentation Server for AVProjectUi
Serves the Doxygen-generated documentation via HTTP and opens it in the default browser.
Enhanced with UML diagram viewer and navigation.
"""

import http.server
import socketserver
import webbrowser
import os
import sys
import signal
import threading
import time
import json
import urllib.parse
from pathlib import Path

class UMLDiagramFinder:
    """Find and categorize UML diagrams in the documentation."""
    
    def __init__(self, docs_dir):
        self.docs_dir = Path(docs_dir)
        self.diagrams = {}
        self._scan_for_diagrams()
    
    def _scan_for_diagrams(self):
        """Scan for SVG diagram files and categorize them."""
        diagram_patterns = {
            'class': ['class', 'inherit'],
            'collaboration': ['coll'],
            'call_graph': ['cgraph'],
            'caller_graph': ['graph'],
            'include': ['dep'],
            'directory': ['dir'],
            'namespace': ['namespace']
        }
        
        # Find all SVG files
        svg_files = list(self.docs_dir.rglob("*.svg"))
        
        for svg_file in svg_files:
            rel_path = svg_file.relative_to(self.docs_dir)
            filename = svg_file.stem.lower()
            
            # Categorize based on filename patterns
            category = 'other'
            for cat, patterns in diagram_patterns.items():
                if any(pattern in filename for pattern in patterns):
                    category = cat
                    break
            
            if category not in self.diagrams:
                self.diagrams[category] = []
            
            self.diagrams[category].append({
                'name': svg_file.stem,
                'path': str(rel_path),
                'size': svg_file.stat().st_size,
                'title': self._generate_title(svg_file.stem)
            })
        
        # Sort diagrams by name within each category
        for category in self.diagrams:
            self.diagrams[category].sort(key=lambda x: x['name'])
    
    def _generate_title(self, filename):
        """Generate a readable title from filename."""
        # Remove common prefixes and suffixes
        title = filename.replace('class', '').replace('inherit', '').replace('graph', '')
        title = title.replace('_', ' ').strip()
        return title.title() if title else filename

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """Custom HTTP handler with UML diagram viewer."""
    
    def __init__(self, *args, docs_dir=None, uml_finder=None, **kwargs):
        self.docs_dir = docs_dir
        self.uml_finder = uml_finder
        super().__init__(*args, directory=docs_dir, **kwargs)
    
    def do_GET(self):
        """Handle GET requests with special UML viewer routes."""
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/uml-viewer':
            self._serve_uml_viewer()
        elif parsed_path.path == '/api/uml-diagrams':
            self._serve_uml_api()
        else:
            super().do_GET()
    
    def _serve_uml_viewer(self):
        """Serve the UML diagram viewer page."""
        html = self._generate_uml_viewer_html()
        self._send_html_response(html)
    
    def _serve_uml_api(self):
        """Serve UML diagrams data as JSON API."""
        data = {
            'diagrams': self.uml_finder.diagrams,
            'total': sum(len(diagrams) for diagrams in self.uml_finder.diagrams.values())
        }
        self._send_json_response(data)
    
    def _send_html_response(self, html):
        """Send HTML response."""
        self.send_response(200)
        self.send_header('Content-type', 'text/html; charset=utf-8')
        self.end_headers()
        self.wfile.write(html.encode('utf-8'))
    
    def _send_json_response(self, data):
        """Send JSON response."""
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode('utf-8'))
    
    def _generate_uml_viewer_html(self):
        """Generate the UML viewer HTML page."""
        total_diagrams = sum(len(diagrams) for diagrams in self.uml_finder.diagrams.values())
        
        return f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>UML Diagrams - AVProjectUi</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{ 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: #f5f7fa;
            color: #2d3748;
            line-height: 1.6;
        }}
        .container {{ max-width: 1200px; margin: 0 auto; padding: 20px; }}
        .header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 2rem;
            border-radius: 12px;
            margin-bottom: 2rem;
            text-align: center;
        }}
        .header h1 {{ font-size: 2.5rem; margin-bottom: 0.5rem; }}
        .header p {{ font-size: 1.1rem; opacity: 0.9; }}
        .stats {{
            background: white;
            border-radius: 8px;
            padding: 1.5rem;
            margin-bottom: 2rem;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        .stats h2 {{ color: #4a5568; margin-bottom: 1rem; }}
        .stat-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 1rem;
        }}
        .stat-item {{
            background: #f7fafc;
            padding: 1rem;
            border-radius: 6px;
            text-align: center;
        }}
        .stat-number {{ font-size: 2rem; font-weight: bold; color: #667eea; }}
        .stat-label {{ color: #718096; text-transform: uppercase; font-size: 0.875rem; }}
        .category {{
            background: white;
            border-radius: 8px;
            margin-bottom: 2rem;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            overflow: hidden;
        }}
        .category-header {{
            background: #edf2f7;
            padding: 1rem 1.5rem;
            border-bottom: 1px solid #e2e8f0;
        }}
        .category-title {{ font-size: 1.25rem; font-weight: 600; color: #2d3748; }}
        .category-count {{ color: #718096; font-size: 0.875rem; }}
        .diagram-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 1rem;
            padding: 1.5rem;
        }}
        .diagram-card {{
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            overflow: hidden;
            transition: transform 0.2s, box-shadow 0.2s;
        }}
        .diagram-card:hover {{
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.15);
        }}
        .diagram-preview {{
            background: #f9f9f9;
            padding: 1rem;
            text-align: center;
            min-height: 200px;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        .diagram-preview img {{
            max-width: 100%;
            max-height: 180px;
            border-radius: 4px;
        }}
        .diagram-info {{
            padding: 1rem;
            background: white;
        }}
        .diagram-title {{ font-weight: 600; margin-bottom: 0.5rem; color: #2d3748; }}
        .diagram-meta {{ font-size: 0.875rem; color: #718096; }}
        .nav-links {{
            text-align: center;
            margin-bottom: 2rem;
        }}
        .nav-links a {{
            display: inline-block;
            margin: 0 1rem;
            padding: 0.5rem 1rem;
            background: white;
            color: #667eea;
            text-decoration: none;
            border-radius: 6px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            transition: all 0.2s;
        }}
        .nav-links a:hover {{
            background: #667eea;
            color: white;
            transform: translateY(-1px);
        }}
        .empty-state {{
            text-align: center;
            padding: 3rem;
            color: #718096;
        }}
        .empty-state svg {{
            width: 64px;
            height: 64px;
            margin-bottom: 1rem;
            opacity: 0.5;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🏗️ UML Diagrams</h1>
            <p>Visual architecture and relationships for AVProjectUi</p>
        </div>
        
        <div class="nav-links">
            <a href="/index.html">📖 Documentation Home</a>
            <a href="/api/uml-diagrams">📊 JSON API</a>
            <a href="#" onclick="window.print()">🖨️ Print View</a>
        </div>
        
        <div class="stats">
            <h2>📊 Diagram Statistics</h2>
            <div class="stat-grid">
                <div class="stat-item">
                    <div class="stat-number">{total_diagrams}</div>
                    <div class="stat-label">Total Diagrams</div>
                </div>
                <div class="stat-item">
                    <div class="stat-number">{len(self.uml_finder.diagrams)}</div>
                    <div class="stat-label">Categories</div>
                </div>
            </div>
        </div>'''

        # Generate categories
        for category, diagrams in self.uml_finder.diagrams.items():
            if not diagrams:
                continue
                
            category_title = category.replace('_', ' ').title()
            category_icons = {
                'class': '🏛️',
                'collaboration': '🤝',
                'call_graph': '📞',
                'caller_graph': '📲',
                'include': '📦',
                'directory': '📁',
                'namespace': '🗂️',
                'other': '📋'
            }
            icon = category_icons.get(category, '📋')
            
            html += f'''
        <div class="category">
            <div class="category-header">
                <div class="category-title">{icon} {category_title}</div>
                <div class="category-count">{len(diagrams)} diagrams</div>
            </div>
            <div class="diagram-grid">'''
            
            for diagram in diagrams:
                size_kb = diagram['size'] // 1024
                html += f'''
                <div class="diagram-card">
                    <div class="diagram-preview">
                        <img src="{diagram['path']}" alt="{diagram['title']}" loading="lazy" 
                             onerror="this.parentElement.innerHTML='<div style=\\"color:#718096\\">📊 Diagram Preview</div>'">
                    </div>
                    <div class="diagram-info">
                        <div class="diagram-title">{diagram['title'] or diagram['name']}</div>
                        <div class="diagram-meta">
                            📁 {diagram['name']}<br>
                            📏 {size_kb} KB
                        </div>
                    </div>
                </div>'''
            
            html += '''
            </div>
        </div>'''
        
        if not self.uml_finder.diagrams or total_diagrams == 0:
            html += '''
        <div class="empty-state">
            <svg fill="currentColor" viewBox="0 0 20 20">
                <path d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"/>
            </svg>
            <h3>No UML Diagrams Found</h3>
            <p>Generate documentation with UML diagrams enabled to see visual representations here.</p>
        </div>'''
        
        html += '''
    </div>
    
    <script>
        // Add click handler for diagram cards
        document.querySelectorAll('.diagram-card').forEach(card => {
            card.addEventListener('click', function() {
                const img = this.querySelector('img');
                if (img && img.src) {
                    window.open(img.src, '_blank');
                }
            });
        });
        
        // Add keyboard navigation
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape') {
                window.history.back();
            }
        });
    </script>
</body>
</html>'''
        
        return html

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
            self.docs_dir = script_dir.parent.parent / "docs" / "html"
        
        self.docs_dir = self.docs_dir.resolve()
        
        # Validate documentation directory
        if not self.docs_dir.exists():
            raise FileNotFoundError(f"Documentation directory not found: {self.docs_dir}")
        
        index_file = self.docs_dir / "index.html"
        if not index_file.exists():
            raise FileNotFoundError(f"index.html not found in: {self.docs_dir}")
        
        # Initialize UML diagram finder
        self.uml_finder = UMLDiagramFinder(self.docs_dir)
    
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
            # Create custom handler factory
            docs_dir_str = str(self.docs_dir)
            uml_finder = self.uml_finder
            
            def handler_factory(*args, **kwargs):
                return CustomHTTPRequestHandler(*args, docs_dir=docs_dir_str, uml_finder=uml_finder, **kwargs)
            
            # Create server
            self.server = socketserver.TCPServer(("localhost", self.port), handler_factory)
            
            print(f"🚀 Starting enhanced documentation server...")
            print(f"📁 Serving directory: {self.docs_dir}")
            print(f"🌐 Server URL: http://localhost:{self.port}")
            print(f"📖 Documentation: http://localhost:{self.port}/index.html")
            print(f"🏗️ UML Viewer: http://localhost:{self.port}/uml-viewer")
            
            # Show UML statistics
            total_diagrams = sum(len(diagrams) for diagrams in self.uml_finder.diagrams.values())
            if total_diagrams > 0:
                print(f"📊 Found {total_diagrams} UML diagrams in {len(self.uml_finder.diagrams)} categories")
                for category, diagrams in self.uml_finder.diagrams.items():
                    if diagrams:
                        print(f"   • {category.replace('_', ' ').title()}: {len(diagrams)} diagrams")
            else:
                print("⚠️  No UML diagrams found - make sure documentation is generated with UML enabled")
            
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
                url = f"http://localhost:{self.port}/uml-viewer"
                print(f"🔗 Opening UML viewer: {url}")
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
