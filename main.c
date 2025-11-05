#include <stdlib.h>
#include <stdio.h>

#include <libusb.h>

#include "configuration.h"
#include "dfu_file.h"

/**
 * \brief
 *
 * \param device_node_root
 */
static void print_devices(struct ConfigurationNode *device_node_root) {
    struct ConfigurationNode *device_node = device_node_root;
    while (device_node != nullptr) {
        printf("\nDevice info:"
               "\n\tName: %s"
               "\n\tSerial: %s"
               "\n\tVid/Pid: %04x:%04x"
               "\n\tDevice number: %u"
               "\n\tAlt: %u",
               device_node->device.alt_name,
               device_node->device.serial_name,
               device_node->device.vendor_id,
               device_node->device.product_id,
               device_node->device.dev_number,
               device_node->device.alt_setting);
        device_node = device_node->next;
    }
}

int main() {
    libusb_context *ctx;
    const int init_res = libusb_init(&ctx);

    if (init_res) {
        const char *error = libusb_error_name(init_res);
        printf("Error initializing libusb: %s\n", error);
        exit(1);
    }

    ssize_t device_num = 0;
    libusb_device **device_list;
    device_num = libusb_get_device_list(ctx, &device_list);

    for (int i = 0; i < device_num; ++i) {
        libusb_device *dev = device_list[i];
        struct ConfigurationNode *confs = find_configurations(dev);
        if (confs == nullptr) {
            continue;
        }
        print_devices(confs);
        free_device_tree(confs);
    }
    libusb_free_device_list(device_list, 1);
    // int ec;
    // auto file = load_file("", &ec);
    libusb_exit(ctx);
    return 0;
}
