all: write session

write: 
	gcc write.c ../../src/notif.c -o writer

session:
	gcc session.c ../../src/notif.c ../../src/rsa.c ../../src/sec_session.c -o session