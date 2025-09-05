nclink-适配器（无证书）
 **最新代码请看其他分支** 
包含nclink、mtconnect、opc-ua等协议适配器
和有证书的所有功能保持一致
后续任务：
<<<<<<< HEAD
1.流量采集功能还未集成（3月前）(已完成)
2.移植到windows平台（先尝试适配器）（已完成）
=======
1.流量采集功能还未集成（3月前）
2.移植到windows平台（先尝试适配器）  
  
mosquitto默认运行在localhost only mode，为了实现其他设备到mqtt服务器的连接需要修改配置文件,修改内容如下
```
# Place your local configuration in /etc/mosquitto/conf.d/
#
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.example

pid_file /run/mosquitto/mosquitto.pid

persistence true
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log

include_dir /etc/mosquitto/conf.d

log_timestamp true
log_timestamp_format [%Y-%m-%d %H-%M:%S]

allow_anonymous true
listener 1883 0.0.0.0
```
增加了对匿名的支持和监听设置，（当然也可以不允许匿名然后增加认证文件的设置  

按照官方要求，应该重新编写一个配置文件放在`/etc/mosquitto/conf.d`下，不过想在默认配置中直接改也可以
>>>>>>> 3ac98901868e9eb493a7192993cc21eb4a12b52f




