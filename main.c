#include "load.h"
#include "configuration.h"
#include "dfu_file.h"
#include "portable.h"
#include "transfer.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <libusb.h>

/**
 * \brief
 *
 * \param file
 * \param path
 * \return
 */
wor_bootio_nodiscard__
static int dfu_file_read(struct DfuFile *file, const char *path) {
    int ec;
    uint8_t *buffer = wor_bootio_nullptr__;
    FILE *local_file = fopen(path, "rb");

    if (local_file == wor_bootio_nullptr__) {
        ec = -1;
        goto out;
    }

    ec = fseek(local_file, 0, SEEK_END);
    file->size.total = ftell(local_file);
    ec = fseek(local_file, 0, SEEK_SET);
    if (ec != 0) {
        goto out;
    }

    if (file->size.total > SIZE_MAX) {
        ec = 2;
        goto out;
    }

    buffer = calloc(file->size.total, sizeof(uint8_t));
    const size_t read_bytes = fread(buffer, sizeof(uint8_t), file->size.total, local_file);
    if (read_bytes != file->size.total) {
        goto out;
    }
    ec = load_file(file, buffer, read_bytes);

out:
    if (buffer != wor_bootio_nullptr__) {
        free(buffer);
        buffer = wor_bootio_nullptr__;
    }
    if (local_file != wor_bootio_nullptr__) {
        ec = fclose(local_file);
        local_file = wor_bootio_nullptr__;
    }
    return ec;
}

/**
 * \brief
 *
 * \param file
 * \return
 */
wor_bootio_nodiscard__
static int dfu_file_write(struct DfuFile *file) {
    if (file == wor_bootio_nullptr__ || file->data == wor_bootio_nullptr__) {
        return -1;
    }
    int ec;
    wor_bootio_constexpr__ char mode[] = "wb";
    FILE *out = fopen(file->path, mode);
    if (out == wor_bootio_nullptr__) {
        ec = remove(file->path);
        if (ec != 0) {
            return -1;
        }
        out = fopen(file->path, mode);
        if (out == wor_bootio_nullptr__) {
            return -1;
        }
    }
    ec = (int)fwrite(file->data, sizeof(*file->data), file->size.total, out);
    return ec;
}

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
    libusb_context* ctx;
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

    dfu_file_set_path(&file, "test.bin");
    ec = load_file(&file, buf, bytes_count);
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
