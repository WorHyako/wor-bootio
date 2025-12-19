# Feature roadmap

Legend:

- ✅ Implemented
- 🚧 In progress
- 📌 Backlog / planned

### DFU protocol (USB DFU 1.x)

| Feature                      | Status | Notes |
|------------------------------|:------:|-------|
| DFU_DNLOAD                   |   ✅    |       |
| DFU_UPLOAD                   |   ✅    |       |
| DFU_GETSTATUS                |   ✅    |       |
| DFU_CLRSTATUS                |   ✅    |       |
| DFU_DETACH                   |   🚧   |       |
| DFU state machine validation |   🚧   |       |

## DfuSe extensions

| Feature                  | Status | Notes                |
|--------------------------|--------|----------------------|
| DfuSe memory map parsing | ✅      | Segment/page model   |
| Automatic page erase     | ✅      | Driven by memory map |
| SET_ADDRESS command      | ✅      | DfuSe vendor command |
| ERASE_PAGE command       | ✅      | Page-level erase     |
| MASS_ERASE command       | ✅      | Full flash erase     |
| LEAVE command            | ✅      | Exit DFU mode        |
| UNPROTECT command        | 📌     | Planned              |
