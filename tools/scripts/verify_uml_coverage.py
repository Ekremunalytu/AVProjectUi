#!/usr/bin/env python3
"""
Verification script to ensure all classes from source code are represented in UML
"""

import os
import re
from pathlib import Path

def find_cpp_classes(src_dir):
    """Find all C++ classes in header files"""
    classes = set()
    
    for root, dirs, files in os.walk(src_dir):
        # Skip test directories
        if 'test' in root.lower():
            continue
            
        for file in files:
            if file.endswith('.h') or file.endswith('.hpp'):
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r', encoding='utf-8') as f:
                        content = f.read()
                        
                    # Find class declarations
                    class_pattern = r'class\s+(\w+)'
                    matches = re.findall(class_pattern, content)
                    for match in matches:
                        if not match.startswith('Q'):  # Skip Qt internal classes
                            classes.add(match)
                            
                except Exception as e:
                    print(f"Error reading {filepath}: {e}")
    
    return classes

def check_uml_coverage(dot_file, source_classes):
    """Check which classes are represented in the UML file"""
    try:
        with open(dot_file, 'r', encoding='utf-8') as f:
            uml_content = f.read()
    except Exception as e:
        print(f"Error reading UML file: {e}")
        return set(), set()
    
    covered_classes = set()
    for class_name in source_classes:
        if class_name in uml_content:
            covered_classes.add(class_name)
    
    missing_classes = source_classes - covered_classes
    return covered_classes, missing_classes

def main():
    project_root = Path(__file__).parent.parent.parent
    src_dir = project_root / 'src'
    uml_file = project_root / 'docs' / 'comprehensive_architecture.dot'
    
    print("🔍 AVProjectUi UML Coverage Verification")
    print("=" * 50)
    
    # Find all classes in source code
    print(f"📁 Scanning source directory: {src_dir}")
    source_classes = find_cpp_classes(src_dir)
    print(f"   Found {len(source_classes)} classes in source code")
    
    # Check UML coverage
    print(f"📊 Checking UML file: {uml_file}")
    covered, missing = check_uml_coverage(uml_file, source_classes)
    
    print(f"\n✅ COVERED CLASSES ({len(covered)}/{len(source_classes)}):")
    for class_name in sorted(covered):
        print(f"   ✓ {class_name}")
    
    if missing:
        print(f"\n❌ MISSING CLASSES ({len(missing)}):")
        for class_name in sorted(missing):
            print(f"   ✗ {class_name}")
    else:
        print(f"\n🎉 COMPLETE COVERAGE! All {len(source_classes)} classes are represented in the UML diagram.")
    
    # Coverage percentage
    coverage_percent = (len(covered) / len(source_classes)) * 100 if source_classes else 0
    print(f"\n📈 Coverage: {coverage_percent:.1f}%")
    
    if coverage_percent >= 90:
        print("🌟 Excellent coverage!")
    elif coverage_percent >= 75:
        print("👍 Good coverage!")
    else:
        print("⚠️  Coverage could be improved.")

if __name__ == "__main__":
    main()
