#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#define DEV_I2C "/dev/i2c-0"
#define SLAVE_ADDR_MCP9800 0x49 /* MCP9800 I2C slave address */
#define SLAVE_ADDR_EMC1438 0x4C /* EMC1438 I2C slave address */
#define SLAVE_ADDR_EMC2305 0x4D /* EMC2305 I2C slave address */
#include <linux/input.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#include <modbus/modbus.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "modbus_ex.h"

#define NB_CONNECTION    5

#define LED_RED_DEV			"/sys/class/leds/red/brightness"
#define LED_GREEN_DEV		"/sys/class/leds/green/brightness"
#define LED_BLUE_DEV		"/sys/class/leds/blue/brightness"

static int gLedRedfd = -1;
static int gLedGreenfd = -1;
static int gLedBluefd = -1;

modbus_t *ctx = NULL;
int server_socket = -1;
modbus_mapping_t *mb_mapping;

int gKeyStatus = 0;

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

unsigned char MCP9800_init(void)
{
	int fd;
	int ret;
	unsigned char buf[2];
	//union i2c_smbus_data buf;
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
		//printf("MCP9800 of Temperature is %d\n", buf[1]);
	// close device node
	close(fd);
	return buf[1];
}
//unsigned char* EMC2305_init(unsigned char* reg)
uint16_t * EMC2305_init(uint8_t *Fan_rpm_setting)
{
	int fd;
	int ret;
	uint8_t byte_date;
	uint8_t *data;
	uint16_t *buf = malloc(5 * sizeof(uint16_t));
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
			//byte_date = i2c_smbus_read_byte_data(fd,fan1_rpm);
			//printf("EMC2305 FAN1 reg 0x%x is 0x%x\n",fan1_rpm, byte_date);
			
			
			/*printf("EMC2305 FAN1 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | i2c_smbus_read_byte_data(fd,0x3F) >> 3));
			printf("EMC2305 FAN2 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x4E) << 5) | i2c_smbus_read_byte_data(fd,0x4F) >> 3));
			printf("EMC2305 FAN3 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x5E) << 5) | i2c_smbus_read_byte_data(fd,0x5F) >> 3));
			printf("EMC2305 FAN4 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x6E)<< 5) | i2c_smbus_read_byte_data(fd,0x6F) >> 3));
			printf("EMC2305 FAN5 of RPM is %d\n", (uint16_t) ((i2c_smbus_read_byte_data(fd,0x7E) << 5) | i2c_smbus_read_byte_data(fd,0x7F) >> 3));*/
			
			/*//block 最多只能讀32筆 byte ( I2C_SMBUS_BLOCK_MAX = 32 Linux 預設)
			ret = i2c_smbus_read_i2c_block_data(fd, 0x30, 0x20, data);
			printf("EMC2305 FAN1 reg 0x30 is 0x%x (byte_data)\n", i2c_smbus_read_byte_data(fd,0x30));
			printf("EMC2305 FAN1 reg 0x3%x is 0x%x (block_data)\n",0,data[0]);
			printf("EMC2305 FAN1 of RPM is %d (block_data)\n", 8190 -(0x1FFF & ((uint16_t) (data[0x0E] << 8| data[0x0F]))>> 3));
			printf("EMC2305 FAN2 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x1E] << 8| data[0x1F])>> 3)));
			ret = i2c_smbus_read_i2c_block_data(fd, 0x50, 0x20, data);
			printf("EMC2305 FAN3 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x0E] << 8| data[0x0F])>> 3)));
			printf("EMC2305 FAN4 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x1E] << 8| data[0x1F])>> 3)));
			ret = i2c_smbus_read_i2c_block_data(fd, 0x70, 0x10, data);
			printf("EMC2305 FAN5 of RPM is %d (block_data)\n", (uint16_t) 8190 - ((uint16_t) ((data[0x0E] << 8| data[0x0F])>> 3)));*/
			
			//usleep(10000);
			
			//s32 i2c_smbus_write_block_data(const struct i2c_client *client, u8 command, u8 length, const u8 *values)
			//s32 i2c_smbus_write_byte_data(const struct i2c_client *client, u8 command, u8 value)
			
			i2c_smbus_write_byte_data(fd, 0x30, Fan_rpm_setting[0]);
			i2c_smbus_write_byte_data(fd, 0x40, Fan_rpm_setting[1]);
			i2c_smbus_write_byte_data(fd, 0x50, Fan_rpm_setting[2]);
			i2c_smbus_write_byte_data(fd, 0x60, Fan_rpm_setting[3]);
			i2c_smbus_write_byte_data(fd, 0x70, Fan_rpm_setting[4]);
			
			printf("EMC2305 FAN1 of RPM is %d\n",  0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | i2c_smbus_read_byte_data(fd,0x3F)) >> 3));
			buf[0] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x3E)<< 5) | i2c_smbus_read_byte_data(fd,0x3F)) >> 3);
			buf[1] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x4E)<< 5) | i2c_smbus_read_byte_data(fd,0x4F)) >> 3);
			buf[2] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x5E)<< 5) | i2c_smbus_read_byte_data(fd,0x5F)) >> 3);
			buf[3] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x6E)<< 5) | i2c_smbus_read_byte_data(fd,0x6F)) >> 3);
			buf[4] = 0x1FFF & ((uint16_t) ((i2c_smbus_read_byte_data(fd,0x7E)<< 5) | i2c_smbus_read_byte_data(fd,0x7F)) >> 3);
			
			
		/*	uint8_t rpm_setting[32] = {0};
			rpm_setting[0] = fan1_rpm;
			rpm_setting[1] = fan1_rpm;
			i2c_smbus_write_block_data(fd, 0x3C, 2, rpm_setting);*/
	
	// close device node
	close(fd);
	return (buf);
}
void key_proc(void *param)
{
	int keys_fd;
	struct input_event t;
	
	//pthread_obj obj = (pthread_obj)param;
	
	if( (keys_fd = open("/dev/input/event0", O_RDONLY)) <= 0 )
	{
		printf("open event error\r\n");
		return;
	}
	
	while(1)
	{
		if( read(keys_fd, &t, sizeof(t)) == sizeof(t))
		{
			if( t.type == EV_KEY )
			{
				if( t.value ==0 || t.value == 1 )
				{
					printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
				}
				
				if( t.value )
				{
					gKeyStatus = 1;
				}
			}
		}
		
		usleep(100*1000);
	}
	
	close(keys_fd);
}

