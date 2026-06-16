import csv, os, statistics

def sys_stats(path):
    if not os.path.exists(path): return None
    cols = {}
    with open(path, newline='', encoding='utf-8') as f:
        for row in csv.DictReader(f):
            for k,v in row.items():
                try: cols.setdefault(k,[]).append(float(v))
                except: pass
    if not cols: return None
    core_keys = sorted([k for k in cols if k.startswith('cpu') and k != 'cpu_total'])
    res = {
        'cpu_total_avg': statistics.mean(cols.get('cpu_total',[0])),
        'cpu_total_max': max(cols.get('cpu_total',[0])),
        'temp_avg': statistics.mean(cols.get('temp_c',[0])),
        'temp_max': max(cols.get('temp_c',[0])),
        'freq_avg': statistics.mean(cols.get('freq_mhz',[0])),
        'throttled': sum(1 for t in cols.get('throttled',[]) if t != 0),
        'cores': core_keys,
    }
    for k in core_keys:
        res[f'{k}_avg'] = statistics.mean(cols.get(k,[0]))
        res[f'{k}_max'] = max(cols.get(k,[0]))
    return res

def main_stats(path):
    cols = {}
    with open(path, newline='', encoding='utf-8') as f:
        lines = f.readlines()
    data = [ln for ln in lines if not ln.lstrip().startswith('#')]
    for row in csv.DictReader(data):
        for k,v in row.items():
            try: cols.setdefault(k,[]).append(float(v))
            except: pass
    n = len(cols.get('frame',[]))
    drops = int(sum(cols.get('block_drops',[0])))
    bg_spf_nz = [v for v in cols.get('bg_spf',[]) if v>0]
    bg_sps_nz = [v for v in cols.get('bg_sps',[]) if v>0]
    spf = sorted(bg_spf_nz)[len(bg_spf_nz)//2] if bg_spf_nz else 480
    sps = sorted(bg_sps_nz)[len(bg_sps_nz)//2] if bg_sps_nz else 48000
    dl = spf/sps*1000
    exec_over = sum(1 for t in cols.get('exec_ms',[]) if t > dl)
    exec_avg = statistics.mean(cols.get('exec_ms',[0]))
    duration_min = n / (sps/spf) / 60 if sps else 0
    return {'n':n,'drops':drops,'exec_over':exec_over,'exec_avg':exec_avg,'dl':dl,'duration_min':duration_min,'spf':int(spf),'sps':int(sps)}

runs = [
    ('48k','default','log_20260615_203222_48000_default'),
    ('48k','rr',     'log_20260615_203730_48000_rr'),
    ('48k','fifo',   'log_20260615_204238_48000_fifo'),
    ('96k','default','log_20260615_204746_96000_default'),
    ('96k','rr',     'log_20260615_205254_96000_rr'),
    ('96k','fifo',   'log_20260615_205802_96000_fifo'),
    ('192k','default','log_20260615_210310_192000_default'),
    ('192k','rr',    'log_20260615_210818_192000_rr'),
    ('192k','fifo',  'log_20260615_211326_192000_fifo'),
]

base_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'logs', 'EXP-01')

for rate, sched, base in runs:
    m = main_stats(os.path.join(base_dir, f'{base}.csv'))
    s = sys_stats(os.path.join(base_dir, f'{base}_sys.csv'))
    dpm = m['drops']/m['duration_min'] if m['duration_min'] else 0
    print(f"[{rate} {sched}] frames={m['n']} drops={m['drops']} ({dpm:.1f}/min) exec_avg={m['exec_avg']:.1f}ms exec>dl={m['exec_over']}/{m['n']} duration={m['duration_min']:.1f}min sps={m['sps']}")
    if s:
        core_avgs = ' '.join(f"cpu{i}={s.get(f'cpu{i}_avg',0):.0f}%" for i in range(len(s['cores'])))
        print(f"  sys: temp_avg={s['temp_avg']:.1f}C temp_max={s['temp_max']:.1f}C throttle={s['throttled']} freq={s['freq_avg']:.0f}MHz | {core_avgs}")
