#ifndef __MQTTS__
#define __MQTTS__

#define JSON_CERTIFICATE_LEN    4096

#define CLIENT_CERT_MAX_LEN     2048
#define CLIENT_CERT_DATA_ADDR  
#define CLIENT_CERT_LEN_ADDR 

#define CLIENT_KEY_MAX_LEN      2048
#define CLIENT_KEY_DATA_ADDR  
#define CLIENT_KEY_LEN_ADDR

#define CA_CERT_MAX_LEN         2048
#define CA_CERT_DATA_ADDR       
#define CA_CERT_LEN_ADDR

void mqtt_app_start(void);

#endif