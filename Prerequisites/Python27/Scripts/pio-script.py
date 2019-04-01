#!E:\esphome-tools-dc1-master\Prerequisites\Python27\python.exe
# EASY-INSTALL-ENTRY-SCRIPT: 'platformio==3.6.5','console_scripts','pio'
__requires__ = 'platformio==3.6.5'
import re
import sys
from pkg_resources import load_entry_point

if __name__ == '__main__':
    sys.argv[0] = re.sub(r'(-script\.pyw?|\.exe)?$', '', sys.argv[0])
    sys.exit(
        load_entry_point('platformio==3.6.5', 'console_scripts', 'pio')()
    )
