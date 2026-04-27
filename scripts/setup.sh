#!/usr/bin/env bash
set -euo pipefail

python3 -m venv venv 
venv/bin/pip install --upgrade pip 
venv/bin/pip install pip-tools 
mkdir -p .logs 
mkdir -p build 
mkdir -p build_test 
touch .logs/unit_tests.log



