#include "load.h"

#include "memory_segment.h"
#include "dfuse_command.h"
#include "transfer.h"

#include <libusb.h>

int upload_dfuse(const struct Configuration *config,
                 const uint8_t *buffer,
                 const size_t start_address,
                 const uint16_t expected_size,
                 uint16_t chunk_size) {
    if (config == nullptr) {
        return LIBUSB_ERROR_OTHER;
    }
    struct MemorySegmentNode *segment_list = nullptr;
    int ec = parse_memory_segments(config->alt_name, &segment_list);

    if (ec != 0) {
        return LIBUSB_ERROR_OTHER;
    }
    const struct MemorySegment *segment = find_segment(segment_list, start_address);
    if (segment == nullptr || segment->type != MemorySegmentType_Readable) {
        return LIBUSB_ERROR_OTHER;
    }
    uint8_t transfer_count = 0;
    ec = dfuse_set_address(config, start_address);
    if (ec < 0) {
        return ec;
    }
    ++transfer_count;
    ec = dfu_abort(config);
    if (ec < 0) {
        return ec;
    }
    ++transfer_count;

    ec = upload_dfu(config, buffer, expected_size, chunk_size, transfer_count);
    ++transfer_count;
    if (ec < 0) {
        return ec;
    }
    free_memory_segment_list(segment_list);

    ec = dfu_abort(config);
    if (ec < 0) {
        return ec;
    }
    return 0;
}

int download_dfuse(const struct Configuration *config,
                   uint8_t *buffer,
                   const uint16_t expected_size,
                   uint16_t chunk_size) {

}
