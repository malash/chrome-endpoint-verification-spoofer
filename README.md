# chrome-endpoint-verification-spoofer

<p align="center">
  <img width="200" src="https://github.com/user-attachments/assets/a0adcb20-2cb8-4800-a29f-6be001bb6587" alt="logo">
</p>

[English](./README.md) | [中文](./README.zh-CN.md)

A macOS project for spoofing the device info Chrome reports — in particular for the
**Endpoint Verification** extension. In the injected process it spoofs:

- the **serial number** (`IOPlatformSerialNumber`)
- the **macOS version**

![chrome://version screenshot](https://github.com/user-attachments/assets/8a576812-1146-46c5-a6bd-c9bad14eea5e)

> **⚠️ Vibe coded — use with caution.** This project was built entirely through AI-assisted vibe
> coding and has not been audited. The implementation may contain subtle bugs or edge cases that
> were never considered. Review carefully and use at your own risk.

## Prepare

On the target machine:

1. **Disable SIP.**
2. **Disable library validation** so `DYLD_INSERT_LIBRARIES` can inject into a binary
   with macOS library validation (like Chrome):

   ```bash
   sudo defaults write /Library/Preferences/com.apple.security.libraryvalidation.plist DisableLibraryValidation -bool true
   ```

## Usage

```bash
make
cp spoof.conf.example spoof.conf
build/spoof "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
```

Verify with `chrome://version` (the "OS" line).

> `navigator.userAgent` won't show it — Chrome freezes the macOS version there at `10_15_7`.

## Config

`spoof.conf`

```
serial=C02XXXXXXXXX
os_version=26.5.2
os_version_string=Version 26.5.2 (Build 25F84)
```

- Read at each launch — edit and relaunch, no rebuild.
- Omit a key to leave that value real (e.g. drop `os_version` to spoof only the serial).
- Override the config path with the `SPOOF_CONFIG` env var.

## How it works

- **Launcher** (`spoof`): sets `DYLD_INSERT_LIBRARIES=<dir>/spoof-lib.dylib` and
  `SPOOF_CONFIG=<dir>/../spoof.conf`, then `execvp`s the target.
- **dylib** (`spoof-lib.dylib`): on load, reads the config, then
  - interposes `IORegistryEntryCreateCFProperty` to return the fake serial for
    `IOPlatformSerialNumber`;
  - swizzles `NSProcessInfo`'s `operatingSystemVersion` / `operatingSystemVersionString`.

  These are the same native APIs Chrome's device-signal collection reads, so it is what
  Endpoint Verification reports. The dylib runs in every process it loads into.

## Spoof the latest macOS version

```bash
scripts/latest-macos.sh
```

This command updates `spoof.conf` to the latest macOS version.

## Test

```bash
make test
```
