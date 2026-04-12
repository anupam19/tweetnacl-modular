#!/usr/bin/env python3
"""Syntax-check all Python files in the test bench."""
import py_compile
import os
import sys

errors = []
for root, dirs, files in os.walk('tests/python'):
    for f in files:
        if f.endswith('.py'):
            path = os.path.join(root, f)
            try:
                py_compile.compile(path, doraise=True)
            except py_compile.PyCompileError as e:
                errors.append(str(e))

if errors:
    print('ERRORS:')
    for e in errors:
        print(e)
    sys.exit(1)
else:
    print('All Python files compile OK')
    print(f'Checked {sum(1 for r, d, fs in os.walk("tests/python") for f in fs if f.endswith(".py"))} files')
