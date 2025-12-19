// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#include "dfuse_command.h"

#include "bootio_error.h"
#include "transfer.h"
#include "dfu_status.h"
#include "portable.h"

#include <assert.h>
#include <libusb.h>

/**
 * \enum DfuSeCommand
 * \brief Contains DfuSe special commands.
 */
enum DfuSeCommand
#if __STDC_VERSION__ == 202311L
        : uint8_t
#endif
    {
    /**
     * \brief Set address.
     */
    DfuSeCommand_SetAddress = 0x21,
    /**
     * \brief Erase page (use with address).
     */
    DfuSeCommand_ErasePage = 0x41,
    /**
     * \brief Mass flash erasing (use with no address).
     */
    DfuSeCommand_MassErase = 0x41,
    /**
     * \brief Leave dfu mode and return to application mode.
     */
    DfuSeCommand_Leave = 0x91
};

#if __STDC_VERSION__ == 202311L
/**
 * \brief Max count of trying to receive the status answer.
*/
constexpr uint8_t timeout_number = 2;
#else
/**
 * \brief Max count of trying to receive the status answer.
*/
#define timeout_number 2
#endif

/**
 * \brief Sends a command to the device using the provided configuration.
 *
 * This function sends a specific command to the device and manages the resulting status and state transitions.
 * It handles retries and ensures proper synchronization before returning the result.
 *
 * \param command Pointer to the command data to be sent to the device.
 * \param length Length of the command data in bytes.
 * \param config Pointer to the device \c Configuration structure.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_Transfer_Pipe - device falls in error.
 *         - \c BootIoError_Transfer_ - transfer error.
 */
wor_bootio_nodiscard__
static int send_command(const uint8_t *command, const uint8_t length, const struct Configuration *config) {
    assert(config != wor_bootio_nullptr__);
    int ec = transfer_out(config, command, length, 0);
    if (ec < 0) {
        return ec;
    }

    uint8_t timeout_count = 0;
    struct DeviceDfuStatus status;
    uint32_t timeout = 10;

    do {
        ec = get_status(config, &status);
        if (ec >= 0) {
            timeout = status.timeout;
        } else if (ec == BootIoError_Transfer_Pipe && timeout_count < timeout_number) {
            status.state = DfuState_DownloadBusy;
            timeout = 1;
        }

        if (status.state == DfuState_DownloadBusy) {
            wor_bootio_sleep_ms(timeout);
        }
    } while (status.state != DfuState_DownloadIdle && ++timeout_count < timeout_number);

    if (status.status != DfuStatus_Ok) {
        return BootIoError_Transfer_Pipe;
    }
    return ec;
}

int dfuse_cmd_set_address(const struct Configuration *config, const uint32_t address) {
    uint8_t command[5];
    command[0] = DfuSeCommand_SetAddress;
    command[1] = address & 0xff;
    command[2] = (address >> 8) & 0xff;
    command[3] = (address >> 16) & 0xff;
    command[4] = (address >> 24) & 0xff;
    return send_command(command, 5, config);
}

int dfuse_cmd_erase_page(const struct Configuration *config, const uint32_t address) {
    uint8_t command[5];
    command[0] = DfuSeCommand_ErasePage;
    command[1] = address & 0xff;
    command[2] = (address >> 8) & 0xff;
    command[3] = (address >> 16) & 0xff;
    command[4] = (address >> 24) & 0xff;
    return send_command(command, 5, config);
}

int dfuse_cmd_mass_erase(const struct Configuration *config) {
    wor_bootio_constexpr__ uint8_t command[] = {DfuSeCommand_MassErase};
    return send_command(command, 1, config);
}

int dfuse_cmd_leave(const struct Configuration *config) {
    wor_bootio_constexpr__ uint8_t command[] = {DfuSeCommand_Leave};
    return send_command(command, 1, config);
}
