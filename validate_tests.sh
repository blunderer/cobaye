#!/bin/sh

INPUTFILE=$1

# test printfs
ALLOWED_FUNC="-e cobaye_printf( -e sprintf( -e snprintf( -e vsprintf( -e vsnprintf( -e fprintf( -e vfprintf( -e cobaye_scanf( -e sscanf( -e fscanf( -e vfscanf("
FORBIDDEN_FUNC="printf( scanf( getchar( gets( putchar( puts("
FORBIDDEN_STREAMS="stderr stdout stdin"
FORBIDDEN_STREAMS_FUNCS="-e fprintf( -e vfprintf( -e fscanf( -e vfscanf( -e fwrite( -e fread("

ERROR=$(
for PATTERN in $FORBIDDEN_FUNC; do
	grep -nhs $PATTERN $INPUTFILE | grep -v $ALLOWED_FUNC | cut -d: -f1 2> /dev/null | while read line; do
		echo "Error: $INPUTFILE line $line: test files must not use this function. Use either cobaye_printf or cobaye_scanf for user interaction\n"
	done
done 
for PATTERN in $FORBIDDEN_STREAMS; do
	grep -nhs $PATTERN $INPUTFILE | grep $FORBIDDEN_STREAMS_FUNCS | cut -d: -f1 2> /dev/null | while read line; do
		echo "Error: $INPUTFILE line $line: test files must not read or write from stdin/stdout/stderr. Use either cobaye_printf or cobaye_scanf for user interaction\n"
	done
done 
) 

if [ -n "$ERROR" ]; then
	echo -e $ERROR | while read error; do
		echo $error
	done
	exit 1
fi

exit 0
