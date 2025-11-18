#pragma once

#include <stdint.h>

#include "portable.h"

/**
 * \brief
 *
 * \param crc
 * \param buf
 * \param len
 *
 * \return
 */
wor_bootio_nodiscard__
uint32_t get_crc32(uint32_t crc, const uint8_t *buf, size_t len);
