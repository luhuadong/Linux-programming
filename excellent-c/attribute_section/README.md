通过命令“ld --verbose”获取默认链接脚本

在 .bss 的数据段前添加自定义数据段：

```
  _init_start = .;
  .myinit  :
  {
	*(.myinit)
  }
  _init_end = .;
```

"_init_start"和"_init_end"是我们用于识别数据段开始和结束的在链接脚本中定义的变量，而".myinit"则是数据段的名称，其中 `.myinit:{*(.myinit)}` 表示 .o 中的 .myinit 数据段（输入段）保存到 bin 中的 .myinit 数据段（输出段）中。

编译：

```
$ gcc -c section -o section.o
```

执行如下命令查看段信息：

```
$ readelf -S section.o
```

链接成可执行的 bin 文件：

```
$ gcc -T ldscript.lds section.o -o section
```

执行如下命令查看 bin 文件的段分布情况：

```
$ readelf -S section
```

运行应用程序：

```
$ ./section
```

