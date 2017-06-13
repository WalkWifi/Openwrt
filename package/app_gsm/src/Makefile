# build uart_sim executable when user executes "make"

app_gsm: app_gsm.o
	$(CC) $(LDFLAGS) app_gsm.o -o app_gsm

app_gsm.o: app_gsm.c
	$(CC) $(CFLAGS) -c app_gsm.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o app_gsm
