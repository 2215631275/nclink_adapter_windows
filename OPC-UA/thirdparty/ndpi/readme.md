## 编译代码：
```
linux:
gcc pcap_to_flows.c -o pcap -I ./include/ ./lib/libndpi.a -lpcap -lm
windows: 
1.安装msys2
2.安装npcap，将wpcap.lib和Packet.lib复制到./nDPI/src/lib中
3.在MSYS2 MINGW64中用管理员权限打开输入以下命令：
gcc pcap_to_flows.c -o pcap.exe -I ./include/ ./lib/libndpi.lib ./lib/wpcap.lib -lws2_32
或者直接make
```
## 功能介绍：
1. 分析静态包
```
sudo ./pcap <包名称> <输出文件名称.csv>
```
2. 动态抓包
```
sudo ./pcap
//然后根据提示选择网卡，结果保存在test.csv文件中
```

## 修改流量解析文件
```
进入./lib/protocols/ 找到对应的协议进行修改
修改完成后返回../lib/目录下执行：
./make
```