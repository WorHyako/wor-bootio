# Feature roadmap

Legend:

- ✅ Done
- 🚧 In progress
- 📌 Backlog / planned
- ❓ Maybe planned

### Containers

| Feature    | Status | Notes                    |
|------------|:------:|--------------------------|
| Raw buffer |   ✅    | Done                     |
| .dfu       |   🚧   | In work with `DfuFile`   |
| .dfuse     |   🚧   | In work with `DfuSeFile` |
| .bin       |   📌   |                          |
| .elf       |   📌   |                          |
| .hex       |   📌   |                          |

### DFU protocol (USB DFU 1.x)

| Feature                      | Status | Notes                                                              |
|------------------------------|:------:|--------------------------------------------------------------------|
| Downloading                  |   ✅    | Done with `download_dfu(...)`                                      |
| Uploading                    |   ✅    | Done with `upload_dfu(...)`                                        |
| DFU_GETSTATUS                |   ✅    | Done with `get_status(...)`                                        |
| DFU_CLRSTATUS                |   ✅    | Done with `dfu_clear_status(...)`                                  |
| DFU_DETACH                   |   ✅    | Done with `dfu_detach(...)`                                        |
| DFU state machine validation |   ✅    | Done in upload/download processes;<br/>Done in `dfu_status.c` file |

### DfuSe extensions

| Feature                  | Status | Notes                                  |
|--------------------------|:------:|----------------------------------------|
| Downloading              |   ✅    | Done with `download_dfuse(...)`        |
| Uploading                |   ✅    | Done with `upload_dfuse(...)`          |
| DfuSe memory map parsing |   ✅    | Done with `parse_memory_segments(...)` |
| SET_ADDRESS command      |   ✅    | Done with `dfuse_cmd_set_address(...)` |
| ERASE_PAGE command       |   ✅    | Done with `dfuse_cmd_erase_page(...)`  |
| MASS_ERASE command       |   ✅    | Done with `dfuse_cmd_mass_erase(...)`  |
| LEAVE command            |   ✅    | Done with `dfuse_cmd_leave(...)`       |
| UNPROTECT command        |   📌   |                                        |

### Converters

| From \ To | raw | .bin | .hex | .elf | .dfu | .dfuse |
|-----------|:---:|:----:|:----:|:----:|:----:|:------:|
| raw       |  —  |  📌  |  📌  |  📌  |  📌  |   📌   |
| .bin      |  ❓  |  —   |  📌  |  ❓   |  ❓   |   ❓    |
| .hex      |  ❓  |  ❓   |  —   |  ❓   |  ❓   |   ❓    |
| .elf      |  ❓  |  ❓   |  ❓   |  —   |  ❓   |   ❓    |
| .dfu      |  ❓  |  ❓   |  ❓   |  ❓   |  —   |   ❓    |
| .dfuse    |  ❓  |  ❓   |  ❓   |  ❓   |  ❓   |   —    |

### Test coverage

| Feature                        | Status | Notes |
|--------------------------------|:------:|-------|
| CMock generation               |   🚧   |       |
| Unit tests (CRC32)             |   📌   |       |
| Unit tests (memory segment)    |   📌   |       |
| Unit tests (configuration)     |   📌   |       |
| Unit tests (transfer)          |   📌   |       |
| Unit tests (dfu loading)       |   📌   |       |
| Unit tests (dfuse loading)     |   📌   |       |
| Static analyzer support        |   📌   |       |
| Sanitizer support (ASan/UBSan) |   📌   |       |
