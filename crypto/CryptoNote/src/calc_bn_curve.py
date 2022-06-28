"""
Abandoned draft
"""
from sage.all import *
from sympy import randprime

while True:
    ppq = int(randprime(0, int(2 ** 32)))
    FF=GF(ppq)
    PR = PolynomialRing(FF, names=('x',)); (x,) = PR._first_ngens(1)
    f1 = (36*x**4+36*x**3+24*x**2+6*x+1) +1 - (6*x**2+1)
    f2 = (36*x**4-36*x**3+24*x**2-6*x+1) +1 - (6*x**2+1)
    u1 = f1.roots()
    u2 = f2.roots()
    if len(u1) > 0 or len(u2) > 0:
        re1 = [int(x[0]) for x in u1]
        re2 = [int(x[0]) for x in u2]
        print(ppq, re1, re2)
        break

m=256
P=lambda x:36*x**4+36*x**3+24*x**2+6*x+1
l=1;r=2**(m//4+1)
while l<r:
    mid=(l+r)>>1
    mid+=1
    if P(-mid).bit_length()>m:
        r = mid - 1
    else:
        l = mid

def do_test(n): # coprime is required for directly
    # print([n % x for x in [2 ,3, 5 ,7 ,11,13]])
    assert n % ppq == 0
    n //= ppq
    return is_prime(Integer(n))

x = l
lx = (x // ppq) * ppq
while True:
    while True:
        fnd = False
        for px in re2:
            #print(lx, px)
            x = lx + px
            t = 6*x**2+1
            p=P(-x)
            n=p+1-t
            if is_prime(p) and do_test(n):
                fnd = True
                break
        if fnd:
            break
        for px in re1:
            #print(lx, px)
            x = lx + px
            t = 6*x**2+1
            p=P(x)
            n=p+1-t
            if is_prime(p) and do_test(n):
                fnd = True
                break
        if fnd:
            break
        lx-=ppq
        if lx < 0:
            print("done")
            exit()

    b=0
    while True:
        while True:
            b+=1
            if Mod(b+1, p).is_square():
                break
        y = Mod(b+1, p).nth_root(2)
        E = EllipticCurve(GF(p), [0, b])
        G = E(1, y)
        if n * G == E(0):
            break

    print("-------------------------------")
    print('x =', x)
    print('p =', p)
    print('n =', n)
    print('b =', b)
    print('y =', y)
    lx-=ppq

"""
x = 7530851732716315292
p = 115792089237315859598975831192233221747373845583936294049875884847122387488473
n = 115792089237315859598975831192233221747033563217015356079839810772609069216889
b = 10
y = 112718189488240576404337616980935239690697685349208295473191911423173215773366
"""
