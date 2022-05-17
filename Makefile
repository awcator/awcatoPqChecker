CC=gcc
# PATH to read OpenLDAP headers
LDAP_INC=-I/tmp/Installer/openldap-2.4.57/include \
	 -I/tmp/Installer/openldap-2.4.57/openldap/servers/slapd
INCS=$(LDAP_INC)
LDAP_LIB=-lldap_r -llber
CRACKLIB_LIB=-lcrack
CC_FLAGS=-g -O2 -Wall -fpic
#set this if you want logs support or comment it
DEBUG_OPT=-DDEBUG
OPT=$(CC_FLAGS)  $(DEBUG_OPT)
LIBS=$(LDAP_LIB) $(CRACKLIB_LIB)
#WhereToInstall binary...Osxnia location: /usr/lib/ldap. Archlinux: /usr/lib/openldap
LIBDIR=/usr/lib/openldap/

all: 	check_password

check_password.o:
	$(CC) $(OPT) -c $(INCS) check_password.c

check_password: clean check_password.o
	$(CC) -shared -o check_password.so check_password.o $(CRACKLIB_LIB)

install: check_password
	cp -f check_password.so $(LIBDIR)

clean:
	$(RM) check_password.o check_password.so check_password.lo
	$(RM) -r .libs

