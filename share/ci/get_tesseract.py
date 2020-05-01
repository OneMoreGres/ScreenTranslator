import common as c
from config import bitness, msvc_version, build_dir, dependencies_dir, build_type
import os
import platform

c.print('>> Installing tesseract')

install_dir = dependencies_dir
url = 'https://github.com/tesseract-ocr/tesseract/archive/4.1.1.tar.gz'
required_version = '4.1.1'

build_type_flag = 'Debug' if build_type == 'debug' else 'Release'

# compatibility flags
os.environ['NO_AVX2'] = '1'  # default
compat_flags = ''
if 'NO_AVX2' in os.environ:
    compat_flags += ' -D USE_AVX2=OFF '
if 'NO_AVX' in os.environ:
    compat_flags += ' -D USE_AVX=OFF '
if 'NO_FMA' in os.environ:
    compat_flags += ' -D USE_FMA=OFF '
if 'NO_BMI2' in os.environ:
    compat_flags += ' -D USE_BMI2=OFF '
if 'NO_SSE4' in os.environ:
    compat_flags += '  -D USE_SSE4_1=OFF -D USE_SSE4_2=OFF '
if 'NO_OPT' in os.environ:
    compat_flags += ' -D CMAKE_CXX_FLAGS_RELEASE="/MD /Od /Od0 /DNDEBUG" '
    compat_flags += ' -D CMAKE_C_FLAGS_RELEASE="/MD /Od /Od0 /DNDEBUG" '

cache_file = install_dir + '/tesseract.cache'
cache_file_data = required_version + build_type_flag + compat_flags


def check_existing():
    if not os.path.exists(cache_file):
        return False
    with open(cache_file, 'r') as f:
        cached = f.read()
        if cached != cache_file_data:
            return False

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

cmake_args = '"{0}" -DCMAKE_INSTALL_PREFIX="{1}" -DLeptonica_DIR="{1}/cmake" \
-DBUILD_TRAINING_TOOLS=OFF -DBUILD_TESTS=OFF'.format(src_dir, install_dir)

if platform.system() == "Windows":
    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)
    cmake_args += ' ' + c.get_cmake_arch_args(bitness=bitness)

c.set_make_threaded()
c.run('cmake {}'.format(cmake_args))

if len(compat_flags) > 0:
    c.run('cmake {} .'.format(compat_flags))
    c.run('cmake {} .'.format(compat_flags))  # for sure :)

c.run('cmake --build . --config {}'.format(build_type_flag))
c.run('cmake --build . --target install --config {}'.format(build_type_flag))

with open(cache_file, 'w') as f:
    f.write(cache_file_data)

if not check_existing():  # create links
    c.print('>> Build failed')
    exit(1)
