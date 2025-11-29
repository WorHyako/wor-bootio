#pragma once

#include "configuration.h"

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
 * \brief Performs a USB control transfer to retrieve the current status of the DFU device.
 *
 * \param config A pointer to the \c Configuration structure containing the device's configuration details.
 * \param buffer A pointer to the buffer where the retrieved status information will be stored.
 *               The buffer should have sufficient space to accommodate the status data.
 * \return The number of bytes successfully transferred.
 *         Possible output on failure:
 *         - \c BootIoError_Other - error on memory allocation.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int transfer_status(const struct Configuration *config, uint8_t *buffer);

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
