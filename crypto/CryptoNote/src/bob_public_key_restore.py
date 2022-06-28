from sage.all import *
from json import load
from signed_message_verifier import SM2
from ring_signature import transaction_curve

"""
Abandoned draft
# sm2p256v1
Fq = GF(0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF)
E = EllipticCurve(Fq, [0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC, 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93])
G = E(0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7, 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0)
Fn = GF(0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123)
E.set_order(Fn.order())
"""

Fq = GF(0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED)
E = EllipticCurve(Fq, [0x2AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA984914A144, 0x7B425ED097B425ED097B425ED097B425ED097B425ED097B4260B5E9C7710C864])
G = E(0x2AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD245A, 0x20AE19A1B8A086B4E01EDD2C7748D14C923D4D7E6D7C61B229E9C5A27ECED3D9)
Fn = GF(0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED)
G.set_order(Fn.order())

helper = SM2(public_key=None, private_key=None)
text, sign = load(open('signed_message_from_bob.json'))

assert len(sign) % 2 == 0
para_len = len(sign) // 2

r = int(sign[0:para_len], 16)
s = int(sign[para_len:], 16)
e = int(helper._sm3_z(text.encode()), 16)

def recovery_from_R(R):
    rsi = Fn(r + s).inverse_of_unit()
    return int(rsi) * R - int(rsi * s) * G

def recovery_from_Rx(x):
    return [recovery_from_R(R) for R in E.lift_x(x, all=True)]

# e + x == r (mod n), e + x < e + q < u n
def recovery_from_r():
    u = (e + Fq.order()) // Fn.order() + 1
    return reduce(lambda a, b: a + b, [recovery_from_Rx(r + i * Fn.order() - e) for i in range(u)])

tE, tG = transaction_curve()

def curve_transfer(P):
    x, y = P.xy()
    p = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED
    delta = 0X2AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD2451
    return tE((x - delta + p) % p, y)

possible_bob_pub = [curve_transfer(pk) for pk in recovery_from_r()]

# from secret import bob_priv, bob_pub
# bob_pub = tE(*bob_pub)
# assert bob_priv * tG == bob_pub
# print("possible count:", len(possible_bob_pub))
# print('found at', possible_bob_pub.index(bob_pub))
