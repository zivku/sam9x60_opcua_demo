#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <modbus/modbus.h>
#include "modbus_ex.h"

typedef struct MODBUS_TASK_PARAM__
{
	char slave_addr[128];
	int slave_port;
}MODBUS_TASK_PARAM, *pMODBUS_TASK_PARAM;

pthread_mutex_t gMutex;

int writeLed(modbus_t *ctx, POS_ADDR addr, BIT_VALUE val)
{
	int rc = 0;
	uint8_t read_data;
	
	pthread_mutex_lock(&gMutex);
	rc = modbus_write_bit(ctx, addr, val);
    if (rc != 1) {
        printf("[writeLed] FAILED\n");
        return -1;
    }

    rc = modbus_read_bits(ctx, addr, 1, &read_data);
    pthread_mutex_unlock(&gMutex);
    
    if (rc != 1) {
        printf("[writeLed] FAILED (nb points %d)\n", rc);
        return -1;
    }
    
    return 0;
}

int readKey(modbus_t *ctx, POS_ADDR addr, BIT_VALUE *val)
{
	int rc = 0;
	pthread_mutex_lock(&gMutex);
	rc = modbus_read_input_bits(ctx, addr, 1, (uint8_t*)val);
	pthread_mutex_unlock(&gMutex);
	
	if (rc != 1) {
        printf("[readKey] FAILED (nb points %s)\n", modbus_strerror(errno));
        return -1;
    }
    
    return 0;
}


uint16_t *Fan_RPM,*FAN_setting;
void monitorSlaveStatus(void *param)
{
	pMODBUS_TASK_PARAM pkeyTaskParam = (pMODBUS_TASK_PARAM)param;
	modbus_t *ctx;
	int rc;
	BIT_VALUE keyValue;
	
	
	printf("[monitorSlaveStatus] thread created\r\n");
	
	ctx = modbus_new_tcp(pkeyTaskParam->slave_addr, pkeyTaskParam->slave_port);
	
	if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "monitorSlaveStatus] Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }
	
	while(1)
	{
		rc = readKey(ctx, POS_KEY_RED, &keyValue);
		if( rc < 0 )
		{
			printf("[monitorSlaveStatus] Read key error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		if( keyValue != 0 )
		{
			printf("[monitorSlaveStatus] Key pressed\r\n");
		}
		
		
		usleep(100*1000);
	}
	
close:
	modbus_close(ctx);
    modbus_free(ctx);
}
#define LOOP          1
#define SERVER_ID     0
#define ADDRESS_START 0
#define ADDRESS_END   1
int main(int argc, char *argv[])
{
    modbus_t *ctx;
    int rc;
    int i;
    MODBUS_TASK_PARAM keyTaskParam;
    pthread_t keyMonitorId;
    pthread_mutex_init(&gMutex, NULL);

	if( argc != 2 )
	{
		printf("USAGE: simple_server_03 SERVER_IP\r\n");
		return -1;
	}

#if 1
	// ****************************************************	
	// key check task
	// ****************************************************	
	strcpy(keyTaskParam.slave_addr, argv[1]);
	keyTaskParam.slave_port = 1502;
	
	if( pthread_create(&keyMonitorId, NULL, (void*)monitorSlaveStatus, &keyTaskParam) != 0 )
	{
		printf("Create slave status monitor thread fail\r\n");
	}
	pthread_detach(keyMonitorId);
	pthread_join(keyMonitorId, NULL);
#endif	
	// ****************************************************	
	// end of task process
	// ****************************************************	
	
	// ****************************************************	
	// master connection process
	// ****************************************************	
    ctx = modbus_new_tcp(argv[1], 1502);
    //modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "[main] Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
	
	
	//modbus_t *ctx;
    //int rc;
    int nb_fail;
    int nb_loop;
    int addr;
    int nb=5;
    uint8_t *tab_rq_bits;
    uint8_t *tab_rp_bits;
    uint16_t *tab_rq_registers;
    uint16_t *tab_rw_rq_registers;
    uint16_t *tab_rp_registers;
	
	 /* Allocate and initialize the different memory spaces */
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
   // while (nb_loop++ < LOOP) {
	   while (1) {
		   usleep(30*1000);
        for (addr = ADDRESS_START; addr < ADDRESS_END; addr++) {
            int i;
			rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
			/*for (uint8_t i = 0; i < nb; i++) {
				printf("Address = %d, value %d \n", addr,tab_rp_registers[i]);
				tab_rw_rq_registers[i] = tab_rp_registers[i];
			}*/
			printf("Address = %d, Read register (FAN Setting) value %d \n", addr,tab_rp_registers[0]);
			/*tab_rw_rq_registers[0]++;
			rc = modbus_write_and_read_registers(ctx, addr, nb, tab_rw_rq_registers, addr, nb, tab_rp_registers);
			for (uint8_t i = 0; i < nb; i++) {
				printf("Address = %d, value %d \n", addr,tab_rp_registers[i]);
			}
			//tab_rw_rq_registers[0]++;
			printf("Address = %d, value %d \n", addr,tab_rp_registers[0]);*/
			
			rc =  modbus_read_input_registers(ctx, addr, nb, tab_rq_registers);
			printf("Address = %d, Read input register (FAN RPM) value %d \n", addr,tab_rq_registers[0]);
			
			if(tab_rp_registers[0] == 255) tab_rp_registers[0]=0;
			tab_rp_registers[0]++;
			rc = modbus_write_registers(ctx, addr, 1, tab_rp_registers);
			
		}
	}	
	
#if 0
	for(i=0; i<2; i++)
	{
		printf("[main] write LED_R ON\r\n");
		rc = writeLed(ctx, POS_LED_RED, ON);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		usleep(30*1000);
		
		printf("[main] write LED_R OFF\r\n");
		rc = writeLed(ctx, POS_LED_RED, OFF);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_GREEN);
			goto close;
		}
		
		usleep(30*1000);
		
		printf("[main] write LED_G ON\r\n");
		rc = writeLed(ctx, POS_LED_GREEN, ON);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_BLUE);
			goto close;
		}
		
		usleep(30*1000);
	
		printf("[main] write LED_G OFF\r\n");
		rc = writeLed(ctx, POS_LED_GREEN, OFF);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		usleep(30*1000);
		
		printf("[main] write LED_B ON\r\n");
		rc = writeLed(ctx, POS_LED_BLUE, ON);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_GREEN);
			goto close;
		}
		
		usleep(30*1000);
		
		printf("[main] write LED_B OFF\r\n");
		rc = writeLed(ctx, POS_LED_BLUE, OFF);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_BLUE);
			goto close;
		}
		
		usleep(30*1000);
	}
#endif
	while(1);
close:

	pthread_mutex_destroy(&gMutex);
	
     /* Free the memory */
    free(tab_rq_bits);
    free(tab_rp_bits);
    free(tab_rq_registers);
    free(tab_rp_registers);
    free(tab_rw_rq_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
