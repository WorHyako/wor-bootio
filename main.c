#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <libusb.h>

#include "load.h"
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
    struct ConfigurationNode *confs = nullptr;
    int ec = libusb_init(&ctx);

    if (ec) {
        const char *error = libusb_error_name(ec);
        printf("Error initializing libusb: %s\n", error);
        exit(1);
    }

    ssize_t device_num = 0;
    libusb_device **device_list;
    device_num = libusb_get_device_list(ctx, &device_list);

    for (int i = 0; i < device_num; ++i) {
        libusb_device *dev = device_list[i];
        confs = find_configurations(dev);
        if (confs == nullptr) {
            continue;
        }
        if (confs->device.vendor_id == 0x0483 && confs->device.product_id == 0xdf11) {
            break;
        }
        print_devices(confs);
    }
    ec = libusb_open(confs->device.device, &confs->device.device_handle);
    if (ec != LIBUSB_SUCCESS) {
        printf("Error to open device");
        return 1;
    }

    const int fd = open("test.bin", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    uint8_t buf[1024];
    ec = upload_dfu(&confs->device, buf, 1024, 1024);
    free_device_tree(confs);
    libusb_close(confs->device.device_handle);
    libusb_exit(ctx);
    return 0;
}
