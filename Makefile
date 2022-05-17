CC=gcc
# PATH to read OpenLDAP headers
LDAP_INC=-I/usr/include/openldap/include -I/usr/include/openldap/servers/slapd -I/tmp/Installer/openldap-2.4.57/servers/slapd -I/tmp/Installer/openldap-2.4.57/include -I./ 
INCS=$(LDAP_INC)
LDAP_LIB=-lldap_r -llber
CRACKLIB_LIB=-lcrack
CC_FLAGS=-g -O2 -Wall -fpic
#Remove comment to have logs support
#DEBUG_OPT=-DDEBUG
OPT=$(CC_FLAGS)  $(DEBUG_OPT)
LIBS=$(LDAP_LIB) $(CRACKLIB_LIB)
#WhereToInstall binary...Osxnia location: /usr/lib/ldap. Archlinux: /usr/lib/openldap
LIBDIR=/usr/lib/ldap/

all: 	check_password

check_password.o:
	$(CC) $(OPT) -c $(INCS) qualityChecker.c

check_password: clean check_password.o
	$(CC) -shared -o passwordqualityChecker.so qualityChecker.o $(CRACKLIB_LIB)

install: check_password
	cp -f passwordqualityChecker.so $(LIBDIR)
	cp -f pquality.conf /etc/ldap/pquality.conf 

clean:
	$(RM) qualityChecker.o passwordqualityChecker.so passwordqualityChecker.lo
	$(RM) -r .libs

