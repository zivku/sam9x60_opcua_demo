
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <modbus/modbus.h>
#include "../open62541/build/open62541.h"

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





static void
addVariable(UA_Server *server , const char *ObjectNode_baseId, int ObjectNode_idNumber, const char *baseId, 
    int idNumber,uint16_t modbus_read_input, const char *baseId_W, int idNumber_W,uint16_t modbus_SetRPM_read) {
	char ObjectnodeIdString[50];
    sprintf(ObjectnodeIdString, "%s%d", ObjectNode_baseId, ObjectNode_idNumber);
	UA_VariableAttributes ottr = UA_VariableAttributes_default;
	/* Add the variable node to the information model */
    UA_NodeId myObjectnodeId= UA_NODEID_STRING(1, ObjectnodeIdString);
    UA_QualifiedName myObjectnodeIntegerName = UA_QUALIFIEDNAME(1, ObjectnodeIdString);
    UA_NodeId ObjectNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId ObjectReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myObjectnodeId, ObjectNodeId,
                              ObjectReferenceNodeId, myObjectnodeIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), ottr, NULL, &myObjectnodeId);
	
    /* Define the attribute of the myInteger variable node */
	char nodeIdString[50];
    sprintf(nodeIdString, "%s%d", baseId, idNumber);
	
	/* Define the attribute of the myInteger variable node */
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = modbus_read_input;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US",nodeIdString);
    attr.displayName = UA_LOCALIZEDTEXT("en-US",nodeIdString);
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	
	
    /* Add the variable node to the information model */
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, nodeIdString);
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, nodeIdString);
    //UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    //UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	//UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, myObjectnodeId,//parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
	
	/* Define the attribute of the myInteger variable node */
	char nodeIdString1[50];
    sprintf(nodeIdString1, "%s%d", baseId_W, idNumber_W);
	
	/* Define the attribute of the myInteger variable node */
    UA_VariableAttributes attr1 = UA_VariableAttributes_default;
    UA_Int32 myInteger1 = modbus_SetRPM_read;
    UA_Variant_setScalar(&attr1.value, &myInteger1, &UA_TYPES[UA_TYPES_INT32]);
    attr1.description = UA_LOCALIZEDTEXT("en-US",nodeIdString1);
    attr1.displayName = UA_LOCALIZEDTEXT("en-US",nodeIdString1);
    attr1.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr1.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
	
	
    /* Add the variable node to the information model */
    UA_NodeId myIntegerNodeId1 = UA_NODEID_STRING(1, nodeIdString1);
    UA_QualifiedName myIntegerName1 = UA_QUALIFIEDNAME(1, nodeIdString1);
    //UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    //UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	//UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    UA_NodeId parentReferenceNodeId1 = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId1, myObjectnodeId,//parentNodeId,
                              parentReferenceNodeId1, myIntegerName1,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr1, NULL, NULL);
	
}

static void 
writeVariable(UA_Server *server, const char *ObjectNode_baseId, int ObjectNode_idNumber,
				const char *baseId, int idNumber,uint16_t modbus_value,
				const char *baseId_W, int idNumber_W, uint16_t modbus_value_W ) {
    //UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "RPM1.value");
	char nodeIdString[50];
    sprintf(nodeIdString, "%s%d", baseId, idNumber);
	UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, nodeIdString);
	
    /* Write a different integer value */
    UA_Int32 myInteger = modbus_value;
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, myIntegerNodeId, myVar);

    /* Set the status code of the value to an error code. The function
     * UA_Server_write provides access to the raw service. The above
     * UA_Server_writeValue is syntactic sugar for writing a specific node
     * attribute with the write service. */
    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = myIntegerNodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
    wv.value.hasStatus = true;
    UA_Server_write(server, &wv);

    /* Reset the variable to a good statuscode with a value */
    wv.value.hasStatus = false;
    wv.value.value = myVar;
    wv.value.hasValue = true;
    UA_Server_write(server, &wv);
	
	//UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "RPM1.value");
	char nodeIdString1[50];
    sprintf(nodeIdString1, "%s%d", baseId_W, idNumber_W);
	UA_NodeId myIntegerNodeId1 = UA_NODEID_STRING(1, nodeIdString1);
	
    /* Write a different integer value */
    UA_Int32 myInteger1 = modbus_value_W;
    UA_Variant myVar1;
    UA_Variant_init(&myVar1);
    UA_Variant_setScalar(&myVar1, &myInteger1, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, myIntegerNodeId1, myVar1);

    /* Set the status code of the value to an error code. The function
     * UA_Server_write provides access to the raw service. The above
     * UA_Server_writeValue is syntactic sugar for writing a specific node
     * attribute with the write service. */
    UA_WriteValue wv1;
    UA_WriteValue_init(&wv1);
    wv1.nodeId = myIntegerNodeId1;
    wv1.attributeId = UA_ATTRIBUTEID_VALUE;
    wv1.value.status = UA_STATUSCODE_BADNOTCONNECTED;
    wv1.value.hasStatus = true;
    UA_Server_write(server, &wv1);

    /* Reset the variable to a good statuscode with a value */
    wv1.value.hasStatus = false;
    wv1.value.value = myVar1;
    wv1.value.hasValue = true;
    UA_Server_write(server, &wv1);

	
}

