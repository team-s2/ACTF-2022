var express = require('express');
var router = express.Router();
let fs = require("fs");
const path = require('path');

/* GET home page. */
router.post('/', function (req, res, next) {
    console.log("flag", req.body);

    // address check
    var address = req.body.address;
    if (!address.startsWith('0x') || isNaN(parseInt(address, 16))) {
        res.send("FAIL");
        return;
    }

    var web3 = res.app.locals.web3;
    var sealer = res.app.locals.game_address;
    var privkey = res.app.locals.game_privkey;
    var contract = res.app.locals.game_contract_obj;
    var contract_addr = res.app.locals.game_contract;

    if (address == sealer) {
        res.send("FAIL");
        return;
    }

    const checkWin = async () => {
        try {
            checkTransaction = await web3.eth.accounts.signTransaction(
                {
                    from: sealer,
                    to: contract_addr,
                    data: contract.methods.checkWin(address).encodeABI(),
                    gas: '4194304', // 0xc00000
                },
                privkey
            );

            Receipt = await web3.eth.sendSignedTransaction(
                checkTransaction.rawTransaction
            );

            console.log(Receipt);
            if (Receipt.status == true) {
                res.send(process.env.WINFLAG);
            }
            else {
                res.send("FAIL");
            }
        }
        catch (err) {
            console.log(err);
            res.send("FAIL");
        }
    }

    checkWin();
});

module.exports = router;
