import common as c
from config import app_version
import sys
import os
import io
import urllib
import platform
from paramiko import SSHClient, WarningPolicy, RSAKey, SSHException

files = sys.argv[1:]
c.print('>> Uploading artifacts to sourceforge {}'.format(files))

for f in files:
    if not os.path.exists(f):
        c.print('>> File "{}" not exists. Exiting'.format(f))
        exit(0)

pkey_name = 'SF_PKEY'
if not pkey_name in os.environ:
    c.print('>> No sf pkey set. Exiting')
    exit(0)

api_name = 'SF_API'
if not api_name in os.environ:
    c.print('>> No sf api set. Exiting')
    exit(0)

pkey_data = io.StringIO(os.environ[pkey_name])
pkey = None
try:
    pkey = RSAKey.from_private_key(pkey_data)
except SSHException as e:
    c.print('>> Sf pkey error "{}". Exiting'.format(e))
    exit(0)

ssh = SSHClient()
ssh.set_missing_host_key_policy(WarningPolicy())
ssh.connect('frs.sourceforge.net', username='onemoregres', pkey=pkey)
sftp = ssh.open_sftp()
target_path = 'test' # 'bin/v' + app_version
try:
    remote_path = '/home/frs/project/screen-translator/'
    for part in target_path.split('/'):
        existing = sftp.listdir(remote_path)
        remote_path = remote_path + part + '/'
        if not part in existing:
            sftp.mkdir(remote_path)
    existing = sftp.listdir(remote_path)
    for f in files:
        file_name = os.path.basename(f)
        if file_name in existing:
            c.print('>> File "{}" already exists. Removing'.format(file_name))
            sftp.remove(remote_path + file_name)
        sftp.put(f, remote_path + file_name)
except IOError as err:
    c.print('>> SFTP error "{}". Exiting'.format(err))
    exit(0)

sftp.close()
ssh.close()

api_key = os.environ[api_name]
base_url = 'https://sourceforge.net/projects/screen-translator/files/' + target_path
for f in files:
    file_name = os.path.basename(f)
    url = base_url + '/' + file_name

    data = {'api_key': api_key}
    if platform.system() == "Windows":
        data['default'] = 'windows'
    elif platform.system() == "Darwin":
        data['default'] = 'mac'
    else:
        data['default'] = 'linux'

    raw_data = urllib.parse.urlencode(data).encode('utf-8')
    try:
        request = urllib.request.Request(
            url, method='PUT', headers={"Accept": "application/json"}, data=raw_data)
        with urllib.request.urlopen(request) as r:
            pass
        c.print('>> Updated info for "{}"'.format(url), r.status, r.reason)
    except Exception as e:
        c.print('>> Update info for "{}" failed {}'.format(url, e))
