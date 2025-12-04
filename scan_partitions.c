#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define SECTOR_SIZE 512

/* GPT Header structure (only the fields we need) */
struct gpt_header {
    char signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_crc32;
    uint32_t reserved;
    uint64_t current_lba;
    uint64_t backup_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    unsigned char disk_guid[16];
    uint64_t partition_entries_lba;
    uint32_t num_partition_entries;
    uint32_t sizeof_partition_entry;
    uint32_t partition_entries_crc32;
} __attribute__((packed));

/* Single GPT Partition Entry */
struct gpt_entry {
    unsigned char type_guid[16];
    unsigned char uniq_guid[16];
    uint64_t first_lba;
    uint64_t last_lba;
    uint64_t flags;
    uint16_t name[36];   // UTF-16LE name
} __attribute__((packed));

static void print_guid(const unsigned char guid[16]) {
    printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
           "%02x%02x%02x%02x%02x%02x",
        guid[3], guid[2], guid[1], guid[0],
        guid[5], guid[4],
        guid[7], guid[6],
        guid[8], guid[9],
        guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);
}

static void print_utf16le_name(uint16_t *name) {
        int i = 0;
    for (i = 0; i < 36; i++) {
        uint16_t c = name[i];
        if (c == 0) break;
        if (c < 128)
            printf("%c", (char)c);
        else
            printf("?");
    }
}

static void print_gpt_entries(struct gpt_header *hdr, int fd) {
    size_t entries_size = hdr->num_partition_entries * hdr->sizeof_partition_entry;
    struct gpt_entry *entries = malloc(entries_size);

    if (!entries) {
        perror("malloc");
        exit(1);
    }

    if (lseek(fd, hdr->partition_entries_lba * SECTOR_SIZE, SEEK_SET) < 0) {
        perror("lseek entries");
        exit(1);
    }
    if (read(fd, entries, entries_size) != entries_size) {
        perror("read entries");
        exit(1);
    }
        uint32_t i = 0;
    for (i = 0; i < hdr->num_partition_entries; i++) {
        struct gpt_entry *e = &entries[i];

        int empty = 1;
                int b = 0;
                for (b = 0; b < 16; b++) {
            if (e->type_guid[b] != 0) {
                empty = 0;
                break;
            }
        }
        if (empty)
            continue;

        printf("Partition %u:\n", i + 1);
        printf("  Type GUID: ");
        print_guid(e->type_guid);
        printf("\n");

        printf("  Unique GUID: ");
        print_guid(e->uniq_guid);
        printf("\n");

        printf("  First LBA: %llu\n",
               (unsigned long long)e->first_lba);
        printf("  Last LBA:  %llu\n",
               (unsigned long long)e->last_lba);
        printf("  Flags: 0x%llx\n",
               (unsigned long long)e->flags);

        printf("  Name: ");
        print_utf16le_name(e->name);
        printf("\n\n");
    }

    free(entries);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/sdX\n", argv[0]);
        return 1;
    }

    const char *disk = argv[1];
    int fd = open(disk, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* Read GPT header from LBA1 */
    unsigned char sector[SECTOR_SIZE];
    if (lseek(fd, SECTOR_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        return 1;
    }
    if (read(fd, sector, SECTOR_SIZE) != SECTOR_SIZE) {
        perror("read header");
        return 1;
    }

    struct gpt_header *hdr = (struct gpt_header *)sector;

    if (memcmp(hdr->signature, "EFI PART", 8) != 0) {
        fprintf(stderr, "Not a GPT disk (signature mismatch)\n");
        return 1;
    }

    printf("== GPT Header ==\n");
    printf("Revision: %u\n", hdr->revision);
    printf("Header size: %u\n", hdr->header_size);
    printf("Disk GUID: ");
    print_guid(hdr->disk_guid);
    printf("\n");
    printf("Partition entries LBA: %llu\n",
           (unsigned long long)hdr->partition_entries_lba);
    printf("Number of entries: %u\n", hdr->num_partition_entries);
    printf("Entry size: %u\n\n", hdr->sizeof_partition_entry);

    printf("== GPT Partition Entries ==\n");
    print_gpt_entries(hdr, fd);

    /* ============================================================
     * Read Backup GPT Header
     * ============================================================ */
    printf("\n== Backup GPT Header ==\n");

    uint64_t backup_hdr_lba = hdr->backup_lba;
    unsigned char backup_sector[SECTOR_SIZE];

    if (lseek(fd, backup_hdr_lba * SECTOR_SIZE, SEEK_SET) < 0) {
        perror("lseek backup header");
        return 1;
    }
    if (read(fd, backup_sector, SECTOR_SIZE) != SECTOR_SIZE) {
        perror("read backup header");
        return 1;
    }

    struct gpt_header *bhdr = (struct gpt_header *)backup_sector;

    if (memcmp(bhdr->signature, "EFI PART", 8) != 0) {
        fprintf(stderr, "Backup header signature invalid\n");
        return 1;
    }

    printf("Revision: %u\n", bhdr->revision);
    printf("Header size: %u\n", bhdr->header_size);
    printf("Disk GUID: ");
    print_guid(bhdr->disk_guid);
    printf("\n");
    printf("Partition entries LBA: %llu\n",
           (unsigned long long)bhdr->partition_entries_lba);
    printf("Number of entries: %u\n", bhdr->num_partition_entries);
    printf("Entry size: %u\n\n", bhdr->sizeof_partition_entry);

    printf("== Backup GPT Partition Entries ==\n");
    print_gpt_entries(bhdr, fd);

    close(fd);
    return 0;
}
