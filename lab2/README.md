# 安装
## 安装Apache2
sudo apt-get update
sudo apt-get install apache2

## 安装MySQL
sudo apt-get install mysql-server

## 安装php
sudo apt install php

# 数据库
 
输入sudo cat /etc/mysql/debian.cnf查看默认账户密码
## 连接数据库
mysql -u your_username -p
## 创建数据库
CREATE DATABASE users;
## 进入数据库
USE users;
## 创建表
CREATE TABLE user_info (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL,
    password VARCHAR(255) NOT NULL
);

# 网页
将两个php文件放在/var/www/html

在/etc/apache2/apache2.conf中添加
<IfModule dir_module>
    DirectoryIndex index.php index.html
</IfModule>

以上操作均在主机a上进行，在b的浏览器上输入a的ip地址就可以连接到a的网站
# 运行实验
主机a
在lab2文件夹中输入命令 
make
sudo insmod hello-world.ko
主机b
注册账号并登陆
在主机a上执行sudo dmesg查看账号密码

