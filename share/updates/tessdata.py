import sys
import os
import subprocess

if len(sys.argv) < 2:
    print("Usage:", sys.argv[0], "<tessdata_dir> [<download_url>]")
    exit(1)

tessdata_dir = sys.argv[1]


download_url = "https://github.com/tesseract-ocr/tessdata_best/raw/master"
if len(sys.argv) > 2:
    download_url = sys.argv[2]

files = {}
with os.scandir(tessdata_dir) as it:
    for f in it:
        if not f.is_file() or f.name in ["LICENSE", "README.md"]:
            continue
        name = f.name[:f.name.index('.')]
        files.setdefault(name, []).append(f.name)

print(',"recognizers": {')
comma = ''
for name, file_names in files.items():
    print(' {}"{}":{{"files":['.format(comma, name))
    comma = ', '
    for file_name in file_names:
        git_cmd = ['git', 'log', '-1', '--pretty=format:%cI', file_name]
        date = subprocess.run(git_cmd, cwd=tessdata_dir, universal_newlines=True,
                              stdout=subprocess.PIPE, check=True).stdout
        print('  {{"url":"{}/{}", "path":"$tessdata$/{}", "date":"{}"}}'.format(
            download_url, file_name, file_name, date))
    print(' ]}')
print('}')
