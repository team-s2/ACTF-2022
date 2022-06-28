// requirements part
var createError = require('http-errors');
var express = require('express');
var path = require('path');
var cookieParser = require('cookie-parser');
var logger = require('morgan');
var Web3 = require('web3');
let fs = require("fs");
const { recoverKeystore } = require('ethereum-keystore')

var indexRouter = require('./routes/index');
var syncRouter = require('./routes/sync');
var logRouter = require('./routes/log')
var flagRouter = require('./routes/flag')

var app = express();

// start to initialize the game contract
var web3 = new Web3(process.env.WEB3_PROVIDER);
var provider = web3.currentProvider;
// check health
if (provider == null) {
  console.log("web3 initialized failed, exit")
  process.exit()
}
console.log("current running provider:", provider);

// we shall access web3 through provider, put it into global
app.locals.providerIP = process.env.IP_ADDR;
app.locals.web3 = web3;

deployContract = async () => {
  // then we start deploy game contract here
  contractjson = JSON.parse(fs.readFileSync("contracts/BetToken.json"));
  contractabi = contractjson["abi"];
  contractbytecode = contractjson["bytecode"];

  betContract = new web3.eth.Contract(contractabi);
  contractDeployTx = betContract.deploy({ data: contractbytecode });

  // we need address and privkey here
  datadir = process.env.DATADIR;
  keystore_path = path.join(datadir, "keystore.txt")
  keystorejson = JSON.parse(fs.readFileSync(keystore_path));
  password_path = path.join(datadir, "password.txt")
  passwordcontent = fs.readFileSync(password_path).toString();
  sealeraddr = keystorejson["address"];
  privateKey = await recoverKeystore(keystorejson, passwordcontent)

  console.log("recover seal address:", sealeraddr, "its privkey:", privateKey)
  // hope we get private key for now

  app.locals.game_contract_obj = betContract;
  app.locals.game_address = sealeraddr;
  app.locals.game_privkey = privateKey;

  createTransaction = await web3.eth.accounts.signTransaction(
    {
      from: sealeraddr,
      data: contractDeployTx.encodeABI(),
      gas: '4194304', // 0xc00000
    },
    privateKey
  );

  console.log("signed address:", createTransaction.transactionHash);

  createReceipt = await web3.eth.sendSignedTransaction(
    createTransaction.rawTransaction
  );

  console.log("created address:", createReceipt.contractAddress);
  app.locals.game_contract = createReceipt.contractAddress;
};

deployContract();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', [indexRouter]);
app.use('/sync', [syncRouter]);
app.use('/log', [logRouter]);
app.use('/flag', [flagRouter]);


// catch 404 and forward to error handler
app.use(function (req, res, next) {
  next(createError(404));
});

// error handler
app.use(function (err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});

module.exports = app;