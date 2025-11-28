#include "transfer.h"

#include "bootio_error.h"
#include "portable.h"

#include <assert.h>
#include <libusb.h>

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
/**
 * \brief Max count of device's waiting for an answer after status request.
 */
constexpr uint16_t max_wait_count = 10;
#else
/**
 * \brief Default timeout (ms) for transfer.
 */
#define default_timeout 200
/**
 * \brief Max count of device's waiting for an answer after status request.
 */
#define max_wait_count 10
#endif

/**
 * \brief Mutable timeout (ms) for transfer.
*/
static uint32_t bootio_transfer_timeout = default_timeout;

int transfer_in(const struct Configuration *config,
                uint8_t *buf,
                const uint16_t chunk_size,
                const uint16_t transfer_count) {
    assert(config != wor_bootio_nullptr__);
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
    assert(config != wor_bootio_nullptr__);
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

int get_status(const struct Configuration *config, struct DeviceDfuStatus *status) {
    if (config == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
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
