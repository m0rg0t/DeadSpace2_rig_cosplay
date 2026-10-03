"""Compile only the two real animation functions against checked host stubs."""
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path(os.environ.get('LED_SKETCH_SOURCE', str(
    ROOT / 'main_back_rig/sketch_main_rig/sketch_main_rig.ino')))
text = SOURCE.read_text(encoding='utf-8')
count = re.search(r'^#define NUM_LEDS (\d+)\s*$', text, re.MULTILINE)
if not count:
    raise RuntimeError('Cannot locate the sketch LED count')


def extract(name):
    match = re.search(r'void ' + re.escape(name) + r'\(int speed = 50\)\s*\{', text)
    if not match:
        raise RuntimeError('Cannot locate animation: ' + name)
    level = 1
    end = match.end()
    # These small functions contain no braces in comments or strings.
    while level and end < len(text):
        level += (text[end] == '{') - (text[end] == '}')
        end += 1
    if level:
        raise RuntimeError('Unbalanced animation body: ' + name)
    return text[match.start():end]


with tempfile.TemporaryDirectory(prefix='rig-host-tests-') as directory:
    directory = Path(directory)
    (directory / 'animations.inc').write_text(
        extract('runTopToBottomLeds') + '\n' + extract('runBottomAndBackLeds'),
        encoding='utf-8')
    # 17 is the actual strip; small/odd/even counts check loop boundaries too.
    failures = []
    for leds in sorted({1, 2, 3, 16, 17, 18, int(count.group(1))}):
        executable = directory / ('led-test-' + str(leds))
        command = shlex.split(os.environ.get('CXX', 'g++')) + [
            '-std=c++11', '-Wall', '-Wextra', '-Werror', '-pedantic',
            '-DNUM_LEDS=' + str(leds), '-I', str(directory),
            str(ROOT / 'tests/led_animation_test.cpp'), '-o', str(executable)]
        subprocess.run(command, check=True)
        if subprocess.run([str(executable)]).returncode:
            failures.append(leds)
    if failures:
        raise SystemExit('Failed LED counts: ' + ', '.join(map(str, failures)))
