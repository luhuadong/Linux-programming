## 说明

先将 A1.c 和 A2.c 编译成目标文件（x.o）

```shell
gcc -c A1.c A2.c
```

接着生成静态库文件（x.a）

```shell
ar crv libafile.a A1.o A2.o
```

使用静态库，编译可执行文件

```shell
gcc -o test test.c libafile.a
```

也可以这么指定

```shell
gcc -o test test.c -L./ -lafile
```



