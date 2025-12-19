// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#include "transfer.h"

#include "bootio_error.h"
#include "portable.h"

#include <assert.h>
#include <libusb.h>
#include <stdlib.h>

/**
 * \enum DfuCommand
 * \brief
 */
enum DfuCommand
#if __STDC_VERSION__ == 202311L
    : uint8_t
#endif
{
    DfuCommand_Detach = 0x00,
    DfuCommand_Download = 0x01,
    DfuCommand_Upload = 0x02,
    DfuCommand_GetStatus = 0x03,
    DfuCommand_ClrStatus = 0x04,
    DfuCommand_GetState = 0x05,
    DfuCommand_Abort = 0x06,
};

#if __STDC_VERSION__ == 202311L
/**
 * \brief Default timeout (ms) for transfer.
 */
constexpr uint32_t default_timeout = 200;
#else
/**
 * \brief Default timeout (ms) for transfer.
 */
#define default_timeout 200
#endif

/**
 * \brief Shortcut to assert \c Configuration and \c device_handle
 *
 * \param file \c Configuration pointer
 */
#define assert_config(config) \
assert(config != wor_bootio_nullptr__);\
assert(config->device_handle != wor_bootio_nullptr__)

/**
 * \brief Mutable timeout (ms) for transfer.
*/
static uint32_t bootio_transfer_timeout = default_timeout;

int transfer_in(const struct Configuration *config,
                uint8_t *buf,
                const uint16_t chunk_size,
                const uint16_t transfer_count) {
    assert_config(config);
    assert(buf != wor_bootio_nullptr__);
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS |
                                   LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Upload,
                                   transfer_count,
                                   config->interface_number,
                                   buf,
                                   chunk_size,
                                   bootio_transfer_timeout);
}

int transfer_out(const struct Configuration *config,
                 const uint8_t *buf,
                 const uint16_t chunk_size,
                 const uint16_t transfer_count) {
    assert_config(config);
    assert(buf != wor_bootio_nullptr__);
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Download,
                                   transfer_count,
                                   config->interface_number,
                                   (uint8_t *)buf,
                                   chunk_size,
                                   bootio_transfer_timeout);
}

int transfer_status(const struct Configuration *config, const uint8_t *buffer) {
    assert_config(config);
    if (buffer == wor_bootio_nullptr__) {
        return BootIoError_Other;
    }
    const int ec = libusb_control_transfer(config->device_handle,
                                           LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                           DfuCommand_GetStatus,
                                           0,
                                           config->interface_number,
                                           (uint8_t *)buffer,
                                           6,
                                           bootio_transfer_timeout);
    return ec;
}

int dfu_clear_status(const struct Configuration *config) {
    assert_config(config);
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_ClrStatus,
                                   0,
                                   config->interface_number,
                                   wor_bootio_nullptr__,
                                   0,
                                   bootio_transfer_timeout);
}

int dfu_abort(const struct Configuration *config) {
    assert_config(config);
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Abort,
                                   0,
                                   config->interface_number,
                                   wor_bootio_nullptr__,
                                   0,
                                   bootio_transfer_timeout);
}

int dfu_detach(const struct Configuration *config, const uint8_t detach_timeout) {
    assert_config(config);
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Detach,
                                   detach_timeout,
                                   config->interface_number,
                                   wor_bootio_nullptr__,
                                   0,
                                   bootio_transfer_timeout);
}

void set_device_timeout(const uint32_t new_timeout) {
    bootio_transfer_timeout = new_timeout;
}
