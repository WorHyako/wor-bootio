#pragma once

#include <stdint.h>

/**
 * \brief
 *
 * See https://docs.kernel.org/usb/functionfs-desc.html.
 */
struct DfuFunctionalDescriptor {
    /**
     * \brief Size of the descriptor (bytes).
     */
    uint8_t length;
    /**
     * \brief USB_DT_DFU_FUNCTIONAL.
     */
    uint8_t type;
    /**
     * \brief DFU attributes.
     */
    uint8_t attributes;
    /**
     * \brief Maximum time to wait after DFU_DETACH (ms).
     */
    uint16_t detach_timeout;
    /**
     * \brief Maximum number of bytes per control-write.
     */
    uint16_t transfer_size;
    /**
     * \brief DFU Spec version (BCD).
     */
    uint16_t version;
};
