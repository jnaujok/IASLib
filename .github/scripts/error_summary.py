"""Summarises compiler errors in a build log: a count per file, then the
first few distinct errors of each file, so a failed CI run shows the whole
picture in a short log."""
import re
import sys
from collections import OrderedDict

pattern = re.compile(r'(?:error|fatal error)(?: [A-Z]+[0-9]+)?:')
files = OrderedDict()
for line in open(sys.argv[1], errors='replace'):
    if not pattern.search(line):
        continue
    line = re.sub(r' \[[^\]]*\.vcxproj\]', '', line.strip())
    line = re.sub(r'^.*?IASLib[\\/]IASLib[\\/]', '', line)     # repository-relative
    match = re.match(r'([^(:]+)', line)
    name = match.group(1) if match else '?'
    files.setdefault(name, [])
    if line not in files[name]:
        files[name].append(line)

print('%d files with errors' % len(files))
for name, errors in sorted(files.items(), key=lambda item: -len(item[1])):
    print('%5d  %s' % (len(errors), name))
print()
for name, errors in files.items():
    for error in errors[:int(sys.argv[2]) if len(sys.argv) > 2 else 4]:
        print(error[:300])
