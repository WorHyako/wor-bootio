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
[[nodiscard]]
int dfuse_set_address(const struct Configuration *config, uint32_t address);
