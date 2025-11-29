#pragma once

#include <stdint.h>

/**
 * \enum DfuFuncDtAttributes
 * \brief Contains flags of functional descriptor's attributes.
 */
enum DfuFuncDtAttributes
#if __STDC_VERSION__ == 202311L
        : uint8_t
#endif
    {
    /**
     * \brief
     */
    DfuFuncDtAttributes_ManifestTolerant = 0b100,
    /**
     * \brief
     */
    DfuFuncDtAttributes_Detach = 0b1000
};

/**
 * \struct DfuFunctionalDescriptor
 * \brief Represents functional descriptor of the usb device.
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
