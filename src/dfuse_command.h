#pragma once

#include "configuration.h"

/**
 * \brief Sends a DfuSe command to set the target address for device operations.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific details.
 * \param address The target address to be set.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_Transfer_Pipe - can't check status after command executing.
 *         - \c BootIoError_InvalidParam - possible empty config.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int dfuse_cmd_set_address(const struct Configuration *config, uint32_t address);

/**
 * \brief Sends a DfuSe command to erase a specific page at the given address.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific details.
 * \param address The memory address of the page to be erased.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_Transfer_Pipe - can't check status after command executing.
 *         - \c BootIoError_InvalidParam - possible empty config.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int dfuse_cmd_erase_page(const struct Configuration *config, uint32_t address);

/**
 * \brief Sends a DfuSe command to perform a mass erase operation on the target device.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific details.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_Transfer_Pipe - can't check status after command executing.
 *         - \c BootIoError_InvalidParam - possible empty config.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int dfuse_cmd_mass_erase(const struct Configuration *config);

/**
 * \brief Sends a DfuSe command to instruct the device to leave DFU mode and reboot.
 *
 * \param config Pointer to the \c Configuration structure containing device-specific details.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_Transfer_Pipe - can't check status after command executing.
 *         - \c BootIoError_InvalidParam - possible empty config.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
int dfuse_cmd_leave(const struct Configuration *config);
