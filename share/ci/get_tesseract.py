import common as c
from config import bitness, msvc_version, build_dir, dependencies_dir
import os
import platform

c.print('>> Installing tesseract')

install_dir = dependencies_dir
url = 'https://github.com/tesseract-ocr/tesseract/archive/4.1.1.tar.gz'
required_version = '4.1.1'


def check_existing():
    if platform.system() == "Windows":
        dll = install_dir + '/bin/tesseract41.dll'
        lib = install_dir + '/lib/tesseract41.lib'
        if not os.path.exists(dll) or not os.path.exists(lib):
            return False
        c.symlink(dll, install_dir + '/bin/tesseract.dll')
        c.symlink(lib, install_dir + '/lib/tesseract.lib')
    elif platform.system() == "Darwin":
        lib = install_dir + '/lib/libtesseract.4.1.1.dylib'
        if not os.path.exists(lib):
            return False
        c.symlink(lib, install_dir + '/lib/libtesseract.dylib')
    else:
        if not os.path.exists(install_dir + '/lib/libtesseract.so'):
            return False

    includes_path = install_dir + '/include/tesseract'
    if len(c.get_folder_files(includes_path)) == 0:
        return False

    version_file = install_dir + '/cmake/TesseractConfig-version.cmake'
    if not os.path.exists(version_file):
        return False

    with open(version_file, 'rt') as f:
        existing_version = f.readline()[22:27]  # set(Tesseract_VERSION 1.78.0)
        if existing_version != required_version:
            return False
    return True


if check_existing():
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

os.environ['PKG_CONFIG_PATH'] = install_dir + '/lib/pkgconfig'

cmake_args = '"{}" -DCMAKE_INSTALL_PREFIX="{}" -DBUILD_TRAINING_TOOLS=OFF \
    -DBUILD_TESTS=OFF'.format(src_dir, install_dir)

if platform.system() == "Windows":
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)
    cmake_args += ' ' + c.get_cmake_arch_args(bitness=bitness)

c.set_make_threaded()
c.run('cmake {}'.format(cmake_args))
c.run('cmake --build . --config Release')
c.run('cmake --build . --target install --config Release')

if not check_existing(): # create links
    c.print('>> Build failed')
    exit(1)
