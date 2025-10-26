# scripts/aliases.py
# Ajoute une commande custom "deploy" pour executer uploadfs + upload
Import("env")

# Crée un alias pour combiner les deux cibles
env.Alias("deploy", ["uploadfs", "upload"])

print("🧰 Alias 'deploy' (uploadfs + upload) chargé avec succès ✅")