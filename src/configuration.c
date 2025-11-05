#include "configuration.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libusb.h>

/**
 * \brief
 */
enum {
    /**
     * \brief
     */
    DT_DFU = 0x21
};

/**
 * \brief
 *
 * \param desc_list
 * \param length
 * \param func_dt
 *
 * \return
 */
[[nodiscard]]
static int fill_descriptor(const uint8_t *desc_list, uint8_t length, struct DfuFunctionalDescriptor *func_dt) {
    int desc_idx = 0;

    if (length < 2) {
        return -1;
    }

    while (desc_idx + 1 < length) {
        const int current_desc_len = desc_list[desc_idx];
        if (current_desc_len == 0) {
            return -1;
        }
        if (desc_list[desc_idx + 1] == DT_DFU) {
            memcpy(func_dt, desc_list + desc_idx, sizeof(struct DfuFunctionalDescriptor));
            return 0;
        }
        desc_idx += (int)desc_list[desc_idx];
    }
    return -1;
}

void free_device_tree(struct ConfigurationNode *device_node_root) {
    if (device_node_root == nullptr) {
        return;
    }
    while (device_node_root != nullptr) {
        struct ConfigurationNode *next = device_node_root->next;
        free(device_node_root);
        device_node_root = next;
    }
}

struct ConfigurationNode *find_configurations(libusb_device *dev) {
    int ec;
    struct libusb_device_descriptor desc;
    struct ConfigurationNode *conf_node_root = nullptr;
    struct ConfigurationNode *conf_node_head = nullptr;

    libusb_get_device_descriptor(dev, &desc);

    for (int cfg_idx = 0; cfg_idx < desc.bNumConfigurations; ++cfg_idx) {
        struct libusb_config_descriptor *cfg = nullptr;
        ec = libusb_get_config_descriptor(dev, cfg_idx, &cfg);
        if (ec != LIBUSB_SUCCESS || cfg == nullptr) {
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
            if (intf == nullptr) {
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
                libusb_device_handle *dev_handle;
                if (libusb_open(dev, &dev_handle) != LIBUSB_SUCCESS) {
                    continue;
                }
                ec = libusb_get_descriptor(dev_handle, DT_DFU, 0, (void *)&func_dt, sizeof(func_dt));

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
            if (intf == nullptr) {
                continue;
            }
            for (int alt_idx = 0; alt_idx < intf->num_altsetting; ++alt_idx) {
                unsigned char alt_name[UINT_FAST8_MAX];
                unsigned char serial_name[UINT_FAST8_MAX];
                const struct libusb_interface_descriptor *alt = &intf->altsetting[alt_idx];

                if (alt->bInterfaceClass != 0xfe || alt->bInterfaceSubClass != 1) {
                    continue;
                }
                if (alt->bInterfaceProtocol != 2) {
                    continue;
                }

                libusb_device_handle *dev_handle;
                ec = libusb_open(dev, &dev_handle);
                if (ec != LIBUSB_SUCCESS) {
                    printf("Error to open device");
                    break;
                }
                ec = libusb_get_string_descriptor_ascii(dev_handle,
                                                        alt->iInterface,
                                                        alt_name,
                                                        127);
                if (alt->iInterface == 0 || ec == 0) {
                    strcpy((char *)alt_name, "unknown");
                } else {
                    alt_name[127] = '\0';
                }
                ec = libusb_get_string_descriptor_ascii(dev_handle,
                                                        desc.iSerialNumber,
                                                        serial_name,
                                                        127);
                if (desc.iSerialNumber == 0 || ec == 0) {
                    strcpy((char *)serial_name, "unknown");
                } else {
                    serial_name[127] = '\0';
                }
                libusb_close(dev_handle);

                // struct ConfigurationNode conf_node;
                struct ConfigurationNode *new_node = malloc(sizeof(struct ConfigurationNode));
                if (new_node == nullptr) {
                    libusb_free_config_descriptor(cfg);
                    return nullptr;
                }
                new_node->next = nullptr;

                if (conf_node_head == nullptr) {
                    conf_node_head = new_node;
                    conf_node_root = conf_node_head;
                } else {
                    conf_node_head->next = new_node;
                    conf_node_head = new_node;
                }

                struct Configuration *conf = &new_node->device;
                strcpy(conf->serial_name, (char *)serial_name);
                strcpy(conf->alt_name, (char *)alt_name);
                conf->device = libusb_ref_device(dev);
                conf->bcd_device = desc.bcdDevice;
                conf->vendor_id = desc.idVendor;
                conf->product_id = desc.idProduct;
                conf->interface = alt->bInterfaceNumber;
                conf->alt_setting = alt->bAlternateSetting;
                conf->dev_number = libusb_get_device_address(dev);
                conf->bus_number = libusb_get_bus_number(dev);
                conf->func_dt = func_dt;
                conf->handle = nullptr;
                conf->max_packet_size = desc.bMaxPacketSize0;
            }
        }
        libusb_free_config_descriptor(cfg);
    }
    return conf_node_root;
}
