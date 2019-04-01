# -*- coding: utf-8 -*-

from esphome.const import __version__
import requests
import sys
type = sys.getfilesystemencoding()

def main():
    #local_version = '1.0'
    local_version = __version__
    online_version = requests.get('https://pypi.python.org/pypi/esphome/json').json()['info']['version']
    #print online_version
    if local_version == online_version:
        a = u'已安装最新版本: %s'%local_version
        #print a.decode('UTF-8').encode(type)
        print a.encode(type)
    else:
        b = u'有新版本(%s)可升级！'%online_version
        #print b.decode('UTF-8').encode(type)
        print b.encode(type)

if __name__ == '__main__':
    main()