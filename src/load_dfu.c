#include "load.h"

#include "bootio_error.h"
#include "transfer.h"
#include "portable.h"

#include <stdlib.h>

#include <libusb.h>

int upload_dfu(struct Configuration *config,
               const uint8_t *buffer,
               const size_t expected_size,
               const size_t chunk_size) {
    if (buffer == wor_bootio_nullptr__ || config == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
    size_t total_bytes = 0;

    int ec = libusb_open(config->device, &config->device_handle);
    if (ec < 0) {
        return ec;
    }
    for (uint16_t transfer_count = 2; total_bytes < expected_size; transfer_count++) {
        uint8_t *buf_head = (uint8_t *)buffer + total_bytes;
        const int bytes = transfer_in(config, buf_head, chunk_size, transfer_count);

        if (bytes < 0) {
            ec = bytes;
            break;
        }

        total_bytes += bytes;

        if (bytes < chunk_size) {
            ec = BootIoError_Success;
            break;
        }
    }
    if (ec < 0) {
        goto out;
    }
    ec = wait_for_dfu_idle(config);
out:
    libusb_close(config->device_handle);
    config->device_handle = wor_bootio_nullptr__;
    return ec < 0
               ? ec
               : (int)total_bytes;
}

int download_dfu(struct Configuration *config,
                 uint8_t *buffer,
                 const size_t expected_size,
                 size_t chunk_size) {
    if (buffer == wor_bootio_nullptr__ || config == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
    size_t total_bytes = 0;
    int ec = libusb_open(config->device, &config->device_handle);
    if (ec < 0) {
        return ec;
    }
    for (uint32_t i = 0, transfer_count = 2; total_bytes < expected_size; ++i, ++transfer_count) {
        if (expected_size - total_bytes < chunk_size) {
            chunk_size = expected_size - total_bytes;
        }
        const uint8_t *buf_head = buffer + (chunk_size * i);
        const int bytes_sent = transfer_out(config, buf_head, chunk_size, transfer_count);
        if (bytes_sent < 0) {
            goto out;
        }

        total_bytes += bytes_sent;

        ec = wait_for_download_idle(config);
        if (ec < 0) {
            goto out;
        }
    }

    ec = abort_and_wait_idle(config);
    if (ec < 0) {
        goto out;
    }

    wor_bootio_constexpr__ uint8_t terminating_byte = 0x00;
    ec = transfer_out(config, &terminating_byte, 0, 0);

    if (ec < 0) {
        return ec;
    }

    ec = wait_for_manifest(config);
out:
    libusb_close(config->device_handle);
    config->device_handle = wor_bootio_nullptr__;
    return ec < 0
               ? ec
               : (int)total_bytes;
}
