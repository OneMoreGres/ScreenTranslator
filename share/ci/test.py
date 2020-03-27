import common as c
from config import *
import os
import platform
import glob

c.print('>> Testing {} on {}'.format(app_name, os_name))

c.add_to_path(os.path.abspath(qt_dir + '/bin'))

if platform.system() == "Windows":
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)

c.recreate_dir(build_dir)
os.chdir(build_dir)

c.set_make_threaded()
c.run('qmake {} "{}"'.format(os.environ.get('QMAKE_FLAGS', ''), test_pro_file))
make_cmd = c.get_make_cmd()
c.run(make_cmd)

for file in glob.glob('./**/tests*', recursive=True):
    print(file)
    c.run(file, silent=False)
