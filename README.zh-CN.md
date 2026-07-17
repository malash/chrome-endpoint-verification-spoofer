# chrome-endpoint-verification-spoofer

<p align="center">
  <img width="200" src="https://github.com/user-attachments/assets/a0adcb20-2cb8-4800-a29f-6be001bb6587" alt="logo">
</p>

[English](./README.md) | [中文](./README.zh-CN.md)

一个用于为 Chrome 伪造上报设备信息的项目——尤其针对 **Endpoint Verification** 插件。在被注入的进程里伪造:

- **序列号**（`IOPlatformSerialNumber`）
- **macOS 版本**

![chrome://version 截图](https://github.com/user-attachments/assets/8a576812-1146-46c5-a6bd-c9bad14eea5e)

> **⚠️ 由 AI 辅助编写，请谨慎使用。**
> 本项目完全通过 AI 辅助的"氛围编程"构建，未经审计。实现中可能存在难以察觉的 bug 或从未被考虑到的边界情况。请仔细审查，风险自负。

## 准备

在目标机器上:

1. **关闭 SIP。**
2. **关闭库验证（library validation）**，这样 `DYLD_INSERT_LIBRARIES` 才能注入到启用了
   macOS 库验证的二进制中（如 Chrome）:

   ```bash
   sudo defaults write /Library/Preferences/com.apple.security.libraryvalidation.plist DisableLibraryValidation -bool true
   ```

## 用法

```bash
make
cp spoof.conf.example spoof.conf
build/spoof "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
```

用 `chrome://version` 的 “OS” 行验证。

> `navigator.userAgent` 看不出来——Chrome 把那里的 macOS 版本冻结在 `10_15_7`。

## 配置

`spoof.conf`

```
serial=C02XXXXXXXXX
os_version=26.5.2
os_version_string=Version 26.5.2 (Build 25F84)
```

- 每次启动时读取——改完重启即可,无需重新编译。
- 省略某个键就保留该项真实值（例如去掉 `os_version` 就只伪造序列号）。
- 可用 `SPOOF_CONFIG` 环境变量覆盖配置路径。

## 原理

- **启动器**（`spoof`）:设置 `DYLD_INSERT_LIBRARIES=<dir>/spoof-lib.dylib` 和
  `SPOOF_CONFIG=<dir>/../spoof.conf`,然后 `execvp` 执行目标。
- **dylib**（`spoof-lib.dylib`）:加载时读取配置,然后
  - interpose `IORegistryEntryCreateCFProperty`,对 `IOPlatformSerialNumber` 返回伪造序列号;
  - swizzle `NSProcessInfo` 的 `operatingSystemVersion` / `operatingSystemVersionString`。

  这些正是 Chrome 采集设备信号读取的原生 API,所以也就是 Endpoint Verification 上报的内容。dylib 会在它被加载进的每个进程里生效。

## 伪造为最新 macOS 版本

```bash
scripts/latest-macos.sh
```

该命令会将 `spoof.conf` 更新为最新的 macOS 版本。

## 测试

```bash
make test
```
