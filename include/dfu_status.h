#pragma once

#include "portable.h"
#include "configuration.h"

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

/**
 * \brief Waits for the device to enter the DFU Download Idle state.
 *
 * This function actively polls the device status to confirm if it has
 * transitioned to the "Download Idle" state. It handles delays, retries,
 * and intermediary error states during this process.
 *
 * \param config A pointer to the \c Configuration structure containing the device configuration details.
 * \return \c BootIoError_Success (0) if the device reaches the Dfu Download Idle state successfully,
 *         Possible output on failure:
 *         - \c BootIoError_Other - device falls in error.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int wait_for_download_idle(const struct Configuration *config);

/**
 * \brief Waits for the device to enter the DFU manifest state.
 *
 * This function polls the device's current DFU state until it transitions to the
 * manifest state or until the maximum number of retries is reached. If the device
 * is in an error state, it will attempt to clear the error status.
 *
 * \param config A pointer to the \c Configuration structure containing device configuration details.
 * \return \c BootIoError_Success (0) if successful and the device reaches the manifest state.
 *         Possible output on failure:
 *         - \c BootIoError_Other - device falls in error.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int wait_for_manifest(const struct Configuration *config);

/**
 * \brief Waits for the DFU state to transition to idle.
 *
 * This function repeatedly checks the device's DFU status until the state becomes idle
 * or until a maximum number of attempts is reached. It may also handle certain DFU states
 * such as \c Manifest or \c ManifestWaitReset based on the configuration.
 *
 * \param config A pointer to the \c Configuration structure holding the device's setup and
 * operational parameters.
 * \return \c BootIoError_Success (0) if the state transitions to idle successfully.
 *         Possible output on failure:
 *         - \c BootIoError_Other - device falls in error.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int wait_for_dfu_idle(const struct Configuration *config);

/**
 * \brief Aborts any ongoing DFU operation and waits for the device to enter an idle state.
 *
 * This function ensures that any currently running Device Firmware Update (DFU)
 * process is safely aborted, and then it waits until the device reaches an idle state.
 *
 * \param config A pointer to the \c Configuration structure containing device configuration details.
 * \return \c BootIoError_Success (0) if the operation completes successfully.
 *         Possible output on failure:
 *         - \c BootIoError_Other - device falls in error.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int abort_and_wait_idle(const struct Configuration *config);

/**
 * \brief Retrieves the current status of the device in DFU mode.
 *
 * \param config A pointer to the \c Configuration structure containing device-specific configuration details,
 *               such as the device handle and interface number.
 * \param status A pointer to the \c DeviceDfuStatus structure where the retrieved status information will
 *               be stored, including the device status, timeout, state, and string index.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_Transfer_ - transfer error.
 *         - \c BootIoError_InvalidParam - wrong parameters.
 */
wor_bootio_nodiscard__
int get_status(const struct Configuration *config, struct DeviceDfuStatus *status);

/**
 * \brief Sends a USB control transfer command to clear the status of the device in DFU mode.
 *
 * \param config A pointer to the \c Configuration structure containing device-specific configuration details.
 * \return Zero on success or a \c BootIoError_Transfer_ error code on failure.
 */
wor_bootio_nodiscard__
int dfu_clear_status(const struct Configuration *config);
