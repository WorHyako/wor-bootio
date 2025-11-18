#pragma once

#include "portable.h"

#include <stdint.h>

/**
 * \brief
 */
enum MemorySegmentType
#if __STDC_VERSION__ == 202311L
        : uint8_t
#endif
    {
    /**
     * \brief
     */
    MemorySegmentType_Readable = 0b1,
    /**
     * \brief
     */
    MemorySegmentType_Erasable = 0b10,
    /**
     * \brief
     */
    MemorySegmentType_Writable = 0b100,
};

#pragma pack(push, 1)

/**
 * \brief
 */
struct MemoryDistance {
    /**
     * \brief
     */
    uint32_t start;
    /**
     * \brief
     */
    uint32_t end;
};

/**
 * \brief
 */
struct MemorySegment {
    /**
     * \brief
     */
    struct MemoryDistance distance;
    /**
     * \brief
     */
    uint32_t page_size;
    /**
     * \brief
     */
    enum MemorySegmentType type;
};

/**
 * \brief
 */
struct MemorySegmentNode {
    /**
     * \brief
     */
    struct MemorySegment memory_segment;
    /**
     * \brief
     */
    struct MemorySegmentNode *next;
};

#pragma pack(pop)

/**
 * \brief
 *
 * \param descriptor
 * \param segment_list
 *
 * \return
 */
wor_bootio_nodiscard__
int parse_memory_segments(const char *descriptor, struct MemorySegmentNode **segment_list);

/**
 * \brief
 *
 * \param segment_list
 */
void free_memory_segment_list(struct MemorySegmentNode *segment_list);

/**
 * \brief
 *
 * \param segment_list
 * \param address
 * \return
 */
wor_bootio_nodiscard__
struct MemorySegment* find_segment(struct MemorySegmentNode *segment_list, uint32_t address);

/**
 * \brief
 *
 * \param segment_list
 * \param address
 * \return
 */
wor_bootio_nodiscard__
struct MemorySegmentNode* find_segment_node(struct MemorySegmentNode *segment_list, uint32_t address);
