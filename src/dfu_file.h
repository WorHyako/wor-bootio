#pragma once

#include <stddef.h>
#include <stdint.h>

#include "configuration.h"

#pragma pack(push, 1)

/**
 * \brief
 */
struct DfuFile {
    /**
     * \brief Path
     */
    const char *path;

    /**
     * \brief Firmware data.
     */
    uint8_t *data;

    /**
     * \brief File offsets.
     */
    struct {
        /**
         * \brief Total size.
         */
        size_t total;
        /**
         *
         */
        size_t prefix;
        /**
         * \brief DFU suffix offset.
         */
        size_t suffix;
    } size;

    /**
     * \brief DFU suffix struct.
     */
    struct {
        /**
         * \brief Vendor ID.
         */
        uint16_t id_vendor;
        /**
         * \brief Product ID.
         */
        uint16_t id_product;
        /**
         * \brief Device version.
         */
        uint16_t bcd_device;
        /**
         * \brief DFU version.
         */
        uint16_t bcd_dfu;
        /**
         * \brief DFU signature. Must be "UFD" (0x55 0x46 0x44).
         */
        uint8_t ucd_dfu_signature[3];
        /**
         * \brief DFU suffix length.
         */
        uint8_t length;
        /**
         * \brief Word CRC32.
         */
        uint32_t crc;
    } dfu_suffix;

    struct {
        size_t address;
    } dfu_prefix;
};

#pragma pack(pop)

/**
 * \brief
 *
 * \param file
 * \param data
 * \param size
 *
 * \return
 */
wor_bootio_nodiscard__
int load_file(struct DfuFile *file, uint8_t *data, size_t size);

/**
 * \brief
 *
 * \param file
 */
void dfu_file_free(struct DfuFile *file);

/**
 * \param
 *
 * \param file
 * \param path
 */
void dfu_file_set_path(struct DfuFile *file, const char *path);
