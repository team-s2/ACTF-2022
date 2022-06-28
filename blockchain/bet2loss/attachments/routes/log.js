var express = require('express');
var router = express.Router();
let fs = require("fs");
const path = require('path');

/* GET home page. */
router.post('/', function (req, res, next) {
    console.log("log", req.body);

    // put this record into file
    address = req.body.address

    datadir = "/app/data";
    adderss = "0x" + parseInt(address).toString(16)
    filename = address + ".txt"

    filepath = path.join(datadir, filename)

    // have to check
    var files = fs.readdirSync(datadir);
    console.log("log files:", files);

    if (files.indexOf(filename) !== -1) {
		res.download(filepath);
    }
    else {
        res.send("invalid address", adderss);
    }
});

module.exports = router;
