// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#pragma once

#include <stdint.h>

#include "portable.h"

/**
 * \brief Calculates the CRC32 checksum for the given buffer.
 *
 * This function computes the CRC32 checksum for a given data buffer using
 * an initial CRC value. It iterates through the buffer and updates the
 * CRC value.
 *
 * \param crc The initial CRC value.
 * \param buf Pointer to the data buffer for which the CRC32 is to be calculated.
 * \param len The length of the data buffer.
 * \return The computed CRC32 checksum.
 */
wor_bootio_nodiscard__
uint32_t get_crc32(uint32_t crc, const uint8_t *buf, size_t len);
