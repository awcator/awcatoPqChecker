#include <stdio.h>
#include <stdlib.h>
#define LOG_PATH "/var/log/ldap_password_quality_check"
#define LOG_MSG(msg) { 
   FILE* fp=fopen (LOG_PATH, "w+");
   fprintf(fp,"%s [%s:%d]\n", msg, __FILE__, __LINE__); 
   fclose(fp);
}