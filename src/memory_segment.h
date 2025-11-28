#pragma once

#include "portable.h"

#include <stdint.h>

/**
 * \enum MemorySegmentType
 * \brief Represents the type of memory segment with specific access permissions.
 */
enum MemorySegmentType
#if __STDC_VERSION__ == 202311L
        : uint8_t
#endif
    {
    /**
     * \brief Readable memory segment.
     */
    MemorySegmentType_Readable = 0b1,
    /**
     * \brief Erasable memory segment.
     */
    MemorySegmentType_Erasable = 0b10,
    /**
     * \brief Writable memory segment.
     */
    MemorySegmentType_Writable = 0b100,
};

#pragma pack(push, 1)

/**
 * \struct MemoryDistance
 * \brief Represents a memory segment position.
 */
struct MemoryDistance {
    /**
     * \brief Start address.
     */
    uint32_t start;
    /**
     * \brief End address.
     */
    uint32_t end;
};

/**
 * \struct MemorySegment
 * \brief Represents a memory segment description.
 */
struct MemorySegment {
    /**
     * \brief Memory segment position.
     */
    struct MemoryDistance distance;
    /**
     * \brief Page size for current memory segment.
     */
    uint32_t page_size;
    /**
     * \brief Current memory segment type.
     */
    enum MemorySegmentType type;
};

/**
 * \struct MemorySegmentNode
 * \brief Represents a single node in a linked list of memory segment descriptions.
 *
 * This structure is used to chain multiple memory segments description together
 * in a list. Each node contains a memory segment and a pointer to the next node in the list.
 *
 * See functions \c parse_memory_segments(...), \c free_memory_segment_list(...), \c find_segment(...),
 * \c find_segment_node(...).
 */
struct MemorySegmentNode {
    /**
     * \brief Current memory segment info.
     */
    struct MemorySegment memory_segment;
    /**
     * \brief Pointer to the next memory segment description node.
     */
    struct MemorySegmentNode *next;
};

#pragma pack(pop)

/**
 * \brief Parses device descriptors and builds a list of memory segment nodes.
 *
 * This function reads memory segment information from the provided descriptor string and constructs
 * a linked list of memory segment nodes. The structure of the descriptor string must follow a specific
 * format, f.e. "@Internal Flash /0x08000000/1*001Ka,2*001Kg".
 * The resulting linked list is returned through the provided pointer.
 *
 * Must call \c free_memory_segment_list after work with \c segment_list.
 *
 * \param descriptor A string containing the memory segment descriptors in a specific format.
 *                   Must not be a \c null pointer.
 * \param segment_list A pointer to a pointer where the head of the created memory segment node list
 *                     will be stored. Must not be a \c null pointer.
 * \return Returns \c BootIoError_Success (0) on success.
 *         On failure, it returns specific error codes.
 *         Possible return values:
 *         - \c BootIoError_Configuration_ - wrong or unknown descriptor.
 *         - \c BootIoError_InvalidParam - wrong parameters.
 */
wor_bootio_nodiscard__
int parse_memory_segments(const char *descriptor, struct MemorySegmentNode **segment_list);

/**
 * \brief Frees a linked list of memory segment nodes.
 *
 * This function releases the memory associated with a linked list of memory segment nodes, ensuring all nodes
 * in the list are properly freed and no memory leaks occur.
 *
 * \param segment_list Pointer to the head of the memory segment node list
 *                     to be freed. If the list is empty or already \c null,
 *                     no action is taken.
 */
void free_memory_segment_list(struct MemorySegmentNode *segment_list);

/**
 * \brief Searches for a memory segment in the linked list that contains the specified address.
 *
 * It's just a shortcut of:
 * \code
 * struct MemorySegmentNode* node = find_segment_node(...);
 * return &node->memory_segment;
 * \endcode
 *
 * \param segment_list A pointer to the head of the linked list of memory segments.
 * \param address The memory address to search for within the segment list.
 * \return A pointer to the memory segment containing the specified address,
 *         or a \c null pointer if no segment contains the address.
 */
wor_bootio_nodiscard__
struct MemorySegment* find_segment(struct MemorySegmentNode *segment_list, uint32_t address);

/**
 * \brief Finds the memory segment node in the list that corresponds to the given address.
 *
 * This function searches through a linked list of memory segment nodes to find
 * the node where the provided address falls within the segment's address range.
 *
 * \param segment_list Pointer to the head of the linked list of memory segment nodes.
 * \param address The address to search for in the memory segments.
 * \return Pointer to the memory segment node that contains the address, or \c null if no such node is found.
 */
wor_bootio_nodiscard__
struct MemorySegmentNode* find_segment_node(struct MemorySegmentNode *segment_list, uint32_t address);
