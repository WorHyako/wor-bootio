// SPDX-License-Identifier: MIT
// Copyright (c) 2025 WorHyako

#include "configuration.h"

#include "bootio_error.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libusb.h>

#if __STDC_VERSION__ == 202311L
/**
 * \brief Value from descriptor that means Dfu descriptor type.
 */
static constexpr uint32_t DtType_Dfu = 0x21;
#else
/**
 * \brief Value from descriptor that means Dfu descriptor type.
 */
#define DtType_Dfu 0x21
#endif

/**
 * \brief Fills a DFU functional descriptor from a list of descriptors.
 *
 * This function scans through a list of descriptors to find a descriptor of type \c DtType_Dfu .
 * If a DFU functional descriptor is found, it copies the descriptor data into the provided
 * \c DfuFunctionalDescriptor structure.
 *
 * \param desc_list The list of descriptors to scan. Must not be \c null .
 * \param length The total length of the descriptor list.
 * \param func_dt A pointer to a \c DfuFunctionalDescriptor structure where the found descriptor will be stored.
 * \return Returns \c BootIoError_Success (0) on success if the DFU functional descriptor is found and copied.
 *         Possible output values on failure:
 *         - \c BootIoError_InvalidParam - \c desc_list param is empty.
 *         - \c BootIoError_Configuration_DescriptorLength - descriptor length is too short.
 *         - \c BootIoError_Other - can't find descriptor.
 */
wor_bootio_nodiscard__
static int fill_descriptor(const uint8_t *desc_list, const uint8_t length, struct DfuFunctionalDescriptor *func_dt) {
    if (length < 2) {
        return BootIoError_Configuration_DescriptorLength;
    }
    if (desc_list == wor_bootio_nullptr__) {
        return BootIoError_InvalidParam;
    }

    int desc_idx = 0;
    while (desc_idx + 1 < length) {
        const int current_desc_len = desc_list[desc_idx];
        if (current_desc_len == 0) {
            return BootIoError_Configuration_DescriptorLength;
        }
        if (desc_list[desc_idx + 1] == DtType_Dfu) {
            memcpy(func_dt, desc_list + desc_idx, sizeof(struct DfuFunctionalDescriptor));
            return BootIoError_Success;
        }
        desc_idx += (int)desc_list[desc_idx];
    }
    return BootIoError_Other;
}

void free_device_tree(struct ConfigurationNode *device_node_root) {
    if (device_node_root == wor_bootio_nullptr__) {
        return;
    }
    while (device_node_root != wor_bootio_nullptr__) {
        struct ConfigurationNode *next = device_node_root->next;
        free(device_node_root);
        device_node_root = next;
    }
}

void free_configuration(struct Configuration *config) {
    if (config == wor_bootio_nullptr__) {
        return;
    }
    free(config->alt_name);
    config->alt_name = wor_bootio_nullptr__;
    free(config->serial_name);
    config->serial_name = wor_bootio_nullptr__;
}

struct ConfigurationNode *find_configurations(libusb_device *dev) {
    if (dev == wor_bootio_nullptr__) {
        return wor_bootio_nullptr__;
    }

    struct libusb_device_descriptor desc;
    struct ConfigurationNode *conf_node_root = wor_bootio_nullptr__;
    struct ConfigurationNode *conf_node_head = wor_bootio_nullptr__;

    libusb_get_device_descriptor(dev, &desc);

