from pwn import *
from secret import alice_priv, alice_pub, bob_priv, bob_pub, carol_pub
from json import dumps
from blockchain_service import BCTransaction
from ring_signature import transaction_curve, serialize2json

E, G = transaction_curve()

rr = process(['python3', 'blockchain_service.py'])

def submit_to_rpc_1(type_str, transaction=None):
    req = dict()
    req['type'] = type_str
    if transaction is not None:
        req['transaction'] = transaction
    json = dumps(req)
    rr.sendlineafter(b'req> ', json.encode())
    recv = rr.recvuntil(b'\n').strip()
    return recv

def generate_show_state(txi, pub):
    key_images = []
    txos = [E(carol_pub), E(bob_pub), E(alice_pub), E(bob_pub), E(carol_pub)][:txi+1]
    pk_owned = dict()
    pk_owned[pub] = [txi]
    return serialize2json(pk_owned, txos, key_images)

def transfer_to(txo_id, priv, pub):
    # transfer txo_id, priv to pub
    transaction = BCTransaction.generate(txo_id, pub, priv, generate_show_state(txo_id, priv * G))
    return submit_to_rpc_1("new_transaction", transaction.serialize())

transfer_to(2, alice_priv, E(bob_pub))
print(transfer_to(3, bob_priv, E(carol_pub)).decode())
rr.interactive()
rr.close()
