import common as c
from config import *
import os
import sys

artifact_name = '{}-{}.dmg'.format(app_name, app_version)
if len(sys.argv) > 1 and sys.argv[1] == 'artifact_name':  # subcommand
    c.print(artifact_name)
    exit(0)
artifact_path = os.path.abspath(artifact_name)

c.print('>> Making mac deploy')

os.chdir(build_dir)
build_target = build_dir + '/' + target_name + '.app'
built_dmg = build_dir + '/' + target_name + '.dmg'
c.run('{}/bin/macdeployqt "{}" -dmg'.format(qt_dir, build_target))
os.rename(built_dmg, artifact_path)
