#!/usr/bin/env python3
"""
Comprehensive UML Class Diagram Generator for AVProjectUi
Generates a detailed UML diagram with all classes, methods, and properties from the entire src directory.
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional
from dataclasses import dataclass, field
from collections import defaultdict

@dataclass
class ClassMethod:
    """Represents a class method"""
    name: str
    return_type: str = ""
    parameters: List[str] = field(default_factory=list)
    visibility: str = "public"  # public, private, protected
    is_static: bool = False
    is_virtual: bool = False
    is_const: bool = False

@dataclass
class ClassMember:
    """Represents a class member variable"""
    name: str
    type: str
    visibility: str = "private"
    is_static: bool = False
    is_const: bool = False

@dataclass
class ClassInfo:
    """Represents a C++ class"""
    name: str
    namespace: str = ""
    base_classes: List[str] = field(default_factory=list)
    methods: List[ClassMethod] = field(default_factory=list)
    members: List[ClassMember] = field(default_factory=list)
    file_path: str = ""
    is_interface: bool = False
    documentation: str = ""

class UMLGenerator:
    """Generates comprehensive UML diagrams from C++ source code"""
    
    def __init__(self, src_path: str):
        self.src_path = Path(src_path)
        self.classes: Dict[str, ClassInfo] = {}
        self.namespaces: Set[str] = set()
        self.relationships: List[Tuple[str, str, str]] = []  # (from, to, type)
        
    def parse_cpp_files(self):
        """Parse all header files in the source directory"""
        print("🔍 Scanning source files...")
        
        for header_file in self.src_path.rglob("*.h"):
            if self._should_skip_file(header_file):
                continue
                
            print(f"  📄 Parsing {header_file.relative_to(self.src_path)}")
            self._parse_header_file(header_file)
    
    def _should_skip_file(self, file_path: Path) -> bool:
        """Check if file should be skipped"""
        skip_patterns = [
            "test_", "Test", "mock", "Mock", 
            "autogen", "moc_", "_autogen"
        ]
        return any(pattern in str(file_path) for pattern in skip_patterns)
    
    def _parse_header_file(self, file_path: Path):
        """Parse a single header file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except Exception as e:
            print(f"  ⚠️ Error reading {file_path}: {e}")
            return
            
        # Remove comments
        content = self._remove_comments(content)
        
        # Find namespaces
        namespaces = self._extract_namespaces(content)
        
        # Find classes
        classes = self._extract_classes(content, str(file_path))
        
        for class_info in classes:
            # Determine namespace context
            for ns in namespaces:
                if class_info.name in content:  # Simple heuristic
                    class_info.namespace = ns
                    break
            
            self.classes[class_info.name] = class_info
            if class_info.namespace:
                self.namespaces.add(class_info.namespace)
    
    def _remove_comments(self, content: str) -> str:
        """Remove C++ comments from content"""
        # Remove single-line comments
        content = re.sub(r'//.*$', '', content, flags=re.MULTILINE)
        # Remove multi-line comments
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        return content
    
    def _extract_namespaces(self, content: str) -> List[str]:
        """Extract namespace declarations"""
        namespaces = []
        namespace_pattern = r'namespace\s+(\w+)\s*\{'
        matches = re.finditer(namespace_pattern, content)
        for match in matches:
            namespaces.append(match.group(1))
        return namespaces
    
    def _extract_classes(self, content: str, file_path: str) -> List[ClassInfo]:
        """Extract class definitions from content"""
        classes = []
        
        # Pattern to match class declarations
        class_pattern = r'class\s+(\w+)(?:\s*:\s*([^{]+))?\s*\{'
        matches = re.finditer(class_pattern, content)
        
        for match in matches:
            class_name = match.group(1)
            inheritance = match.group(2) if match.group(2) else ""
            
            # Parse inheritance
            base_classes = []
            if inheritance:
                base_classes = [base.strip().split()[-1] for base in inheritance.split(',')]
                base_classes = [base for base in base_classes if base and base != 'public' and base != 'private' and base != 'protected']
            
            # Create class info
            class_info = ClassInfo(
                name=class_name,
                base_classes=base_classes,
                file_path=file_path
            )
            
            # Find class body
            class_start = match.end()
            class_body = self._extract_class_body(content, class_start)
            
            if class_body:
                # Parse methods and members
                self._parse_class_members(class_body, class_info)
            
            classes.append(class_info)
        
        # Also look for struct definitions
        struct_pattern = r'struct\s+(\w+)(?:\s*:\s*([^{]+))?\s*\{'
        matches = re.finditer(struct_pattern, content)
        
        for match in matches:
            struct_name = match.group(1)
            inheritance = match.group(2) if match.group(2) else ""
            
            base_classes = []
            if inheritance:
                base_classes = [base.strip().split()[-1] for base in inheritance.split(',')]
                base_classes = [base for base in base_classes if base and base != 'public' and base != 'private' and base != 'protected']
            
            class_info = ClassInfo(
                name=struct_name,
                base_classes=base_classes,
                file_path=file_path
            )
            
            struct_start = match.end()
            struct_body = self._extract_class_body(content, struct_start)
            
            if struct_body:
                self._parse_class_members(struct_body, class_info)
            
            classes.append(class_info)
        
        return classes
    
    def _extract_class_body(self, content: str, start_pos: int) -> str:
        """Extract the body of a class from the content"""
        brace_count = 1
        pos = start_pos
        
        while pos < len(content) and brace_count > 0:
            if content[pos] == '{':
                brace_count += 1
            elif content[pos] == '}':
                brace_count -= 1
            pos += 1
        
        if brace_count == 0:
            return content[start_pos:pos-1]
        return ""
    
    def _parse_class_members(self, class_body: str, class_info: ClassInfo):
        """Parse methods and member variables from class body"""
        current_visibility = "private"  # Default for classes
        
        lines = class_body.split('\n')
        i = 0
        
        while i < len(lines):
            line = lines[i].strip()
            
            # Skip empty lines and preprocessor directives
            if not line or line.startswith('#'):
                i += 1
                continue
            
            # Check for visibility modifiers
            if line.endswith(':'):
                if 'public' in line:
                    current_visibility = "public"
                elif 'private' in line:
                    current_visibility = "private"
                elif 'protected' in line:
                    current_visibility = "protected"
                i += 1
                continue
            
            # Look for method declarations
            method_match = re.search(r'(\w+(?:\s*<[^>]*>)?)\s+(\w+)\s*\([^)]*\)(?:\s*const)?(?:\s*=\s*0)?(?:\s*override)?', line)
            if method_match and not line.endswith(';'):
                # This might be a method declaration
                return_type = method_match.group(1)
                method_name = method_match.group(2)
                
                # Skip if it looks like a variable declaration
                if not any(keyword in line for keyword in ['(', 'operator']):
                    i += 1
                    continue
                
                method = ClassMethod(
                    name=method_name,
                    return_type=return_type,
                    visibility=current_visibility,
                    is_const='const' in line,
                    is_virtual='virtual' in line,
                    is_static='static' in line
                )
                
                # Extract parameters
                param_match = re.search(r'\(([^)]*)\)', line)
                if param_match:
                    params_str = param_match.group(1).strip()
                    if params_str:
                        method.parameters = [p.strip() for p in params_str.split(',')]
                
                class_info.methods.append(method)
            
            # Look for member variable declarations
            elif ';' in line and not line.strip().startswith('//'):
                # This might be a member variable
                var_match = re.search(r'(\w+(?:\s*<[^>]*>)?(?:\s*\*)?)\s+(\w+)(?:\s*=\s*[^;]+)?;', line)
                if var_match:
                    var_type = var_match.group(1)
                    var_name = var_match.group(2)
                    
                    # Skip if it's likely a method declaration
                    if '(' not in line or var_name.endswith('()'):
                        member = ClassMember(
                            name=var_name,
                            type=var_type,
                            visibility=current_visibility,
                            is_static='static' in line,
                            is_const='const' in line
                        )
                        class_info.members.append(member)
            
            i += 1
    
    def generate_dot_file(self, output_path: str):
        """Generate GraphViz DOT file for UML diagram"""
        print(f"🎨 Generating comprehensive UML diagram...")
        
        dot_content = self._generate_dot_content()
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(dot_content)
        
        print(f"✅ UML DOT file generated: {output_path}")
        return output_path
    
    def _generate_dot_content(self) -> str:
        """Generate DOT file content"""
        dot = []
        dot.append('digraph "AVProjectUi_Comprehensive_Architecture" {')
        dot.append('    // Graph settings')
        dot.append('    rankdir=LR;')
        dot.append('    splines=ortho;')
        dot.append('    nodesep=0.8;')
        dot.append('    ranksep=1.2;')
        dot.append('    bgcolor=white;')
        dot.append('')
        dot.append('    // Node styling')
        dot.append('    node [')
        dot.append('        shape=record,')
        dot.append('        style=filled,')
        dot.append('        fontname="Arial",')
        dot.append('        fontsize=10,')
        dot.append('        margin=0.1')
        dot.append('    ];')
        dot.append('')
        dot.append('    // Edge styling')
        dot.append('    edge [')
        dot.append('        fontname="Arial",')
        dot.append('        fontsize=8,')
        dot.append('        color=gray40')
        dot.append('    ];')
        dot.append('')
        
        # Group classes by namespace/module
        modules = self._group_classes_by_module()
        
        for module_name, classes in modules.items():
            dot.append(f'    // {module_name} Module')
            dot.append(f'    subgraph "cluster_{module_name}" {{')
            dot.append(f'        label="{module_name.title()} Module";')
            dot.append('        style=filled;')
            dot.append('        fillcolor=lightgray;')
            dot.append('        fontsize=12;')
            dot.append('        fontname="Arial Bold";')
            dot.append('')
            
            for class_name in classes:
                class_info = self.classes[class_name]
                dot.append(self._generate_class_node(class_info))
            
            dot.append('    }')
            dot.append('')
        
        # Add inheritance relationships
        dot.append('    // Inheritance relationships')
        for class_name, class_info in self.classes.items():
            for base_class in class_info.base_classes:
                if base_class in self.classes:
                    dot.append(f'    "{base_class}" -> "{class_name}" [arrowhead=onormal, color=blue];')
        
        # Add composition/aggregation relationships (simplified)
        dot.append('    // Composition relationships')
        for class_name, class_info in self.classes.items():
            for member in class_info.members:
                member_type = member.type.replace('std::', '').replace('*', '').replace('&', '').strip()
                if member_type in self.classes and member_type != class_name:
                    dot.append(f'    "{class_name}" -> "{member_type}" [arrowhead=diamond, color=green];')
        
        dot.append('}')
        return '\n'.join(dot)
    
    def _group_classes_by_module(self) -> Dict[str, List[str]]:
        """Group classes by their module/namespace"""
        modules = defaultdict(list)
        
        for class_name, class_info in self.classes.items():
            # Determine module from file path
            path_parts = Path(class_info.file_path).parts
            module = "core"  # default
            
            for part in path_parts:
                if part in ['security', 'infrastructure', 'presentation', 'storage', 'application', 'core']:
                    module = part
                    break
            
            # Use namespace if available
            if class_info.namespace:
                module = class_info.namespace
            
            modules[module].append(class_name)
        
        return dict(modules)
    
    def _generate_class_node(self, class_info: ClassInfo) -> str:
        """Generate DOT node for a class"""
        lines = []
        
        # Determine node color based on type
        if class_info.is_interface or class_info.name.startswith('I'):
            fillcolor = 'lightblue'
        elif any(keyword in class_info.file_path.lower() for keyword in ['manager', 'service']):
            fillcolor = 'lightgreen'
        elif 'types' in class_info.file_path.lower() or 'config' in class_info.file_path.lower():
            fillcolor = 'lightyellow'
        else:
            fillcolor = 'white'
        
        # Class name with stereotype
        stereotype = ""
        if class_info.is_interface or class_info.name.startswith('I'):
            stereotype = "\\<\\<interface\\>\\>"
        elif class_info.base_classes:
            stereotype = "\\<\\<class\\>\\>"
        
        class_label = f"{stereotype}\\n{class_info.name}" if stereotype else class_info.name
        
        # Build the record structure
        sections = [f"{{{class_label}}}"]
        
        # Add public members (limit to most important ones)
        public_members = [m for m in class_info.members if m.visibility == "public"][:5]
        if public_members:
            member_lines = []
            for member in public_members:
                member_line = f"{member.name}: {member.type}"
                member_lines.append(member_line)
            sections.append("{" + "\\l".join(member_lines) + "\\l}")
        
        # Add public methods (limit to most important ones)
        public_methods = [m for m in class_info.methods if m.visibility == "public"][:8]
        if public_methods:
            method_lines = []
            for method in public_methods:
                params = "..." if len(method.parameters) > 2 else ", ".join(method.parameters)
                method_line = f"{method.name}({params}): {method.return_type}"
                method_lines.append(method_line)
            sections.append("{" + "\\l".join(method_lines) + "\\l}")
        
        record_content = "|".join(sections)
        
        node_def = f'    "{class_info.name}" ['
        node_def += f'label="{record_content}", '
        node_def += f'fillcolor={fillcolor}'
        node_def += '];'
        
        return node_def
    
    def print_statistics(self):
        """Print parsing statistics"""
        print(f"\n📊 Parsing Statistics:")
        print(f"  Total classes found: {len(self.classes)}")
        print(f"  Namespaces found: {len(self.namespaces)}")
        
        total_methods = sum(len(cls.methods) for cls in self.classes.values())
        total_members = sum(len(cls.members) for cls in self.classes.values())
        
        print(f"  Total methods: {total_methods}")
        print(f"  Total members: {total_members}")
        
        # Group by modules
        modules = self._group_classes_by_module()
        print(f"\n📦 Classes by module:")
        for module, classes in modules.items():
            print(f"  {module}: {len(classes)} classes")

def main():
    """Main function"""
    if len(sys.argv) < 2:
        print("Usage: python generate_comprehensive_uml.py <src_path> [output_path]")
        sys.exit(1)
    
    src_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else "docs/comprehensive_architecture.dot"
    
    if not os.path.exists(src_path):
        print(f"❌ Source path not found: {src_path}")
        sys.exit(1)
    
    print("🚀 Starting comprehensive UML generation...")
    
    generator = UMLGenerator(src_path)
    generator.parse_cpp_files()
    generator.print_statistics()
    generator.generate_dot_file(output_path)
    
    print(f"\n🎉 Comprehensive UML diagram generated successfully!")
    print(f"📄 Output file: {output_path}")
    print(f"\n💡 To generate PNG image:")
    print(f"   dot -Tpng {output_path} -o docs/comprehensive_architecture.png")

if __name__ == "__main__":
    main()
