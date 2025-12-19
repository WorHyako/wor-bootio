// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#include "dfu_status.h"

#include "bootio_error.h"
#include "transfer.h"
#include "dfu_func_dt.h"

#include <libusb.h>

#if __STDC_VERSION__ == 202311L
/**
 * \brief Max count of device's waiting for an answer after status request.
 */
constexpr uint16_t max_wait_count = 10;
#else
/**
 * \brief Max count of device's waiting for an answer after status request.
 */
#define max_wait_count 10
#endif

int wait_for_download_idle(const struct Configuration *config) {
    int ec;
    struct DeviceDfuStatus status;
    uint16_t i = 0;
    do {
        ec = get_status(config, &status);
        /// Next condition works only in DfuSe case.
        if (ec == BootIoError_Transfer_Pipe) {
            wor_bootio_sleep_ms(1);
            continue;
        }

        if (ec < 0) {
            break;
        }
        if (status.state == DfuState_Error) {
            ec = BootIoError_Other;
            break;
        }
        wor_bootio_sleep_ms(status.timeout);
    } while (++i < max_wait_count && status.state != DfuState_DownloadIdle);
    return ec > -1 && status.status != DfuStatus_Ok
               ? BootIoError_Other
               : ec;
}

int wait_for_manifest(const struct Configuration *config) {
    int ec;
    struct DeviceDfuStatus status;
    uint16_t i = 0;
    do {
        ec = get_status(config, &status);
        if (ec < 0) {
            break;
        }

        if (status.state == DfuState_Error) {
            ec = dfu_clear_status(config);
            if (ec < 0) {
                break;
            }
        }
        wor_bootio_sleep_ms(status.timeout);
    } while (++i < max_wait_count && status.state != DfuState_Manifest);
    return ec > -1 && status.status != DfuStatus_Ok
               ? BootIoError_Other
               : ec;
}

int wait_for_dfu_idle(const struct Configuration *config) {
    int ec;
    struct DeviceDfuStatus status;

    const int manifest_tolerant = config->func_dt.attributes & DfuFuncDtAttributes_ManifestTolerant;

    uint16_t i = 0;
    do {
        ec = get_status(config, &status);
        if (ec < 0) {
            break;
        }
        switch (status.state) {
        case DfuState_ManifestSync:
        case DfuState_Manifest:
            break;
        case DfuState_ManifestWaitReset:
            if (manifest_tolerant) {
                return ec;
            }
            ec = libusb_reset_device(config->device_handle);
            if (ec < 0) {
                return ec;
            }
            break;
        default:
            break;
        }
    } while (++i < max_wait_count && status.state != DfuState_Idle);
    return ec > -1 && status.status != DfuStatus_Ok
               ? BootIoError_Other
               : ec;
}

int abort_and_wait_idle(const struct Configuration *config) {
    if (config == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
    int ec = dfu_abort(config);
    if (ec < 0) {
        return ec;
    }

    return wait_for_dfu_idle(config);
}

int get_status(const struct Configuration *config, struct DeviceDfuStatus *status) {
    if (config == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
    wor_bootio_constexpr__ uint8_t buffer[6] = { 0x00, 0x00, 0x00, 0x00, DfuState_Error, 0x00 };
    const int ec = transfer_status(config, buffer);
    if (ec < 0) {
        return ec;
    }
    if (ec != 6) {
        return BootIoError_Other;
    }

    status->status = buffer[0];
    status->timeout = (0xff & buffer[3]) << 16
                      | (0xff & buffer[2]) << 8
                      | 0xff & buffer[1];
    status->state = buffer[4];
    status->str_idx = buffer[5];
    return BootIoError_Success;
}
