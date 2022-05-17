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
#include <crack.h>
//If Need of logs?
#if defined(DEBUG)
FILE *LOG_FILE;
#endif
//Global Defines
//Path to cracklibs DICTPATH
#define CRACKLIB_DICTPATH "/var/cache/cracklib/cracklib_dict"
// standrard lcoation where libcrack installs: /usr/share/cracklib/pw_dict -ArchLinux

//Path to read config for this module
#define CONFIG_FILE "/tmp/pquality.conf"

#define MSG_MIN_LENGTH_ERROR "Password for dn=\"%s\" is too short (%d/%d)"
#define MSG_MAX_LENGTH_ERROR "Password for dn=\"%s\" is too large (%d/%d)"
#define MSG_MIN_LC_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) lowercase characters"
#define MSG_MIN_UC_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) upppercase characters"
#define MSG_MIN_DIGITS_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) digits"
#define MSG_MIN_SC_ERROR "Password for dn=\"%s\" should contain atleast (%d/%d) Special characters"
#define MSG_UNSUPPORTED_CHAR_ERROR "Password for dn=\"%s\" contains forbidden characters. "
#define MSG_USERNAME_VARIENT_ERROR "Password for dn=\"%s\" should not contain username inside"


//prototypes
static char* getValues(char*);
int check_password (char *pPasswd, char **ppErrStr, Entry *pEntry);
void abortAndRaise(char *msg,char *dn,int reality,int expected,char **ppErrStr);

