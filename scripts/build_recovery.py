"""Build recovery alone: python scripts/build_recovery.py -e OKABE_PHONE_S3_EXP.

Also used as a PlatformIO post script to produce recovery.bin beside firmware.bin.
The child uses recovery/ as its source directory and a separate build tree.
"""
import argparse
import csv
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys


def archive_elf(project_dir, elf):
    elf = Path(elf)
    if not elf.is_file():
        return
    digest = hashlib.sha256(elf.read_bytes()).hexdigest()
    archive = Path(project_dir) / ".pio" / "elf-archive"
    archive.mkdir(parents=True, exist_ok=True)
    destination = archive / (digest + ".elf")
    if not destination.exists():
        shutil.copy2(elf, destination)
    print(f"Crash decoding ELF: {destination}")


def build_recovery(project_dir, pio_env, python, output=None):
    project_dir = Path(project_dir).resolve()
    build_root = project_dir / ".pio" / "recovery"
    child_env = os.environ.copy()
    child_env.update(
        OKABE_RECOVERY_PASS="1",
        PLATFORMIO_SRC_DIR=str(project_dir / "recovery"),
        PLATFORMIO_BUILD_DIR=str(build_root),
    )
    # The same environment preserves all board flags and pin assignments.
    subprocess.run(
        [str(python), "-m", "platformio", "run", "-e", pio_env],
        cwd=project_dir, env=child_env, check=True,
    )
    binary = build_root / pio_env / "firmware.bin"
    if not binary.is_file():
        raise RuntimeError(f"Recovery binary not found: {binary}")
    output = Path(output) if output else binary.with_name("recovery.bin")
    output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(binary, output)
    print(f"Recovery binary: {output}")
    archive_elf(project_dir, binary.with_suffix(".elf"))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-e", "--environment", default="OKABE_PHONE_S3_EXP")
    args = parser.parse_args()
    build_recovery(Path(__file__).resolve().parent.parent, args.environment, sys.executable)
else:
    Import("env")
    if os.environ.get("OKABE_RECOVERY_PASS") != "1":
        def post_build(source, target, env):
            archive_elf(env.subst("$PROJECT_DIR"), env.subst("$BUILD_DIR/${PROGNAME}.elf"))
            build_recovery(
                env.subst("$PROJECT_DIR"), env.subst("$PIOENV"),
                env.subst("$PYTHONEXE"),
                Path(env.subst("$BUILD_DIR")) / "recovery.bin",
            )

        env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_build)

        # A factory recovery partition changes PlatformIO's default app offset.
        # Select partitions by name rather than assuming the first app is main.
        partition_csv = Path(env.subst("$PARTITIONS_TABLE_CSV"))
        with partition_csv.open() as stream:
            partitions = {
                row[0].strip(): [field.strip() for field in row]
                for row in csv.reader(stream)
                if row and not row[0].lstrip().startswith("#")
            }
        recovery = partitions.get("recovery_app")
        if recovery:
            main = partitions.get("app0")
            if not main or recovery[1:3] != ["app", "factory"]:
                raise RuntimeError("Recovery requires recovery_app (app,factory) and app0")

            def number(value):
                if value.upper().endswith("K"):
                    return int(value[:-1], 0) * 1024
                if value.upper().endswith("M"):
                    return int(value[:-1], 0) * 1024 * 1024
                return int(value, 0)

            recovery_offset, recovery_size = map(number, recovery[3:5])
            main_offset, main_size = map(number, main[3:5])
            if max(recovery_offset, main_offset) < min(
                recovery_offset + recovery_size, main_offset + main_size
            ):
                raise RuntimeError("Recovery and main app partitions overlap")
            env.Replace(ESP32_APP_OFFSET=hex(main_offset))
            env.BoardConfig().update("build.app_partition_name", "app0")
            env.BoardConfig().update("upload.maximum_size", main_size)
            recovery_bin = str(Path(env.subst("$BUILD_DIR")) / "recovery.bin")
            env.Append(FLASH_EXTRA_IMAGES=[(hex(recovery_offset), recovery_bin)])
            # Normalize uploader flags too: cached integration data may already
            # contain the recovery image, while a fresh configuration does not.
            from SCons.Script import COMMAND_LINE_TARGETS
            if "upload" in COMMAND_LINE_TARGETS:
                if env.subst("$UPLOAD_PROTOCOL") != "esptool":
                    raise RuntimeError("Combined recovery upload currently requires esptool")

                flags = list(env.get("UPLOADERFLAGS", []))
                cleaned = []
                index = 0
                while index < len(flags):
                    if (index + 1 < len(flags) and
                            env.subst(str(flags[index + 1])).strip('"') == recovery_bin):
                        index += 2
                    else:
                        cleaned.append(flags[index])
                        index += 1
                env.Replace(UPLOADERFLAGS=cleaned + [hex(recovery_offset), recovery_bin])

                def prepare_upload(source, target, env):
                    # Also runs when firmware.bin is already up to date.
                    post_build(source, target, env)
                    for path, limit in (
                        (recovery_bin, recovery_size),
                        (env.subst("$BUILD_DIR/${PROGNAME}.bin"), main_size),
                    ):
                        if Path(path).stat().st_size > limit:
                            raise RuntimeError(f"Image exceeds its partition: {path}")

                env.AddPreAction("upload", prepare_upload)
