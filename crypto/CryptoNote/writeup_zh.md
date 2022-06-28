## CryptoNote

`blockchain_service.py` 仿照 HyperLedger Fabric 的架构[1]（不是重点）和 Monero 的 CryptoNote 交易[2]编了个“区块链”交易系统。

Monero 使用 ed25519，而由于椭圆曲线使用 SageMath 实现，而它不支持 twisted edwards curve，所以这里采用 montgomery curve 上对应的 curve25519。

### 还原 Bob 的公钥

这里使用了 gmssl 库来做 SM2 签名。由于 gmssl 库只支持 short weierstrass curve，所以对公钥和曲线相应的变换[3]。

我们知道以太坊上的交易不需要附带公钥，只需要有签名和恢复 id（或者说 v）即可。验证时只需要使用 ecrecover 算法[4]从消息和签名中还原出公钥即可。

裸的 SM2 签名也具有这样的性质（只要消息的摘要 $e$ 与公钥无关）。

SM2 签名的过程[5]是（记曲线的基点、曲线的阶、曲线基域的阶、公钥、私钥、消息的摘要分别为 $G,n,q,K,k,e$）：

1.  在 $[1, n)$ 中随机选取 $d$；
1. $R=[d]G$；
2. $r\equiv e+R.x \pmod n$；
3. $s\equiv (1+k)^{-1}(d-rk)\pmod n$；
4. 输出 $(r,s)$。

变换 4 中的式子，可以得到 $k\equiv (r+s)^{-1}d-(r+s)^{-1}s\pmod n$；同乘 $G$ 可以得到 $[k]G=[(r+s)^{-1}d]G-[(r+s)^{-1}s]G$；即 $K=[(r+s)^{-1}]R-[(r+s)^{-1}s]G$。

所以只要确定 $R$ 就可以确定 $K$。

而考虑 3 中的式子，注意到 $R.x\leq q$，所以 $0\leq R.x+e\leq q+e$，将 3 式中的式子变形为 $r=e+R.x-un$，其中 $u$ 为满足 $0\leq un\leq q+e$ 的整数。

枚举所有可能的 $u$ 与 $R.y$ 的奇偶情况即可得到 $R$，总共有 $2\lceil\frac{q+e}{n}\rceil$ 种可能，都试一试即可。

###  还原 Carol 的公钥

这里使用的证明是Pederson承诺的零知识范围证明的一种，基于若干个环签名来实现。有关这种范围证明的细节可以参考 [11] 的第 5 节。

环签名是一种签名技术，验证者可以确定签名是否是由由若干个公钥组成的公钥环中的某个公钥所对应的私钥生成，但无法确认签名者具体是用哪个私钥签的。

环签名本身具有完全匿名性，即使公钥系统被破解也不会泄露“签名者是用哪个私钥进行签名”的信息（但可伪造签名），但这里错误地复用了名为 back's LSAG 的一次性环签名，它的匿名性依赖 ECDDHP 的困难性。有关一次性环签名的细节可以参考[7]。

完成这道题目不需要了解一次性环签名的其他细节，只需要知道它带有一个“密钥镜像” $I=k\mathcal{H}_p(K)$，其中 $\mathcal{H}_p$ 是把参数映射到椭圆曲线上的点的密码学安全哈希函数，$K,k$ 分别为签名实际使用的的公钥和私钥，其中 $K=kG$，$G$ 为椭圆曲线的基点。

这里选用了一个特殊的曲线，看起来只是使用了阶更大的曲线使得 $[0,2^{256}]$ 的范围证明有意义，但这同时也是一条 embedding degree 为 $1$[9] 的 type-A pairing-friendly curve[10]，因此可以构造双线性映射 $e$。

双线性映射 $e$ 的存在可以破坏 ECDDHP 的困难性，枚举公钥环上的每个公钥，只有实际被用来签名的公钥 $K$ 满足 $e(K,\mathcal{H}_p(K))=e(kG,\mathcal{H}_p(K))=e(G,\mathcal{H}_p(K))^k=e(G,k\mathcal{H}_p(K))=e(G,I)$。

