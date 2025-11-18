#pragma once

#include "configuration.h"
#include "dfu_status.h"

#include "portable.h"

/**
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

/**
 * \brief
 *
 * \param config
 * \param buf
 * \param chunk_size
 * \param transfer_count
 *
 * \return
 */
wor_bootio_nodiscard__
int transfer_in(const struct Configuration *config,
                uint8_t *buf,
                uint16_t chunk_size,
                uint16_t transfer_count);

/**
 * \brief
 *
 * \param config
 * \param buf
 * \param chunk_size
 * \param transfer_count
 * \return
 */
wor_bootio_nodiscard__
int transfer_out(const struct Configuration *config,
                 const uint8_t *buf,
                 uint16_t chunk_size,
                 uint16_t transfer_count);

/**
 * \brief
 *
 * \param config
 * \param status
 * \return
 */
wor_bootio_nodiscard__
int get_status(const struct Configuration *config, struct DeviceDfuStatus *status);

/**
 * \brief
 *
 * \param config
 * \return
 */
wor_bootio_nodiscard__
int dfu_abort(const struct Configuration *config);

/**
 * \brief
 *
 * \param config
 * \param detach_timeout
 * \return
 */
wor_bootio_nodiscard__
int dfu_detach(const struct Configuration *config, uint8_t detach_timeout);

/**
 * \brief
 *
 * \param new_timeout
 */
void set_device_timeout(uint32_t new_timeout);
