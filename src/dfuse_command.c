#include "dfuse_command.h"

#include "transfer.h"
#include "dfu_status.h"
#include "portable.h"

#include <libusb.h>

/**
 * \brief
 */
enum DfuSeCommand
#if __STDC_VERSION__ == 202311L
        : uint8_t
#endif
    {
    /**
     * \brief
     */
    DfuSeCommand_SetAddress = 0x21,
    /**
     * \brief
     */
    DfuSeCommand_ErasePage = 0x41,
    /**
     * \brief
     */
    DfuSeCommand_MassErase = 0x41,
    /**
     * \brief
     */
    DfuSeCommand_Leave = 0x91
};

/**
 * \brief
*/
#if __STDC_VERSION__ == 202311L
constexpr uint8_t timeout_number = 2;
#else
#define timeout_number 2
#endif

/**
 * \brief
 *
 * \param command
 * \param length
 * \param config
 * \return
 */
wor_bootio_nodiscard__
static int send_command(const uint8_t *command, const uint8_t length, const struct Configuration *config) {
    if (config == wor_bootio_nullptr__) {
        return -1;
    }

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
        } else if (ec == LIBUSB_ERROR_PIPE && timeout_count < timeout_number) {
            status.state = DfuState_DownloadBusy;
            timeout = 1;
        }

        if (status.state == DfuState_DownloadBusy) {
            wor_bootio_sleep_ms(timeout);
        }
    } while (status.state != DfuState_DownloadIdle && ++timeout_count < timeout_number);

    if (status.status != DfuStatus_Ok) {
        return LIBUSB_ERROR_PIPE;
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
