#pragma once

#include "configuration.h"
#include "dfu_file.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Uploads firmware to a device using DFU protocol.
 *
 * Opens and closes the device.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific parameters.
 * \param buffer Pointer to the firmware data buffer to be uploaded.
 * \param expected_size The total size of the firmware data to be transferred, in bytes.
 * \param chunk_size The size of each data chunk to be transferred in a single operation, in bytes.
 *
 * \return The total number of bytes successfully uploaded on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - \c config or \c buffer is \c null.
 *         - \c BootIoError_Transfer_ - transfer error.
 *         - \c BootIoError_Other - device falls in error.
 */
wor_bootio_nodiscard__
int upload_dfu(struct Configuration *config,
               const uint8_t *buffer,
               size_t expected_size,
               size_t chunk_size);

/**
 * \brief Downloads firmware from a device using the DFU protocol.
 *
 * Transfers firmware data in chunks and waits for the device's state to become idle or manifesting.
 *
 * Opens and closes the device.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific parameters.
 * \param buffer Pointer to the buffer where firmware data will be downloaded.
 * \param expected_size Total size of the firmware data to be downloaded, in bytes.
 * \param chunk_size Maximum size of a single data chunk to be transferred in one operation, in bytes.
 *
 * \return The total number of bytes successfully downloaded on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - \c config or \c buffer is \c null.
 *         - \c BootIoError_Transfer_ - transfer error.
 *         - \c BootIoError_Other - device falls in error.
 */
wor_bootio_nodiscard__
int download_dfu(struct Configuration *config,
                 uint8_t *buffer,
                 size_t expected_size,
                 size_t chunk_size);

/**
 * \brief Uploads firmware to a specific memory address using DfuSe protocol.
 *
 * This method initializes communication with the device, sets the target memory address, and transfers the firmware
 * data in chunks. After the transfer, it finalizes the operation and ensures the device is properly closed.
 *
 * Opens and closes the device.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific parameters.
 * \param buffer Pointer to the firmware data buffer to be uploaded.
 * \param start_address The starting memory address on the device where the firmware data will be uploaded.
 * \param expected_size The total size of the firmware data to be transferred, in bytes.
 * \param chunk_size The size of each data chunk to be transferred in a single operation, in bytes.
 *
 * \return The total number of bytes successfully uploaded on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - \c config or \c buffer is \c null.
 *         - \c BootIoError_Other - error in finding the memory segment or device falls in error.
 *         - \c BootIoError_Transfer_ - transfer error.
 *         - \c BootIoError_Configuration_ - descriptor parsing error.
 */
wor_bootio_nodiscard__
int upload_dfuse(struct Configuration *config,
                 const uint8_t *buffer,
                 size_t start_address,
                 size_t expected_size,
                 uint16_t chunk_size);

/**
 * \brief Downloads firmware to a device using the DfuSe protocol.
 *
 * Handles the device communication, erases relevant memory pages, transfers firmware data in chunks,
 * and concludes the operation with a manifest phase.
 *
 * Opens and closes the device.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific parameters.
 * \param file Pointer to the \c DfuFile structure containing the firmware data and file offsets.
 * \param start_address The starting memory address on the device where the firmware will be written.
 * \param chunk_size The size of each firmware data chunk to be transferred, in bytes.
 *
 * \return The total number of bytes successfully downloaded on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - \c config or \c buffer is \c null.
 *         - \c BootIoError_Transfer_ - transfer error.
 *         - \c BootIoError_Configuration_ - descriptor parsing error.
 *         - \c BootIoError_Other - device falls in error.
 */
wor_bootio_nodiscard__
int download_dfuse(struct Configuration *config,
                   struct DfuFile *file,
                   size_t start_address,
                   uint16_t chunk_size);

#ifdef __cplusplus
}
#endif
