
#Note

LibModbus is a simple modbus toolbox for Linux.
This library contents master, slave and serial port configurationroutines.

You have to install libmodbus for Linux first, in Ubuntu like that:

```
sudo apt-get install libmodbus-dev 
```

After installing libmodbus, just type ```#include <modbus.h>``` on the top
of your program and compile it with the following line:

```
gcc -o my_program my_program.c -lmodbus
```

More detail: http://pes.free.fr/
