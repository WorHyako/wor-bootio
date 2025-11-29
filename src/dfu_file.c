#include "dfu_file.h"

#include "crc32.h"
#include "bootio_error.h"
#include "portable.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#if __STDC_VERSION__ == 202311L
/**
 * \brief DFU suffix length.
 */
static constexpr uint8_t DfuSuffixSize = 16;
/**
 * \brief DFU prefix length.
 */
static constexpr uint8_t DfuPrefixSize = 8;
#else
/**
 * \brief DFU suffix length.
 */
#define DfuSuffixSize 16
/**
 * \brief DFU prefix length.
 */
#define DfuPrefixSize 8
#endif

/**
 * \brief Shortcut to assert \c DfuFile and dfu file's \c data
 *
 * \param file \c DfuFile pointer
 */
#define assert_file(file) \
assert(file != wor_bootio_nullptr__);\
assert(file->data != wor_bootio_nullptr__)

/**
 * \brief Fills the given \c DfuFile structure with the provided data and size.
 *
 * This function assigns the provided data pointer and size to the
 * \c data and \c total members of the \c DfuFile structure.
 *
 * \warning Param \c data will be set to \c null during filling.
 *
 * \param file Pointer to the \c DfuFile structure to be filled.
 * \param data Pointer to the data buffer to be assigned.
 * \param size Total size of the data buffer.
 */
static void dfu_file_fill(struct DfuFile *file, const uint8_t *data, const size_t size) {
    assert_file(file);
    if (size == 0) {
        return;
    }
    file->data = (uint8_t *)data;
    file->size.total = size;
    data = wor_bootio_nullptr__;
}

/**
 * \brief Parses the DFU suffix of a given DFU file and validates its components.
 *
 * Validates the DFU suffix length, signature, and CRC value. Updates the \c dfu_suffix structure in the
 * provided \c DfuFile object based on the parsed data.
 *
 * \param file Pointer to the \c DfuFile structure containing the DFU file data and associated metadata.
 *             Must not be \c null.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - total file size less than min DFU suffix length.
 *         - \c BootIoError_File - suffix parsing error.
 */
wor_bootio_nodiscard__
static int parse_dfu_suffix(struct DfuFile *file) {
    assert_file(file);
    int ec = BootIoError_Success;
    if (file->size.total < DfuSuffixSize) {
        ec = BootIoError_InvalidParam;
        return ec;
    }
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
    memcpy(file->dfu_suffix.ucd_dfu_signature, (uint8_t *)dfu_suffix + 8, 3 * sizeof(uint8_t));
    if (memcmp(file->dfu_suffix.ucd_dfu_signature, default_ucd_dfu_signature, 3) != 0) {
        ec = BootIoError_File_UcdSignature;
        return ec;
    }

    uint32_t crc = 0xffffffff;
    for (uint32_t i = 0; i < file->size.total - 4; i++) {
        crc = get_crc32(crc, file->data + i, 1);
    }
    memcpy(&file->dfu_suffix.crc, dfu_suffix + 12, sizeof(uint32_t));
    if (file->dfu_suffix.crc != crc) {
        ec = BootIoError_File_Crc;
        return ec;
    }

    file->size.suffix = dfu_suffix[11];
    if (file->size.suffix < DfuSuffixSize || file->size.suffix > file->size.total) {
        file->size.suffix = DfuSuffixSize;
        ec = BootIoError_File_SuffixLength;
    }

    memcpy(&file->dfu_suffix.bcd_device, dfu_suffix, sizeof(uint16_t));
    memcpy(&file->dfu_suffix.id_product, dfu_suffix + 2, sizeof(uint16_t));
    memcpy(&file->dfu_suffix.id_vendor, dfu_suffix + 4, sizeof(uint16_t));
    memcpy(&file->dfu_suffix.bcd_dfu, dfu_suffix + 6, sizeof(uint16_t));
    return ec;
}

/**
 * \brief Parses the DFU prefix of a given \c DfuFile.
 *
 * This function analyzes the data in the provided \c DfuFile DFU prefix and extracts relevant information,
 * such as the address field, after validating the file's content and ensuring its integrity.
 *
 * \param file Pointer to a \c DfuFile structure containing the DFU binary data.
 *             Must not be \c null, and the file's size and data should be valid.
 *
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - total file size less than min DFU prefix length.
 *         - \c BootIoError_File - prefix parsing error.
 */
wor_bootio_nodiscard__
static int parse_dfu_prefix(struct DfuFile *file) {
    assert_file(file);
    int ec = 0;
    if (file->size.total < DfuPrefixSize) {
        ec = BootIoError_InvalidParam;
        return ec;
    }
    file->dfu_prefix.address = 0x00;

    uint32_t firmware_size_from_prefix = 0;
    const uint32_t firmware_size = (uint32_t)(file->size.total - file->size.suffix) - DfuPrefixSize;
    if (file->data[0] == 0x01 && file->data[1] == 0x00) {
        memcpy(&firmware_size_from_prefix, file->data + 4, sizeof(uint32_t));
    }
    if (firmware_size == firmware_size_from_prefix) {
        uint16_t address;
        memcpy(&address, file->data + 2, sizeof(uint16_t));
        file->dfu_prefix.address = address * 1024;
        ec = 0;
    } else {
        ec = BootIoError_File_PrefixType;
    }
    return ec;
}

/**
 * \brief Parses and processes raw data to populate a \c DfuFile structure.
 *
 * This function fills the given \c DfuFile structure with the provided raw data,
 * then validates and extracts information from the DFU suffix and prefix.
 *
 * \param file A pointer to the \c DfuFile structure to populate. Must not be \c null.
 * \param data A pointer to the raw data buffer. Must not be \c null.
 * \param size The size of the raw data buffer. Must be non-zero.
 * \return Returns \c BootIoError_Success (0) on success.
 *         - For possible output on failure see \c parse_dfu_suffix(...) and \c parse_dfu_prefix(...).
 */
wor_bootio_nodiscard__
static int parse_raw_data(struct DfuFile *file, const uint8_t *data, const size_t size) {
    assert(data != wor_bootio_nullptr__);
    assert_file(file);
    dfu_file_fill(file, data, size);
    const int ec_suffix = parse_dfu_suffix(file) == 0;
    const int ec_prefix = parse_dfu_prefix(file) == 0;
    return ec_suffix == 0 ? ec_prefix : ec_suffix;
}

int load_file(struct DfuFile *file, uint8_t *data, const size_t size) {
    if (file == wor_bootio_nullptr__ || data == wor_bootio_nullptr__ || size == 0) {
        return BootIoError_InvalidParam;
    }
    if (file->data != wor_bootio_nullptr__) {
        file->data = wor_bootio_nullptr__;
        free((char *)file->data);
    }
    file->size.total = 0;
    file->size.suffix = 0;
    file->size.prefix = 0;

    int ec = parse_raw_data(file, data, size);
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
