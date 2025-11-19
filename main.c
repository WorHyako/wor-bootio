#include "load.h"
#include "configuration.h"
#include "dfu_file.h"
#include "dfuse_command.h"
#include "portable.h"
#include "transfer.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <libusb.h>

/**
 * \brief
 *
 * \param device_node_root
 */
static void print_devices(struct ConfigurationNode *device_node_root) {
    struct ConfigurationNode *device_node = device_node_root;
    while (device_node != wor_bootio_nullptr__) {
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
    struct ConfigurationNode *confs = wor_bootio_nullptr__;
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
        if (confs == wor_bootio_nullptr__) {
            continue;
        }
        if (confs->device.vendor_id == 0x0483 && confs->device.product_id == 0xdf11) {
            break;
        }
        print_devices(confs);
    }
    if (confs == wor_bootio_nullptr__) {
        libusb_exit(ctx);
        return -1;;
    }
    ec = libusb_open(confs->device.device, &confs->device.device_handle);
    if (ec != LIBUSB_SUCCESS) {
        printf("Error to open device");
        return 1;
    }

    uint8_t buf[150000];
    const int bytes_count = upload_dfuse(&confs->device, buf, 0x08001000, 150000, 1024);
    if (bytes_count < 0) {
        ec = bytes_count;
        return -1;
    }
    struct DfuFile file;

    dfu_file_set_name(&file, "test.bin");
    dfu_file_fill(&file, buf, bytes_count);
    ec = dfu_file_write(&file);
    if (ec < 0) {
        printf("Error to write file.\n");
    }

    ec = download_dfuse(&confs->device, &file, 0x8001000, 1024);

    /**
     * Reconnecting device
     */
    const int manifest_tolerant = confs->device.func_dt.attributes & DfuFuncDtAttributes_ManifestTolerant;
    if (manifest_tolerant) {
        ec = dfu_detach(&confs->device, 100);
        ec = libusb_reset_device(confs->device.device_handle);
    }

    dfu_file_free(&file);
    free_device_tree(confs);
    libusb_close(confs->device.device_handle);
    libusb_exit(ctx);
    return 0;
}
