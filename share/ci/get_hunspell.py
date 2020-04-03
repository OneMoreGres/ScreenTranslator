import common as c
from config import bitness, msvc_version, build_dir, dependencies_dir, build_type
import os
import platform

c.print('>> Installing hunspell')

install_dir = dependencies_dir
url = 'https://github.com/hunspell/hunspell/files/2573619/hunspell-1.7.0.tar.gz'
required_version = '1.7.0'


def check_existing():
    if platform.system() == "Windows":
        dll = install_dir + '/bin/hunspell.dll'
        lib = install_dir + '/lib/hunspell.lib'
        if not os.path.exists(dll) or not os.path.exists(lib):
            return False
    elif platform.system() == "Darwin":
        lib = install_dir + '/lib/libhunspell.1.7.0.dylib'
        if not os.path.exists(lib):
            return False
        c.symlink(lib, install_dir + '/lib/libhunspell.dylib')
    else:
        lib = install_dir + '/lib/libhunspell-1.7.so'
        if not os.path.exists(lib):
            return False
        c.symlink(lib, install_dir + '/lib/libhunspell.so')

    includes_path = install_dir + '/include/hunspell'
    if len(c.get_folder_files(includes_path)) == 0:
        return False

    version_file = install_dir + '/lib/pkgconfig/hunspell.pc'
    if not os.path.exists(version_file):
        return False

    with open(version_file, 'rt') as f:
        lines = f.readlines()
        for l in lines:
            if not l.startswith('Version'):
                continue
            existing_version = l[9:14]  # Version: 1.7.0
            if existing_version != required_version:
                return False
            break
    return True


if check_existing():
    c.print('>> Using cached')
    exit(0)

archive = os.path.basename(url)
c.download(url, archive)

src_dir = os.path.abspath('hunspell_src')
c.extract(archive, '.')
c.symlink(c.get_archive_top_dir(archive), src_dir)

c.ensure_got_path(install_dir)

c.recreate_dir(build_dir)
os.chdir(build_dir)

c.set_make_threaded()

if platform.system() != "Windows":
    c.run('autoreconf -i {}'.format(src_dir))
    c.run('{}/configure --prefix={}'.format(src_dir, install_dir))
    c.run('make')
    c.run('make install')
else:
    lib_src = os.path.join(src_dir, 'src', 'hunspell')
    sources = []
    with os.scandir(lib_src) as it:
        for f in it:
            if not f.is_file() or not f.name.endswith('.cxx'):
                continue
            sources.append('${SRC_DIR}/' + f.name)

    headers = ['${SRC_DIR}/atypes.hxx', '${SRC_DIR}/hunspell.h', '${SRC_DIR}/hunspell.hxx',
               '${SRC_DIR}/hunvisapi.h', '${SRC_DIR}/w_char.hxx']

    cmake_file = os.path.join(build_dir, 'CMakeLists.txt')
    with open(cmake_file, 'w') as f:
        f.write('project(hunspell)\n')
        f.write('cmake_minimum_required(VERSION 3.11)\n')
        f.write('set(SRC_DIR "{}")\n'.format(lib_src).replace('\\', '/'))
        f.write('\n')
        f.write('add_library(hunspell SHARED {})\n'.format(' '.join(sources)))
        f.write('\n')
        f.write('add_compile_definitions(HAVE_CONFIG_H _WIN32 BUILDING_LIBHUNSPELL)\n')
        f.write('\n')
        f.write('install(FILES {} \
DESTINATION include/hunspell)\n'.format(' '.join(headers)))
        f.write('\n')
        f.write('install(TARGETS hunspell \
RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)\n')
        f.write('\n')
        f.write('set(prefix "${CMAKE_INSTALL_PREFIX}")\n')
        f.write('set(VERSION "{}")\n'.format(required_version))
        f.write('configure_file({}/hunspell.pc.in \
${{CMAKE_CURRENT_BINARY_DIR}}/hunspell.pc @ONLY)\n'.format(src_dir.replace('\\', '/')))
        f.write('install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hunspell.pc \
DESTINATION lib/pkgconfig)\n')

    env_cmd = c.get_msvc_env_cmd(bitness=bitness, msvc_version=msvc_version)
    c.apply_cmd_env(env_cmd)
    cmake_args = '"{}" -DCMAKE_INSTALL_PREFIX="{}" {}'.format(
        build_dir, install_dir, c.get_cmake_arch_args(bitness=bitness))
    c.run('cmake {}'.format(cmake_args))
    build_type_flag = 'Debug' if build_type == 'debug' else 'Release'
    c.run('cmake --build . --config {}'.format(build_type_flag))
    c.run('cmake --build . --target install --config {}'.format(build_type_flag))

if not check_existing():  # create links
    c.print('>> Build failed')
    exit(1)
