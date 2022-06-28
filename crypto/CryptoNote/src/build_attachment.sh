#!/bin/bash

rm -rf ../attachment
mkdir ../attachment

mkdir crypto_note
cp blockchain_service.py crypto_note/
cp ring_signature.py crypto_note/
cp signed_message_verifier.py crypto_note/
cp signed_message_from_bob.json crypto_note/
cp range_proof_from_carol.json crypto_note/
cp range_proof_verifier.py crypto_note/
cp secret_demo.py crypto_note/secret.py
tar czvf crypto_note.tar.gz crypto_note/
rm -rf crypto_note

mv crypto_note.tar.gz ../attachment

mkdir hint1
cp bob_signed_generate.py hint1/
cp carol_range_proof_generate.py hint1/
tar czvf hint1.tar.gz hint1/
rm -rf hint1

mv hint1.tar.gz ../attachment
