gsnap is a tool which can capture screen (screenshot) base on framebuffer.

Compile for Linux:
```
$(CC) gsnap.c -ljpeg -lpng -o gsnap
```
So, you should install libjpeg.so and libpng.so firstly.

Usage:
```
gsnap <jpeg|png file> <framebuffer dev>
```

Example:
```
gsnap test.png /dev/fb0
```
