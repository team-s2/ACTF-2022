document.addEventListener("DOMContentLoaded", function () {
	componentHandler.upgradeElement(document.getElementById('toastDiv'));

	let list = document.getElementById("list");
	function lresize() {
		list.style.marginLeft = Math.max((document.body.scrollWidth - Math.max(475, list.scrollWidth)) / 2, 0) + "px";
		list.style.marginTop = Math.max((document.body.scrollHeight - header.scrollHeight - Math.max(524, list.scrollHeight)) / 2, 0) + "px";
	}
	window.addEventListener("resize", lresize);
	lresize();

	function loadWeb3(provider, gameAddr) {
		const web3 = new Web3(provider);
		window.web3 = web3;
		const acct = web3.eth.accounts.create();
		document.getElementById('my_account').innerHTML = acct.address;
		document.getElementById('request').addEventListener('click', function () {
			document.getElementById('request').MaterialButton.disable();
			setTimeout(() => {
				document.getElementById('request').MaterialButton.enable();
			}, 60000);
			let formData = new FormData();
    		formData.append('address', acct.address);
			fetch(provider + 'faucet' ,{
				method: 'POST',
				body: formData
			}).then(res => {
				res.text().then(v => {
					const text = (res.ok ? 'Success' : 'Fail') + ': ' + v;
					document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: text });
					setTimeout(() => refreshAll(), 25000);
				});
			});
		});
		const Game = new web3.eth.Contract([{"inputs":[],"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"previousOwner","type":"address"},{"indexed":true,"internalType":"address","name":"newOwner","type":"address"}],"name":"OwnershipTransferred","type":"event"},{"inputs":[],"name":"airdrop","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"}],"name":"airdroprecord","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"}],"name":"balances","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"uint256","name":"value","type":"uint256"},{"internalType":"uint256","name":"mod","type":"uint256"}],"name":"bet","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"candidate","type":"address"}],"name":"checkWin","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"}],"name":"logger","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"owner","outputs":[{"internalType":"address","name":"","type":"address"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"renounceOwnership","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"seal","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"newOwner","type":"address"}],"name":"transferOwnership","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"transferTo","outputs":[],"stateMutability":"pure","type":"function"}], gameAddr);
		window.Game = Game;
		let nonce = 0;
		async function buildTransaction(callee, isNotOK) {
			try {
				nonce = Math.max(await web3.eth.getTransactionCount(acct.address), nonce);
				const tx = {
					from: acct.address,
					gasPrice: await web3.eth.getGasPrice(),
					gas: (await callee.estimateGas({from: acct.address})) + 10000,
					to: callee._parent._address,
					value: "0",
					data: callee.encodeABI(),
					nonce: nonce
				};
				nonce++;
				const signed = await acct.signTransaction(tx);
				document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Created: ' + signed.transactionHash });
				web3.eth.sendSignedTransaction(signed.rawTransaction).then(v => {
					document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Success: ' + signed.transactionHash });
					setTimeout(() => refreshAll(), 1000);
				}).catch(err => {
					document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Failed: ' + (err ? err.toString() : 'Unknown') });
					if (isNotOK) isNotOK();
				});
				return;
			} catch (err) {
				document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Failed: ' + (err ? err.toString() : 'Unknown') });
				if (isNotOK) isNotOK();
			}
		}
		function refreshAll() {
			web3.eth.getBalance(acct.address).then(v => {
				document.getElementById('my_eth_chance').innerHTML = web3.utils.fromWei(v, 'ether');
			});
			Game.methods.balances(acct.address).call().then(v => {
				document.getElementById('my_balance').innerHTML = v;
			});
			Game.methods.logger(acct.address).call().then(v => {
				document.getElementById('my_chance').innerHTML = 20 - parseInt(v);
			});
		}
		setInterval(() => refreshAll(), 25000);
		document.getElementById('refresh').addEventListener('click', () => {
			document.getElementById('refresh').MaterialButton.disable();
			setTimeout(() => {
				document.getElementById('refresh').MaterialButton.enable();
			}, 1500);
			refreshAll();
		});
		refreshAll();
		document.getElementById('new_airdrop').addEventListener('click', function () {
			document.getElementById('new_airdrop').MaterialButton.disable();
			buildTransaction(Game.methods.airdrop(), () => {
				document.getElementById('new_airdrop').MaterialButton.enable();
			});
		});
		const syncBetServer = async (betval, betmod) => {
			var syncReq = new XMLHttpRequest();
			syncReq.open("POST", "/sync");
			syncReq.addEventListener("load", () => { });
			syncReq.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
			var senddata = "address=" + acct.address + "&value=" + betval.toString() + "&mod=" + betmod.toString();
			syncReq.send(senddata);
		};
		document.getElementById('do_bet').addEventListener('click', function () {
			document.getElementById('do_bet').MaterialButton.disable();
			setTimeout(() => {
				document.getElementById('do_bet').MaterialButton.enable();
			}, 1500);
			const value = Number(document.getElementById('bet_value').value);
			const mod = Number(document.getElementById('bet_mod').value);
			if (Number.isNaN(value) || Number.isNaN(mod)) {
				document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Number is NaN' });
				return;
			}
			if (!(mod >= 2 && mod <= 12)) {
				document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Mod not in [2, 12]' });
				return;
			}
			if (!(value >= 0 && value <= mod)) {
				document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'Mod not in [0, mod)' });
				return;
			}
			if (parseInt(document.getElementById('my_balance').innerHTML) < 10) {
				document.getElementById('toastDiv').MaterialSnackbar.showSnackbar({ message: 'No money' });
				return;
			}
			buildTransaction(Game.methods.bet(value, mod));
			syncBetServer(value, mod);
		});
		const onClickGetFlag = async () => {
			document.getElementById('check_flag').MaterialButton.disable();
			setTimeout(() => {
				document.getElementById('check_flag').MaterialButton.enable();
			}, 1500);
			var syncReq = new XMLHttpRequest();
			syncReq.open("POST", "/flag");
			syncReq.responseType = 'text';
			syncReq.onload = function () {
				if (syncReq.readyState === syncReq.DONE) {
					if (syncReq.status === 200) {
						if ('FAIL' === syncReq.responseText) return;
						document.getElementById('my_flag').innerHTML = syncReq.responseText;
					}
				}
			};
			syncReq.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
			var senddata = "address=" + acct.address;
			syncReq.send(senddata);
		};
		document.getElementById('check_flag').addEventListener('click', onClickGetFlag);

		// !!! THE BELOW FUNCTION IS DEBUG ONLY !!!
		const onClickLog = async () => {
			var syncReq = new XMLHttpRequest();
			syncReq.open("POST", "/log");
			syncReq.responseType = 'text';
			syncReq.onload = function () {
				if (syncReq.readyState === syncReq.DONE) {
					if (syncReq.status === 200) {
						document.getElementById('debug_log').innerHTML = syncReq.responseText;
					}
				}
			};
			syncReq.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
			var senddata = "address=" + acct.address;
			syncReq.send(senddata);
		};
		// !!! THE ABOVE FUNCTION IS DEBUG ONLY !!!
	}

	function loadBasicCfg(cfg) {
		if (!cfg.provider.endsWith('/')) cfg.provider += '/';
		document.title = cfg.title;
		document.getElementById("app_title").innerHTML = cfg.title;
		document.getElementById('geth_url').innerHTML = cfg.provider;
		document.getElementById('game_addr').innerHTML = cfg.contract;
		loadWeb3(cfg.provider, cfg.contract);
	}
	fetch('/config').then(res => res.json()).then(loadBasicCfg);
	// loadBasicCfg({ title: 'bet2loss', provider: "http://123.60.36.208:8545/", contract: '0x227A3800cC2BD99106776EF4DB54F43aBB2eD876' });
});