    for (int cfg_idx = 0; cfg_idx < desc.bNumConfigurations; ++cfg_idx) {
        int ec;
        struct libusb_config_descriptor *cfg = wor_bootio_nullptr__;
        ec = libusb_get_config_descriptor(dev, cfg_idx, &cfg);
        if (ec != BootIoError_Success || cfg == wor_bootio_nullptr__) {
            continue;
        }

        struct DfuFunctionalDescriptor func_dt;
        const uint8_t *desc_list = cfg->extra;
        const int list_len = cfg->extra_length;

        ec = fill_descriptor(desc_list, list_len, &func_dt);
        if (ec == 0) {
            goto dfu_found;
        }
        for (int intf_idx = 0; intf_idx < cfg->bNumInterfaces; ++intf_idx) {
            const struct libusb_interface *intf = &cfg->interface[intf_idx];
            if (intf == wor_bootio_nullptr__) {
                break;
            }
            for (int alt_idx = 0; alt_idx < intf->num_altsetting; ++alt_idx) {
                const struct libusb_interface_descriptor *alt = &intf->altsetting[alt_idx];
                if (alt->bInterfaceClass != 0xfe || alt->bInterfaceSubClass != 0x01) {
                    continue;
                }
                ec = fill_descriptor(alt->extra, alt->extra_length, &func_dt);
                if (ec == 0) {
                    goto dfu_found;
                }
                libusb_device_handle *dev_handle = wor_bootio_nullptr__;
                if (libusb_open(dev, &dev_handle) != BootIoError_Success) {
                    continue;
                }
                ec = libusb_get_descriptor(dev_handle, DtType_Dfu, 0, (void *)&func_dt, sizeof(func_dt));

                libusb_close(dev_handle);
                if (ec > 0) {
                    goto dfu_found;
                }
            }
        }
        libusb_free_config_descriptor(cfg);
        continue;

dfu_found:
        if (func_dt.length == 7) {
            func_dt.version = libusb_cpu_to_le16(0x0100);
        } else if (func_dt.length < 9) {
            func_dt.version = libusb_cpu_to_le16(0x0100);
            func_dt.transfer_size = 0;
        }

        for (int intf_idx = 0; intf_idx < cfg->bNumInterfaces; ++intf_idx) {
            const struct libusb_interface *intf = &cfg->interface[intf_idx];
            if (intf == wor_bootio_nullptr__) {
                continue;
            }
            for (int alt_idx = 0; alt_idx < intf->num_altsetting; ++alt_idx) {
                unsigned char alt_name[126];
                unsigned char serial_name[126];
                const struct libusb_interface_descriptor *alt = &intf->altsetting[alt_idx];

                if (alt->bInterfaceClass != 0xfe || alt->bInterfaceSubClass != 1) {
                    continue;
                }
                if (alt->bInterfaceProtocol != 2) {
                    continue;
                }

                libusb_device_handle *dev_handle = wor_bootio_nullptr__;
                ec = libusb_open(dev, &dev_handle);
                if (ec != BootIoError_Success) {
                    ec = BootIoError_Device_Open;
                    break;
                }
                ec = libusb_get_string_descriptor_ascii(dev_handle,
                                                        alt->iInterface,
                                                        alt_name,
                                                        126);
                if (alt->iInterface == 0 || ec == 0) {
                    strcpy((char *)alt_name, "unknown");
                }
                ec = libusb_get_string_descriptor_ascii(dev_handle,
                                                        desc.iSerialNumber,
                                                        serial_name,
                                                        126);
                if (desc.iSerialNumber == 0 || ec == 0) {
                    strcpy((char *)serial_name, "unknown");
                }
                libusb_close(dev_handle);

                struct ConfigurationNode *new_node = malloc(sizeof(struct ConfigurationNode));
                if (new_node == wor_bootio_nullptr__) {
                    free_device_tree(conf_node_root);
                    libusb_free_config_descriptor(cfg);
                    return wor_bootio_nullptr__;
                }
                new_node->next = wor_bootio_nullptr__;

                if (conf_node_head == wor_bootio_nullptr__) {
                    conf_node_head = new_node;
                    conf_node_root = conf_node_head;
                } else {
                    conf_node_head->next = new_node;
                    conf_node_head = new_node;
                }

                struct Configuration *conf = &new_node->device;
                conf->serial_name = calloc(126, sizeof(char));
                strcpy(conf->serial_name, (char *)serial_name);
                conf->alt_name = calloc(126, sizeof(char));
                strcpy(conf->alt_name, (char *)alt_name);
                conf->device = libusb_ref_device(dev);
                conf->bcd_device = desc.bcdDevice;
                conf->vendor_id = desc.idVendor;
                conf->product_id = desc.idProduct;
                conf->interface_number = alt->bInterfaceNumber;
                conf->alt_setting = alt->bAlternateSetting;
                conf->dev_number = libusb_get_device_address(dev);
                conf->bus_number = libusb_get_bus_number(dev);
                conf->func_dt = func_dt;
                conf->max_packet_size = desc.bMaxPacketSize0;
            }
        }
        libusb_free_config_descriptor(cfg);
    }
    return conf_node_root;
}
