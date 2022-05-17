/**
 * OpenLdap custom module for password quality checking 
 */
//standrard Libs
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
//From openldap Source
#include <portable.h>
#include <slap.h>
//From crakLib 
#include "crack.h"
//If Need of logs?
#if defined(DEBUG)
#include <debug_loger.h>
#endif
//Global Defines
//Path to cracklibs DICTPATH
#define CRACKLIB_DICTPATH "/usr/share/cracklib/pw_dict"
//Path to read config for this module
#define CONFIG_FILE "/etc/ldap/pquality.conf"

#define MSG_MIN_LENGTH_ERROR "Password for dn=\"%s\" is too short (%d/%d)"
#define MSG_MAX_LENGTH_ERROR "Password for dn=\"%s\" is too large (%d/%d)"
#define MSG_MIN_LC_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) lowercase characters"
#define MSG_MIN_UC_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) upppercase characters"
#define MSG_MIN_SC_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) Special characters"
#define MSG_UNSUPPORTED_CHAR_ERROR "Password for dn=\"%s\" contains forbidden characters. "
#define MSG_USERNAME_VARIENT_ERROR "Password for dn=\"%s\" should not contain username inside"
//prototypes
static int getValues(char*);
int check_password (char *pPasswd, char **ppErrStr, Entry *pEntry);
void abortAndRaise(char *msg,char *dn,int reality,int expected,char **ppErrStr);

bool startsWith(const char *pre, const char *str){
    size_t lenpre = strlen(pre),lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

int getValues(char* parameter){
    FILE *f=fopen(CONFIG_FILE,"r");
    char line[50];
    while(fgets(line,50,f)!=NULL){
        if(line[0]=='#')continue;
        if(startsWith(parameter,line)){
            char subbuff[20]; 
            int k=strlen(line)-strlen(parameter)+1;
            memcpy( subbuff, &line[strlen(parameter)],k);
            subbuff[k] = '\0';
            return atoi(subbuff);
        }
    }
    return -1;
}

void abortAndRaise(char *msg,char *dn,int reality,int expected,char **ppErrStr){
    char *ERROR_MSG = (char *) ber_memalloc(64);
    //int mem_len = realloc_error_message(&ERROR_MSG, 64,strlen(msg) +strlen(dn) + 1);
    ERROR_MSG=ber_memalloc(strlen(msg) +strlen(dn) + 1+64);
    sprintf (ERROR_MSG, msg, dn, reality,expected);
    *ppErrStr = strdup (ERROR_MSG);
	ber_memfree(ERROR_MSG);
}

int check_password (char *pPasswd, char **ppErrStr, Entry *pEntry){
    //MinLength Check
    short config_value=getValues("MIN_LENGTH");
    int password_length=strlen (pPasswd);
    if(password_length<config_value){
        abortAndRaise(MSG_MIN_LENGTH_ERROR,pEntry->e_name.bv_val,password_length,config_value,ppErrStr);
        return (EXIT_FAILURE);
    }

    //MaxLength Check
    short config_value=getValues("MAX_LENGTH");
    int password_length=strlen (pPasswd);
    if(password_length>config_value){
        abortAndRaise(MSG_MAX_LENGTH_ERROR,pEntry->e_name.bv_val,password_length,config_value,ppErrStr);
        return (EXIT_FAILURE);
    }

    //MinNumberofLowerCases
    short config_value=getValues("MIN_LOWER");
    int password_length=strlen (pPasswd);
    for(int i=0;i<password_length;i++){
        if(pPasswd[i]>='a' && pPasswd<='z')config_value--;
    }
    if(config_value>0){
        abortAndRaise(MSG_MIN_LC_ERROR,pEntry->e_name.bv_val,password_length,config_value,ppErrStr);
        return (EXIT_FAILURE);
    }

    //MinNumberofUpperCases
    short config_value=getValues("MIN_UPPER");
    int password_length=strlen (pPasswd);
    for(int i=0;i<password_length;i++){
        if(pPasswd[i]>='A' && pPasswd<='Z')config_value--;
    }
    if(config_value>0){
        abortAndRaise(MSG_MIN_UC_ERROR,pEntry->e_name.bv_val,password_length,config_value,ppErrStr);
        return (EXIT_FAILURE);
    }

    else{
        *ppErrStr = strdup ("");
	    return (LDAP_SUCCESS);
    }
    *ppErrStr = strdup ("");
	return (LDAP_SUCCESS);
}
