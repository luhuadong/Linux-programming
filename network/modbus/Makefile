
CFLAG=-lmodbus

all: modbus_tcp_test.c modbus_rtu_test.c
	gcc -Wall modbus_tcp_test.c -o modbus_tcp_test $(CFLAG)
	gcc -Wall modbus_rtu_test.c -o modbus_rtu_test $(CFLAG)

clean:
	rm -rf *.o modbus_tcp_test modbus_rtu_test

