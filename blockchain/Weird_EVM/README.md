# Weird EVM

## description

emm… this EVM looks strange.

## depoly

See depoly directory. Put hints/geth into this(depoly) directory, and then execute docker compose -d.

You will get faucet(port:8080)/game(port:2000)/geth filtered RPC(port:8454)

The flag is `ACTF{tH1S_i5_a_h@rd_$0rK_CaL!Ed_A@a_be20049f3607a32f660677281df6febd4e6}`.

## hints

See hints directory.

There are 3 big binaries in this directory.

To ease the pressure of the repo, here are the netdisk link:

https://pan.baidu.com/s/1K7D_h_pCksRYQnVmPqxGyw?pwd=ku74

The administrator can issue the following 4 hints according to the level of the players and their situation.

0. players can found the changes by try `eth.call` on the RPC service.
1. the modified geth (players can found the changes locally.)
2. ~~the modified solc (players can found the changes by compiling their own contracts.)~~ (`debug.traceTransaction` is enough)
3. ~~the modified evm (players can decompile bytecode into asm directly)~~ (`debug.traceTransaction` is enough)
4. provide sources(solution/bn256.sol solution/ed25519.sol solution/Game.sol)

## solution

See exploits directory. (Require put hints/solc into this(solution) directory)

Modify `w3 = Web3(Web3.HTTPProvider('http://127.0.0.1:8545'))` into correct geth RPC address.

And run it by python3.

## patches

See patches directory.

1. the changes of geth ([diff patch](./patches/0001-Add-precompiled-and-marshal-opcodes.patch) based on [commit 01e5e9c2c3fa1cf7a9747148dca22d59ff9839b6](https://github.com/ethereum/go-ethereum/commit/01e5e9c2c3fa1cf7a9747148dca22d59ff9839b6))
2. the changes of solc (based on [0.8.15](https://github.com/ethereum/solidity/releases/tag/v0.8.15), change [libevmasm/Instruction.h](./patches/Instruction.h))
3. the changes of panoramix decompiler(may be useful) to decompile such opcodes (base on [commit 876a8de26e8ed98c45e77a959d3f0b248d9efa44](https://github.com/palkeo/panoramix/commit/876a8de26e8ed98c45e77a959d3f0b248d9efa44), change [panoramix/utils/opcode_dict.py](./patches/opcode_dict.py))

## writeup

1. In this geth, opcodes has been changed, detect it by tries and compare. (after the release of hint1, use `debug_traceTransaction` in RPC or use `debug.traceTransaction` in interactive JavaScript environment is very useful, found `OpCode`'s `String` method to get the corresponding map is another way. For specific restore methods, refer to `hints/opcodeTableGotter.sh`)
2. In this geth, there are 6 new precompiled contracts (point-add, scalar-mult, hash-to-point on both ed25519 and BN256G1(compressed)) (just serach for `RequiredGas` in IDA, you can found all precompiled contracts' names and features in the correspondding `Run` function)
3. The depolyed contracts give you $xG_2$(on ed25519) $H_2$(on BN256G1), require you to calculate $xH_2$ and provide a DLEQ proof for $xG_2$ and $xH_2$. (You can guess the feature based on the hint() function and other information in ABI)
4. This MSAG-based range-proof-like DLEQ proof acorss groups can be found in this [note](https://www.getmonero.org/resources/research-lab/pubs/MRL-0010.pdf).
5. To solve this problem, you do not need to implement anything about blinders.

Changes can be found in [patches](#patches), official solution can be found in [solution](#solution).
