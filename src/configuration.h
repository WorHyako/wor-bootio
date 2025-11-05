#pragma once

#include <stdint.h>

#include "dfu_func_dt.h"

/**
 * \brief
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
    uint8_t interface;
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
    char alt_name[127];
    /**
     * \brief Serial name.
     */
    char serial_name[127];
    /**
     * \brief Device descriptor handle.
     */
    struct libusb_device *device;
    /**
     * \brief Device handle.
     */
    struct libusb_device_handle *handle;
};

/**
 * \brief
 */
struct ConfigurationNode {
    /**
     * \brief Current device info.
     */
    struct Configuration device;
    /**
     * \brief Next node.
     */
    struct ConfigurationNode *next;
};

/**
 * \brief
 *
 * \param device_node_root
 */
void free_device_tree(struct ConfigurationNode *device_node_root);

/**
 * \brief
 *
 * \param dev
 *
 * \return
 */
[[nodiscard]]
struct ConfigurationNode *find_configurations(struct libusb_device *dev);
