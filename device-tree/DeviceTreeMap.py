#!/usr/bin/env python
# -*- coding=utf8 -*-
"""
# Author: wanghan
# Created Time : Sun 07 Jan 2018 06:27:32 PM CST
# File Name: DeviceTreeMap.py
# Description: Get Devices Tree Map
"""

import sys
import os
import re
import getopt

g_dt_file = ""
g_more_info = False
g_devicetreemap_version = "2.1.0"

def usage():
    print(
"""
    Usage:
        DeviceTreeMap.py -f [dts/dtsi]

        -h or --help：显示帮助信息
        -v or --version：显示版本
        -f or --file：指定dts/dtsi文件
""")

class DeviceTreeMap(object):
    def __init__(self, filepath, parent = None, level = 0):
        self.name = os.path.basename(filepath)
        self.dir = os.path.dirname(os.path.abspath(filepath))
        self.filepath = os.path.abspath(filepath)
        self.level = level
        self.parent = parent
        self.child = []

    def printChild(self):
        n = 0
        if not self.level == 0:
            if self.level > 1:
                while self.level > n:
                    if len(self.parent.parent.child) > self.parent.parent.child.index(self.parent)+1:
                        if self.level != n+1 :
                            print("\t", end=' ')
                    else:
                        if self.level != n+1 :
                            print("\t", end=' ')
                    n = n + 1
            print("\t" + self.name)
        else:
            print(self.name)
        n = 0
        while len(self.child) > n:
            self.child[n].printChild()
            n = n + 1

    def printMap(self):
        print("")
        print("-----------------------------------")
        print("")
        self.printChild()
        print("")
        print("-----------------------------------")
        print("")

    def createTreeMap(self):
        theChild = None
        try:
            for line in open(self.filepath):
                #if line.startswith("#include"):
                res = re.match('^\t*?\#include', line) # 按正则表达式，逐行遍历dts/dtsi文件
                if (res != None):
                    #print(line) # 打印符合正则表达式的行
                    dtsiName = line.split()[1].strip('\"') # 获取include关键字后面的文件名
                    if not dtsiName.endswith(".h>"):
                        if g_more_info:
                            print("[%d] %s/%s" %(self.level, self.dir, dtsiName)) # 打印本文件include的dtsi文件
                        theChild = DeviceTreeMap(os.path.join(self.dir, dtsiName), self, self.level+1)
                        self.child.append(theChild)
                        theChild.createTreeMap()
                    pass
                pass
            pass
        except FileNotFoundError:
            print("Warning : Not Fount %s !" %self.filepath)

if __name__ == '__main__':

    if len(sys.argv)  == 1:
        usage()
        sys.exit(-1)

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hvf:m", ["help", "version", "file=", "more_info"])   # sys.argv[1:] 过滤掉第一个参数(它是脚本名称，不是参数的一部分)
    except getopt.GetoptError:
        print("argv error,please input")

    for cmd, arg in opts:  # 使用一个循环，每次从opts中取出一个两元组，赋给两个变量。cmd保存选项参数，arg为附加参数。接着对取出的选项参数进行处理。
        if cmd in ("-h", "--help"):
            usage()
            sys.exit()
        elif cmd in ("-v", "--version"):
            print("Version : %s"  % g_devicetreemap_version)
            sys.exit()
        elif cmd in ("-f", "--file"):
            g_dt_file = os.path.abspath(arg)
        elif cmd in ("-m", "--more_info"):
            g_more_info = True

    if g_dt_file == "":
        print("Error : Not found (.dts or .dtsi) file!")
        sys.exit(-1)

    #print("g_dt_file:" + g_dt_file)

    rootDeviceTree = DeviceTreeMap(g_dt_file)
    #print("name    :" + rootDeviceTree.name)
    #print("filepath:" + rootDeviceTree.filepath)
    rootDeviceTree.createTreeMap()
    rootDeviceTree.printMap()
