var express = require('express');
var router = express.Router();
let fs = require("fs");
const path = require('path');

/* GET home page. */
router.post('/', function (req, res, next) {
    console.log("sync", req.body);

    // address check
    var address = req.body.address;
    if (!address.startsWith('0x') || isNaN(parseInt(address, 16))) {
        res.send("invalid address", address);
        return;
    }

    address = "0x" + parseInt(address, 16).toString(16)
    filename = address + ".txt"
    // put this record into file
    filedata = JSON.stringify(req.body)
    
    datadir = "/app/data";
    filepath = path.join(datadir, filename)

    fs.appendFile(filepath, filedata, function (err) {
        if (err) {
            res.send("FAIL");
        }
        res.send("OK");
    });
});

module.exports = router;
