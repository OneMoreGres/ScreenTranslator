import sys
import os
import hashlib

download_url = "https://raw.githubusercontent.com/OneMoreGres/ScreenTranslator/master"

if len(sys.argv) > 1:
    download_url = sys.argv[1]

subdir = 'translators'
root = os.path.abspath(os.path.basename(__file__) + '/../../..')
translators_dir = root + '/' + subdir

files = {}
it = os.scandir(translators_dir)
for f in it:
    if not f.is_file() or not f.name.endswith('.js'):
        continue
    name = f.name[:f.name.index('.')]
    files[name] = f.name

print(',"translators":{')
comma = ''
for name in sorted(files.keys()):
    file_name = files[name]
    print(' {}"{}": {{"files":['.format(comma, name))
    comma = ','
    md5 = hashlib.md5()
    size = 0
    with open(os.path.join(translators_dir, file_name), 'rb') as f:
        data = f.read()
        size = len(data)
        md5.update(data)
    print('  {{"url":"{}/{}", "path":"$translators$/{}", "md5":"{}", "size":{}}}'.format(
        download_url, subdir + '/' + file_name, file_name,
        md5.hexdigest(), size))
    print(' ]}')
print('}')
