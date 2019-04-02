修改于Samuel-0-0（https://github.com/Samuel-0-0/phicomm_dc1-esphome）。

修正电流、功率显示问题，支持1000W以上大功率设备的正常使用，支持WEBSERVER、OTA升级。

## 「 关于DC1固件生成、刷写工具箱 」

此工具用于DC1固件的生成与刷写，请先接好线并将固件配置文件存放于config_yaml目录内。

- 项目的整体情况，请[参考文档内的说明](https://github.com/Samuel-0-0/phicomm_dc1-esphome)
- 配置文件请按照说明修改https://github.com/kukudemajia/phicomm_dc1_esphome/blob/master/config_yaml/dc1_config.yaml
![image](https://github.com/kukudemajia/phicomm_dc1_esphome/blob/master/esphome_config.png?raw=true)

- TTL刷固件的接线及进入刷写模式方法请[参考文档内的说明](https://github.com/Samuel-0-0/phicomm_dc1-esphome/blob/master/cookbook)。


以上步骤都完成后，打开工具，执行菜单：(1)编译固件，确认无报错后执行菜单：(3)升级DC1固件。

> 初次使用必须TTL线刷，后续可以通过WEBSERVER、OTA升级。
> 菜单(3)升级DC1固件 支持TTL和OTA双模式选择。

![image](https://github.com/kukudemajia/phicomm_dc1_esphome/blob/master/%E5%B7%A5%E5%85%B7%E7%95%8C%E9%9D%A2%E6%88%AA%E5%9B%BE.jpg?raw=true)


## 工具下载地址

右上角「 Clone or download 」  →  「 Download  ZIP 」


# 错误解决
遇到未知错误，请删除config_yaml文件夹内.esphome和build下与配置文件对应的文件和文件夹！

如下图的错误：
![image](https://github.com/kukudemajia/phicomm_dc1_esphome/blob/master/%E7%BC%96%E8%AF%91%E9%94%99%E8%AF%AF%E8%AF%B4%E6%98%8E.png?raw=true)
