#!/usr/bin/env python3
"""Link sibling roo repositories for this standalone hardware benchmark."""
from pathlib import Path
project=Path(__file__).resolve().parent
repositories=project.parents[2]
(project/'lib').mkdir(exist_ok=True)
for name in ('roo_windows','roo_collections','roo_display','roo_fonts_basic',
             'roo_fonts_material','roo_locale','roo_logging','roo_icons',
             'roo_io','roo_scheduler','roo_time','roo_backport'):
    target=repositories/name
    if not (target/'src').is_dir():
        raise SystemExit(f'Missing sibling library: {target}')
    link=project/'lib'/name
    if not link.exists(): link.symlink_to(target,target_is_directory=True)
    elif link.resolve()!=target.resolve(): raise SystemExit(f'Conflicting library link: {link}')
print('Linked local libraries; run PlatformIO from',project)
