#include "load.h"

#include "transfer.h"

#include <libusb.h>

#include <stdlib.h>
#include <unistd.h>

/**
 * \brief
 */
constexpr uint16_t max_wait_count = 10;

/**
 * \brief
 *
 * \param config
 * \return
 */
[[nodiscard]]
static int wait_for_download_idle(const struct Configuration *config) {
    int ec;
    struct DeviceDfuStatus status;
    uint16_t i = 0;
    do {
        ec = get_status(config, &status);
        if (ec < 0) {
            break;
        }
        if (status.state == DfuState_Error) {
            ec = -1;
            break;
        }
        sleep(status.timeout);
    } while (++i < max_wait_count || status.state != DfuState_DownloadIdle);
    return ec > -1 && status.status == DfuStatus_Ok
               ? -1
               : ec;
}

/**
 * \brief
 *
 * \param config
 * \return
 */
[[nodiscard]]
static int wait_for_dfu_idle(const struct Configuration *config) {
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
        sleep(status.timeout);
    } while (++i < max_wait_count || status.state != DfuState_Idle);
    return ec;
}

int upload_dfu(const struct Configuration *config,
               const uint8_t *buffer,
               const size_t expected_size,
               size_t chunk_size) {
    if (buffer == nullptr) {
        return LIBUSB_ERROR_OVERFLOW;
    }
    size_t total_bytes = 0;
    uint8_t *buf_head = (uint8_t *)buffer;
    int ec = 0;

    for (uint16_t transfer_count = 2; total_bytes < expected_size; transfer_count++) {
        /**
         * TODO: maybe useless, coz `bytes < chunk_size` means end of uploading for USB device.
         */
        if (expected_size - total_bytes < chunk_size) {
            chunk_size = (int)(expected_size - total_bytes);
        }

        const int bytes = transfer_in(config, buf_head, chunk_size, transfer_count);
        if (bytes < 0) {
            ec = bytes;
            break;
        }

        buf_head += bytes;
        total_bytes += bytes;
        if (total_bytes >= expected_size || bytes < chunk_size) {
            ec = LIBUSB_SUCCESS;
            break;
        }
    }
    if (ec < 0) {
        return ec;
    }
    ec = wait_for_dfu_idle(config);
    return ec < 0
               ? ec
               : (int)total_bytes;
}

int download_dfu(const struct Configuration *config,
                 uint8_t *buffer,
                 const size_t expected_size,
                 size_t chunk_size) {
    if (buffer == nullptr || config == nullptr) {
        return LIBUSB_ERROR_OVERFLOW;
    }
    size_t total_bytes = 0;
    int ec = 0;

    uint32_t transfer_count = 0;
    for (uint32_t i = 0; total_bytes < expected_size; ++i) {
        if (expected_size - total_bytes < chunk_size) {
            chunk_size = expected_size - total_bytes;
        }
        const uint8_t *buf_head = buffer + chunk_size * i;
        transfer_count = i + 2;
        const int bytes_sent = transfer_out(config, buf_head, chunk_size, transfer_count);
        if (bytes_sent < 0) {
            ec = bytes_sent;
            break;
        }

        total_bytes += bytes_sent;

        if (bytes_sent < chunk_size) {
            ec = LIBUSB_SUCCESS;
            break;
        }
        ec = wait_for_download_idle(config);
        if (ec < 0) {
            break;
        }
    }
    transfer_count++;
    if (ec < 0) {
        return ec;
    }

    constexpr uint8_t terminating_byte = 0x00;
    ec = transfer_out(config, &terminating_byte, 0, transfer_count);
    if (ec < 0) {
        return ec;
    }

    ec = wait_for_dfu_idle(config);
    return ec;
}
