/*
 * bminstall.c
 */

#include "mbr.h"
#include "boot.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <sys/stat.h>


#define PROGRAM_NAME "bminstall"

void usage()
{
	fprintf(stderr, "Usage: %s [option] [install_device]\n", PROGRAM_NAME);
	fprintf(stderr, "  -p, --payload    payload to use\n");
	fprintf(stderr, "  -h, --help       give this help list\n");
}

static inline int safe_open(const char *path, int oflag)
{
	int fd = open(path, oflag);
	if (fd == -1) {
		fprintf(stderr, "open: %s: %s\n", path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return fd;
}

static inline ssize_t safe_read(int fd, void *buf, size_t nbyte)
{
	ssize_t n = read(fd, buf, nbyte);
	if (n == -1) {
		fprintf(stderr, "read: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return n;
}

static inline ssize_t safe_write(int fd, void *buf, size_t nbyte)
{
	ssize_t n = write(fd, buf, nbyte);
	if (n == -1) {
		fprintf(stderr, "write: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return n;
}

static inline void safe_lseek(int fd, off_t offset, int whence)
{
	if (lseek(fd, offset, whence) == -1) {
		fprintf(stderr, "lseek: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static inline off_t safe_getsize(int fd)
{
	struct stat st;
	if (fstat(fd, &st) == -1) {
		fprintf(stderr, "fstat: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return st.st_size;
}

void assert_not_dir(const char *filepath)
{
	struct stat st;

	if (stat(filepath, &st) == -1) {
		fprintf(stderr, "stat: `%s': %s\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((st.st_mode & (S_IFCHR | S_IFREG)) == 0) {
		fprintf(stderr, "install: %s: invalid input\n", filepath);
		exit(EXIT_FAILURE);
	}
}

void assert_rw_access(const char *filepath)
{
	if (access(filepath, R_OK | W_OK) == -1) {
		fprintf(stderr, "install: %s: %s\n", filepath, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void install_legacy_boot(const char *device, const char *payload)
{
	int fdin1, fdin2, fdout;
	ssize_t blksize;
	unsigned char blkchar[512];
	off_t part_size, payload_size;
	struct load_info info;
	struct partentry part;
	char *firstsec = "boot/boot.bin";

	/* basic checks */
	assert_not_dir(firstsec);
	assert_rw_access(firstsec);

	/* open */
	fdin1 = safe_open(firstsec, O_RDONLY);
	fdin2 = safe_open(payload, O_RDONLY);
	fdout = safe_open(device, O_RDWR);

	/* get sizes */
	part_size = safe_getsize(fdout) - 512;
	payload_size = safe_getsize(fdin2);

	/* write mbr */
	blksize = safe_read(fdin1, blkchar, sizeof(blkchar));
	while (blksize) {
		safe_write(fdout, blkchar, blksize);
		blksize = safe_read(fdin1, blkchar, sizeof(blkchar));
	}

	/* write payload, from second sector */
	safe_lseek(fdout, 512, SEEK_SET);
	blksize = safe_read(fdin2, blkchar, sizeof(blkchar));
	while (blksize) {
		safe_write(fdout, blkchar, blksize);
		blksize = safe_read(fdin2, blkchar, sizeof(blkchar));
	}

	/* set payload info */
	info.index = 1;
	info.kbalign = 4;
	info.kbsize = payload_size / 1024;
	if (payload_size % 1024) {
		info.kbsize += 1;
	}

	safe_lseek(fdout, MBR_LOAD_INFO_OFFSET, SEEK_SET);
	safe_write(fdout, &info, sizeof(info));

	/* set one partition entry (max 256MiB) */
	if (part_size > 0x10000000) {
		part_size = 0x10000000;
	}

	part.status = 0;
	part.type = 0x83;
	part.start_index = 1;
	part.numof_sectors = part_size / 512;
	lba2chs(part.start_index, &part.start_head);
	lba2chs(part.start_index + part.numof_sectors - 1, &part.end_head);
	safe_lseek(fdout, MBR_PART_TABLE_OFFSET, SEEK_SET);
	safe_write(fdout, &part, sizeof(part));

	close(fdin1);
	close(fdin2);
	close(fdout);
}

void install(const char *payload, const char *device)
{
	/* sanity check */
	if (!payload || !device) {
		fprintf(stderr, "bug: install: payload=%p, device=%p\n",
			(void *) payload, (void *) device);
		exit(EXIT_FAILURE);
	}

	/* basic checks */
	assert_not_dir(payload);
	assert_not_dir(device);
	assert_rw_access(payload);
	assert_rw_access(device);

	/* install */
	install_legacy_boot(device, payload);
}

int main(int argc, char *argv[])
{
	int ch = 0;
	char *device = NULL, *payload = NULL;

	enum {
		OPTION_HELP = CHAR_MAX + 1,
		OPTION_PAYLOAD,
	};

	struct option longopts[] = {
		{ "help"    ,  no_argument       , NULL, 'h' },
		{ "payload" ,  required_argument , NULL, 'p' },
		{ NULL      ,  0                 , NULL,  0  }
	};

	/* parse args */
	while ((ch = getopt_long(argc, argv, "hp:", longopts, NULL)) != -1) {
		switch (ch) {
			case 'h':
				usage();
				exit(EXIT_FAILURE);

			case 'p':
				payload = optarg;
				break;

			default:
				fprintf(stderr, "Try '%s --help for more information.\n",
					PROGRAM_NAME);
				exit(EXIT_FAILURE);
		}
	}

	argc -= optind;
	argv += optind;

	if (!argc) {
		fprintf(stderr, "install device isn't specified.\n");
		exit(EXIT_FAILURE);
	}

	if (!payload) {
		fprintf(stderr, "payload isn't specified.\n");
		exit(EXIT_FAILURE);
	}

	device = *argv;
	install(payload, device);
	return EXIT_SUCCESS;
}
