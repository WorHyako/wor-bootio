#pragma once

#include <stdint.h>

/**
 * \enum DfuState
 * \brief Represents all possibles states of Dfu/DfuSe protocol.
 */
enum DfuState
#if __STDC_VERSION__ == 202311L
    : uint8_t
#endif
{
    DfuState_AppIdle = 0x00,
    DfuState_AppDetach = 0x01,
    DfuState_Idle = 0x02,
    DfuState_DownloadSync = 0x03,
    DfuState_DownloadBusy = 0x04,
    DfuState_DownloadIdle = 0x05,
    DfuState_ManifestSync = 0x06,
    DfuState_Manifest = 0x07,
    DfuState_ManifestWaitReset = 0x08,
    DfuState_UploadIdle = 0x09,
    DfuState_Error = 0x0a
};

/**
 * \enum DfuStatus
 * \brief Represents all possibles statuses of Dfu/DfuSe protocol.
 */
enum DfuStatus
#if __STDC_VERSION__ == 202311L
    : uint8_t
#endif
{
    DfuStatus_Ok = 0x0,
    DfuStatus_ErrorTarget = 0x01,
    DfuStatus_ErrorFile = 0x02,
    DfuStatus_ErrorWrite = 0x03,
    DfuStatus_ErrorErase = 0x04,
    DfuStatus_ErrorCheckErase = 0x05,
    DfuStatus_ErrorProg = 0x06,
    DfuStatus_ErrorVerify = 0x07,
    DfuStatus_ErrorAddress = 0x08,
    DfuStatus_ErrorNotDone = 0x09,
    DfuStatus_ErrorFirmware = 0x0a,
    DfuStatus_ErrorVendor = 0x0b,
    DfuStatus_ErrorUsbR = 0x0c,
    DfuStatus_ErrorPor = 0x0d,
    DfuStatus_ErrorUnknown = 0x0e,
    DfuStatus_ErrorStalledPkt = 0x0f
};

#pragma pack(push, 1)

/**
 * \struct DeviceDfuStatus
 * \brief The device responds to the DFU_GETSTATUS request with a payload packet containing the following data.
 */
struct DeviceDfuStatus {
    /**
     * \brief An indication of the status resulting from the execution of the most recent request.
     */
    enum DfuStatus status;
    /**
     * \brief Minimum time, in milliseconds, that the host should wait before sending a later request,
     */
    uint32_t timeout;
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
