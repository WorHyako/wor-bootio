#pragma once

#include "dfu_func_dt.h"
#include "portable.h"

#include <stdint.h>

/**
 * \struct Configuration
 * \brief Holds the configuration of a USB device.
 *
 * This structure encapsulates various attributes required to interact with a USB device, including bus number, device
 * identifiers, descriptors, interface numbers, and alternative settings. Additionally, it stores strings such
 * as alternative interface name and serial name, as well as handles to device and device descriptor.
 *
 * \warning Field \c device_handle may be \c null, so be careful to use it.
 * Usually it's used internally to store a device handle in downloading, uploading or parsing process.
 * After using it will usually be closed internally also.
 */
struct Configuration {
    /**
     * \brief Device bus number.
     */
    uint16_t bus_number;
    /**
     * \brief Device number on bus.
     */
    uint16_t dev_number;
    /**
     * \brief Vendor ID.
     */
    uint16_t vendor_id;
    /**
     * \brief Product ID.
     */
    uint16_t product_id;
    /**
     * \brief Devide version.
     */
    uint16_t bcd_device;
    /**
     * \brief USB configuration number.
     */
    uint8_t configuration;
    /**
     * \brief Interface number.
     */
    uint8_t interface_number;
    /**
     * \brief Function descriptor info.
     */
    struct DfuFunctionalDescriptor func_dt;
    /**
     * \brief Alternative setting number.
     */
    uint8_t alt_setting;
    /**
     * \brief Max EP0 packet size.
     */
    uint8_t max_packet_size;
    /**
     * \brief Alternative interface name.
     */
    char *alt_name;
    /**
     * \brief Serial name.
     */
    char *serial_name;
    /**
     * \brief Device descriptor handle.
     */
    struct libusb_device *device;
    /**
     * \brief Device handle.
     */
    struct libusb_device_handle *device_handle;
};

/**
 * \struct ConfigurationNode
 * \brief Represents a single node in the configuration-linked list.
 *
 * The \c ConfigurationNode structure is used to store information about a device's configuration
 * and provides a link to the next node in the list, forming a linked list of devices.
 */
struct ConfigurationNode {
    /**
     * \brief Current configuration info.
     */
    struct Configuration device;
    /**
     * \brief Pointer to the next configuration node.
     */
    struct ConfigurationNode *next;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Frees the memory allocated for a device tree.
 *
 * This function releases the memory associated with a device tree node and all its later nodes in a linked list.
 *
 * \param device_node_root A pointer to the root node of the device tree to be freed.
 *                         If this pointer is \c null, the function does nothing.
 */
void free_device_tree(struct ConfigurationNode *device_node_root);

/**
 * \brief Frees allocated resources in a \c Configuration structure.
 *
 * This function releases all dynamically allocated memory associated with
 * the provided \c Configuration structure, specifically \c alt_name and \c serial_name.
 *
 * \param config A pointer to the \c Configuration structure whose resources are to be freed.
 *               If this pointer is \c null, the function will immediately return.
 */
void free_configuration(struct Configuration *config);

/**
 * \brief Finds and retrieves the configurations available for a given USB device.
 *
 * This function examines the configurations of a specific USB device to build a linked list of configurations.
 * It processes configuration descriptors, interface descriptors, and DFU functional descriptors where applicable.
 *
 * \param dev A pointer to the \c libusb_device that represents the USB device
 *            whose configurations are to be retrieved.
 * \return A pointer to the root of a linked list of \c ConfigurationNode structures
 *         representing the configurations of the device.
 *         Returns \c null if the input device is \c null or if an error occurs.
 */
wor_bootio_nodiscard__
struct ConfigurationNode *find_configurations(struct libusb_device *dev);

#ifdef __cplusplus
}
#endif