/** It follows the main server code, making use of the above definitions. */

static volatile UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}
static void addCurrentTimeVariable(UA_Server *server) {
    UA_NodeId pumpId= UA_NODEID_STRING(1, "SAM9x60 UA DateTime now");; /* get the nodeid assigned by the server */
    UA_VariableAttributes oAttr = UA_VariableAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "SAM9x60 UA DateTime now");
    UA_Server_addVariableNode(server, pumpId,
                            UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                            UA_QUALIFIEDNAME(1, "SAM9x60 UA DateTime now"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
                            oAttr, NULL, &pumpId);
	
	UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_DateTime now = UA_DateTime_now();
	UA_NodeId currentTimeNodeId = UA_NODEID_STRING(1, "Current_Time");
	 UA_QualifiedName currentTimeName =  UA_QUALIFIEDNAME(1, "Current_Time");
    UA_Variant_setScalar(&attr.value, &now, &UA_TYPES[UA_TYPES_DATETIME]);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Current Time");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_Server_addVariableNode(server, currentTimeNodeId, pumpId,//parentNodeId,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), currentTimeName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);	 			  
}

static void updateCurrentTime(UA_Server *server) {
    UA_DateTime now = UA_DateTime_now();
    UA_Variant value;
    UA_Variant_setScalar(&value, &now, &UA_TYPES[UA_TYPES_DATETIME]);
    UA_NodeId currentTimeNodeId = UA_NODEID_STRING(1, "Current_Time");
    UA_Server_writeValue(server, currentTimeNodeId, value);
	
	/* Set the status code of the value to an error code. The function
     * UA_Server_write provides access to the raw service. The above
     * UA_Server_writeValue is syntactic sugar for writing a specific node
     * attribute with the write service. */
    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = currentTimeNodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
    wv.value.hasStatus = true;
    UA_Server_write(server, &wv);

    /* Reset the variable to a good statuscode with a value */
    wv.value.hasStatus = false;
    wv.value.value = value;
    wv.value.hasValue = true;
    UA_Server_write(server, &wv);
}
#define Modbus_Slave_IP "192.168.77.6"
int main(int argc, char *argv[]) {
	uint8_t RPM_setting[6]={0x0F,0x0F,0x0F,0x0F,0x0F};
	uint16_t *RPM_Fan;
	modbus_t *ctx;
	uint8_t *tab_rq_bits;
	uint8_t *tab_rp_bits;
	uint16_t *tab_rq_registers;
	uint16_t *tab_rw_rq_registers;
	uint16_t *tab_rp_registers;
	struct ifaddrs *ifaddr, *ifa;
    int  s;
    char host[NI_MAXHOST],Slave_IP[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if( (strcmp(ifa->ifa_name,"wlan0")==0)&&(  ifa->ifa_addr->sa_family==AF_INET)  )
        {
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\tInterface : <%s>\n",ifa->ifa_name );
            printf("\t  Address : <%s>\n", host);
        }
    }
	
    freeifaddrs(ifaddr);

	
		auto thread_Modbus = [&RPM_setting,&RPM_Fan,&argc,&argv,&host,&Slave_IP,&tab_rq_bits,&tab_rp_bits,&tab_rq_registers,&tab_rw_rq_registers,&tab_rp_registers,&ctx]() {	
				
				//int rc;
				//int nb_fail;
				//int nb_loop;
				int nb=5;
				
				//char* ip= "192.168.77.1";
				MODBUS_TASK_PARAM keyTaskParam;
				//pthread_t keyMonitorId;
			//	pthread_mutex_init(&gMutex, NULL);
				strcpy(keyTaskParam.slave_addr, Modbus_Slave_IP);
				ctx = modbus_new_tcp(Modbus_Slave_IP, 1502);
				printf("host ip : %s \r\n",host);
				int a,b,c,d;
				sscanf(host,"%d.%d.%d.%d",&a,&b,&c,&d);
				printf("a = %d , b= %d , c=%d, d=%d\r\n",a,b,c,d);
				//Slave_IP
				sprintf(Slave_IP,"%d.%d.%d.%d",a,b,c,d);
				d = 218;
				printf("Copy host ip to prepare searching Slave_IP : %s \r\n",Slave_IP);
				//if( argc == 2 || argc == 3)
			
						if(argc == 3)
						{
							printf("argc  %d\r\n",argc);
							printf("The IP as your type  : %s\r\n",argv[1]);
							sprintf(Slave_IP,"%s",argv[1]);
							strcpy(keyTaskParam.slave_addr, argv[1]);
							ctx = modbus_new_tcp(argv[1], 1502);
						}
						/*else if( argc != 2 || argc==1)
						{
							printf("argc = %d, (argc over 2 or argc = 1)\r\n",argc);
							printf("Modbus_Slave_IP(code fixed) : %s\r\n",Modbus_Slave_IP);
							strcpy(keyTaskParam.slave_addr, Modbus_Slave_IP);
							ctx = modbus_new_tcp(Modbus_Slave_IP, 1502);
						}*/
						else
						{
							printf("start to serach MODBUS Server IP base on MODBUS Client IP:%s \r\n",host);
			
							sprintf(Slave_IP,"%d.%d.%d.%d",a,b,c,d);
							ctx = modbus_new_tcp(Slave_IP, 1502);
							
							while (modbus_connect(ctx) == -1) {
								
								
								//modbus_free(ctx); 
								if(d < 255)
									d++;
								else if(d>254)
									d = 1; 
								else
									d=1;
								modbus_close(ctx);
								sprintf(Slave_IP,"%d.%d.%d.%d",a,b,c,d);
								ctx = modbus_new_tcp(Slave_IP, 1502);
								printf("\r\ntry to connecting Server ip) : %s\r\n",Slave_IP);	
								switch(d%10)
								{
									case 2:
										printf("..            \r");
									break;
									case 3:
										printf("...            \r");
									break;
									case 4:
										printf("....           \r");
									break;
									case 5:
										printf(".....          \r");
									break;
									case 6:
										printf("......         \r");
									break;
									case 7:
										printf(".......            \r");
									break;
									case 8:
										printf("........            \r");
									break;
									case 9:
										printf(".........            \r");
									break;
									default:
										printf("...........            \r");
									break;
								}
								
							}
							
							printf("\r\nModbus_Server_IP(auto search Server ip) : %s\r\n",Slave_IP);		
				
						
				}
				keyTaskParam.slave_port = 1502;
			#if 0	
				if( pthread_create(&keyMonitorId, NULL, (void*)monitorSlaveStatus, &keyTaskParam) != 0 )
				{
					printf("Create Server status monitor thread fail\r\n");
				}
				pthread_detach(keyMonitorId);
				pthread_join(keyMonitorId, NULL);
			#endif	
				modbus_set_error_recovery(ctx,
										  (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK |
										  MODBUS_ERROR_RECOVERY_PROTOCOL));

				if (modbus_connect(ctx) == -1) {
					fprintf(stderr, "[main] Connection failed: %s\n", modbus_strerror(errno));
					modbus_free(ctx);
					return -1;
				}				
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
				   while (1) 
				   {
					   usleep(600*1000);
				
						for (int addr = ADDRESS_START; addr < ADDRESS_END; addr++) 
						{
							modbus_read_registers(ctx, addr, nb, tab_rp_registers);
							modbus_read_input_registers(ctx, addr, nb, tab_rq_registers);
							printf("FAN current RPM : %d %d %d %d %d\r\n",tab_rq_registers[0],tab_rq_registers[1],tab_rq_registers[2],tab_rq_registers[3],tab_rq_registers[4]);
							printf("FAN RPM setting : %d %d %d %d %d\r\n",tab_rp_registers[0],tab_rp_registers[1],tab_rp_registers[2],tab_rp_registers[3],tab_rp_registers[4]);	
						}
					}	
	
				//while(1);
			//close:

				//pthread_mutex_destroy(&gMutex);
				
				/*Free the memory*/ 
				free(tab_rq_bits);
				free(tab_rp_bits);
				free(tab_rq_registers);
				free(tab_rp_registers);
				free(tab_rw_rq_registers);

				//Close the connection
				modbus_close(ctx);
				modbus_free(ctx); 
		};
		std::thread thread_Modbus_thread(thread_Modbus);

		printf(" thread_Modbus_threadtest 1!!!!!!!!!!!!!!\r\n");
	
	signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);
	//UA_ServerConfig *config = UA_ServerConfig_new_default();
    //UA_Server *server = UA_Server_new(config);
	UA_Server *server = UA_Server_new();
	UA_ServerConfig_setDefault(UA_Server_getConfig(server));
	
	UA_StatusCode retval = UA_Server_run_startup(server);
	if(retval != UA_STATUSCODE_GOOD)
		return 1;
	
	printf("UA_Server_run_startup test 1!!!!!!!!!!!!!!\r\n");
	

	modbus_read_registers(ctx, 0, 5, tab_rp_registers);
	modbus_read_input_registers(ctx, 0, 5, tab_rq_registers);
	
	addCurrentTimeVariable(server);
	addVariable(server,"Fan",1,"value.RPM",1,tab_rq_registers[0],"Setting.RPM",1,tab_rp_registers[0]);
	addVariable(server,"Fan",2,"value.RPM",2,tab_rq_registers[1],"Setting.RPM",2,tab_rp_registers[1]);
	addVariable(server,"Fan",3,"value.RPM",3,tab_rq_registers[2],"Setting.RPM",3,tab_rp_registers[2]);
	addVariable(server,"Fan",4,"value.RPM",4,tab_rq_registers[3],"Setting.RPM",4,tab_rp_registers[3]);
	addVariable(server,"Fan",5,"value.RPM",5,tab_rq_registers[4],"Setting.RPM",5,tab_rp_registers[4]);
    // Set up a repeating timer to update the current time
    UA_UInt64 interval = 1000; // Update every second
    UA_Server_addRepeatedCallback(server, (UA_ServerCallback)updateCurrentTime, NULL, interval, NULL);
	/*
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)modbus_read_registers(ctx, 0, 5, tab_rp_registers), NULL, interval, NULL);
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)modbus_read_input_registers(ctx, 0, 5, tab_rq_registers), NULL, interval, NULL);
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)writeVariable(server,"Fan",1,"value.RPM",1,tab_rq_registers[0],"Setting.RPM",1,tab_rp_registers[0]), NULL, interval, NULL);
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)writeVariable(server,"Fan",2,"value.RPM",2,tab_rq_registers[1],"Setting.RPM",2,tab_rp_registers[1]), NULL, interval, NULL);
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)writeVariable(server,"Fan",3,"value.RPM",3,tab_rq_registers[2],"Setting.RPM",3,tab_rp_registers[2]), NULL, interval, NULL);
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)writeVariable(server,"Fan",4,"value.RPM",4,tab_rq_registers[3],"Setting.RPM",4,tab_rp_registers[3]), NULL, interval, NULL);
	UA_Server_addRepeatedCallback(server, (UA_ServerCallback)writeVariable(server,"Fan",5,"value.RPM",5,tab_rq_registers[4],"Setting.RPM",5,tab_rp_registers[4]), NULL, interval, NULL);
	*/
	/*addVariable(server,"Fan",1,"value.RPM",1,tab_rp_registers[0]);
	addVariable(server,"Fan",2,"value.RPM",2,tab_rp_registers[1]);
	addVariable(server,"Fan",3,"value.RPM",3,tab_rp_registers[2]);
	addVariable(server,"Fan",4,"value.RPM",4,tab_rp_registers[3]);
	addVariable(server,"Fan",5,"value.RPM",5,tab_rp_registers[4]);
	addVariable(server,"Fan",1,"Setting.RPM",1,tab_rq_registers[0]);
	addVariable(server,"Fan",2,"Setting.RPM",2,tab_rq_registers[1]);
	addVariable(server,"Fan",3,"Setting.RPM",3,tab_rq_registers[2]);
	addVariable(server,"Fan",4,"Setting.RPM",4,tab_rq_registers[3]);
	addVariable(server,"Fan",5,"Setting.RPM",5,tab_rq_registers[4]);
	*/
	
	/*
	writeWrongVariable(server,"Fan",1,"value.RPM",1);
	writeWrongVariable(server,"Fan",2,"value.RPM",2);
	writeWrongVariable(server,"Fan",3,"value.RPM",3);
	writeWrongVariable(server,"Fan",4,"value.RPM",4);
	writeWrongVariable(server,"Fan",5,"value.RPM",5);
	writeWrongVariable(server,"Fan",1,"Setting.RPM",1);
	writeWrongVariable(server,"Fan",2,"Setting.RPM",2);
	writeWrongVariable(server,"Fan",3,"Setting.RPM",3);
	writeWrongVariable(server,"Fan",4,"Setting.RPM",4);
	writeWrongVariable(server,"Fan",5,"Setting.RPM",5);
	*/
	UA_Boolean waitInternal = true;
	 
	 while(running) {
       UA_Server_run_iterate(server, waitInternal);
		modbus_read_registers(ctx, 0, 5, tab_rp_registers);
		modbus_read_input_registers(ctx, 0, 5, tab_rq_registers);
		
		writeVariable(server,"Fan",1,"value.RPM",1,tab_rq_registers[0],"Setting.RPM",1,tab_rp_registers[0]);
		writeVariable(server,"Fan",2,"value.RPM",2,tab_rq_registers[1],"Setting.RPM",2,tab_rp_registers[1]);
		writeVariable(server,"Fan",3,"value.RPM",3,tab_rq_registers[2],"Setting.RPM",3,tab_rp_registers[2]);
		writeVariable(server,"Fan",4,"value.RPM",4,tab_rq_registers[3],"Setting.RPM",4,tab_rp_registers[3]);
		writeVariable(server,"Fan",5,"value.RPM",5,tab_rq_registers[4],"Setting.RPM",5,tab_rp_registers[4]);
	/*	
		writeVariable(server,"Fan",1,tab_rq_registers[0],"value.RPM",1);
		writeVariable(server,"Fan",2,tab_rq_registers[1],"value.RPM",2);
		writeVariable(server,"Fan",3,tab_rq_registers[2],"value.RPM",3);
		writeVariable(server,"Fan",4,tab_rq_registers[3],"value.RPM",4);
		writeVariable(server,"Fan",5,tab_rq_registers[4],"value.RPM",5);
		writeVariable(server,"Fan",1,tab_rp_registers[0],"Setting.RPM",1);
		writeVariable(server,"Fan",2,tab_rp_registers[1],"Setting.RPM",2);
		writeVariable(server,"Fan",3,tab_rp_registers[2],"Setting.RPM",3);
		writeVariable(server,"Fan",4,tab_rp_registers[3],"Setting.RPM",4);
		writeVariable(server,"Fan",5,tab_rp_registers[4],"Setting.RPM",5);
		
		//updateCurrentTime(server);
		*/
    }
	
	printf("test 2!!!!!!!!!!!!!!\r\n");
	//UA_StatusCode retval = UA_Server_run(server, &running);
	
	printf("test 3!!!!!!!!!!!!!!\r\n");
	UA_Server_run_shutdown(server);
	UA_Server_delete(server);
	//UA_ServerConfig_clean(config);
	printf("test 4!!!!!!!!!!!!!!\r\n");
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
	printf("test 5!!!!!!!!!!!!!!\r\n");
	

}
