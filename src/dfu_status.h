#pragma once

#include <stdint.h>

enum DfuCommand : uint8_t {
    COMMAND_DFU_DETACH = 0x00,
    COMMAND_DFU_DNLOAD = 0x01,
    COMMAND_DFU_UPLOAD = 0x02,
    COMMAND_DFU_GETSTATUS = 0x03,
    COMMAND_DFU_CLRSTATUS = 0x04,
    COMMAND_DFU_GETSTATE = 0x05,
    COMMAND_DFU_ABORT = 0x06,
};

enum DfuState : uint8_t {
    STATE_APP_IDLE = 0x00,
    STATE_APP_DETACH = 0x01,
    STATE_DFU_IDLE = 0x02,
    STATE_DFU_DOWNLOAD_SYNC = 0x03,
    STATE_DFU_DOWNLOAD_BUSY = 0x04,
    STATE_DFU_DOWNLOAD_IDLE = 0x05,
    STATE_DFU_MANIFEST_SYNC = 0x06,
    STATE_DFU_MANIFEST = 0x07,
    STATE_DFU_MANIFEST_WAIT_RESET = 0x08,
    STATE_DFU_UPLOAD_IDLE = 0x09,
    STATE_DFU_ERROR = 0x0a
};

enum DfuStatus : uint8_t {
    DFU_STATUS_OK = 0x0,
    DFU_STATUS_ERROR_TARGET = 0x01,
    DFU_STATUS_ERROR_FILE = 0x02,
    DFU_STATUS_ERROR_WRITE = 0x03,
    DFU_STATUS_ERROR_ERASE = 0x04,
    DFU_STATUS_ERROR_CHECK_ERASED = 0x05,
    DFU_STATUS_ERROR_PROG = 0x06,
    DFU_STATUS_ERROR_VERIFY = 0x07,
    DFU_STATUS_ERROR_ADDRESS = 0x08,
    DFU_STATUS_ERROR_NOTDONE = 0x09,
    DFU_STATUS_ERROR_FIRMWARE = 0x0a,
    DFU_STATUS_ERROR_VENDOR = 0x0b,
    DFU_STATUS_ERROR_USBR = 0x0c,
    DFU_STATUS_ERROR_POR = 0x0d,
    DFU_STATUS_ERROR_UNKNOWN = 0x0e,
    DFU_STATUS_ERROR_STALLEDPKT = 0x0f
};

#pragma pack(push, 1)

/**
 * \brief The device responds to the DFU_GETSTATUS request with a payload packet containing the following data.
 */
struct DeviceDfuStatus {
    /**
     * \brief An indication of the status resulting from the execution of the most recent request.
     */
    enum DfuStatus status;
    /**
     * \brief Minimum time, in milliseconds, that the host should wait before sending
     * a subsequent DFU_GETSTATUS request,
     */
    uint32_t timeout : 24;
    /**
     * \brief An indication of the state that the device is going to enter
     * immediately following transmission of this response.
     */
    enum DfuState state;
    /**
     * \brief Index of status description in string table.
     */
    uint8_t str_idx;
};

#pragma pack(pop)
