#pragma once

#include "configuration.h"

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

/**
 * \struct DfuFile
 * \brief Represents a DFU file structure.
 */
struct DfuFile {
    /**
     * \brief File path if there is non-virtual location.
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
         * \brief DFU prefix length.
         */
        size_t prefix;
        /**
         * \brief DFU suffix length.
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

    /**
     * \brief
     */
    struct {
        /**
         * \brief
         */
        size_t address;
    } dfu_prefix;
};

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Loads firmware data into a DFU file structure from the raw buffer.
 *
 * This function processes the provided firmware data and populates the
 * specified \c DfuFile structure with the parsed information.
 *
 * \param file Pointer to the \c DfuFile structure that will be populated.
 * \param data Pointer to the firmware data to be loaded.
 * \param size Size of the firmware data in bytes.
 * \return Returns \c BootIoError_Success (0) on success.
 *         Possible output on failure:
 *         - \c BootIoError_InvalidParam - wrong parameters.
 *         - \c BootIoError_File_ - file parsing errors.
 *
 * \warning Param \c data will be set to \c null during loading.
 * \warning Must call \c dfu_file_free after work with \c DfuFile.
 */
wor_bootio_nodiscard__
int load_file(struct DfuFile *file, uint8_t *data, size_t size);

/**
 * \brief Frees resources associated with a \c DfuFile object.
 *
 * This function releases memory allocated for the fields of the \c DfuFile structure and resets pointers within it
 * to \c null. It ensures proper cleanup to avoid memory leaks.
 *
 * \param file Pointer to the \c DfuFile object to be freed.
 *             If the pointer is \c null, the function does nothing.
 */
void dfu_file_free(struct DfuFile *file);

/**
 * \brief Sets the file path for the given DFU file.
 *
 * This function updates the path property of the specified \c DfuFile structure with the provided path.
 * If a previous path is already set, it is deallocated before setting the new path.
 *
 * \param file Pointer to the \c DfuFile structure to update.
 * \param path A string containing the new file path.
 */
void dfu_file_set_path(struct DfuFile *file, const char *path);

#ifdef __cplusplus
}
#endif
