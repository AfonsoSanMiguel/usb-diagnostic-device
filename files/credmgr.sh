#!/bin/bash

set -euo pipefail

DIR="/home/raspberrypi/project"
KEY="$DIR/.credkey"
ENC="$DIR/credentials.enc"

PLAIN="$(mktemp -p /dev/shm 2>/dev/null || mktemp)"
trap 'shred -u "$PLAIN" 2>/dev/null || rm -f "$PLAIN"' EXIT

if [[ ! -f "$KEY" ]]; then
    openssl rand -base64 48 > "$KEY"
    chmod 600 "$KEY"
    echo "[i] Keyfile criada: $KEY (chmod 600)"
fi

decrypt() {
    if [[ -f "$ENC" ]]; then
        openssl enc -d -aes-256-cbc -pbkdf2 -in "$ENC" \
            -pass file:"$KEY" > "$PLAIN" 2>/dev/null || : > "$PLAIN"
    else
        : > "$PLAIN"
    fi
}

encrypt() {
    openssl enc -aes-256-cbc -pbkdf2 -salt -in "$PLAIN" \
        -out "$ENC" -pass file:"$KEY"
    chmod 600 "$ENC"
}

decrypt

case "${1:-}" in
  add)
    [[ $# -ge 2 ]] || { echo "Uso: $0 add <maquina>"; exit 1; }
    name="$2"
    read -rs -p "Password sudo de '$name': " pw; echo

    grep -v -F -e "$(printf '%s\t' "$name")" "$PLAIN" > "$PLAIN.tmp" 2>/dev/null || : > "$PLAIN.tmp"
    mv "$PLAIN.tmp" "$PLAIN"
    printf '%s\t%s\n' "$name" "$pw" >> "$PLAIN"
    pw=""
    encrypt
    echo "[ok] Maquina '$name' guardada."
    ;;
  del)
    [[ $# -ge 2 ]] || { echo "Uso: $0 del <maquina>"; exit 1; }
    grep -v -F -e "$(printf '%s\t' "$2")" "$PLAIN" > "$PLAIN.tmp" 2>/dev/null || : > "$PLAIN.tmp"
    mv "$PLAIN.tmp" "$PLAIN"
    encrypt
    echo "[ok] Maquina '$2' removida."
    ;;
  list)
    echo "Maquinas guardadas:"
    if [[ -s "$PLAIN" ]]; then
        cut -f1 "$PLAIN" | grep -v '^#' | grep -v '^$' | sed 's/^/  - /'
    else
        echo "  (nenhuma)"
    fi
    ;;
  *)
    echo "Uso: $0 {add <maquina> | del <maquina> | list}"
    exit 1
    ;;
esac
