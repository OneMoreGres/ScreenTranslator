import common as c
from config import bitness, msvc_version, build_dir, dependencies_dir, build_type
import os
import platform

c.print('>> Installing leptonica')

install_dir = dependencies_dir
url = 'https://github.com/DanBloomberg/leptonica/releases/download/1.82.0/leptonica-1.82.0.tar.gz'
required_version = '1.82.0'


build_type_flag = 'Debug' if build_type == 'debug' else 'Release'

cache_file = install_dir + '/leptonica.cache'
cache_file_data = required_version + build_type_flag


def check_existing():
    if not os.path.exists(cache_file):
        return False
    with open(cache_file, 'r') as f:
        cached = f.read()
        if cached != cache_file_data:
            return False

    if platform.system() == "Windows":
        dll = install_dir + '/bin/leptonica-1.82.0.dll'
        lib = install_dir + '/lib/leptonica-1.82.0.lib'
        if not os.path.exists(dll) or not os.path.exists(lib):
            return False
        c.symlink(dll, install_dir + '/bin/leptonica.dll')
        c.symlink(lib, install_dir + '/lib/leptonica.lib')
    elif platform.system() == "Darwin":
        lib = install_dir + '/lib/libleptonica.1.82.0.dylib'
        if not os.path.exists(lib):
            return False
        c.symlink(lib, install_dir + '/lib/libleptonica.dylib')
    else:
        if not os.path.exists(install_dir + '/lib/libleptonica.so'):
            return False

    includes_path = install_dir + '/include/leptonica'
    if len(c.get_folder_files(includes_path)) == 0:
        return False

    version_file = install_dir + '/lib/cmake/leptonica/LeptonicaConfig-version.cmake'
    if not os.path.exists(version_file):
        return False

    with open(version_file, 'rt') as f:
        existing_version = f.readline()[22:28]  # set(Leptonica_VERSION 1.82.0)
        if existing_version != required_version:
            return False
    return True


if check_existing():
    c.print('>> Using cached')
    exit(0)

archive = os.path.basename(url)
c.download(url, archive)

src_dir = os.path.abspath('leptonica_src')
c.extract(archive, '.')
c.symlink(c.get_archive_top_dir(archive), src_dir)

with open('{}/CMakeLists.txt'.format(src_dir), 'r+') as f:
    data = f.read()
    data = data.replace('pkg_check_modules(WEBP', '#pkg_check_modules(WEBP')
    data = data.replace('if(NOT WEBP', 'if(FALSE')
    f.seek(0, os.SEEK_SET)
    f.write(data)

c.ensure_got_path(install_dir)

c.recreate_dir(build_dir)
os.chdir(build_dir)

cmake_args = '"{}" -DCMAKE_INSTALL_PREFIX="{}" -DBUILD_SHARED_LIBS=ON \
-DSW_BUILD=OFF'.format(src_dir, install_dir,)

if platform.system() == "Windows":
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)
    cmake_args += ' ' + c.get_cmake_arch_args(bitness=bitness)

c.set_make_threaded()
c.run('cmake {}'.format(cmake_args))
build_type_flag = 'Debug' if build_type == 'debug' else 'Release'
c.run('cmake --build . --config {}'.format(build_type_flag))
c.run('cmake --build . --target install --config {}'.format(build_type_flag))

with open(cache_file, 'w') as f:
    f.write(cache_file_data)

if not check_existing():  # create links
    c.print('>> Build failed')
    exit(1)
