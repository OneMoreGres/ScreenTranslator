import common as c
from config import *
import os
import sys
import shutil
from glob import glob

tag = os.environ.get('TAG', '')
artifact_name = '{}-{}{}-{}.zip'.format(app_name, app_version, tag, os_name)
if len(sys.argv) > 1 and sys.argv[1] == 'artifact_name':  # subcommand
    c.print(artifact_name)
    exit(0)
artifact_path = os.path.abspath(artifact_name)

c.print('>> Making win deploy')

if os_name.startswith('win'):
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)

pwd = os.getcwd()
os.chdir(build_dir)

install_dir = os.path.abspath(app_name)
c.recreate_dir(install_dir)

c.run('nmake INSTALL_ROOT="{0}" DESTDIR="{0}" install'.format(install_dir))
c.run('{}/bin/windeployqt.exe "{}"'.format(qt_dir, install_dir))

vcredist_for_ssl_url = ''
vcredist_for_ssl_file = ''
if bitness == '32':
    vcredist_for_ssl_url = 'https://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/vcredist_x86.exe'
    vcredist_for_ssl_file = 'vc_redist.x86.2010.exe'
else:
    vcredist_for_ssl_url = 'https://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/vcredist_x64.exe'
    vcredist_for_ssl_file = 'vc_redist.x64.2010.exe'

c.download(vcredist_for_ssl_url, os.path.join(install_dir, vcredist_for_ssl_file))

libs_dir = os.path.join(dependencies_dir, 'bin')
for file in os.scandir(libs_dir):
    if file.is_file(follow_symlinks=False) and file.name.endswith('.dll'):
        full_name = os.path.join(libs_dir, file.name)
        c.print('>> Copying {} to {}'.format(full_name, install_dir))
        shutil.copy(full_name, install_dir)

for f in glob(ssl_dir + '/bin/*.dll'):
    c.print('>> Copying {} to {}'.format(f, install_dir))
    shutil.copy(f, install_dir)

open(os.path.join(install_dir, 'qt.conf'), 'a').close() # fix for non-latin paths

c.archive(c.get_folder_files(os.path.relpath(install_dir)), artifact_path)

bin_path = install_dir + '\\' + bin_name + '.exe'
c.print('>> Md5 {} {}'.format(bin_path, c.md5sum(bin_path)))
