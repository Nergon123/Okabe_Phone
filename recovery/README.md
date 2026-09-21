# Recovery

A standalone ESP-IDF application using the LCD, MCP23017 keypad and POSIX file
APIs. No phone UI, Wi-Fi or modem is started. Uses the board's configured pins.

## Controls

Up/down selects an item, centre/OK opens it, and decline/back (or left) returns.
These correspond to raw keypad IDs 2, 8, 5 and 9/4. Key transitions are debounced;
a missing keypad is retried every two seconds. Confirmations default to Cancel.

## Storage and updates

- **SPIFFS:** mount/unmount `/spiffs`, show usage, browse files, run the filesystem
  checker, or explicitly format after confirmation. Mount failure never formats.
- **SD:** mount/unmount a FAT card at `/sd`, show capacity and browse directories.
  Set `SD_CS`, `SD_SCK`, `SD_MOSI`, `SD_MISO` in the board's PlatformIO flags.
  The default `-1` pins disable SD. It uses SPI3; LCD uses SPI2. Conflicting,
  reserved and duplicate SD GPIO assignments are rejected. Cards aren't formatted.
- **Install:** browse either mounted volume, select an application `.bin`, then
  choose a non-running OTA partition. Use `firmware.bin`, not a merged flash image,
  bootloader, partition table or filesystem image. This implementation accepts
  plain application images, not signed images with extra signature blocks.
- Before confirmation, recovery checks the file bounds, ESP chip/revision,
  application descriptor, segment lengths, checksum and appended SHA256 digest.
  The confirmation identifies the target, image size, project and version.
- Installation streams the file through IDF OTA APIs, displays progress and
  verifies the resulting image before changing boot selection. Read/write/verify
  failure keeps recovery open. Success offers either staying or rebooting.
- The current S3 layout has only one normal application slot (`app0`). Updating
  it replaces the previous main app; keep power connected during writing.
  Factory recovery is never an update target, so it remains available to retry.
  A build running in the only OTA slot has no eligible update target.

The browser lists up to 256 entries per directory and indicates truncation.
Long selected file paths are shown on the file details screen.

## Partitions

The partition menu displays labels, addresses, sizes, type/subtype, encryption,
read-only status, running/boot markers and app project/version. Opening the list
also prints the partition table to the serial log (115200 baud).

Applications can be verified and eligible OTA apps can be selected and rebooted
into. Factory recovery is entered using the bootloader's recovery key combination.
The bootloader deliberately prefers the normal app when no combination is held.

Confirmed erase is available for writable NVS, SPIFFS, FAT and coredump data
partitions. SPIFFS is unmounted before erasing it; raw erase does not create a
filesystem, so use Format SPIFFS afterwards if needed. NVS erase removes settings.
Apps, OTA metadata, NVS keys, calibration and unknown/read-only partitions are
protected. Partition-table resizing/editing and recovery self-update are not
implemented.

## Build and upload

With a Python interpreter that has PlatformIO installed:

```sh
python scripts/build_recovery.py -e OKABE_PHONE_S3_EXP
# Or: -e OKABE_PHONE_PROTO
```

Output: `.pio/recovery/<environment>/recovery.bin`. Only `recovery/` application
sources and required shared/ESP-IDF components are built; `src/` and recovery
host tests are excluded. This command doesn't flash anything.

Normal firmware builds copy recovery to `.pio/build/<environment>/recovery.bin`.
`pio run -e OKABE_PHONE_S3_EXP -t upload` flashes recovery at `0x10000` and main at
`0x220000`, with the bootloader and partition table. Recovery is rebuilt before
upload even when main is unchanged. Boards without a recovery partition upload
only their normal app.

## Host checks

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -Irecovery \
  recovery/image_check.cpp recovery/tests/image_check_test.cpp -o /tmp/image-check
/tmp/image-check .pio/build/OKABE_PHONE_S3_EXP/firmware.bin

g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -Irecovery/tests/fakes -Irecovery recovery/partition_policy.cpp \
  recovery/tests/partition_policy_test.cpp -o /tmp/partition-policy
