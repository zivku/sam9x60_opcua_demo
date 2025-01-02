#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h> 
#include <iostream>
#include <sstream>
#include <fstream>
#include <linux/gpio.h> 
#include <linux/input.h> 
#include <sys/ioctl.h>
#include <modbus/modbus.h>
#include <sys/stat.h>
#include <signal.h>
#include <thread>
uint16_t *Fan_RPM,*FAN_setting;

#define LOOP          1
#define SERVER_ID     0
#define ADDRESS_START 0
#define ADDRESS_END   1


bool key_active=false;
int main(int argc, char *argv[])
{
    uint16_t RPM_setting[5],RPM_setting_b[5];
	uint16_t *RPM_Fan;
	
	modbus_t *ctx;
    int rc;
    int i;

	if( argc != 2 )
	{
		printf("USAGE: simple_server_03 SERVER_IP\r\n");
		return -1;
	}

	// ****************************************************	
	// master connection process
	// ****************************************************	
    ctx = modbus_new_tcp(argv[1], 1502);

	//modbus_set_debug(ctx, TRUE);
	modbus_set_error_recovery(ctx,
							  (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK |
							  MODBUS_ERROR_RECOVERY_PROTOCOL));

	if (modbus_connect(ctx) == -1) {
		fprintf(stderr, "[main] Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	
    int nb_fail;
    int nb_loop;
    int addr;
    int nb=5;
    uint8_t *tab_rq_bits;
    uint8_t *tab_rp_bits;
    uint16_t *tab_rq_registers;
    uint16_t *tab_rw_rq_registers;
    uint16_t *tab_rp_registers;

	 // Allocate and initialize the different memory spaces 
    //nb = ADDRESS_END - ADDRESS_START;

    tab_rq_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

    tab_rp_bits = (uint8_t *) malloc(nb * sizeof(uint8_t));
    memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

    tab_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

    tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

    tab_rw_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
    memset(tab_rw_rq_registers, 0, nb * sizeof(uint16_t));

    nb_loop = nb_fail = 0;
	
	
	
	auto thread_Button_LEDs = [&RPM_setting,&RPM_Fan]() {	
		int red,green,blue,SW2,SW4,ret;
		char value;
		struct input_event event0,event1;
		
		//RPM_Fan = EMC2301_init(RPM_setting);
		
		red = open("/sys/class/leds/red/brightness", O_WRONLY| O_NONBLOCK);
		green = open("/sys/class/leds/green/brightness", O_WRONLY| O_NONBLOCK);
		blue = open("/sys/class/leds/blue/brightness", O_WRONLY| O_NONBLOCK);
		SW2 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
		SW4 = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
		while(1)
		{	
			//RPM_setting_b[0] =RPM_setting[0];
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
							printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
							if(RPM_setting[0] < 240)
								RPM_setting[0]++;
						break;
						case 2:
							write(red, "1", 1);
							printf("red light on:%d, value:%d RPM_setting:%d\n",event0.type, event0.value,RPM_setting[0] );
							printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
							if(RPM_setting[0] < 240)
								RPM_setting[0]= RPM_setting[0]+10;
						break;
						default:
							write(red, "0", 1);
						break;
					}
					key_active = true;
					
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
							printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
							if(RPM_setting[0] > 20)
								RPM_setting[0]--;
						break;
						case 2:
							write(green, "1", 1);
							printf("green light on:%d, value:%d RPM_setting:%d\n",event1.type, event1.value,RPM_setting[0] );
							printf("EMC2301 of Fan1 RPM is 13b'%b   %d  0X%x\r\n", RPM_Fan[0],RPM_Fan[0],RPM_Fan[0]);
							if(RPM_setting[0] > 20)
								RPM_setting[0]= RPM_setting[0]-10;
						break;
						default:
							write(green, "0", 1);
						break;
					}
					key_active = true;
				}	
				usleep(1000*5);
		}	
		close(red);    
		close(green);   
		close(blue);   
		close(SW2);    
		close(SW4); 
		
		
	};
	std::thread thread_Button_LEDs_thread(thread_Button_LEDs);
	

 	
	
	   while (1) 
	   {
			for (addr = ADDRESS_START; addr < ADDRESS_END; addr++) 
			{
				rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
				RPM_setting[0]=tab_rp_registers[0];
			
				
				rc =  modbus_read_input_registers(ctx, addr, nb, tab_rq_registers);
				RPM_Fan[0] = tab_rq_registers[0];
				
				if(key_active == true)
					{	
						tab_rp_registers[0] = RPM_setting[0];
						modbus_write_registers(ctx, 0, 1, tab_rp_registers);
						key_active = false;
					}
			}
	}	

close:

     // Free the memory 
    free(tab_rq_bits);
    free(tab_rp_bits);
    free(tab_rq_registers);
    free(tab_rp_registers);
    free(tab_rw_rq_registers);

    // Close the connection 
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
