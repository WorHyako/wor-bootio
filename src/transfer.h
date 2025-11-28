#pragma once

#include "configuration.h"
#include "dfu_status.h"

#include "portable.h"

/**
 * \brief Performs a USB control transfer to receive data from the device.
 *
 * \param config A pointer to the \c Configuration structure containing device configuration details.
 * \param buf A pointer to the buffer where the received data will be stored.
 * \param chunk_size The maximum number of bytes to be transferred in one operation.
 * \param transfer_count The block or transfer index for the upload operation.
 * \return The number of bytes successfully transferred, or a \c BootIoError_Transfer_ error code on failure.
 */
wor_bootio_nodiscard__
int transfer_in(const struct Configuration *config,
                uint8_t *buf,
                uint16_t chunk_size,
                uint16_t transfer_count);

/**
 * \brief Performs a USB control transfer to send data from the host to the device.
 *
 * \param config A pointer to the \c Configuration structure containing the device's configuration and connection
 *               details.
 * \param buf A pointer to the buffer containing the data to be sent to the device.
 * \param chunk_size The number of bytes to be transferred in this operation.
 * \param transfer_count The transfer block index for this operation.
 * \return The number of bytes successfully transferred, or a \c BootIoError_Transfer_ error code on failure.
 */
wor_bootio_nodiscard__
int transfer_out(const struct Configuration *config,
                 const uint8_t *buf,
                 uint16_t chunk_size,
                 uint16_t transfer_count);

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

/**
 * \brief Sends a USB DFU abort command to cancel the current operation.
 *
 * \param config A pointer to the \c Configuration structure containing the device handle, interface number,
 *               and relevant configuration details.
 * \return Zero on success or a \c BootIoError_Transfer_ error code on failure.
 */
wor_bootio_nodiscard__
int dfu_abort(const struct Configuration *config);

/**
 * \brief Sends a DFU detach command to the device to initiate re-enumeration or leave DFU mode.
 *
 * \param config A pointer to the \c Configuration structure containing device and connection details.
 * \param detach_timeout The time, in milliseconds, the device will wait before detaching.
 * \return Zero on a successful transfer, or a \c BootIoError_Transfer_ error code on failure.
 */
wor_bootio_nodiscard__
int dfu_detach(const struct Configuration *config, uint8_t detach_timeout);

/**
 * \brief Sets a new timeout value for the device transfers.
 *
 * \param new_timeout The new timeout value, in milliseconds, to be used for device operations.
 */
void set_device_timeout(uint32_t new_timeout);

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
