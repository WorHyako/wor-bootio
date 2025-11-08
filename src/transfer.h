#pragma once

#include "configuration.h"
#include "dfu_status.h"

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
[[nodiscard]]
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
[[nodiscard]]
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
[[nodiscard]]
int get_status(const struct Configuration *config, struct DeviceDfuStatus *status);

/**
 * \brief
 *
 * \param new_timeout
 */
void set_device_timeout(uint32_t new_timeout);
