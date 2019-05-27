#!/bin/bash

lines=9  # 这个值是指这个脚本的行数加1，这个脚本共有8行
tail -n+$lines $0 > /tmp/hello.tar.bz2  # $0表示脚本本身，这个命令用来把从 $lines 开始的内容写入一个 /tmp 目录的 hello.tar.bz2 文件里
cd /tmp
tar jxvf hello.tar.bz2
cp hello /bin
exit 0
