import sys
import os
import hashlib

download_url = "https://raw.githubusercontent.com/OneMoreGres/ScreenTranslator/master"

if len(sys.argv) > 1:
    download_url = sys.argv[1]

subdir = 'translators'
root = os.path.abspath(os.path.basename(__file__) + '/..')
translators_dir = root + '/' + subdir

files = {}
with os.scandir(translators_dir) as it:
    for f in it:
        if not f.is_file() or not f.name.endswith('.js'):
            continue
        name = f.name[:f.name.index('.')]
        files[name] = f.name

print(',"translators":{')
comma = ''
for name, file_name in files.items():
    print(' {}"{}": {{"files":['.format(comma, name))
    comma = ','
    md5 = hashlib.md5()
    with open(os.path.join(translators_dir, file_name), 'rb') as f:
        md5.update(f.read())
    print('  {{"url":"{}/{}", "path":"$translators$/{}", "md5":"{}"}}'.format(
        download_url, subdir + '/' + file_name, file_name, md5.hexdigest()))
    print(' ]}')
print('}')
