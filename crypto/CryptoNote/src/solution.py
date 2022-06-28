from pwn import *
from secret import alice_priv, alice_pub
from json import dumps
from blockchain_service import BCTransaction
import blockchain_service
from ring_signature import transaction_curve, OTRS, serialize2json

E, G = transaction_curve()

small_points = [P for P in E(0).division_points(8) if P != E(0)]
class FakeOTRS(OTRS):
    def __init__(self, E, G):
        super().__init__(E, G)
        self.small_point = E(0)

    def signature(self, Ks: list, m: bytes, k: int, bind_message=None, your_bind=None):
        while True:
            print(".", end='')
            I, c_0, r = super().signature(Ks, m, k, bind_message, your_bind)
            I = I + self.small_point
            if self.verify(Ks, m, (I, c_0, r), bind_message):
                print("!")
                return I, c_0, r

    def switch_small_point(self, id):
        if id == None:
            self.small_point = E(0)
        else:
            self.small_point = small_points[id]

blockchain_service.otrs = FakeOTRS(E, G)

def submit_to_rpc(rpc, type_str, transaction=None):
    req = dict()
    req['type'] = type_str
    if transaction is not None:
        req['transaction'] = transaction
    json = dumps(req)
    rpc.sendlineafter(b'req> ', json.encode())
    recv = rpc.recvuntil(b'\n').strip()
    return recv

def generate_show_state(txi, pub):
    key_images = []
    txos = [G] * txi
    txos.append(pub)
    pk_owned = dict()
    pk_owned[pub] = [txi]
    return serialize2json(pk_owned, txos, key_images)

def transfer_to(rpc, txo_id, priv, pub, fake_id=None):
    # transfer txo_id, priv to pub
    blockchain_service.otrs.switch_small_point(fake_id)
    transaction = BCTransaction.generate(txo_id, pub, priv, generate_show_state(txo_id, priv * G), ring_size=1)
    return submit_to_rpc(rpc, "new_transaction", transaction.serialize())

print("get bob possible pubkey")
from bob_public_key_restore import possible_bob_pub
print("get carol possible pubkey")
from carol_public_key_restore import possible_carol_pub

# context(log_level='debug')

for i, bob_pub in enumerate(possible_bob_pub):
    for j, carol_pub in enumerate(possible_carol_pub):
        print("try", i, j)
        # rr = process(['python3', 'blockchain_service.py'])
        rr = remote('127.0.0.1', 8080)
        print("pay to bob")
        transfer_to(rr, 2, alice_priv, bob_pub)
        print("pay to carol")
        print(transfer_to(rr, 2, alice_priv, carol_pub, 1).decode())
        rr.close()
