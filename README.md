# Wor::BootIo

## Description

**bootio** is a host-side C library for interacting with USB DFU / DfuSe devices using libusb.
It is designed for programmatic and embedded tooling use (SDKs, custom flashers, CI pipelines), not as an end-user CLI
utility.

The library provides a deterministic, explicit API for:

- DFU control transfers (`DNLOAD`, `UPLOAD`, `GETSTATUS`, `CLRSTATUS`, `DETACH`)
- DfuSe-specific commands (`SET_ADDRESS`, `ERASE_PAGE`, `MASS_ERASE`, `LEAVE`)
- Parsing DfuSe memory maps
- Handling DFU suffix and CRC32

Feature map see is the [FeatureMap.md](FeatureMap.md) file.

---

## License

Licensed under the MIT License.

See the [LICENSE](LICENSE) file for details.

## Requirements

- CMake (3.23 minimum)
- vcpkg (2025.09.17 minimum)
- C17/23 (MSVC, LLVM Clang)

---

## Building

```shell
mkdir build && cd build
cmake ..
cmake --build .
```

