import common as c
from config import ssl_dir, os_name
import sys
import xml.etree.ElementTree as ET

c.print('>> Downloading ssl for Qt for {}'.format(os_name))

if os_name == 'linux':
    os_url = 'linux_x64'
    tool_name = 'tools_openssl_x64'
    root_path = 'Tools/OpenSSL/binary'
elif os_name == 'win32':
    os_url = 'windows_x86'
    tool_name = 'tools_openssl_x86'
    root_path = 'Tools/OpenSSL/Win_x86'
elif os_name == 'win64':
    os_url = 'windows_x86'
    tool_name = 'tools_openssl_x64'
    root_path = 'Tools/OpenSSL/Win_x64'
elif os_name == 'macos':
    exit(0)

base_url = 'https://download.qt.io/online/qtsdkrepository/{}/desktop/{}' \
    .format(os_url, tool_name)
updates_file = 'Updates-{}-{}.xml'.format(tool_name, os_name)
c.download(base_url + '/Updates.xml', updates_file)

updates = ET.parse(updates_file)
updates_root = updates.getroot()
url = ''
file_name = ''
for i in updates_root.iter('PackageUpdate'):
    name = i.find('Name').text
    if not 'qt.tools.openssl' in name:
        continue

    archives = i.find('DownloadableArchives')
    if archives.text is None:
        continue

    version = i.find('Version').text
    url = base_url + '/' + name + '/' + version + archives.text
    file_name = archives.text

if len(url) == 0:
    c.print('>> No ssl url found')
    exit(1)

c.download(url, file_name)
c.extract(file_name, '.')

c.symlink(root_path, ssl_dir)
