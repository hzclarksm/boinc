#!/bin/sh

scriptPath=$(readlink -f $(dirname $0))

libPath=${scriptPath}/../lib
dbPath=${scriptPath}/../db
mysqlIncludePath=/usr/include/mysql
mysqlLibPath=/usr/lib64/mysql

cd ${scriptPath}

g++ -I.. -I${libPath} -I${dbPath} -I${mysqlIncludePath} \
    gzip_validator.cpp \
    validator.o validate_util.o validate_util2.o libsched.a \
    -L${libPath} -lboinc -lboinc_crypt -L${mysqlLibPath} -lmysqlclient \
    -o gzip_validator
