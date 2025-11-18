#pragma once

#include "configuration.h"
#include "dfu_file.h"

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
wor_bootio_nodiscard__
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
wor_bootio_nodiscard__
int download_dfu(const struct Configuration *config,
                 uint8_t *buffer,
                 size_t expected_size,
                 size_t chunk_size);

/**
 * \brief
 *
 * \param config
 * \param buffer
 * \param start_address
 * \param expected_size
 * \param chunk_size
 *
 * \return
 */
wor_bootio_nodiscard__
int upload_dfuse(const struct Configuration *config,
                 const uint8_t *buffer,
                 size_t start_address,
                 size_t expected_size,
                 uint16_t chunk_size);

/**
 * \brief
 *
 * \param config
 * \param file
 * \param start_address
 * \param chunk_size
 *
 * \return
 */
wor_bootio_nodiscard__
int download_dfuse(const struct Configuration *config,
                   struct DfuFile *file,
                   size_t start_address,
                   uint16_t chunk_size);
