#!/bin/sh
cd src
chmod a+r password.h
make
chmod a-r password.h
cd ..
mv src/backend backend
cd etc/transfer
make
cd ../..
chmod a+x etc/mail/send_mail.sh
