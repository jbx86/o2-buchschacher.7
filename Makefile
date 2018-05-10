all: oss user

oss: oss.c proj7.h
	gcc -o oss oss.c -lrt

user: user.c proj7.h
	gcc -o user user.c -lrt
