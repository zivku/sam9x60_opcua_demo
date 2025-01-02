#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <string.h> 
#include <errno.h>

#include <stdint.h>
#include <inttypes.h>


#include <linux/gpio.h> 
#include <linux/input.h> 
#include <sys/ioctl.h>
//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>
//#include <i2c/smbus.h>
#include <sys/stat.h>
#include <pthread.h>
extern "C"
{
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}
#define DEV_I2C "/dev/i2c-0"

//SAM9x60 Hobby board
#define SLAVE_ADDR_MCP9800 0x49 /* MCP9800 I2C slave address */ 

//ADM00879 - EMC2305 & EMC1438 FAN CTRL AND TEMP DEMO
#define SLAVE_ADDR_EMC1438 0x4C /* EMC1438 I2C slave address */
#define SLAVE_ADDR_EMC2305 0x4D /* EMC2305 I2C slave address */

//APP_DFAE_AddOn 4
#define SLAVE_ADDR_EMC2301 0x2F /* EMC2301 I2C slave address */
#define SLAVE_ADDR_EMC1414 0x4C /* EMC1414 I2C slave address */
#define SLAVE_ADDR_PAC1954 0x1F /* PAC1954 I2C slave address */

unsigned char EMC1438_init(void)
{
	int fd;
	int ret;
	unsigned char buf[2];
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_EMC1438) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}
		buf[1] = 0; // clear receive buffer
		ret = read(fd, &buf[1], 1);
		
		if (ret != 1) {
			printf("ERROR read() data\n");
			return 0;
		}	
		//printf("EMC1438 of Temperature is %d\n", buf[1]);
	// close device node
	close(fd);
	return buf[1];
}
unsigned char EMC1414_init(void)
{
	int fd;
	int ret;
	unsigned char buf[2];
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_EMC1414) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}
		buf[1] = 0; // clear receive buffer
		ret = read(fd, &buf[1], 1);
		
		if (ret != 1) {
			printf("ERROR read() data\n");
			return 0;
		}	
		//printf("EMC1438 of Temperature is %d\n", buf[1]);
	// close device node
	close(fd);
	return buf[1];
}
unsigned char MCP9800_init(void)
{
	int fd;
	int ret;
	unsigned char buf[2];

	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_MCP9800) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}
		buf[1] = 0; // clear receive buffer
		ret = read(fd, &buf[1], 1);
		
		if (ret != 1) {
			printf("ERROR read() data\n");
			return 0;
		}	

	close(fd);
	return buf[1];
}

uint8_t fan_setting[14] = {0xFF,0x02,0x2B,0x28,0x2A,0x19,0x10,0x66,0x27,0x00,0x00,0xF8,0xFF};
uint16_t * EMC2301_init(uint8_t *Fan_rpm_setting)
{
	int fd;
	uint16_t *buf =(uint16_t*) malloc(5 * sizeof(uint16_t));
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_EMC2301) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}
			for(uint8_t j=0x30;j<=0x40;j=j+0x10)
				for (uint8_t i=1;i<=0x0D;i++)
				{	
					if(i==4)
						i++;
					i2c_smbus_write_byte_data(fd, j+i, fan_setting[0+i]);
				}
			//Fan_rpm_setting[0] = i2c_smbus_read_byte_data(fd,0x30);
			i2c_smbus_write_byte_data(fd, 0x30, Fan_rpm_setting[0]);	
			buf[0] = 0x1FFF & (8190 -  ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | (i2c_smbus_read_byte_data(fd,0x3F) >> 3))));
			 
	// close device node
	close(fd);
	return (buf);

}
uint16_t * EMC2305_init(uint8_t *Fan_rpm_setting)
{
	int fd;
	uint16_t *buf =(uint16_t*) malloc(5 * sizeof(uint16_t));
	// open device node
	fd = open(DEV_I2C, O_RDWR);

	if (fd < 0) {
		printf("ERROR open %s ret=%d\n", DEV_I2C, fd);
		return 0;
	}
	
	if (ioctl(fd, I2C_SLAVE, SLAVE_ADDR_EMC2305) < 0) {
		printf("ERROR ioctl() set slave address\n");
		return 0;
	}

			for(uint8_t j=0x30;j<=0x80;j=j+0x10)
				for (uint8_t i=1;i<=0x0D;i++)
				{	
					if(i==4)
						i++;
					i2c_smbus_write_byte_data(fd, j+i, fan_setting[0+i]);
					
				}
			
			i2c_smbus_write_byte_data(fd, 0x30, Fan_rpm_setting[0]);
			i2c_smbus_write_byte_data(fd, 0x40, Fan_rpm_setting[1]);
			i2c_smbus_write_byte_data(fd, 0x50, Fan_rpm_setting[2]);
			i2c_smbus_write_byte_data(fd, 0x60, Fan_rpm_setting[3]);
			i2c_smbus_write_byte_data(fd, 0x70, Fan_rpm_setting[4]);
			
			buf[0] = 0x1FFF & (8190 -  ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | (i2c_smbus_read_byte_data(fd,0x3F) >> 3))));
			buf[1] = 0x1FFF & (8190 -  ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x4E)<< 5) | (i2c_smbus_read_byte_data(fd,0x4F) >> 3))));
			buf[2] = 0x1FFF & (8190 -  ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x5E)<< 5) | (i2c_smbus_read_byte_data(fd,0x5F) >> 3))));
			buf[3] = 0x1FFF & (8190 -  ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x6E)<< 5) | (i2c_smbus_read_byte_data(fd,0x6F) >> 3))));
			buf[4] = 0x1FFF & (8190 -  ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x7E)<< 5) | (i2c_smbus_read_byte_data(fd,0x7F) >> 3))));
			

	
	// close device node
	close(fd);
	return (buf);
}

