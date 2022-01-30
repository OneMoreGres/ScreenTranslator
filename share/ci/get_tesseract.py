import common as c
from config import bitness, msvc_version, build_dir, dependencies_dir, build_type
import os
import platform

c.print('>> Installing tesseract')

install_dir = dependencies_dir
url = 'https://github.com/tesseract-ocr/tesseract/archive/4.1.3.tar.gz'
required_version = '4.1.3'

build_type_flag = 'Debug' if build_type == 'debug' else 'Release'

# compatibility flags
compat_flags = ''
if os.environ.get('NO_AVX2', '0') == '1':
    compat_flags += ' -D USE_AVX2=OFF '
if os.environ.get('NO_AVX512', '0') == '1':
    compat_flags += ' -D USE_AVX512BW=OFF -D USE_AVX512CD=OFF \
-D USE_AVX512DQ=OFF -D USE_AVX512ER=OFF -D USE_AVX512F=OFF -D USE_AVX512IFMA=OFF \
-D USE_AVX512PF=OFF -D USE_AVX512VBMI=OFF -D USE_AVX512VL=OFF '
if os.environ.get('NO_AVX', '0') == '1':
    compat_flags += ' -D USE_AVX=OFF '
if os.environ.get('NO_FMA', '0') == '1':
    compat_flags += ' -D USE_FMA=OFF '
if os.environ.get('NO_BMI2', '0') == '1':
    compat_flags += ' -D USE_BMI2=OFF '
if os.environ.get('NO_SSE4', '0') == '1':
    compat_flags += '  -D USE_SSE4_1=OFF -D USE_SSE4_2=OFF '
if os.environ.get('NO_OPT', '0') == '1':
    compat_flags += ' -D CMAKE_CXX_FLAGS_RELEASE="/MD /Od /Od0 /DNDEBUG" '
    compat_flags += ' -D CMAKE_C_FLAGS_RELEASE="/MD /Od /Od0 /DNDEBUG" '
if len(os.environ.get('MARCH', '')) > 0:
    compat_flags += ' -D TARGET_ARCHITECTURE={} '.format(os.environ['MARCH'])

lib_suffix = os.environ.get('TAG', '')
if len(lib_suffix) > 0:
    lib_suffix = '-' + lib_suffix


def check_existing():
    includes_path = install_dir + '/include/tesseract'
    if len(c.get_folder_files(includes_path)) == 0:
        return False

    if platform.system() == "Windows":
        lib = install_dir + '/bin/tesseract{}.dll'.format(lib_suffix)
        orig_lib = install_dir + '/bin/tesseract41.dll'
    elif platform.system() == "Darwin":
        lib = install_dir + '/lib/libtesseract{}.dylib'.format(lib_suffix)
        orig_lib = install_dir + '/lib/libtesseract.{}.dylib'.format(required_version)
    else:
        lib = install_dir + '/lib/libtesseract{}.so'.format(lib_suffix)
        orig_lib = install_dir + '/lib/libtesseract.so.{}'.format(required_version)

    if os.path.exists(lib):
        return True
    if os.path.exists(orig_lib):
        os.rename(orig_lib, lib)
        return True

    return False


if check_existing() and not 'FORCE' in os.environ:
    c.print('>> Using cached')
    exit(0)

archive = 'tesseract-' + os.path.basename(url)
c.download(url, archive)

src_dir = os.path.abspath('tesseract_src')
c.extract(archive, '.')
c.symlink(c.get_archive_top_dir(archive), src_dir)

if platform.system() == "Windows": 
    # workaround for not found 'max'
    modify_data = ''
    modify_file = '{}/src/ccmain/thresholder.cpp'.format(src_dir)
    with open(modify_file, 'r') as f:
        modify_data = f.read()

    if modify_data.find('<algorithm>') == -1:
        modify_data = modify_data.replace(
        '''<tuple>''',
        '''<tuple>\n#include <algorithm>''')

    with open(modify_file, 'w') as f:
        f.write(modify_data)

    # ignore libtiff
    modify_data = ''
    modify_file = '{}/CMakeLists.txt'.format(src_dir)
    with open(modify_file, 'r') as f:
        modify_data = f.read()

    if modify_data.find('#pkg_check_modules(TIFF libtiff-4)') == -1:
        modify_data = modify_data.replace(
        '''pkg_check_modules(TIFF libtiff-4)''',
        '''#pkg_check_modules(TIFF libtiff-4)''')

    with open(modify_file, 'w') as f:
        f.write(modify_data)



c.ensure_got_path(install_dir)

c.recreate_dir(build_dir)
os.chdir(build_dir)

cmake_args = '"{0}" -DCMAKE_INSTALL_PREFIX="{1}" -DLeptonica_DIR="{1}/cmake" \
-DBUILD_TRAINING_TOOLS=OFF -DBUILD_TESTS=OFF -DBUILD_SHARED_LIBS=ON -DSW_BUILD=OFF \
'.format(src_dir, install_dir)

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

if not check_existing():  # add suffix
    c.print('>> Build failed')
    exit(1)
