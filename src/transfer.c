#include "transfer.h"

#include <libusb.h>

/**
 * \brief
 */
static constexpr uint32_t default_timeout = 5'000;

/**
 * \brief
 */
static uint32_t timeout = default_timeout;

int transfer_in(const struct Configuration *config,
                uint8_t *buf,
                const uint16_t chunk_size,
                const uint16_t transfer_count) {
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS |
                                   LIBUSB_RECIPIENT_INTERFACE,
                                   COMMAND_DFU_UPLOAD,
                                   transfer_count,
                                   config->interface,
                                   buf,
                                   chunk_size,
                                   timeout);
}

int transfer_out(const struct Configuration *config,
                 const uint8_t *buf,
                 const uint16_t chunk_size,
                 const uint16_t transfer_count) {
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   COMMAND_DFU_DNLOAD,
                                   transfer_count,
                                   config->interface,
                                   (uint8_t*)buf,
                                   chunk_size,
                                   timeout);
}

int get_status(const struct Configuration *config, struct DeviceDfuStatus *status) {
    const int ec = libusb_control_transfer(config->device_handle,
                                           LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                           COMMAND_DFU_GETSTATUS,
                                           0,
                                           config->interface,
                                           (uint8_t *)status,
                                           sizeof(struct DeviceDfuStatus),
                                           timeout);
    if (ec < 0) {
        return ec;
    }
    if (ec != 6) {
        return LIBUSB_ERROR_OTHER;
    }
    return LIBUSB_SUCCESS;
}

void set_device_timeout(const uint32_t new_timeout) {
    timeout = new_timeout;
}
