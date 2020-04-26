import common as c
from config import *
import os
import sys
import subprocess as sub
import shutil
from glob import glob

if len(sys.argv) > 1 and sys.argv[1] == 'glibc_version':  # subcommand
    sub.run('ldd --version | head -n 1 | grep -Po "\\d\\.\\d\\d"', shell=True)
    exit(0)

artifact_name = '{}-{}.AppImage'.format(app_name, app_version)
if len(sys.argv) > 1 and sys.argv[1] == 'artifact_name':  # subcommand
    c.print(artifact_name)
    exit(0)
artifact_path = os.path.abspath(artifact_name)

c.print('>> Making appimage')

base_url = 'https://github.com/probonopd/linuxdeployqt/releases/download'
continuous_url = base_url + '/continuous/linuxdeployqt-continuous-x86_64.AppImage'
tagged_url = base_url + '/6/linuxdeployqt-6-x86_64.AppImage'
linuxdeployqt_url = tagged_url
linuxdeployqt_original = os.path.basename(linuxdeployqt_url)

c.download(linuxdeployqt_url, linuxdeployqt_original)
c.run('chmod a+x {}'.format(linuxdeployqt_original))

linuxdeployqt_bin = os.path.abspath('linuxdeployqt')
c.symlink(linuxdeployqt_original, linuxdeployqt_bin)

os.chdir(build_dir)

install_dir = os.path.abspath('appdir')
c.recreate_dir(install_dir)
c.run('make INSTALL_ROOT={0} DESTDIR={0} install'.format(install_dir))

if c.is_inside_docker():
    c.run('{}  --appimage-extract'.format(linuxdeployqt_bin))
    linuxdeployqt_bin = os.path.abspath('squashfs-root/AppRun')

os.environ['LD_LIBRARY_PATH'] = dependencies_dir + '/lib'
os.environ['VERSION'] = app_version
# debug flags: -unsupported-bundle-everything -unsupported-allow-new-glibc
flags = '' if os.getenv("DEBUG") is None else '-unsupported-allow-new-glibc'

out_lib_dir = install_dir + '/usr/lib'
os.makedirs(out_lib_dir, exist_ok=True)
for f in glob(ssl_dir + '/lib/lib*.so.*'):
    c.print('>> Copying {} to {}'.format(f, out_lib_dir))
    shutil.copy(f, out_lib_dir)

c.run('{} {}/usr/share/applications/*.desktop {} -appimage -qmake={}/bin/qmake'.format(
    linuxdeployqt_bin, install_dir, flags, qt_dir))

c.run('mv {}-{}*.AppImage "{}"'.format(app_name, app_version, artifact_path))

bin_path = install_dir + '/usr/bin/' + bin_name
c.print('>> Md5 {} {}'.format(bin_path, c.md5sum(bin_path)))
