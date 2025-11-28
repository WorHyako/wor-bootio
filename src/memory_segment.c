#include "memory_segment.h"

#include "bootio_error.h"
#include "portable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_memory_segments(const char *descriptor, struct MemorySegmentNode **segment_list) {
    if (descriptor == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }
    struct MemorySegmentNode *segment_list_head = *segment_list;
    int scanf_idx;
    int ec;
    char *name = wor_bootio_nullptr__;
    uint32_t page_size;
    uint32_t address_start;
    uint32_t page_count;
    char segment_size_type;
    char segment_type;

    name = malloc(strlen(descriptor));

    ec = sscanf(descriptor, "@%[^/]%n", name, &scanf_idx);
    if (ec < 1) {
        free(name);
        return BootIoError_Configuration_DescriptorString;
    }

    descriptor += scanf_idx;

    while (ec = sscanf(descriptor, "/0x%x/%n", &address_start, &scanf_idx), ec > 0) {
        descriptor += scanf_idx;

        while (ec = sscanf(descriptor,
                           "%d*%d%c%c%n",
                           &page_count,
                           &page_size,
                           &segment_size_type,
                           &segment_type,
                           &scanf_idx), ec > 0) {
            descriptor += scanf_idx;
            switch (segment_size_type) {
            case 'B':
                break;
            case 'K':
                page_size *= 1024;
                break;
            case 'M':
                page_size *= 1024 * 1024;
                break;
            default:
                break;
            }

            struct MemorySegmentNode *segment_node = malloc(sizeof(struct MemorySegmentNode));
            struct MemorySegment *segment = &segment_node->memory_segment;
            segment->distance.start = address_start;
            segment->distance.end = address_start + page_count * page_size;
            segment->page_size = page_size;
            segment->type = segment_type & 0b111;
            segment_node->next = wor_bootio_nullptr__;

            if (segment_list_head == wor_bootio_nullptr__) {
                *segment_list = segment_node;
                segment_list_head = *segment_list;
            } else {
                segment_list_head->next = segment_node;
                segment_list_head = segment_node;
            }

            address_start += page_count * page_size;
            if (*descriptor == ',') {
                descriptor++;
            } else {
                break;
            }
        }
    }
    free(name);

    return BootIoError_Success;
}

void free_memory_segment_list(struct MemorySegmentNode *segment_list) {
    while (segment_list != wor_bootio_nullptr__) {
        struct MemorySegmentNode *next = segment_list->next;
        free(segment_list);
        segment_list = wor_bootio_nullptr__;
        segment_list = next;
    }
}

struct MemorySegment *find_segment(struct MemorySegmentNode *segment_list, const uint32_t address) {
    struct MemorySegmentNode* node = find_segment_node(segment_list, address);
    return node == wor_bootio_nullptr__ ? wor_bootio_nullptr__ : &node->memory_segment;
}

struct MemorySegmentNode *find_segment_node(struct MemorySegmentNode *segment_list, const uint32_t address) {
    if (segment_list == wor_bootio_nullptr__) {
        return wor_bootio_nullptr__;
    }
    while (segment_list->next != wor_bootio_nullptr__) {
        if (address >= segment_list->memory_segment.distance.start
            && address <= segment_list->memory_segment.distance.end) {
            return segment_list;
        }
        segment_list = segment_list->next;
    }
    return wor_bootio_nullptr__;
}