此外，也可以使用 subgroup membership test，由于所给的椭圆曲线群并不是一个循环群，有极大的可能使得只有实际被用来签名的公钥 $K$ 满足 $I,\mathcal{H}_p(K)$ 在一个循环子群上（或者说，线性相关）。而当且仅当 $P,Q$ 在一个循环子群上时，它们的 Weil 配对 $e(P,Q)=1$。

从而我们可以从范围证明中恢复 Carol 公钥的 $x$ 坐标，这样 Carol 的公钥只有两种可能，都试一试即可。

### 构造双花交易

由于你只有一个 token 却需要给人两个 token，所以需要构造双花交易。

这里使用了类似 Monero 币的交易系统（这里删去了匿名地址等内容，因此对密钥镜像做了不影响分析的小修改），因此判断是否双花采用的是判断你的签名是否有同样的密钥镜像。

有关双花的漏洞可以用 `cryptonote double spend` 等关键字搜到一个漏洞披露[8]，也可以在[6]中看到对相应补丁的注释。有关此漏洞的简要描述如下：

ed25519 曲线的阶是 $8l$ 其中 $l$ 是一个大素数，而 ed25519 推荐的基点的阶是 $l$，而由于 ed25519 曲线是一个循环群，因此我们可以找到阶在范围 $[2,8]$  的 $7$ 个点，记其中一点为 $X$。而 $c_iI=c_i(I+X)$ 在 $c_i$ 均为 $8$ 的倍数时恒成立。因此，$c_i$ 满足条件时，可以用 $I'=I+X$ 来冒充密钥镜像，达到双花的目的。

#### 非预期解

环签名验证采用了 $c_{i+1}=\mathcal{H}_n(m,L_i,R_i)$，而实际上应该加入密钥镜像 $I$ 变为 $c_{i+1}=\mathcal{H}_n(I,m,L_i,R_i)$ 确保 $I$ 先于 $c_{\pi}$ 产生，否则可根据 $c_{\pi}$ 任意伪造 $I$:

签名时首先令 $R_i=\beta\mathcal{H}_p(\ldots),\beta\neq \alpha$，最后令 $I=\frac{\beta-r_{\pi}}{c_{\pi}}\mathcal{H}_p(\ldots)$ 即可。

看来出题人还是要对 Fiat-Shamir 有敬畏之心QaQ。

### 参考文献

1. HyperLedger Fabric Key Concepts, <https://hyperledger-fabric.readthedocs.io/en/latest/blockchain.html>.
2. Saberhagen, N.V. (2013). CryptoNote v 2.0.
3. Convert between short weierstrass curve and montgomery curve, <https://blog.csdn.net/mutourend/article/details/96293640>.
4. ECRecover and Signature Verification in Ethereum, <https://coders-errand.com/ecrecover-signature-verification-ethereum/>.
5. SM2 Digital Signature Algorithm, <https://datatracker.ietf.org/doc/html/draft-shen-sm2-ecdsa-02>
6. Alonso, K.M. (2018). Zero to Monero : First Edition.
7. koe, K.M. (2020). Zero to Monero: Second Edition.
8. Disclosure of a Major Bug in CryptoNote Based Currencies, <https://www.getmonero.org/2017/05/17/disclosure-of-a-major-bug-in-cryptonote-based-currencies.html>.
9. Koblitz, N., & Menezes, A. (2005). Pairing-based cryptography at high security levels. In IMA International Conference on Cryptography and Coding (pp. 13-36). Springer, Berlin, Heidelberg.
10. Chatterjee, S., Menezes, A., & Rodrıguez-Henrıquez, F. (2016). On instantiating pairing-based protocols with elliptic curves of embedding degree one. IEEE Transactions on Computers, 66(6), 1061-1070.
11. Noether, S., & Mackenzie, A. (2016). Ring confidential transactions. Ledger, 1, 1-18.
