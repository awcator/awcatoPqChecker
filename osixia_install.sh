#!/bin/bash
mkdir -p /tmp/Installer/
cd /tmp/Installer/
#apt install gcc
#apt install libdb-dev sudo apt install libltdl-dev
#apt install libpwquality-dev cracklib-runtime libdb-dev libcrack2-dev
wget https://www.openldap.org/software/download/OpenLDAP/openldap-release/openldap-2.4.57.tgz
tar -xvf openldap-2.4.57.tgz
cd openldap-2.4.57/
./configure && make depend
cd ..
git clone https://github.com/awcator/awcatoPqChecker
cd awcatoPqChecker
make install