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

void write_tty(const char *buffer, int count);
bool send_i2c(const char *i2c_port, uint8_t address);
bool send_to_device(const char *device, uint8_t brightness);

int main(void)
{
	printf("Welcome to the World of PHYTEC!\n");

	const char buf[] = { "\nWelcome from console\n" };
	write_tty(buf, strlen(buf));

//	send_i2c("/dev/i2c-1", 0x62);

	const char led_device[] = "/sys/class/leds/user-led1/brightness";
	send_to_device(led_device, 0x64);
	usleep(200 * 1000);
	send_to_device(led_device, 0x0);

	return 0;
}

void write_tty(const char *buffer, int count)
{
	int out;
	out = open("/dev/console", O_RDWR);
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
