var express = require('express');
var router = express.Router();
var path = require('path');

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile(path.join(__dirname, '..', 'public', 'index.html')); // new version
  // res.render('index', { title: 'bet2loss', provider: "http://" + res.app.locals.providerIP + ":8545", contract: res.app.locals.game_contract }); // old version
});

router.get('/config', function(req, res, next) {
  res.json({ title: 'bet2loss', provider: "http://" + res.app.locals.providerIP + ":8545", contract: res.app.locals.game_contract });
});

module.exports = router;
