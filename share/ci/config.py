from os import getenv, path

app_name = 'ScreenTranslator'
app_version = '3.0.0'

target_name = app_name
qt_version = '5.14.0'
qt_modules = ['qtbase', 'qttools', 'icu',
              'qttranslations', 'qtx11extras', 'qtwebengine', 'qtwebchannel',
              'qtdeclarative', 'qtlocation', 'opengl32sw', 'd3dcompiler_47']
qt_dir = path.abspath('qt')

build_dir = path.abspath('build')
dependencies_dir = path.abspath('deps')
pro_file = path.abspath(path.dirname(__file__) +
                        '/../../screen-translator.pro')
ts_files_dir = path.abspath(path.dirname(__file__) + '/../../translations')

os_name = getenv('OS', 'linux')
app_version += {'linux': '', 'macos': '-experimental',
                'win32': '', 'win64': ''}[os_name]
bitness = '32' if os_name == 'win32' else '64'
msvc_version = getenv('MSVC_VERSION', '2017/Community')
