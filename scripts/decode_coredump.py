#!/usr/bin/env python3
"""Decode a recovery-exported crash-NNNN.bin using its matching firmware ELF."""
import argparse
import hashlib
import importlib.util
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent


def elf_digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def resolve_elf(dump, explicit=None, root=ROOT):
    summary = dump.with_suffix(".txt")
    digest = None
    if summary.is_file():
        match = re.search(r"^ELF SHA256: ([0-9a-fA-F]{64})$", summary.read_text(), re.MULTILINE)
        if match:
            digest = match.group(1).lower()
    if explicit:
        path = Path(explicit).resolve()
        if not path.is_file():
            raise ValueError(f"ELF not found: {path}")
        if digest and elf_digest(path) != digest:
            raise ValueError("ELF SHA256 does not match the saved crash. Use the original build's ELF.")
        return path
    if not digest:
        raise ValueError("No full ELF SHA256 in the companion .txt file. Supply --elf with the crashed build's ELF.")
    candidates = [root / ".pio" / "elf-archive" / (digest + ".elf")]
    for directory in (root / ".pio" / "build", root / ".pio" / "recovery"):
        candidates.extend(directory.glob("*/firmware.elf"))
    for path in candidates:
        if path.is_file() and elf_digest(path) == digest:
            return path.resolve()
    raise ValueError(f"Matching ELF {digest} not found in .pio/elf-archive; supply --elf.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dump", type=Path)
    parser.add_argument("--elf", type=Path, help="ELF from the exact crashed build")
    parser.add_argument("--chip", choices=["esp32s3", "esp32"], default="esp32s3")
    parser.add_argument("--gdb", type=Path)
    parser.add_argument("--debug", action="store_true", help="Open interactive GDB instead of a report")
    parser.add_argument("--dry-run", action="store_true", help="Check ELF selection and print decoder command")
    args = parser.parse_args()
    try:
        dump = args.dump.resolve()
        if not dump.is_file():
            raise ValueError(f"Dump not found: {dump}")
        elf = resolve_elf(dump, args.elf)
        python = ROOT / ".pio" / "coredump-tools" / "bin" / "python"
        if not python.is_file():
            if importlib.util.find_spec("esp_coredump"):
                python = Path(sys.executable)
            else:
                raise ValueError("Install decoder: python -m venv .pio/coredump-tools && "
                                 ".pio/coredump-tools/bin/pip install -r scripts/requirements-coredump.txt")
        core_dir = Path(os.environ.get("PLATFORMIO_CORE_DIR", Path.home() / ".platformio"))
        gdb = args.gdb or core_dir / "packages" / "tool-xtensa-esp-elf-gdb" / "bin" / f"xtensa-{args.chip}-elf-gdb"
        if not gdb.is_file():
            raise ValueError("Xtensa GDB not found; provide --gdb /path/to/xtensa-...-gdb")
        command = [str(python), "-m", "esp_coredump", "--chip", args.chip,
                   "dbg_corefile" if args.debug else "info_corefile", "-t", "raw",
                   "-c", str(dump), "--gdb", str(gdb), str(elf)]
        print(f"Matching ELF: {elf}", flush=True)
        if args.dry_run:
            import shlex
            print(shlex.join(command))
            return 0
        return subprocess.run(command, check=False).returncode
    except (OSError, ValueError) as error:
        print(f"Decode failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
