#!/usr/bin/env python3
"""
CompileForge Real-World Evaluation Harness

This script clones an open-source C++ repository (e.g. fmt, spdlog, catch2),
runs CompileForge analysis and impact prediction, and records actual metrics.

Usage:
  python3 run_evaluation.py --repo https://github.com/fmtlib/fmt.git --name fmt
"""

import argparse
import json
import os
import subprocess
import sys
import time

def run_cmd(cmd, cwd=None):
    res = subprocess.run(cmd, shell=True, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return res.returncode, res.stdout, res.stderr

def main():
    parser = argparse.ArgumentParser(description="CompileForge Real-World Evaluation Harness")
    parser.add_argument("--repo", help="Git repository URL to evaluate")
    parser.add_argument("--name", default="external_project", help="Project name")
    parser.add_argument("--branch", default="master", help="Git branch/tag")
    args = parser.parse_args()

    if not args.repo:
        print("[EVALUATION] REAL-WORLD EVALUATION: NOT RUN (No target repository URL provided)")
        print("To run an evaluation, specify: python3 run_evaluation.py --repo <git_url> --name <proj_name>")
        return 0

    print(f"[EVALUATION] Starting evaluation of {args.name} ({args.repo})...")
    work_dir = os.path.join(os.getcwd(), "evaluations", "work", args.name)
    os.makedirs(work_dir, exist_ok=True)

    # Clone repository
    if not os.path.exists(os.path.join(work_dir, ".git")):
        code, out, err = run_cmd(f"git clone --depth 50 --branch {args.branch} {args.repo} {work_dir}")
        if code != 0:
            print(f"[ERROR] Failed to clone repository: {err}")
            return 1

    # Run CompileForge analyze
    start_time = time.time()
    code, out, err = run_cmd(f"./build/compileforge analyze {work_dir} --format json --output evaluations/{args.name}_report.json")
    duration = time.time() - start_time

    if code == 0:
        print(f"[SUCCESS] CompileForge analyzed {args.name} in {duration:.3f}s")
        print(f"Report saved to: evaluations/{args.name}_report.json")
    else:
        print(f"[ERROR] Analysis failed: {err}")
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())
