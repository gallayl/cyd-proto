"""
PlatformIO pre-build script: generates Berry constant tables.
Runs the Berry COC tool if the generate/ directory is missing.

Supports two Berry source locations:
  1. PlatformIO lib_deps (default): .pio/libdeps/<env>/berry/
  2. Local component submodule:     components/berry_lang/berry-lang/
"""
import os
import subprocess
Import("env")

berry_lib_dir = os.path.join(env["PROJECT_LIBDEPS_DIR"], env["PIOENV"], "berry")

if not os.path.isdir(berry_lib_dir):
    berry_lib_dir = os.path.join(
        env["PROJECT_DIR"], "components", "berry_lang", "berry-lang"
    )

if not os.path.isdir(berry_lib_dir):
    print("Berry: source directory not found — skipping code generation.")
    Return()

generate_dir = os.path.join(berry_lib_dir, "generate")
marker = os.path.join(generate_dir, "be_const_strtab.h")
berry_conf = os.path.join(env["PROJECT_SRC_DIR"], "berry", "berry_conf.h")
coc_tool = os.path.join(berry_lib_dir, "tools", "coc", "coc")

if not os.path.isfile(marker):
    print("Berry: generating constant tables...")
    os.makedirs(generate_dir, exist_ok=True)
    subprocess.check_call(
        ["python3", coc_tool, "-o", generate_dir,
         os.path.join(berry_lib_dir, "src"),
         os.path.join(berry_lib_dir, "default"),
         "-c", berry_conf],
        cwd=berry_lib_dir,
    )
    print("Berry: generation complete.")
