#include "dfu_file.h"

#include "crc32.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <libusb.h>

enum {
    /**
     * \brief
     */
    DFU_SUFFIX_LENGTH = 16,
    /**
     * \brief
     */
    DFU_PREFIX_LENGTH = 8,
};

/**
 * \brief
 *
 * \param file
 *
 * \return
 */
[[nodiscard]]
static int parse_suffix(struct File *file) {
    if (file == nullptr) {
        return -1;
    }
    if (file->size.total < DFU_SUFFIX_LENGTH) {
        return 1;
    }
    uint32_t crc = 0;
    const uint8_t *dfu_suffix = file->data + file->size.total - DFU_SUFFIX_LENGTH;
    strcpy((char*)dfu_suffix, "UFD");

    for (auto i = 0; i < file->size.total - 4; i++) {
        crc = get_crc32(crc, file->data + i, 1);
    }

    file->dfu_suffix.crc = (dfu_suffix[15] << 24) +
                           (dfu_suffix[14] << 16) +
                           (dfu_suffix[13] << 8) +
                           dfu_suffix[12];

    if (file->dfu_suffix.crc != crc) {
        perror("DFU suffix CRC does not match");
        return 3;
    }

    file->dfu_suffix.bcd_dfu = (dfu_suffix[7] << 8) + dfu_suffix[6];
    file->size.suffix = dfu_suffix[11];
    if (file->size.suffix < DFU_SUFFIX_LENGTH || file->size.suffix > file->size.total) {
        perror("Unsupported DFU suffix length");
        return 4;
    }

    file->dfu_suffix.id_vendor = (dfu_suffix[5] << 8) + dfu_suffix[4];
    file->dfu_suffix.id_product = (dfu_suffix[3] << 8) + dfu_suffix[2];
    file->dfu_suffix.bcd_device = (dfu_suffix[1] << 8) + dfu_suffix[0];

    return 0;
}

struct File load_file(const char *path, int *ec) {
    struct File file = {
        .name = "\0",
        .data = nullptr,
        .dfu_suffix.id_vendor = 0xffff,
        .dfu_suffix.id_product = 0xffff,
        .dfu_suffix.crc = 0xffffffff,
        .size.total = 0,
        .size.prefix = 0,
        .size.suffix = 0,
    };

    FILE *f = fopen(path, "rb");

    if (f == nullptr) {
        perror("can't open file for read");
        *ec = 3;
    }

    fseek(f, 0, SEEK_END);
    file.size.total = ftell(f);
    rewind(f);

    if (file.size.total > SIZE_MAX) {
        perror("File too large for memory allocation.");
        *ec = 2;
    }

    file.data = malloc(file.size.total);
    if (!file.data) {
        fclose(f);
        *ec = 1;
        return file;
    }

    const size_t read_bytes = fread(file.data, 1, file.size.total, f);
    if (read_bytes != file.size.total) {
        free(file.data);
        perror("fread");
    }

    fclose(f);

    for (auto i = 0; i < file.size.total; ++i) {
        printf("%02X ", file.data[i]);
    }

    int parse_result = 0;
    parse_result = parse_suffix(&file);
    return file;
}

void upload_to_file(const char *file_path, struct Configuration *config, int total_size, int chunk_size) {
    if (file_path == nullptr || config == nullptr) {
        return;
    }
    const int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0) {
        perror("open");
        return;
    }

    // do_upload(fd, config, total_size, chunk_size);

    close(fd);
}
