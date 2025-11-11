#include "load.h"

#include "transfer.h"

#include <libusb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int upload_dfu(const struct Configuration *config,
               const uint8_t *buffer,
               const size_t expected_size,
               size_t chunk_size,
               uint32_t transfer_count) {
    if (buffer == nullptr) {
        return LIBUSB_ERROR_OVERFLOW;
    }
    size_t total_bytes = 0;
    uint8_t *buf_temp = calloc(expected_size, sizeof(uint8_t));
    int ec = 0;

    while (true) {
        if (expected_size - total_bytes < chunk_size) {
            chunk_size = (int)(expected_size - total_bytes);
        }
        const int bytes = transfer_in(config, buf_temp, chunk_size, transfer_count++);
        if (bytes < 0) {
            ec = bytes;
            break;
        }
        memcpy(buf_temp, buffer + total_bytes, chunk_size);
        if (bytes < chunk_size) {
            ec = LIBUSB_ERROR_IO;
            break;
        }
        total_bytes += bytes;
        if (total_bytes >= expected_size) {
            ec = LIBUSB_SUCCESS;
            break;
        }
    }
    free(buf_temp);
    return ec < 0 ? ec : (int)total_bytes;
}

int download_dfu(const struct Configuration *config,
                 uint8_t *buffer,
                 const size_t
                 expected_size,
                 size_t chunk_size) {
    if (buffer == nullptr) {
        return LIBUSB_ERROR_OVERFLOW;
    }
    size_t total_bytes = 0;
    int ec = 0;
    int transfer_count = 0;
    struct DeviceDfuStatus status;

    while (total_bytes < expected_size) {
        if (expected_size - total_bytes < chunk_size) {
            chunk_size = expected_size - total_bytes;
        }
        uint8_t *buf_head = buffer + chunk_size * transfer_count;
        const int bytes_sent = transfer_out(config, buf_head, chunk_size, transfer_count++);
        if (bytes_sent < 0) {
            ec = bytes_sent;
            break;
        }
        if (bytes_sent < chunk_size) {
            ec = LIBUSB_ERROR_IO;
            break;
        }
        total_bytes += bytes_sent;

        while (true) {
            ec = get_status(config, &status);
            if (ec < 0) {
                break;
            }
            if (status.state == DfuState_AppIdle || status.state == DfuState_Error) {
                break;
            }
            sleep(status.timeout);
        }
        if (status.status != DfuStatus_Ok) {
            ec = LIBUSB_ERROR_IO;
            break;
        }
    }

    if (ec < 0) {
        return ec;
    }

    constexpr uint8_t terminating_byte = 0x00;
    ec = transfer_out(config, &terminating_byte, 1, transfer_count);

    if (ec < 0) {
        printf("Error in sending terminating byte: %s", libusb_error_name(ec));
        return ec;
    }

    while (true) {
        ec = get_status(config, &status);
        if (ec < 0) {
            printf("Error in get_status: %s", libusb_error_name(ec));
            break;
        }
        switch (status.state) {
        case DfuState_Idle:
            break;
        case DfuState_ManifestSync:
        case DfuState_Manifest:
            continue;
        case DfuState_ManifestWaitReset:
            ec = libusb_reset_device(config->device_handle);
            if (ec < 0) {
                printf("Error in libusb_reset_device: %s", libusb_error_name(ec));
            }
            break;
        default:
            break;
        }
    }
    return ec;
}
