版本信息

PAM: Linux-PAM-1.3.0
vsftpd: version 3.0.2

=========================================

Linux-PAM-1.3.0-lu.tar.bz2 是添加了广有席位网关动态密码验证的PAM包，其中动态密码验证位于模块 pam_permit，可执行如下命令进行修改。

# cd <your-dir>
# tar jxvf Linux-PAM-1.3.0-lu.tar.bz2
# cd Linux-PAM-1.3.0/modules/pam_permit/
# vi pam_permit.c

修改完之后

# cd <your-dir>/Linux-PAM-1.3.0
# ./configure --prefix=/<your-dir>/Linux-PAM-1.3.0-install
# make
# make install

编译完之后可以在 <your-dir>/Linux-PAM-1.3.0-install/lib/security/ 找到相应的模块。
