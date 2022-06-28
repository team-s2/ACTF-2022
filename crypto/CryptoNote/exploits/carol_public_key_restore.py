from ring_signature import proof_curve, RangeProof, OTRS, deserialize4json, Hp, proof_H, H as Hb, transaction_curve
from sage.all import *

E, G = proof_curve()
H = proof_H(E)

assert E.order() == E.base_field().order() - 1
A = sqrt(E.order())
assert isinstance(A, Integer)

rp = RangeProof(OTRS(E, G), H)

with open("range_proof_from_carol.json", "r") as f:
    json = f.read()

C, proof = deserialize4json(E, json)

def e(P, Q):
    return P.weil_pairing(Q, A)

def otrs_restore(Ks:list, m:bytes, signature:tuple):
    # two ways to restore, co-DDH or SMT
    ddh_idx = None
    smt_idx = None
    I, c_0, r = signature
    for i, K in enumerate(Ks):
        Q = Hp(E, G, K)
        if e(Q, I) == 1: # SMT way
            assert smt_idx == None
            smt_idx = i
        if e(G, I) == e(K, Q): # co-DDH way
            assert ddh_idx == None
            ddh_idx = i
    assert ddh_idx == smt_idx
    return smt_idx

hash_C = Hb(C)
Cs, sigs = proof
bits = [otrs_restore([C, C - 2**i * H], hash_C, sig) for i, (C, sig) in enumerate(zip(Cs, sigs))]

tE, tG = transaction_curve()
Px = Integer(int(''.join(['0' if x == 0 else '1' for x in bits[::-1]]), 2))
possible_carol_pub = tE.lift_x(Px, all=True)

# from secret import carol_priv, carol_pub
# carol_pub = tE(*carol_pub)
# assert carol_priv * tG == carol_pub
# print('found at', possible_carol_pub.index(carol_pub))
