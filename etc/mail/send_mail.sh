#!/bin/bash

# Script used to send mail using openSSL and curl

RECEIVER_NAME=$1
RECEIVER_MAIL=$2
SUBJECT_LINE=$3
PASSWORD=$4
FILE_NAME=$5
NICE_NAME=$6
SENDER_MAIL=$7

cd etc/mail

echo "nbr of arguments: $#"
echo "subject line: $SUBJECT_LINE"

if [ "$#" -ne 7 ]; then
	echo "usage: receiver_name receiver_mail subject password filename nicename sendermail"
	exit 0
fi

echo "From: '$NICE_NAME' <$SENDER_MAIL>\nTo: '$RECEIVER_NAME' <$RECEIVER_MAIL>\nSubject: $SUBJECT_LINE " > mailheaders_$RECEIVER_MAIL.txt

cat mailheaders_$RECEIVER_MAIL.txt $FILE_NAME > fil.txt

# Sending the mail using curl
curl --url 'smtps://smtp.gmail.com:465' --ssl-reqd --mail-from "$RECEIVER_MAIL" --mail-rcpt "$RECEIVER_MAIL" --upload-file fil.txt --user "$SENDER_MAIL:$PASSWORD" --insecure > /dev/null

rm mailheaders_$RECEIVER_MAIL.txt