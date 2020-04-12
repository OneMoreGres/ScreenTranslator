import sys
import os
import subprocess
import re


def parse_language_names():
    root = os.path.abspath(os.path.basename(__file__) + '/../../..')
    lines = []
    with open(root + '/src/languagecodes.cpp', 'r') as f:
        lines = f.readlines()
    result = {}
    for line in lines:
        all = re.findall(r'"(.*?)"', line)
        if len(all) != 6:
            continue
        result[all[3]] = all[5]
    return result


if len(sys.argv) < 2:
    print("Usage:", sys.argv[0], "<tessdata_dir> [<download_url>]")
    exit(1)

tessdata_dir = sys.argv[1]

download_url = "https://github.com/tesseract-ocr/tessdata_best/raw/master"
if len(sys.argv) > 2:
    download_url = sys.argv[2]

language_names = parse_language_names()

files = {}
it = os.scandir(tessdata_dir)
for f in it:
    if not f.is_file() or f.name in ["LICENSE", "README.md"]:
        continue
    name = f.name[:f.name.index('.')]
    files.setdefault(name, []).append(f.name)

print(',"recognizers": {')
comma = ''
unknown_names = []
for name in sorted(files.keys()):
    file_names = files[name]
    if not name in language_names:
        unknown_names.append(name)
    else:
        name = language_names[name]
    print(' {}"{}":{{"files":['.format(comma, name))
    comma = ', '
    for file_name in file_names:
        git_cmd = ['git', 'log', '-1', '--pretty=format:%cI', file_name]
        date = subprocess.run(git_cmd, cwd=tessdata_dir, universal_newlines=True,
                              stdout=subprocess.PIPE, check=True).stdout
        size = os.path.getsize(os.path.join(tessdata_dir, file_name))
        print('  {{"url":"{}/{}", "path":"$tessdata$/{}", "date":"{}", "size":{}}}'.format(
            download_url, file_name, file_name, date, size))
    print(' ]}')
print('}')

print('unknown names', unknown_names)