int main(int argc, char *argv[]) 
{    
	int red,green,blue,SW2,SW4,ret;
	char value;
	struct input_event event0,event1;
	
	
	uint8_t RPM_setting[5]={0x0F,0x0F,0x0F,0x0F,0x0F,};
	uint16_t *RPM_Fan;
	printf("EMC1414 of Temperature is %d\r\n", EMC1414_init());
	printf("MCP9800 of Temperature is %d\r\n", MCP9800_init());
	
	RPM_Fan = EMC2305_init(RPM_setting);
	printf("EMC2305 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
	printf("EMC2305 of Fan2 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[1],RPM_Fan[1],RPM_Fan[1]);
	printf("EMC2305 of Fan3 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[2],RPM_Fan[2],RPM_Fan[2]);
	printf("EMC2305 of Fan4 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[3],RPM_Fan[3],RPM_Fan[3]);
	printf("EMC2305 of Fan5 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[4],RPM_Fan[4],RPM_Fan[4]);
	
	RPM_Fan = EMC2301_init(RPM_setting);
	printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
	
		red = open("/sys/class/leds/red/brightness", O_WRONLY| O_NONBLOCK);
		green = open("/sys/class/leds/green/brightness", O_WRONLY| O_NONBLOCK);
		blue = open("/sys/class/leds/blue/brightness", O_WRONLY| O_NONBLOCK);
		SW2 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
		SW4 = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
	while(1)
	{		
		
		RPM_Fan = EMC2301_init(RPM_setting);
		
		ret =read(SW2, &event0, sizeof(struct input_event));
		ret = read(SW4, &event1, sizeof(struct input_event));
		
		if(event0.type== EV_KEY)
		{	
			switch(event0.value)
			{
				case 0:
					write(red, "0", 1);
					//printf("red light off, value:%d\n", event0.value);
				break;
				case 1:
					write(red, "1", 1);
					printf("red light on :%d, value:%d RPM_setting:%d\n",event0.type, event0.value ,RPM_setting[0]);
					if(RPM_setting[0] < 255)
						RPM_setting[0]++;
				break;
				case 2:
					write(red, "1", 1);
					printf("red light on:%d, value:%d RPM_setting:%d\n",event0.type, event0.value,RPM_setting[0] );
					if(RPM_setting[0] < 255)
						RPM_setting[0]++;
				break;
				default:
					write(red, "0", 1);
				break;
			}
			printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
		}
		
	    if( event1.type== EV_KEY)
		{	
			
			switch(event1.value)
			{
				case 0:
					write(green, "0", 1);
					//printf("green light off, value:%d\n", event1.value);
				break;
				case 1:
					write(green, "1", 1);
					printf("green light on:%d, value:%d RPM_setting:%d\n",event1.type, event1.value,RPM_setting[0] );
					if(RPM_setting[0] > 0)
						RPM_setting[0]--;
				break;
				case 2:
					write(green, "1", 1);
					printf("green light on:%d, value:%d RPM_setting:%d\n",event1.type, event1.value,RPM_setting[0] );
					if(RPM_setting[0] > 0)
						RPM_setting[0]--;
				break;
				default:
					write(green, "0", 1);
				break;
			}
			printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
		}
	}
	
	close(red);    
	close(green);   
	close(blue);   
	close(SW2);    
	close(SW4); 
	return 0; 
}
	
	
	
	
