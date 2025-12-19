// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#include "bootio_error.h"
#include "load.h"

#include "memory_segment.h"
#include "dfuse_command.h"
#include "transfer.h"
#include "dfu_status.h"
#include "portable.h"

#include <assert.h>
#include <libusb.h>

/**
 * \brief Erases memory pages within the specified address range.
 *
 * This function erases memory pages starting at the given address for the specified size.
 * It operates on memory segments that are writable or erasable. The operation stops if
 * the specified memory segment does not allow erasure or if any errors occur.
 *
 * \param config Pointer to the \c Configuration structure containing device information.
 * \param start_address The starting address of the memory region to be erased.
 * \param size The size of the memory region to be erased, in bytes.
 * \return Returns \c BootIoError_Success (0) on success page erasing.
 *         Possible output values on failure:
 *         - \c BootIoError_Configuration_DescriptorString - can't parse descriptor.
 *         - For other error codes see \c dfuse_cmd_erase_page(...).
 */
wor_bootio_nodiscard__
static int erase_pages(const struct Configuration *config, const size_t start_address, const size_t size) {
    assert(config != wor_bootio_nullptr__);
    struct MemorySegmentNode *segment_list = wor_bootio_nullptr__;
    int ec = parse_memory_segments(config->alt_name, &segment_list);
    if (ec < 0) {
        return BootIoError_Configuration_DescriptorString;
    }

    struct MemorySegmentNode *segment_node = find_segment_node(segment_list, start_address);
    for (const struct MemorySegmentNode *node = segment_node;
         node != wor_bootio_nullptr__ && node->memory_segment.distance.end < start_address + size;
         node = segment_node->next) {
        struct MemorySegment *segment_to_erase = &segment_node->memory_segment;
        if (segment_to_erase->type != MemorySegmentType_Writable
            && segment_to_erase->type != MemorySegmentType_Erasable) {
            break;
        }
        ec = dfuse_cmd_erase_page(config, segment_to_erase->distance.start);
        if (ec < 0) {
            break;
        }
    }
    free_memory_segment_list(segment_list);
    return ec;
}

int upload_dfuse(struct Configuration *config,
                 const uint8_t *buffer,
                 const size_t start_address,
                 const size_t expected_size,
                 const uint16_t chunk_size) {
    if (config == wor_bootio_nullptr__ || buffer == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
    int ec = libusb_open(config->device, &config->device_handle);
    if (ec < 0) {
        return ec;
    }

    struct MemorySegmentNode *segment_list = wor_bootio_nullptr__;
    ec = parse_memory_segments(config->alt_name, &segment_list);
    if (ec < 0) {
        goto out;
    }
    const struct MemorySegment *segment = find_segment(segment_list, start_address);
    if (segment == wor_bootio_nullptr__ || segment->type != MemorySegmentType_Readable) {
        free_memory_segment_list(segment_list);
        ec = BootIoError_Other;
        goto out;
    }
    free_memory_segment_list(segment_list);

    ec = dfuse_cmd_set_address(config, start_address);
    if (ec < 0) {
        goto out;
    }
    ec = dfu_abort(config);
    if (ec < 0) {
        goto out;
    }

    size_t total_bytes = 0;
    for (uint16_t transfer_count = 2; total_bytes < expected_size; transfer_count++) {
        uint8_t *buf_head = (uint8_t *)buffer + total_bytes;
        const int bytes = transfer_in(config, buf_head, chunk_size, transfer_count);
        if (bytes < 0) {
            ec = bytes;
            break;
        }
        if (bytes == 0) {
            ec = BootIoError_Success;
            break;
        }

        total_bytes += bytes;
        if (total_bytes >= expected_size || bytes < chunk_size) {
            ec = BootIoError_Success;
            break;
        }
    }
    if (ec < 0) {
        goto out;
    }
    ec = wait_for_download_idle(config);;
    ec = dfu_abort(config);
out:
    libusb_close(config->device_handle);
    config->device_handle = wor_bootio_nullptr__;

    return ec < 0
               ? ec
               : (int)total_bytes;
}

int download_dfuse(struct Configuration *config,
                   const struct DfuFile *file,
                   const size_t start_address,
                   uint16_t chunk_size) {
    if (config == wor_bootio_nullptr__ || file == wor_bootio_nullptr__ || file->data == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }

    int ec = libusb_open(config->device, &config->device_handle);
    if (ec < 0) {
        return ec;
    }
    const size_t firmware_size = file->size.total - file->size.suffix - file->size.prefix;
    size_t total_bytes = 0;
    ec = erase_pages(config, start_address, firmware_size);
    if (ec < 0) {
        goto out;
    }

    uint8_t *buffer_head = file->data + file->size.prefix;
    ec = dfuse_cmd_set_address(config, start_address);
    if (ec < 0) {
        goto out;
    }
    for (uint32_t transfer_count = 2; total_bytes < firmware_size; ++transfer_count) {
        chunk_size = firmware_size - total_bytes > chunk_size
                         ? chunk_size
                         : firmware_size - total_bytes;
        const int bytes_sent = transfer_out(config,
                                            buffer_head,
                                            chunk_size,
                                            transfer_count);

        if (bytes_sent < 0) {
            ec = bytes_sent;
            goto out;
        }

        buffer_head += bytes_sent;
        total_bytes += bytes_sent;

        ec = wait_for_download_idle(config);
        if (ec < 0) {
            goto out;
        }
    }

    ec = abort_and_wait_idle(config);
    if (ec < 0) {
        goto out;
    }

    wor_bootio_constexpr__ uint8_t terminating_byte = 0x00;
    ec = transfer_out(config, &terminating_byte, 0, 0);

    ec = wait_for_manifest(config);

out:
    libusb_close(config->device_handle);
    config->device_handle = wor_bootio_nullptr__;
    return ec < 0
               ? ec
               : (int)total_bytes;
}
