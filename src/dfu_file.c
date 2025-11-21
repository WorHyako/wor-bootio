#include "dfu_file.h"

#include "crc32.h"
#include "portable.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

/**
 * \brief
 */
enum
#if __STDC_VERSION__ == 202311L
    : uint8_t
#endif
{
    /**
     * \brief
     */
    DfuSuffixSize = 16,
    /**
     * \brief
     */
    DfuPrefixSize = 8
};

/**
 * \brief
 *
 * \param file
 */
void dfu_file_fill(struct DfuFile *file, const uint8_t *data, const size_t size) {
    if (file == wor_bootio_nullptr__ || data == wor_bootio_nullptr__ || size == 0) {
        return;
    }
    file->data = (uint8_t *)data;
    file->size.total = size;
    data = wor_bootio_nullptr__;
}

/**
 * \brief
 *
 * \param file
 *
 * \return
 */
wor_bootio_nodiscard__
static int parse_suffix(struct DfuFile *file) {
    if (file == wor_bootio_nullptr__ || file->size.total < DfuSuffixSize) {
        return -1;
    }
    int ec = 0;
    wor_bootio_constexpr__ uint8_t default_ucd_dfu_signature[3] = { 0x55, 0x46, 0x44 };
    file->dfu_suffix.bcd_device = 0;
    file->dfu_suffix.length = 0;
    file->dfu_suffix.ucd_dfu_signature[0] = 0;
    file->dfu_suffix.ucd_dfu_signature[1] = 0;
    file->dfu_suffix.ucd_dfu_signature[2] = 0;
    file->dfu_suffix.bcd_dfu = 0;
    file->dfu_suffix.id_vendor = 0;
    file->dfu_suffix.id_product = 0;

    const uint8_t *dfu_suffix = file->data + file->size.total - DfuSuffixSize;
    memcpy(file->dfu_suffix.ucd_dfu_signature, (uint8_t *)dfu_suffix + 10, 3 * sizeof(uint8_t));
    if (memcmp(file->dfu_suffix.ucd_dfu_signature, default_ucd_dfu_signature, 3) != 0) {
        ec = -1;
        return ec;
    }

    uint32_t crc = 0xffffffff;
    for (uint32_t i = 0; i < file->size.total - 4; i++) {
        crc = get_crc32(crc, file->data + i, 1);
    }
    memcpy(&file->dfu_suffix.crc, dfu_suffix + 12, sizeof(uint32_t));
    if (file->dfu_suffix.crc != crc) {
        ec = -1;
        return ec;
    }

    memcpy(&file->dfu_suffix.bcd_dfu, dfu_suffix + 6, sizeof(uint16_t));
    file->size.suffix = dfu_suffix[11];
    if (file->size.suffix < DfuSuffixSize || file->size.suffix > file->size.total) {
        file->size.suffix = DfuSuffixSize;
    }

    memcpy(&file->dfu_suffix.bcd_device, dfu_suffix, sizeof(uint16_t));
    memcpy(&file->dfu_suffix.id_product, dfu_suffix + 2, sizeof(uint16_t));
    memcpy(&file->dfu_suffix.id_vendor, dfu_suffix + 4, sizeof(uint16_t));
    return ec;
}

/**
 * \brief
 *
 * \param file
 * \return
 */
wor_bootio_nodiscard__
static int parse_prefix(struct DfuFile *file) {
    int ec = -1;
    file->dfu_prefix.address = 0x00;
    if (file == wor_bootio_nullptr__ || file->data == wor_bootio_nullptr__ || file->size.total < DfuPrefixSize) {
        return ec;
    }
    uint32_t firmware_size_from_prefix = 0;
    const uint32_t firmware_size = (uint32_t)file->size.total - file->size.suffix - DfuPrefixSize;
    if (file->data[0] == 0x01 && file->data[1] == 0x00) {
        memcpy(&firmware_size_from_prefix, file->data + 4, sizeof(uint32_t));
    }
    if (firmware_size == firmware_size_from_prefix) {
        uint16_t address;
        memcpy(&address, file->data + 2, sizeof(uint16_t));
        file->dfu_prefix.address = address * 1024;
        ec = 0;
    }
    return ec;
}

/**
 * \brief
 *
 * \param file
 * \param data
 * \param size
 * \return
 */
wor_bootio_nodiscard__
static int parse_raw_data(struct DfuFile *file, const uint8_t *data, const size_t size) {
    dfu_file_fill(file, data, size);
    const int ec_suffix = parse_suffix(file) == 0;
    const int ec_prefix = parse_prefix(file) == 0;
    return ec_suffix && ec_prefix;;
}

int load_file(struct DfuFile *file, uint8_t *data, size_t size) {
    if (file == wor_bootio_nullptr__ || data == wor_bootio_nullptr__ || size == 0) {
        return -1;
    }
    int ec;
    if (file->data != wor_bootio_nullptr__) {
        file->data = wor_bootio_nullptr__;
        free((char *)file->data);
    }
    file->size.total = 0;
    file->size.suffix = 0;
    file->size.prefix = 0;

    ec = parse_raw_data(file, data, size);
    return ec;
}

void dfu_file_free(struct DfuFile *file) {
    if (file == wor_bootio_nullptr__) {
        return;
    }
    if (file->data != wor_bootio_nullptr__) {
        free(file->data);
        file->data = wor_bootio_nullptr__;
    }
    if (file->path != wor_bootio_nullptr__) {
        free((char *)file->path);
        file->path = wor_bootio_nullptr__;
    }
}

void dfu_file_set_path(struct DfuFile *file, const char *path) {
    if (file == wor_bootio_nullptr__ || path == wor_bootio_nullptr__) {
        return;
    }
    if (file->path != wor_bootio_nullptr__) {
        free((char *)file->path);
        file->path = wor_bootio_nullptr__;
    }
    file->path = strdup(path);
}
