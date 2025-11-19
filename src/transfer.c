#include "transfer.h"

#include "portable.h"

#include <libusb.h>

#if __STDC_VERSION__ == 202311L
/**
 * \brief
 */
constexpr uint32_t default_timeout = 200;
/**
 * \brief
 */
constexpr uint16_t max_wait_count = 10;
#else
#define default_timeout 200
#define max_wait_count 10
#endif

/**
 * \brief
*/
static uint32_t bootio_transfer_timeout = default_timeout;

int transfer_in(const struct Configuration *config,
                uint8_t *buf,
                const uint16_t chunk_size,
                const uint16_t transfer_count) {
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
    return libusb_control_transfer(config->device_handle,
                                   LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                   DfuCommand_Download,
                                   transfer_count,
                                   config->interface_number,
                                   (uint8_t *)buf,
                                   chunk_size,
                                   bootio_transfer_timeout);
}

int get_status(const struct Configuration *config, struct DeviceDfuStatus *status) {
    uint8_t buffer[6] = { 0x00, 0x00, 0x00, 0x00, DfuState_Error, 0x00 };
    const int ec = libusb_control_transfer(config->device_handle,
                                           LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
                                           DfuCommand_GetStatus,
                                           0,
                                           config->interface_number,
                                           buffer,
                                           6,
                                           bootio_transfer_timeout);
    if (ec < 0) {
        return ec;
    }
    if (ec != 6) {
        return LIBUSB_ERROR_OTHER;
    }

    status->status = buffer[0];
    status->timeout = (0xff & buffer[3]) << 16
                      | (0xff & buffer[2]) << 8
                      | 0xff & buffer[1];
    status->state = buffer[4];
    status->str_idx = buffer[5];
    return LIBUSB_SUCCESS;
}

int dfu_clear_status(const struct Configuration *config) {
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

int wait_for_download_idle(const struct Configuration *config) {
    int ec;
    struct DeviceDfuStatus status;
    uint16_t i = 0;
    do {
        ec = get_status(config, &status);
        if (ec == LIBUSB_ERROR_PIPE) {
            wor_bootio_sleep_ms(1);
            continue;
        }

        if (ec < 0) {
            break;
        }
        if (status.state == DfuState_Error) {
            ec = -1;
            break;
        }
        wor_bootio_sleep_ms(status.timeout);
    } while (++i < max_wait_count && status.state != DfuState_DownloadIdle);
    return ec > -1 && status.status != DfuStatus_Ok
               ? -1
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
               ? -1
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
        wor_bootio_sleep_ms(status.timeout);
    } while (++i < max_wait_count && status.state != DfuState_DownloadIdle);
    return ec > -1 && status.status != DfuStatus_Ok
               ? -1
               : ec;
}

int abort_and_wait_idle(const struct Configuration *config) {
    if (config == wor_bootio_nullptr__) {
        return -1;
    }
    int ec = dfu_abort(config);
    if (ec < 0) {
        return ec;
    }

    return wait_for_download_idle(config);
}
