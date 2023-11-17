实验需要两台虚拟机

运行代码的主机需要开启混杂模式

ifconfig ens33 promisc  (ens33是网卡的名字，可以用ifconfig查看网卡信息)

需要输入的四个参数分别为
网关ip
受害者ip
封锁ip（不想让受害者发往的那个ip）
定向ip（攻击者想要受害者发往的ip）
