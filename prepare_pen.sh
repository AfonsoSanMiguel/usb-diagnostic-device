#!/bin/bash

IMAGE="/piusb.bin"
MOUNT="/mnt"
BASE_DIR="${BASE_DIR:-/home/raspberrypi/project}"
PRIVATE_KEY="$BASE_DIR/private.pem"
PUBLIC_KEY="$BASE_DIR/public.pem"
: "${SIGNING_PASSPHRASE:?define SIGNING_PASSPHRASE before running}"

sudo mount $IMAGE $MOUNT -o loop

mkdir -p $MOUNT/signatures

for script in $MOUNT/scripts/*.sh; do
    name=$(basename $script)

    echo "$name" | cat - "$script" | \
        openssl dgst -sha256 \
        -sign $PRIVATE_KEY \
        -passin env:SIGNING_PASSPHRASE \
        -out "$MOUNT/signatures/$name.sig"

    echo "Assinado: $name"
done


sync
sudo umount $MOUNT

echo "Pen preparada com assinaturas RSA."
