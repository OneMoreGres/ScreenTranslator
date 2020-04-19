import sys
import os
import subprocess
import re


def parse_language_names():
    root = os.path.abspath(os.path.basename(__file__) + '/../../..')
    lines = []
    with open(root + '/src/languagecodes.cpp', 'r') as d:
        lines = d.readlines()
    result = {}
    for line in lines:
        if line.startswith('//'):
            continue
        all = re.findall(r'"(.*?)"', line)
        if len(all) != 6:
            continue
        result[all[2]] = all[5]
    return result


if len(sys.argv) < 2:
    print("Usage:", sys.argv[0], "<dict_dir> [<download_url>]")
    exit(1)

dict_dir = sys.argv[1]

download_url = "https://cgit.freedesktop.org/libreoffice/dictionaries/plain"
if len(sys.argv) > 2:
    download_url = sys.argv[2]

mirror_url = "https://translator.gres.biz/resources/dictionaries"

language_names = parse_language_names()

preferred = ['sr.aff', 'sv_FI.aff',
             'en_US.aff', 'de_DE_frami.aff', 'nb_NO.aff']

files = {}
it = os.scandir(dict_dir)
for d in it:
    if not d.is_dir():
        continue

    lang = d.name
    if '_' in lang:
        lang = lang[0:lang.index('_')]

    affs = []
    fit = os.scandir(os.path.join(dict_dir, d.name))
    for f in fit:
        if not f.is_file or not f.name.endswith('.aff'):
            continue
        affs.append(f.name)

    aff = ''
    if len(affs) == 0:
        continue
    if len(affs) == 1:
        aff = affs[0]
    else:
        for p in preferred:
            if p in affs:
                aff = p
                break

    if len(aff) == 0:
        print('no aff for', lang, affs)
        continue

    aff = os.path.join(d.name, aff)
    dic = aff[:aff.rindex('.')] + '.dic'
    if not os.path.exists(os.path.join(dict_dir, dic)):
        print('no dic exists', dic)

    files[lang] = [aff, dic]


print(',"hunspell": {')
comma = ''
unknown_names = []
for lang in sorted(files.keys()):
    file_names = files[lang]
    if not lang in language_names:
        unknown_names.append(lang)
        continue
    lang_name = language_names[lang]
    print(' {}"{}":{{"files":['.format(comma, lang_name))
    comma = ', '
    lang_comma = ''
    for file_name in file_names:
        git_cmd = ['git', 'log', '-1', '--pretty=format:%cI', file_name]
        date = subprocess.run(git_cmd, cwd=dict_dir, universal_newlines=True,
                              stdout=subprocess.PIPE, check=True).stdout
        size = os.path.getsize(os.path.join(dict_dir, file_name))
        installed = lang + file_name[file_name.index('/'):]
        mirror = ',"' + mirror_url + '/' + file_name + \
            '.zip"' if len(mirror_url) > 0 else ''
        print('  {}{{"url":["{}/{}"{}], "path":"$hunspell$/{}", "date":"{}", "size":{}}}'.format(
            lang_comma, download_url, file_name, mirror, installed, date, size))
        lang_comma = ','
    print(' ]}')
print('}')

print('unknown names', unknown_names)
