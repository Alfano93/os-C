/* This file contains the nessesary libraries for the (whatever this is called)
   as well as the authentication information of the Openstack enviroment */

#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <jansson.h>
#include <curl/curl.h> 

/* system definitions like buffer size */ 

#define BUFFER_SIZE    (256 * 1024) /* 256 KB */ 
#define URL_SIZE       256

/*Openstack enviroment variables */

#define OS_USERNAME    "your_username"
#define OS_PASSWORD    "your_password"
#define OS_TENANT_NAME "your_tenant_name"
#define OS_AUTH_URL    "your_auth_url" /* typically found in the project's auth and security tab of horizon */ 
#define OS_NO_CACHE    1 


