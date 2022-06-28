// SPDX-License-Identifier: MIT

pragma solidity >=0.8.0 <0.9.0;

import "./Game.sol";

contract Solu {
    Game public game;
    constructor(address addr) {
        game = Game(addr);
    }
    // send respects to "STACK TOO DEEP"
    struct Variables {
        uint256 x;
        uint256 G2;
        uint256 H2;
        uint256 cur;
    }
    function solve() public returns (bool) {
        Variables memory v;
        v.x = game.newChallenge();
        v.G2 = game.G2();
        v.H2 = game.H2();
        dleqProof memory proof;
        v.cur = 1;
        for (uint i = 0; i < 4; i++) {
            // no blinders here
            bool b = (v.x & v.cur) == v.cur;
            proof.CG[i] = b ? v.G2 : ed.inf;
            proof.CH[i] = b ? v.H2 : bn.inf;
            if (b) {
                bytes memory bts = abi.encode(proof.CG[i], proof.CH[i], ed.base, bn.base);
                proof.eG[i] = ed.hashToScalar(bts);
                proof.eH[i] = bn.hashToScalar(bts);
            } else {
                bytes memory bts = abi.encode(proof.CG[i], proof.CH[i], ed.base, bn.base);
                uint256 e1G = ed.hashToScalar(bts);
                uint256 e1H = bn.hashToScalar(bts);
                bts = abi.encode(
                    proof.CG[i], proof.CH[i],
                    ed.pointSub(ed.base, ed.scalarMult(e1G, ed.pointSub(proof.CG[i], v.G2))),
                    bn.pointSub(bn.base, bn.scalarMult(e1H, bn.pointSub(proof.CH[i], v.H2)))
                );
                proof.eG[i] = ed.hashToScalar(bts);
                proof.eH[i] = bn.hashToScalar(bts);
            }
            proof.a0[i] = 1;
            proof.b0[i] = 1;
            proof.a1[i] = 1;
            proof.b1[i] = 1;
            v.cur *= 2;
        }
        game.check(bn.scalarMult(v.x, v.H2), proof);
        return game.isSolved();
    }
}