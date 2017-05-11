#!/bin/sh
cd src
make
cd ..
mv src/backend backend
cd etc/transfer
make
cd ../..
chmod a+x etc/mail/send_mail.sh
