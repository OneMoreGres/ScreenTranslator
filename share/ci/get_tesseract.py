import common as c
from config import bitness, msvc_version, build_dir, dependencies_dir, build_type
import os
import platform

c.print('>> Installing tesseract')

install_dir = dependencies_dir
required_version = '5.2.0'
url = 'https://github.com/tesseract-ocr/tesseract/archive/{}.tar.gz'.format(required_version)

build_type_flag = 'Debug' if build_type == 'debug' else 'Release'

cache_file = install_dir + '/tesseract.cache'
cache_file_data = required_version + build_type_flag

def check_existing():
    if not os.path.exists(cache_file):
        return False
    with open(cache_file, 'r') as f:
        cached = f.read()
        if cached != cache_file_data:
            return False

    includes_path = install_dir + '/include/tesseract'
    if len(c.get_folder_files(includes_path)) == 0:
        return False

    if platform.system() == "Windows":
        file_name_ver = required_version[0] + required_version[2]
        dll = install_dir + '/bin/tesseract{}.dll'.format(file_name_ver)
        lib = install_dir + '/lib/tesseract{}.lib'.format(file_name_ver)
        if not os.path.exists(dll) or not os.path.exists(lib):
            return False
        c.symlink(dll, install_dir + '/bin/tesseract.dll')
        c.symlink(lib, install_dir + '/lib/tesseract.lib')
    elif platform.system() == "Darwin":
        lib = install_dir + '/lib/libtesseract.{}.dylib'.format(required_version)
        if not os.path.exists(lib):
            return False
        c.symlink(lib, install_dir + '/lib/libtesseract.dylib')
    else:
        lib = install_dir + '/lib/libtesseract.so.{}'.format(required_version)
        if not os.path.exists(lib):
            return False
        c.symlink(lib, install_dir + '/lib/libtesseract.so')

    return True


if check_existing() and not 'FORCE' in os.environ:
    c.print('>> Using cached')
    exit(0)

archive = 'tesseract-' + os.path.basename(url)
c.download(url, archive)

src_dir = os.path.abspath('tesseract_src')
c.extract(archive, '.')
c.symlink(c.get_archive_top_dir(archive), src_dir)

c.ensure_got_path(install_dir)

c.recreate_dir(build_dir)
os.chdir(build_dir)

cmake_args = '"{0}" \
-DCMAKE_INSTALL_PREFIX="{1}" \
-DLeptonica_DIR="{1}/cmake" \
-DSW_BUILD=OFF \
-DBUILD_TRAINING_TOOLS=OFF \
-DBUILD_TESTS=OFF \
-DBUILD_SHARED_LIBS=ON \
-DDISABLE_CURL=ON \
-DDISABLE_ARCHIVE=ON \
-DUSE_SYSTEM_ICU=ON \
-DENABLE_LTO=ON \
-DGRAPHICS_DISABLED=ON \
-DDISABLED_LEGACY_ENGINE=ON \
'.format(src_dir, install_dir)

if platform.system() == "Windows":
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)
    cmake_args += ' ' + c.get_cmake_arch_args(bitness=bitness)

c.set_make_threaded()
c.run('cmake {}'.format(cmake_args))

c.run('cmake --build . --config {}'.format(build_type_flag))
c.run('cmake --build . --target install --config {}'.format(build_type_flag))

with open(cache_file, 'w') as f:
    f.write(cache_file_data)

if not check_existing():  # add suffix
    c.print('>> Build failed')
    exit(1)
