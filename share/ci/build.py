import common as c
from config import *
import os
import platform

c.print('>> Building {} on {}'.format(app_name, os_name))

c.add_to_path(os.path.abspath(qt_dir + '/bin'))
os.environ['ST_DEPS_DIR'] = dependencies_dir

if platform.system() == "Windows":
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)

c.recreate_dir(build_dir)
os.chdir(build_dir)

c.run('lupdate "{}"'.format(pro_file))
c.run('lrelease "{}"'.format(pro_file))

c.set_make_threaded()
build_type_flag = 'debug' if build_type == 'debug' else 'release'
qmake_flags = os.environ.get('QMAKE_FLAGS','') + ' CONFIG+=' + build_type_flag
c.run('qmake {} "{}"'.format(qmake_flags, pro_file))
make_cmd = c.get_make_cmd()
c.run(make_cmd)
