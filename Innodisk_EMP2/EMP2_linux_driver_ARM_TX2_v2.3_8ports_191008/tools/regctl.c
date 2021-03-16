#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

/* Joey modify - avoid warning */
#include <string.h>

#define READ  0x01
#define WRITE 0x02

/*
 *      EXAR ioctls
 */
//#define 	FIOQSIZE		0x5460 
#define		EXAR_READ_REG      	(FIOQSIZE + 1)
#define 	EXAR_WRITE_REG     	(FIOQSIZE + 2)

struct xrioctl_rw_reg {
	unsigned char reg;
	unsigned char regvalue;
};
 
int xrfd = -1;

int read_reg(int reg, int *value)
{
	struct xrioctl_rw_reg regread;
	int result;

	regread.reg = reg;
	result = ioctl(xrfd, EXAR_READ_REG, &regread);
	if (result < 0)
		printf("Cannot do EXAR_READ_REG ioctl (%d)\n", result);
	else
		(*value) = regread.regvalue;
 
	return result;
}

int write_reg(int reg, unsigned char data)
{
	struct xrioctl_rw_reg regwrite;
	int result;

	regwrite.reg = reg;
	regwrite.regvalue = data;
	result = ioctl(xrfd, EXAR_WRITE_REG, &regwrite);
	if (result < 0)
		printf("Cannot do EXAR_READ_REG ioctl (%d)\n", result);

	return result;
}

int main(int argc, char * argv[]) 
{
	int rw;
	int reg, value;
	char * device = "/dev/ttyXR0";

	if (argc < 3) {
		printf("Usage: %s read/write reg value\n", argv[0]);
		return -1;
	}

	if (strcmp("write", argv[1]) == 0)
		rw = WRITE;
	else if (strcmp("read", argv[1]) == 0)
		rw = READ;
	else {
		printf("Invalid command, should be 'read' or 'write'\n");
		return -1;
	}

	reg = strtol(argv[2], 0, 0);

	if (reg < 0x80 || reg > 0x9b) {
		printf("Invalid register, shouold be 0x80~0x9B\n");
		return -1;
	}

	if (rw == WRITE) {
		if (argc < 4) {
			printf("Value for write not given.\n");
			return -1;
		}
		value = strtol(argv[3], 0, 0);
		if (value < 0 || value > 255) {
			printf("Invalid value, should be 0~255\n");
			return -1;
		}
	}

	xrfd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (xrfd == -1) {
		printf("Can't open device %s\n", device);
		return -1;
	}

	if (rw == READ) {
		if (read_reg(reg, &value) >= 0)
			printf("0x%02x\n", value);
		else
			printf("Error: read_reg failed.\n");
	} else if (rw == WRITE) {
		if (write_reg(reg, value) < 0)
			printf("Error: write_reg failed.\n");
	}
		
	close(xrfd);
	return 0;
}


