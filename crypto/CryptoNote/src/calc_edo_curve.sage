"""
generate embed degree one curve
"""

from sage.all import *

"""
while True:
    print("epoch")
    n = random_prime(2**375)
    A = 2 * n
    p = A^2 + 1
    if is_prime(p):
        break
print(f'{n = :#x}')
print(f'{A = :#x}')
print(f'{p = :#x}')
"""

n = 0x6c0edfb182b3b5ce4d8cf2db057c675f0ca6c5631e6accf20cc6f3a987eeecc79b285eb1ef132bf71e24c68aa4ca39
A = 0xd81dbf6305676b9c9b19e5b60af8cebe194d8ac63cd599e4198de7530fddd98f3650bd63de2657ee3c498d15499472
p = 0xb672366c04a2a76427e4f60dbbdfc3b0632d34a5819a44ddf3f01223763741a3126addbae4424bc0e8d6c78a4c083e3c995665a68c58c0f841ccacdf4f0a6fa46f1df43d935b784cf75113327e25c7304becd4a54be1f8ebb114b31802c5

FF = GF(p)
assert A % 4 == 0 or A % 4 == 2
a = (p - 1) if A % 4 == 0 else (p - 4)
E = EllipticCurve(FF, [a, 0])
assert E.order() == p - 1

P = E.random_point()
Q = E.random_point()

assert P.weil_pairing(2 * P, A) == 1

pq = P.weil_pairing(Q, A)
assert pq != 1
xpq = (3 * P).weil_pairing(4 * Q, A)
assert pq^12 == xpq

pq = P.tate_pairing(Q, A, 1)
assert pq != 1
xpq = (3 * P).tate_pairing(4 * Q, A, 1)
assert pq^12 == xpq

print('Tate e(P, Q) =', P.tate_pairing(Q, A, 1))

print(f'curve_gen({p}, [{a}, {3 * p}], {P.xy()[0]}, {P.xy()[1]}, {E.order()})')
print(f'E({Q.xy()[0]}, {Q.xy()[1]})')