bool startsWith(const char *pre, const char *str){
    size_t lenpre = strlen(pre),lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

char* getValues(char* parameter){
    FILE *f=fopen(CONFIG_FILE,"r");
    char line[50];
    while(fgets(line,50,f)!=NULL){
        if(line[0]=='#')continue;
        if(startsWith(parameter,line)){
            char subbuff[20]; 
            int k=strlen(line)-strlen(parameter)+1;
            
            memcpy( subbuff, &line[strlen(parameter)],k);
            subbuff[k] = '\0';
            char *str_to_ret = malloc (sizeof (char) * strlen(subbuff));
            str_to_ret = strdup (subbuff);
            return str_to_ret;
        }
    }
    return NULL;
}

void abortAndRaise(char *msg,char *dn,int reality,int expected,char **ppErrStr){
    char *ERROR_MSG = (char *) ber_memalloc(64);
    //int mem_len = realloc_error_message(&ERROR_MSG, 64,strlen(msg) +strlen(dn) + 1);
    ERROR_MSG=ber_memalloc(strlen(msg) +strlen(dn) + 1+64);
    sprintf (ERROR_MSG, msg, dn, reality,expected);

    #if defined(DEBUG)
    fprintf(LOG_FILE, ERROR_MSG);
    fclose(LOG_FILE);
    #endif

    *ppErrStr = strdup (ERROR_MSG);
	ber_memfree(ERROR_MSG);
}

int check_password (char *pPasswd, char **ppErrStr, Entry *pEntry){
    short config_value=0;
    int password_length=0;
    #if defined(DEBUG)
    LOG_FILE=fopen ("/tmp/qualityCheck_LDAP", "a");
    fprintf(LOG_FILE, "<------------------NEW-ENTRY------------>\n%s for %s\n", pPasswd,pEntry->e_name.bv_val);
    #endif

    //MinLength Check
    char* var=getValues("MIN_LENGTH");
    if(var!=NULL){
        config_value=atoi(var);
        password_length=strlen (pPasswd);
        
        #if defined(DEBUG)
        fprintf(LOG_FILE, "MIN_LENGTH: EXPECTED  %d Recived  %d\n", config_value,password_length);
        #endif

        if(password_length<config_value){
            free(var);
            abortAndRaise(MSG_MIN_LENGTH_ERROR,pEntry->e_name.bv_val,password_length,config_value,ppErrStr);
            return (EXIT_FAILURE);
        }
        free(var);
    }

    
    //MaxLength Check
    var=getValues("MAX_LENGTH");
    if(var!=NULL){
        config_value=atoi(var);

        #if defined(DEBUG)
        fprintf(LOG_FILE, "MAX_LENGTH: EXPECTED  %d Recived  %d\n", config_value,password_length);
        #endif

        if(password_length>config_value){
            free(var);
            abortAndRaise(MSG_MAX_LENGTH_ERROR,pEntry->e_name.bv_val,password_length,config_value,ppErrStr);
            return (EXIT_FAILURE);
        }
        free(var);
    }
    
    //MinNumberofLowerCases
    var=getValues("MIN_LOWER");
    if(var!=NULL){
        config_value=atoi(var);
        for(int i=0;i<password_length;i++){
            if(pPasswd[i]>='a' && pPasswd[i]<='z')config_value--;
        }

        #if defined(DEBUG)
        fprintf(LOG_FILE, "MIN_LOWER: EXPECTED  %d Recived  %d\n", atoi(var),atoi(var)-config_value);
        #endif

        if(config_value>0){
            abortAndRaise(MSG_MIN_LC_ERROR,pEntry->e_name.bv_val,atoi(var)-config_value,atoi(var),ppErrStr);
            free(var);
            return (EXIT_FAILURE);
        }
        free(var);
    }
    
    //MinNumberofUpperCases
    var=getValues("MIN_UPPER");
    if(var!=NULL){
        config_value=atoi(var);
        for(int i=0;i<password_length;i++){
            if(pPasswd[i]>='A' && pPasswd[i]<='Z')config_value--;
        }

        #if defined(DEBUG)
        fprintf(LOG_FILE, "MIN_UPPER: EXPECTED  %d Recived  %d\n", atoi(var),atoi(var)-config_value);
        #endif

        if(config_value>0){
            abortAndRaise(MSG_MIN_UC_ERROR,pEntry->e_name.bv_val,atoi(var)-config_value,atoi(var),ppErrStr);
            free(var);
            return (EXIT_FAILURE);
        }
        free(var);
    }
    
    //MinNumber of Digits
    var=getValues("MIN_DIGITS");
    if(var!=NULL){
        config_value=atoi(var);
        for(int i=0;i<password_length;i++){
            if(pPasswd[i]>='0' && pPasswd[i]<='9')config_value--;
        }

        #if defined(DEBUG)
        fprintf(LOG_FILE, "MIN_DIGITS: EXPECTED  %d Recived  %d\n", atoi(var),atoi(var)-config_value);
        #endif

        if(config_value>0){
            abortAndRaise(MSG_MIN_DIGITS_ERROR,pEntry->e_name.bv_val,atoi(var)-config_value,atoi(var),ppErrStr);
            free(var);
            return (EXIT_FAILURE);
        }
        free(var);
    }
    
    //Forbidden characters
    var=getValues("FORBIDDEN_CHAR");
    if(var!=NULL){
        for(int i=0;i<password_length;i++){
            for(int j=0;j<strlen(var);j++){
                if(var[j]==pPasswd[i] || (pPasswd[i]>=0 && pPasswd[i]<=32)){
                    free(var);
                    abortAndRaise(MSG_UNSUPPORTED_CHAR_ERROR,pEntry->e_name.bv_val,password_length,0,ppErrStr);
                    return (EXIT_FAILURE);
                }
            }
        }
        free(var);
    }
    //CrackLib -- dictionary and password varianets
    var=getValues("USE_CRACKLIB");
    if(var!=NULL){
        if(atoi(var)==1){
            char* CrackLib_Resposne=(char*)FascistCheck(pPasswd, CRACKLIB_DICTPATH);
            #if defined(DEBUG)
            fprintf(LOG_FILE, "CRACK_LIB says: %s", CrackLib_Resposne);
            fclose(LOG_FILE);
            #endif
            if(CrackLib_Resposne!=NULL) {
                *ppErrStr = strdup (CrackLib_Resposne);
                return (EXIT_FAILURE);
            }
        }
    }
    //ELSE-- send success response to ldapserver
    *ppErrStr = strdup ("");
	return (LDAP_SUCCESS);
}
