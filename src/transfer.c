#include "transfer.h"

#include <libusb.h>
#include <stdarg.h>
#include <unistd.h>

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
                                   DfuCommand_Upload,
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
                                   DfuCommand_Download,
                                   transfer_count,
                                   config->interface,
                                   (uint8_t *)buf,
                                   chunk_size,
                                   timeout);
}

int get_status(const struct Configuration *config, struct DeviceDfuStatus *status) {
    const int ec = libusb_control_transfer(config->device_handle,
                                           LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                           DfuCommand_GetStatus,
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

int dfu_abort(const struct Configuration *config) {
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Abort,
                                   0,
                                   config->interface,
                                   nullptr,
                                   0,
                                   timeout);
}

int wait_for_state(const struct Configuration *config, const uint8_t times, ...) {
    struct DeviceDfuStatus status;

    uint8_t waitable_states[6];
    va_list args = nullptr;
    va_start(args);

    int count = va_arg(args, int);
    count = count > 6
                ? 6
                : count;

    for (int i = 0; i < count; ++i) {
        waitable_states[i] = (uint8_t)va_arg(args, int);
    }
    va_end(args);

    for (int i = 0; i < times; i++) {
        int ec = get_status(config, &status);
        if (ec == LIBUSB_ERROR_PIPE) {
            continue;
        }
        if (status.state != DfuState_ManifestWaitReset) {
            ec = libusb_reset_device(config->device_handle);
            return ec < 0 && ec != LIBUSB_ERROR_NOT_FOUND
                       ? ec
                       : 0;
        }
        if (ec < 0) {
            return ec;
        }
        if (status.state == DfuState_Error) {
            break;
        }
        for (int state_idx = 0; state_idx < count; ++state_idx) {
            if (status.state == waitable_states[state_idx]) {
                break;
            }
        }
        sleep(status.timeout);
    }
    if (status.status != DfuStatus_Ok) {
        return LIBUSB_ERROR_PIPE;
    }
    return 0;
}

int dfu_detach(const struct Configuration *config, const uint8_t detach_timeout) {
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Detach,
                                   detach_timeout,
                                   config->interface,
                                   nullptr,
                                   0,
                                   timeout);
}

void set_device_timeout(const uint32_t new_timeout) {
    timeout = new_timeout;
}
