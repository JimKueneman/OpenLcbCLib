# CCS Theia: Disabling the Default Linker File

## Problem

CCS Theia auto-generates a linker command file via SysConfig. If you need a custom linker file (e.g. for a bootloader memory layout), manual edits to the generated file are overwritten on every build.

## Solution: Disable via SysConfig

1. Open the `.syscfg` file in CCS Theia
2. Find the **Project Configuration** module
3. Uncheck / disable the option to **generate the linker file**
4. Add your custom `.cmd` linker file to the project

Once disabled, SysConfig will no longer overwrite your linker file on build.

## Alternative: Exclude from Build

If the project does not use SysConfig for linker generation:

1. Right-click the default `.cmd` (TI Clang) or `.lds` (GCC) linker file in the Explorer view
2. Select **Exclude from Build** (the file will appear crossed out)
3. Add your custom linker file to the project

## MSPM0G3507 Memory Map Reference

| Region              | Address Range                    | Size  |
|---------------------|----------------------------------|-------|
| MAIN Flash          | `0x00000000` - `0x0001FFFF`      | 128KB |
| SRAM (data)         | `0x20200000` - `0x20207FFF`      | 32KB  |
| NONMAIN (BCR/BSL)   | `0x41C00000`                     | -     |

- Flash word size: 8 bytes (64-bit data + 8-bit ECC). All addresses and lengths must be 8-byte aligned.
- Lower 32KB of flash has higher erase/program endurance (suitable for bootloader region).
- No dedicated hardware bootloader region; you define the split in the linker file.

## Linker Settings in Project Properties

Additional linker options are available under:

- **Project Properties -> Build Tools -> Arm Linker -> Basic Options** (stack/heap size)
- **Edit Flags** button for adding/removing linker flags directly
- **Link Order** controls the order files are passed to the linker

## References

- [CCS Theia IDE Guide for MSPM0](https://software-dl.ti.com/msp430/esd/MSPM0-SDK/latest/docs/english/tools/ccs_theia_ide_guide/doc_guide/doc_guide-srcs/ccs_theia_ide_guide.html)
- [CCS Theia Project Management](https://software-dl.ti.com/ccs/esd/documents/users_guide_ccs_theia/ccs_project-management.html)
- [MSPM0 Bootloader User's Guide (SLAU887A)](https://www.ti.com/lit/ug/slau887a/slau887a.pdf)
- [MSPM0G3507 Datasheet](https://www.ti.com/product/MSPM0G3507)
