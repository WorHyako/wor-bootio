#include "load.h"

#include "memory_segment.h"
#include "dfuse_command.h"
#include "transfer.h"
#include "portable.h"

#include <libusb.h>

/**
 * \brief
 *
 * \param config
 * \param start_address
 * \param size
 * \return
 */
wor_bootio_nodiscard__
static int erase_pages(const struct Configuration *config, size_t start_address, size_t size) {
    struct MemorySegmentNode *segment_list = wor_bootio_nullptr__;
    int ec = parse_memory_segments(config->alt_name, &segment_list);
    if (ec < 0) {
        return -1;
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

int upload_dfuse(const struct Configuration *config,
                 const uint8_t *buffer,
                 const size_t start_address,
                 const size_t expected_size,
                 uint16_t chunk_size) {
    if (config == wor_bootio_nullptr__ || config == wor_bootio_nullptr__ || buffer == wor_bootio_nullptr__) {
        return LIBUSB_ERROR_OTHER;
    }
    struct MemorySegmentNode *segment_list = wor_bootio_nullptr__;
    int ec = parse_memory_segments(config->alt_name, &segment_list);

    if (ec != 0) {
        return LIBUSB_ERROR_OTHER;
    }
    const struct MemorySegment *segment = find_segment(segment_list, start_address);
    if (segment == wor_bootio_nullptr__ || segment->type != MemorySegmentType_Readable) {
        free_memory_segment_list(segment_list);
        return LIBUSB_ERROR_OTHER;
    }
    free_memory_segment_list(segment_list);

    ec = dfuse_cmd_set_address(config, start_address);
    if (ec < 0) {
        return -1;
    }
    ec = dfu_abort(config);
    if (ec < 0) {
        return -1;
    }

    uint8_t *buf_head = (uint8_t *)buffer;
    size_t total_bytes = 0;

    for (uint16_t transfer_count = 2; total_bytes < expected_size; transfer_count++) {
        /**
         * TODO: maybe useless, coz `bytes < chunk_size` means end of uploading for USB device.
         */
        if (expected_size - total_bytes < chunk_size) {
            chunk_size = (int)(expected_size - total_bytes);
        }

        const int bytes = transfer_in(config, buf_head, chunk_size, transfer_count);
        if (bytes < 0) {
            ec = bytes;
            break;
        }
        if (bytes == 0) {
            ec = LIBUSB_SUCCESS;
            break;
        }
        total_bytes += bytes;
        buf_head += bytes;

        if (total_bytes >= expected_size || bytes < chunk_size) {
            ec = LIBUSB_SUCCESS;
            break;
        }
    }
    if (ec < 0) {
        return ec;
    }
    ec = wait_for_download_idle(config);;
    ec = dfu_abort(config);

    return ec < 0
               ? ec
               : (int)total_bytes;
}

int download_dfuse(const struct Configuration *config,
                   struct DfuFile *file,
                   const size_t start_address,
                   uint16_t chunk_size) {
    if (config == wor_bootio_nullptr__ || file->data == wor_bootio_nullptr__) {
        return -1;
    }

    size_t total_bytes = 0;
    int ec = 0;

    ec = erase_pages(config, start_address, file->size.total);
    if (ec < 0) {
        goto out;
    }

    uint8_t *buffer_head = file->data;
    ec = dfuse_cmd_set_address(config, start_address);
    if (ec < 0) {
        goto out;
    }
    for (uint32_t transfer_count = 2; total_bytes < file->size.total; ++transfer_count) {
        chunk_size = file->size.total - total_bytes > chunk_size
                         ? chunk_size
                         : file->size.total - total_bytes;
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
    return ec < 0
               ? ec
               : (int)total_bytes;
}
