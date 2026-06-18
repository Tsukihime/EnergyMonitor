import os
import gzip
import shutil
from SCons.Script import COMMAND_LINE_TARGETS

Import("env")

SOURCE_DIR = os.path.join(env.get("PROJECT_DIR"), "webpage")
HTML_FILE = os.path.join(SOURCE_DIR, "index.html")
GZ_FILE = os.path.join(SOURCE_DIR, "index.html.gz")

def compress_html():
    if not os.path.exists(HTML_FILE):
        print(f"\n[GZIP SCRIPT] ERROR: {HTML_FILE} not found!")
        return

    print(f"\n[GZIP SCRIPT] Compressing {HTML_FILE} -> {GZ_FILE}...")
    
    with open(HTML_FILE, 'rb') as f_in:
        with gzip.open(GZ_FILE, 'wb', compresslevel=9) as f_out:
            shutil.copyfileobj(f_in, f_out)
            
    print("[GZIP SCRIPT] Compression finished successfully!\n")

if "clean" not in COMMAND_LINE_TARGETS:
    compress_html()