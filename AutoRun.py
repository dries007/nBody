"""
Auto-Run the nBody problem program and report the stats back in a file.
"""

KERNELS = [
    'CPU',
    'FLOAT',
    'FLOAT3',
    '2D',
]

STOP_THRESHOLD = 300

import csv
import subprocess
import re
import locale

# Set to users preferred locale:
locale.setlocale(locale.LC_ALL, '')
# Or a specific locale:
locale.setlocale(locale.LC_NUMERIC, "nl_BE.UTF-8")

RESULT_RE = re.compile('AVG: (\d+\.?\d*)')


def save_data(times):
    with open('data.csv', 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=['_n'] + KERNELS, delimiter=";")
        writer.writeheader()
        writer.writerows(sorted(times.values(), key=lambda x: x['_n']))


def run_job(bodies, kernel):
    print('Running', bodies, kernel)
    sbp = subprocess.run(('./nBodyAuto', str(bodies), kernel), input='1\n', stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, universal_newlines=True)
    outp = float(RESULT_RE.findall(sbp.stdout)[0])
    print(outp)
    return outp


def main():
    times = {}
    run = KERNELS[:]
    bodies = 2
    while len(run):
        times[bodies] = {'_n': bodies}
        try:
            for kernel in run:
                time = run_job(bodies, kernel)
                times[bodies][kernel] = time
        finally:
            save_data(times)
        for k, v in times[bodies].items():
            if k == '_n':
                continue
            if v > STOP_THRESHOLD:
                print('Removed', k, 'Last run took', v)
                run.remove(k)
        bodies *= 2


if __name__ == '__main__':
    main()
