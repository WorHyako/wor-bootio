#pragma once

#include "configuration.h"

#include <stddef.h>

/**
 * \brief
 *
 * \param config
 * \param buffer
 * \param expected_size
 * \param chunk_size
 *
 * \return
 */
[[nodiscard]]
int upload_dfu(const struct Configuration *config,
               const uint8_t *buffer,
               size_t expected_size,
               size_t chunk_size);

/**
 * \brief
 *
 * \param config
 * \param buffer
 * \param expected_size
 * \param chunk_size
 *
 * \return
 */
[[nodiscard]]
int download_dfu(const struct Configuration *config,
                 uint8_t *buffer,
                 size_t expected_size,
                 size_t chunk_size);

/**
 * \brief
 *
 * \param config
 * \param buffer
 * \param expected_size
 * \param chunk_size
 *
 * \return
 */
[[nodiscard]]
int upload_dfuse(const struct Configuration *config,
                 const uint8_t *buffer,
                 uint16_t expected_size,
                 uint16_t chunk_size);

/**
 * \brief
 *
 * \param config
 * \param buffer
 * \param expected_size
 * \param chunk_size
 *
 * \return
 */
[[nodiscard]]
int download_dfuse(const struct Configuration *config,
                   uint8_t *buffer,
                   uint16_t expected_size,
                   uint16_t chunk_size);
