#pragma once

#include "configuration.h"

#include <stdint.h>

/**
 * \brief
 *
 * \param config
 * \param address
 * \return
 */
wor_bootio_nodiscard__
int dfuse_cmd_set_address(const struct Configuration *config, uint32_t address);

/**
 * \brief
 *
 * \param config
 * \return
 */
wor_bootio_nodiscard__
int dfuse_cmd_erase_page(const struct Configuration *config, uint32_t address);

/**
 * \brief
 *
 * \param config
 * \return
 */
wor_bootio_nodiscard__
int dfuse_cmd_mass_erase(const struct Configuration *config);

/**
 * \brief
 *
 * \param config
 * \return
 */
wor_bootio_nodiscard__
int dfuse_cmd_leave(const struct Configuration *config);
