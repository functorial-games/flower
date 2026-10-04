#!/usr/bin/env python3
"""Require semantic rejection, not compilation failure, of four regressions."""
import os
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/flower.c').read_text()
mutations = {
    'release ignored': ('if (!down) { flower_hold_release(hold,pointer); return false; }', '(void)down;'),
    'pointer identity ignored': (' || pointer!=hold->owner', ''),
    'focus loss ignored': ('if (!focused || !isfinite(frame_seconds)', 'if (!isfinite(frame_seconds)'),
    'simulation disconnected': ('simulation_step(flower,hit);', '(void)flower;'),
}
with tempfile.TemporaryDirectory(prefix='flower-mutants-') as temporary:
    temporary = Path(temporary)
    for name, (before, after) in mutations.items():
        if source.count(before) != 1:
            raise SystemExit(f'Mutation anchor changed: {name}')
        mutant = temporary / 'flower.c'
        mutant.write_text(source.replace(before, after))
        executable = temporary / 'test'
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-O2', '-DNDEBUG',
                        '-I', str(root / 'src'), str(mutant), str(root / 'tests/test_flower.c'),
                        '-lm', '-o', str(executable)], check=True)
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=20)
        if result.returncode != 1 or 'FAIL line' not in result.stderr:
            raise SystemExit(f'Mutant did not fail a runtime check: {name}: {result.returncode}')
        print(f'REJECTED {name}: {result.stderr.strip()}')