int ledInit(void)
{
	if( (gLedRedfd = open(LED_RED_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_RED_DEV);
		return -1;
	}
	
	if( (gLedGreenfd = open(LED_GREEN_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_GREEN_DEV);
		return -1;
	}
	
	if( (gLedBluefd = open(LED_BLUE_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_BLUE_DEV);
		return -1;
	}
	
	return 0;
}

void ledClose(void)
{
	if( gLedRedfd >= 0 )
		close(gLedRedfd);
	if( gLedGreenfd >= 0 )
		close(gLedGreenfd);
	if( gLedBluefd >= 0 )
		close(gLedBluefd);
}

int ledSet(LED_ID id)
{
	switch( id )
	{
		case LED_RED:
			if( gLedRedfd < 0 )
				return -1;
			write(gLedRedfd, "255", 3);
			break;
		case LED_GREEN:
			if( gLedGreenfd < 0 )
				return -1;
			write(gLedGreenfd, "255", 3);
			break;
		case LED_BLUE:
			if( gLedBluefd < 0 )
				return -1;
			write(gLedBluefd, "255", 3);
			break;
		default:
			return -1;
	}
	
	return 0;
}

int ledClean(LED_ID id)
{
	switch( id )
	{
		case LED_RED:
			if( gLedRedfd < 0 )
				return -1;
			write(gLedRedfd, "0", 1);
			break;
		case LED_GREEN:
			if( gLedGreenfd < 0 )
				return -1;
			write(gLedGreenfd, "0", 1);
			break;
		case LED_BLUE:
			if( gLedBluefd < 0 )
				return -1;
			write(gLedBluefd, "0", 1);
			break;
		default:
			return -1;
	}
	
	return 0;
}

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

int main(void)
{
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;
    
    int header_length;
    uint8_t *ptrPackage,emc1438_T;
    SingleBit_t SingleBitValue;

    pthread_t threadKeyID;
    
    ledInit();
	//emc1438_T= EMC1438_init();
	//printf("EMC1438 of Temperature is %d\r\n", emc1438_T);
	//unsigned char* reg;
	printf("EMC1438 of Temperature is %d\r\n", EMC1438_init());
	printf("MCP9800 of Temperature is %d\r\n", MCP9800_init());
	//printf("EMC2305 of Fan1 RPM is %d\r\n", EMC2305_init());
	
	uint8_t RPM_setting[5]={0x0F,0x0F,0x0F,0x0F,0x0F,};
	uint16_t *RPM_Fan;
	RPM_Fan = EMC2305_init(RPM_setting);
	printf("EMC2305 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
	printf("EMC2305 of Fan2 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[1],RPM_Fan[1],RPM_Fan[1]);
	printf("EMC2305 of Fan3 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[2],RPM_Fan[2],RPM_Fan[2]);
	printf("EMC2305 of Fan4 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[3],RPM_Fan[3],RPM_Fan[3]);
	printf("EMC2305 of Fan5 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[4],RPM_Fan[4],RPM_Fan[4]);
	// ********************************************************************************
	// key event monitor
	// ********************************************************************************    
	if( pthread_create(&threadKeyID, NULL, (void*)key_proc, NULL) != 0 )
	{
		printf("Create key process thread fail\r\n");
	}
	
	pthread_detach(threadKeyID);
	pthread_join(threadKeyID, NULL);
	// ********************************************************************************
	
    ctx = modbus_new_tcp(NULL, 1502);
    header_length = modbus_get_header_length(ctx);

    mb_mapping = modbus_mapping_new(10, 10, 10, 10);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for (;;) {
        rdset = refset;
        if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                
                	ptrPackage = query+header_length;
                	switch( *ptrPackage )
					{
						case MODBUS_CMD_WRITE_SIGNLE_COIL: // 5
							// write single coil
							SingleBitValue.address = *(ptrPackage+1) << 8 | *(ptrPackage+2);
							SingleBitValue.value = *(ptrPackage+3) << 8 | *(ptrPackage+4);
							
							printf("Write single bit: %d, %s\r\n", SingleBitValue.address, SingleBitValue.value==0xFF00?"ON":"OFF");
							
							if( SingleBitValue.address == POS_LED_RED ) {
								SingleBitValue.value==0xFF00 ? ledSet(LED_RED) : ledClean(LED_RED);
							} else if ( SingleBitValue.address == POS_LED_GREEN ) {
								SingleBitValue.value==0xFF00 ? ledSet(LED_GREEN) : ledClean(LED_GREEN);
							} else if( SingleBitValue.address == POS_LED_BLUE ) {
								SingleBitValue.value==0xFF00 ? ledSet(LED_BLUE) : ledClean(LED_BLUE);
							}
							break;
							
						case MODBUS_CMD_READ_DESCRETE_INPUTS: // 2
							// read bits
							SingleBitValue.address = *(ptrPackage+1) << 8 | *(ptrPackage+2);
							SingleBitValue.value = *(ptrPackage+3) << 8 | *(ptrPackage+4);
							
							printf("Read descrete inputs bit: %d, %d\r\n", SingleBitValue.address, SingleBitValue.value);
							
							if( SingleBitValue.address == POS_KEY_RED )
							{
								printf("Read single bit: %d\r\n", SingleBitValue.address);

								if( gKeyStatus == 1 )
								{
									printf("Key pressed\r\n");
									mb_mapping->tab_input_bits[POS_KEY_RED] = 1;
								}
							}
							break;
							/*case MODBUS_FC_READ_HOLDING_REGISTERS:  // 3
								printf("MODBUS_FC_READ_HOLDING_REGISTERS\r\n");
							break;
							case MODBUS_FC_READ_INPUT_REGISTERS:  // 4
								printf("MODBUS_FC_READ_INPUT_REGISTERS\r\n");
							break;*/
							
					}
					RPM_Fan = EMC2305_init(RPM_setting);
                    modbus_reply(ctx, query, rc, mb_mapping);
                    
                    switch( *ptrPackage )
					{
						case MODBUS_CMD_READ_DESCRETE_INPUTS: // 2
							// read bits
							SingleBitValue.address = *(ptrPackage+1) << 8 | *(ptrPackage+2);
							SingleBitValue.value = *(ptrPackage+3) << 8 | *(ptrPackage+4);
							
							if( SingleBitValue.address == POS_KEY_RED )
							{
								if( gKeyStatus == 1 )
								{
									gKeyStatus = 0;
									mb_mapping->tab_input_bits[POS_KEY_RED] = 0;
								}
							}
												
							break;
							case MODBUS_FC_READ_HOLDING_REGISTERS:  // 3
								printf("MODBUS_FC_READ_HOLDING_REGISTERS\r\n");
								mb_mapping->tab_registers[0] = RPM_setting[0];
								mb_mapping->tab_registers[1] = RPM_setting[1];
								mb_mapping->tab_registers[2] = RPM_setting[2];
								mb_mapping->tab_registers[3] = RPM_setting[3];
								mb_mapping->tab_registers[4] = RPM_setting[4];
							break;
							
							case MODBUS_FC_READ_INPUT_REGISTERS:  // 4
								printf("MODBUS_FC_READ_INPUT_REGISTERS\r\n");
								mb_mapping->tab_input_registers[0] = RPM_Fan[0];
								mb_mapping->tab_input_registers[1] = RPM_Fan[1];
								mb_mapping->tab_input_registers[2] = RPM_Fan[2];
								mb_mapping->tab_input_registers[3] = RPM_Fan[3];
								mb_mapping->tab_input_registers[4] = RPM_Fan[4];
							break;
							case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:  // 16 0x10
								RPM_setting[0] = (uint8_t) mb_mapping->tab_registers[0]; 
								RPM_setting[1] = (uint8_t) mb_mapping->tab_registers[1]; 
								RPM_setting[2] = (uint8_t) mb_mapping->tab_registers[2]; 
								RPM_setting[3] = (uint8_t) mb_mapping->tab_registers[3]; 
								RPM_setting[4] = (uint8_t) mb_mapping->tab_registers[4]; 
							break;
							
					}
			
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    return 0;
}
