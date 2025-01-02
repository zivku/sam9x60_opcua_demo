#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <string.h> 
#include <linux/gpio.h> 
#include <linux/input.h> 

int main(int argc, char *argv[]) 
{    
	int red,green,blue,SW2,SW4,ret;
	char value;
	struct input_event event0,event1;
	
		red = open("/sys/class/leds/red/brightness", O_WRONLY| O_NONBLOCK);
		green = open("/sys/class/leds/green/brightness", O_WRONLY| O_NONBLOCK);
		blue = open("/sys/class/leds/blue/brightness", O_WRONLY| O_NONBLOCK);
		SW2 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
		SW4 = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
	while(1)
	{		
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
					printf("red light on :%d, value:%d\n",event0.type, event0.value );
				break;
				case 2:
					write(red, "1", 1);
					printf("red light on:%d, value:%d\n",event0.type, event0.value );
				break;
				default:
					write(red, "0", 1);
				break;
			}
		
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
					printf("green light on:%d, value:%d\n",event1.type, event1.value );
				break;
				case 2:
					write(green, "1", 1);
					printf("green light on:%d, value:%d\n",event1.type, event1.value );
				break;
				default:
					write(green, "0", 1);
				break;
			}
		}
	}
	
	close(red);    
	close(green);   
	close(blue);   
	close(SW2);    
	close(SW4); 
	return 0; 
}
	
	
	
	
