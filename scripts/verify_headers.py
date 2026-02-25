#!/usr/bin/env python3
"""
Header verification script for Cyqle1 project.
Checks that all headers compile independently and have proper include guards.
"""

import os
import subprocess
import sys
from pathlib import Path

# Colors for output
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

def check_header_file(header_path):
    """Check a single header file for compilation issues."""
    print(f"{Colors.BLUE}Checking: {header_path}{Colors.NC}")
    
    # Check for include guard
    with open(header_path, 'r') as f:
        content = f.read()
    
    filename = Path(header_path).name.upper().replace('.', '_')
    expected_guard = f"CYQLE1_{filename}"
    
    if f"#ifndef {expected_guard}" not in content:
        print(f"{Colors.YELLOW}  Warning: Missing or incorrect include guard{Colors.NC}")
        print(f"  Expected: #ifndef {expected_guard}")
    
    # Try to compile header standalone
    test_file = f"/tmp/test_{Path(header_path).stem}.cpp"
    with open(test_file, 'w') as f:
        f.write(f'#include "{header_path}"\n')
        f.write('int main() { return 0; }\n')
    
    try:
        result = subprocess.run(
            ['g++', '-std=c++17', '-c', test_file, '-o', '/dev/null'],
            capture_output=True,
            text=True,
            timeout=5
        )
        
        if result.returncode != 0:
            print(f"{Colors.RED}  Error: Header compilation failed{Colors.NC}")
            print(f"  {result.stderr}")
            return False
        else:
            print(f"{Colors.GREEN}  ✓ Header compiles successfully{Colors.NC}")
            return True
            
    except subprocess.TimeoutExpired:
        print(f"{Colors.RED}  Error: Compilation timed out{Colors.NC}")
        return False
    finally:
        if os.path.exists(test_file):
            os.remove(test_file)

def check_source_file(source_path, headers_dir):
    """Check a source file for compilation issues."""
    print(f"{Colors.BLUE}Checking: {source_path}{Colors.NC}")
    
    # Try to compile source file
    try:
        result = subprocess.run(
            ['g++', '-std=c++17', '-I', headers_dir, '-c', source_path, '-o', '/dev/null'],
            capture_output=True,
            text=True,
            timeout=10
        )
        
        if result.returncode != 0:
            print(f"{Colors.RED}  Error: Source compilation failed{Colors.NC}")
            print(f"  {result.stderr}")
            return False
        else:
            print(f"{Colors.GREEN}  ✓ Source compiles successfully{Colors.NC}")
            return True
            
    except subprocess.TimeoutExpired:
        print(f"{Colors.RED}  Error: Compilation timed out{Colors.NC}")
        return False

def main():
    project_root = Path(__file__).parent.parent
    src_dir = project_root / "src"
    
    print(f"{Colors.YELLOW}=== Cyqle1 Header and Source Verification ==={Colors.NC}")
    print(f"Project root: {project_root}")
    print("")
    
    # Find all header files
    header_files = list(src_dir.rglob("*.h")) + list(src_dir.rglob("*.hpp"))
    source_files = list(src_dir.rglob("*.cpp"))
    
    print(f"Found {len(header_files)} header files")
    print(f"Found {len(source_files)} source files")
    print("")
    
    # Check headers
    print(f"{Colors.YELLOW}--- Checking Header Files ---{Colors.NC}")
    header_errors = 0
    for header in header_files:
        if not check_header_file(str(header)):
            header_errors += 1
        print("")
    
    # Check source files
    print(f"{Colors.YELLOW}--- Checking Source Files ---{Colors.NC}")
    source_errors = 0
    for source in source_files:
        if not check_source_file(str(source), str(src_dir)):
            source_errors += 1
        print("")
    
    # Summary
    print(f"{Colors.YELLOW}=== Verification Summary ==={Colors.NC}")
    print(f"Headers: {len(header_files) - header_errors}/{len(header_files)} passed")
    print(f"Sources: {len(source_files) - source_errors}/{len(source_files)} passed")
    print("")
    
    if header_errors == 0 and source_errors == 0:
        print(f"{Colors.GREEN}✓ All files compile successfully!{Colors.NC}")
        return 0
    else:
        print(f"{Colors.RED}✗ Found {header_errors + source_errors} compilation errors{Colors.NC}")
        return 1

if __name__ == "__main__":
    sys.exit(main())