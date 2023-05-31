#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <sys/mman.h>

void write_tty(const char *buffer, int count);
bool send_i2c(const char *i2c_port, uint8_t address);
bool send_to_device(const char *device, uint8_t brightness);
uint8_t* create_shared_memory(uintptr_t *memory_address, size_t size);
bool free_shared_memory(uintptr_t *memory_address, size_t size);
void print_memory(uint8_t *address, size_t size);

int main(void)
{
	printf("\nWelcome to the World of PHYTEC!\n");

	const char buf[] = { "\nWelcome from console\n" };
	write_tty(buf, strlen(buf));

	uintptr_t *address = (uintptr_t*)0x7e0000;
//	uintptr_t *address = (uintptr_t*)0x800000;
//	uintptr_t *address = (uintptr_t*)0x80000000;
	size_t size = 4096;
	uint8_t *shared_memory = create_shared_memory(address, size);
	if (shared_memory < 0)
	{
		printf("Can't allocate memory.\n");
		return -1;
	}

	printf("Memory allocated, pointer: %08lx, %lu bytes.\n", (uintptr_t)shared_memory, size);
	print_memory(shared_memory, 128);
	if (!free_shared_memory(address, size))
	{
		printf("Can't memory deallocate.\n");
		return -1;
	}
	printf("Memory deallocated.\n");

//	send_i2c("/dev/i2c-1", 0x62);

//	const char led_device[] = "/sys/class/leds/user-led1/brightness";
//	send_to_device(led_device, 0x64);
//	usleep(200 * 1000);
//	send_to_device(led_device, 0x0);

	printf("Program finished.\n\n");
	return 0;
}

uint8_t* create_shared_memory(uintptr_t *memory_address, size_t size)
{
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_SHARED | MAP_ANONYMOUS | MAP_FIXED;
	return mmap(memory_address, size, protection, visibility, -1, 0);
}

bool free_shared_memory(uintptr_t *memory_address, size_t size)
{
	return munmap(memory_address, size) == 0;
}

void print_memory(uint8_t *address, size_t size)
{
	for (uint64_t i = 0; i < size; i++)
	{
		if (i % 16 == 0)
			printf("%08lx: ", (uintptr_t)address);
		printf("%02x ", *address);
		address++;
		if (i % 8 == 8 - 1)
			printf(" ");
		if (i % 16 == 16 - 1)
			printf("\r\n");
	}
}

void write_tty(const char *buffer, int count)
{
	int out = open("/dev/console", O_RDWR);
	write(out, buffer, count);
	close(out);
}

bool send_i2c(const char *i2c_port, uint8_t address)
{
	int f = open(i2c_port, O_RDWR);
	if (f < 0)
	{
		printf("Can't open %s.\r\n", i2c_port);
		return false;
	}

	if (ioctl(f, I2C_SLAVE_FORCE, address) < 0)
	{
		printf("Can't set slave mode and set address %d (ioctl).\r\n", address);
		close(f);
		return false;
	}

	uint8_t data[] = { 0x97, 0x80, 0x00, 0x40, 0xe1 };
	int w = i2c_smbus_write_block_data(f, 0x11, sizeof(data), data);
	{
		printf("w %d\r\n", w);
		printf("Can't send data.\r\n");
		close(f);
		return false;
	}

	close(f);

	return true;
}

bool send_to_device(const char *device, uint8_t brightness)
{
	int f = open(device, O_RDWR);
	if (f < 0)
	{
		printf("Can't open device %s.\r\n", device);
		return false;
	}

	char data[64];
	sprintf(data, "%d", brightness);
	int w = write(f, data, sizeof(data));
	if (w < 0)
	{
		printf("w %d %ld\r\n", w, sizeof(data));
		printf("Can't send data.\r\n");
		close(f);
		return false;
	}

	close(f);
	return true;
}
