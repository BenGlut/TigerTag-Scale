# tools/gzip_www.py
# Gzip auto de data/www/*.{html,css,js} avant la création de l'image LittleFS.
# Supprime les anciens .gz pour éviter tout reliquat, puis régénère proprement.

Import("env")
import os, gzip, shutil

print("🔧 Script gzip_www.py chargé - sync & gzip auto")

ROOT = env["PROJECT_DIR"]
WWW  = os.path.join(ROOT, "data", "www")
SRC  = os.path.join(ROOT, "web-src")  # répertoire source des fichiers web

EXTS = {".html", ".css", ".js"}

# Counters for summary
COPIED = 0
GZ_MADE = 0

def gzip_file(src_path):
    gz_path = src_path + ".gz"
    # (Re)crée le .gz avec niveau 9
    with open(src_path, "rb") as f_in, open(gz_path, "wb") as raw_out:
        # Use GzipFile to support deterministic mtime across Python versions
        with gzip.GzipFile(filename=os.path.basename(src_path), mode="wb", fileobj=raw_out, compresslevel=9, mtime=0) as gz:
            shutil.copyfileobj(f_in, gz)
    print(f"[gzip] {os.path.relpath(src_path, ROOT)}  ->  {os.path.relpath(gz_path, ROOT)}")

def clean_old_gz(base_dir):
    for root, _, files in os.walk(base_dir):
        for name in files:
            if name.endswith(".gz"):
                path = os.path.join(root, name)
                os.remove(path)
                print(f"[clean] removed {os.path.relpath(path, ROOT)}")


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)



def sync_from_src(src_dir, dst_dir):
    global COPIED
    COPIED = 0
    print(f"[sync] Checking source directory: {src_dir}")
    if not os.path.isdir(src_dir):
        print(f"[sync] source dir not found: {os.path.relpath(src_dir, ROOT)} (skip)")
        return 0
    for root, _, files in os.walk(src_dir):
        rel = os.path.relpath(root, src_dir)
        out_root = os.path.join(dst_dir, rel) if rel != "." else dst_dir
        ensure_dir(out_root)
        for name in files:
            src_path = os.path.join(root, name)
            dst_path = os.path.join(out_root, name)
            shutil.copy2(src_path, dst_path)
            COPIED += 1
            print(f"[sync] {os.path.relpath(src_path, ROOT)}  ->  {os.path.relpath(dst_path, ROOT)}")
    return COPIED


def build_gz_for_sources(base_dir):
    global GZ_MADE
    GZ_MADE = 0
    for root, _, files in os.walk(base_dir):
        for name in files:
            _, ext = os.path.splitext(name)
            if ext.lower() in EXTS:
                src = os.path.join(root, name)
                gzip_file(src)
                GZ_MADE += 1
    print(f"[gzip] created {GZ_MADE} .gz file(s)")

def pre_gzip_www(source, target, env):
    print(f"[HOOK] pre_gzip_www launched. SRC={SRC}, WWW={WWW}")
    ensure_dir(WWW)
    # 1) synchronise les sources (web-src -> data/www)
    sync_from_src(SRC, WWW)

    # 2) supprime les anciens .gz
    print("[gzip] cleaning old .gz …")
    clean_old_gz(WWW)

    # 3) (re)génère les .gz pour chaque .html/.css/.js présent dans data/www
    print("[gzip] generating fresh .gz …")
    build_gz_for_sources(WWW)
    print("[gzip] done.")
    print(f"[summary] synced {COPIED} file(s) from web-src and gzipped {GZ_MADE} file(s)")

#
# Also hook on program build/upload so gz are always regenerated even on regular builds
try:
    PROG_ELF = os.path.join(env["BUILD_DIR"], env.subst("${PROGNAME}.elf"))
    env.AddPreAction(PROG_ELF, pre_gzip_www)     # when building the firmware ELF
except Exception as _:
    pass

# High-level SCons targets that users commonly run
env.AddPreAction("buildprog", pre_gzip_www)      # pio run
env.AddPreAction("upload", pre_gzip_www)         # pio run -t upload

# Hook: avant la création de l’image LittleFS
env.AddPreAction("$BUILD_DIR/littlefs.bin", pre_gzip_www)
# Also run before explicit FS targets, even if PlatformIO considers the image up-to-date
env.AddPreAction("buildfs", pre_gzip_www)
env.AddPreAction("uploadfs", pre_gzip_www)

# (Optionnel) si ton core utilise spiffs.bin au lieu de littlefs.bin, dé-commente aussi:
# env.AddPreAction("$BUILD_DIR/spiffs.bin", pre_gzip_www)