/tmp/partition-policy
```

The image tests cover valid binaries, truncation, oversized/wrong-chip images,
revision bounds, malformed segments/descriptors, checksum failure and trailers.
The partition tests cover every subtype and the running/recovery/read-only guards.
SHA256 and final OTA image validation also run on-device; host checks do not
substitute for testing storage, keyboard, flash updates and power loss on hardware.

## Guru Meditation after a reset

On S3 layouts with factory recovery, the bootloader routes panic, watchdog and
brownout resets directly to recovery. The screen shows the reset cause/code and
offers Open recovery or Retry normal boot. The Last reset / diagnostics menu
reopens it during the same recovery session. Cold boots, explicit software
restarts, USB resets and deep-sleep wakeups do not trigger crash recovery.

The reset report links to Saved core dump when a valid dump exists. S3 builds
persist the panic reason, task/register state, stacks and internal DRAM in flash.
The first saved crash is retained, so it may precede the latest reset. A reset that
bypasses the panic handler (for example abrupt power loss or some hard watchdog
resets) may not produce a dump. Power loss can erase the reset hint, but an already
completed flash dump remains readable from recovery. The bootloader reads the
ESP-IDF 5.5.x ESP32/S3 RTC hint encoding without clearing it; review this adapter
when changing IDF versions. Boards with no factory partition retain normal boot.

Flash the updated bootloader **and** recovery through the normal combined upload
command to enable automatic routing. Merely updating the main application from
`.bin` does not replace the bootloader.

## Flash core dumps (S3)

The 16 MB layout adds a 1 MiB `coredump` data partition at `0xaf0000`, after
SPIFFS. Existing app and filesystem offsets/sizes are unchanged. Flash the full
updated partition table, bootloader, main app and recovery using the normal
PlatformIO upload command before relying on capture. An app-only update cannot
add the partition. The 4 MB prototype layout is full; capture remains disabled
there rather than shrinking or moving its existing filesystem.

Capture uses ESP-IDF's ELF core-dump payload with SHA256, up to 64 tasks, internal
DRAM capture and a dedicated 4096-byte panic stack. External PSRAM contents are
not included. Recovery validates bounds and checksum before attempting to decode.
It displays the saved panic reason, task, PC, exception cause/address, backtrace
addresses and originating ELF hash; details also go to the serial log.

Choose **Saved core dump → Export to SPIFFS / Export to SD** to write:

- `/spiffs/crsh0000.bin` or `/sd/crsh0000.bin`: raw flash core dump (header, ELF
  payload and checksum). This is **not** an installable application `.bin`.
- The corresponding `crsh0000.txt`: readable summary including the ELF SHA256.

Names increment without overwriting existing exports and work with FAT 8.3.
Writes use an exclusive `.tmp` file, flush it, compare every byte against flash,
then rename. Interrupted `.tmp` files are left untouched on later attempts;
ordinary detected export errors remove the temporary file. If the text export
fails after the raw export succeeds, the UI reports the saved raw path. The flash
dump is retained regardless; export never clears it.

**Clear saved dump** requires confirmation and allows the next crash to be
captured. The first crash is otherwise retained, including if recovery itself
panics. Clear it after exporting when you want to diagnose a new failure. If
the new partition initially reports invalid data, clear it once before testing
capture; normal firmware uploads do not erase the core-dump region.

### Decode function names and source lines on the host

Normal builds/uploads archive main and recovery ELFs by SHA256 under
`.pio/elf-archive/`. Keep that archive alongside exported crashes; cleaning `.pio`
removes it. The decoder selects the exact ELF recorded in the `.txt` summary,
not whichever firmware happened to be built last.

One-time decoder setup (already installed in this workspace):

```sh
python3 -m venv .pio/coredump-tools
.pio/coredump-tools/bin/pip install -r scripts/requirements-coredump.txt
```

Copy both exported files from storage to your computer, then:

```sh
python3 scripts/decode_coredump.py /path/to/crsh0000.bin
# Or provide the crashed build's ELF explicitly:
python3 scripts/decode_coredump.py /path/to/crsh0000.bin --elf /path/to/firmware.elf
# Interactive GDB:
python3 scripts/decode_coredump.py /path/to/crsh0000.bin --debug
```

The wrapper uses Espressif's `esp-coredump` decoder (`-t raw`) and PlatformIO's
Xtensa GDB. A supplied ELF is checked against the summary's full hash when present;
without a summary you must explicitly supply the original ELF. No serial device
is accessed by this file-decoding workflow. Recovery shows numeric addresses;
full symbol/source decoding happens on the host.

Reference: [ESP-IDF core-dump documentation](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32s3/api-guides/core_dump.html).

Additional host checks:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -Irecovery \
  recovery/dump_export.cpp recovery/tests/dump_export_test.cpp -o /tmp/dump-export
/tmp/dump-export
python3 recovery/tests/decode_coredump_test.py
```
