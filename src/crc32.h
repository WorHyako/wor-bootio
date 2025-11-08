#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * \brief
 *
 * \param crc
 * \param buf
 * \param len
 *
 * \return
 */
[[nodiscard]]
uint32_t get_crc32(uint32_t crc, const uint8_t *buf, size_t len);
