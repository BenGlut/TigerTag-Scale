Import("env")
import os
import gzip
import shutil
from pathlib import Path
import re

def minify_html(content):
    """Minification HTML basique"""
    # Supprimer commentaires HTML
    content = re.sub(r'<!--.*?-->', '', content, flags=re.DOTALL)
    # Supprimer espaces multiples
    content = re.sub(r'\s+', ' ', content)
    # Supprimer espaces autour des balises
    content = re.sub(r'>\s+<', '><', content)
    return content.strip()

def minify_css(content):
    """Minification CSS basique"""
    # Supprimer commentaires CSS
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Supprimer espaces multiples
    content = re.sub(r'\s+', ' ', content)
    # Supprimer espaces autour des caractères spéciaux
    content = re.sub(r'\s*([{:;,}])\s*', r'\1', content)
    return content.strip()

def minify_js(content):
    """Minification JS basique (pour minification avancée, utiliser terser)"""
    # Supprimer commentaires //
    content = re.sub(r'//.*?\n', '\n', content)
    # Supprimer commentaires /* */
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Supprimer espaces multiples
    content = re.sub(r'\s+', ' ', content)
    return content.strip()

def build_web_files(*args, **kwargs):
    print("\n" + "="*60)
    print("🔨 BUILD INTERFACE WEB - TigerTagScale")
    print("="*60 + "\n")
    
    source_dir = Path("web-src")
    data_dir = Path("data/www")
    
    # Vérifier que le dossier source existe
    if not source_dir.exists():
        print("❌ ERREUR: Le dossier 'web-src/' n'existe pas!")
        print("   Créez-le et placez-y vos fichiers HTML/CSS/JS")
        return
    
    # Créer dossier destination
    data_dir.mkdir(parents=True, exist_ok=True)
    
    # Nettoyer ancien build
    for file in data_dir.glob("*"):
        if file.is_file():
            file.unlink()
    
    files_processed = 0
    total_original = 0
    total_compressed = 0
    
    # Liste des fichiers à traiter
    web_files = list(source_dir.glob("*.html")) + \
                list(source_dir.glob("*.css")) + \
                list(source_dir.glob("*.js"))
    
    if not web_files:
        print("⚠️  ATTENTION: Aucun fichier HTML/CSS/JS trouvé dans web-src/")
        print("   L'interface web ne sera pas disponible!\n")
        return
    
    # Traiter chaque fichier
    for source_file in web_files:
        print(f"📄 {source_file.name}...")
        
        try:
            # Lire contenu
            content = source_file.read_text(encoding='utf-8')
            original_size = len(content)
            
            # Minification selon type
            if source_file.suffix == '.html':
                content = minify_html(content)
            elif source_file.suffix == '.css':
                content = minify_css(content)
            elif source_file.suffix == '.js':
                content = minify_js(content)
            
            minified_size = len(content)
            
            # Compression GZIP niveau 9 (maximum)
            compressed = gzip.compress(content.encode('utf-8'), compresslevel=9)
            compressed_size = len(compressed)
            
            # Sauvegarder avec extension .gz
            output_file = data_dir / f"{source_file.name}.gz"
            output_file.write_bytes(compressed)
            
            # Statistiques
            ratio = (1 - compressed_size / original_size) * 100 if original_size > 0 else 0
            print(f"   ✅ {original_size:>6} B → {minified_size:>6} B → {compressed_size:>6} B (-{ratio:.1f}%)")
            
            files_processed += 1
            total_original += original_size
            total_compressed += compressed_size
            
        except Exception as e:
            print(f"   ❌ ERREUR: {e}")
            continue
    
    # Copier fichiers binaires (images, etc.) sans compression
    binary_files = [f for f in source_dir.iterdir() 
                    if f.is_file() and f.suffix not in ['.html', '.css', '.js']]
    
    for bin_file in binary_files:
        print(f"📦 {bin_file.name} (copie directe)...")
        shutil.copy(bin_file, data_dir / bin_file.name)
        files_processed += 1
    
    # Résumé final
    print("\n" + "-"*60)
    if total_original > 0:
        total_ratio = (1 - total_compressed / total_original) * 100
        print(f"✨ Build terminé avec succès!")
        print(f"   📦 {files_processed} fichiers traités")
        print(f"   💾 Taille originale:  {total_original:>7} bytes")
        print(f"   💾 Taille compressée: {total_compressed:>7} bytes")
        print(f"   📉 Gain total:        {total_ratio:>6.1f}%")
        print(f"   📂 Destination:       {data_dir.absolute()}")
    else:
        print(f"✨ {files_processed} fichiers copiés")
    print("-"*60 + "\n")

# Hook PlatformIO: exécuter AVANT la création du filesystem
env.AddPreAction("buildfs", build_web_files)

# Hook PlatformIO: exécuter AVANT chaque compilation (pour être sûr)
env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", build_web_files)

print("🔧 Script build_web.py chargé - compression automatique activée")