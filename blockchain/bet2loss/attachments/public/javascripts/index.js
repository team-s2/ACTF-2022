const initialize = () => {

    const onboardButton = document.getElementById('connectButton');
    const airdropButton = document.getElementById('getAirdrop');
    const betButton = document.getElementById('startBet');
    const getFlagButton = document.getElementById('getFlag');
    const refreshButton = document.getElementById('refreshBtn');
    const downloadButton = document.getElementById('downloadContent');

    const activeAddressShow = document.getElementById('activeAddress');
    const gameContractShow = document.getElementById('gameContract');
    const balanceShow = document.getElementById('remainBalance');
    const chanceShow = document.getElementById('remainChance');
    const logShow = document.getElementById('downloadData');
    const flagShow = document.getElementById('flagData');

    let activeAccount

    //Created check function to see if the MetaMask extension is installed
    const isMetaMaskInstalled = () => {
        //Have to check the ethereum binding on the window object to see if it's installed
        const { ethereum } = window;
        return Boolean(ethereum && ethereum.isMetaMask);
    };

    // Till now we can believe the metamask is installed
    function handleAccountChanged(newAccounts) {
        if (newAccounts.length > 0)
            activeAccount = newAccounts[0];
        else
            activeAccount = "";
        console.log("switch active account to", activeAccount);
        activeAddressShow.innerHTML = activeAccount;

        refreshStatus();
    }

    // refrash
    function refreshStatus() {
        updateBalance();
        updateChance();
    }

    const updateBalance = async () => {
        // "balances(address)": "27e235e3",
        var gameContract = gameContractShow.innerHTML;
        var balanceParam = activeAccount.slice(2).padStart(64, '0');
        var balanceData = "0x27e235e3" + balanceParam;
        try {
            var currentBalance = await ethereum.request({
                method: 'eth_call',
                params: [
                    {
                        to: gameContract,
                        from: activeAccount,
                        data: balanceData,
                        gas: '0x493e00'
                    },
                ]
            })
            currentBalance = parseInt(currentBalance);

            balanceShow.innerHTML = currentBalance;

        } catch (err) {
            console.error('Error on get account balance', err);
            balanceShow.innerHTML = 'no idea';
        }
    }

    const updateChance = async () => {
        // "logger(address)"
        var gameContract = gameContractShow.innerHTML;
        var chanceParam = activeAccount.slice(2).padStart(64, '0');
        var chanceData = "0x99866a97" + chanceParam;
        try {
            var currentCount = await ethereum.request({
                method: 'eth_call',
                params: [
                    {
                        to: gameContract,
                        from: activeAccount,
                        data: chanceData,
                        gas: '0x493e00'
                    },
                ]
            })
            currentCount = parseInt(currentCount);
            remainCount = 20 - currentCount;

            chanceShow.innerHTML = remainCount;

            if (remainCount == 0) {
                downloadButton.hidden = false;
            }

        } catch (err) {
            console.error('Error on get account chances', err);
            chanceShow.innerHTML = 'no idea';
        }
    }

    const onClickConnect = async () => {
        try {
            // Will open the MetaMask UI
            // You should disable this button while the request is pending!
            await ethereum.request({ method: 'eth_requestAccounts' });

            ethereum.on('accountsChanged', handleAccountChanged)

            try {
                const newAccounts = await ethereum.request({
                    method: 'eth_accounts',
                })
                handleAccountChanged(newAccounts)

                betButton.disabled = false;
                getFlagButton.disabled = false;
                airdropButton.disabled = false;
                refreshButton.disabled = false;

            } catch (err) {
                console.error('Error on init when getting accounts', err)
            }

        } catch (error) {
            console.error(error);
        }
    };

    const syncBetServer = async (betval, betmod) => {
        var syncReq = new XMLHttpRequest();
        syncReq.open("POST", "/sync");
        syncReq.addEventListener("load", () => { });
        syncReq.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        var senddata = "address=" + activeAccount + "&value=" + betval.toString() + "&mod=" + betmod.toString();
        syncReq.send(senddata);
    };

    const onClickBet = async () => {
        // check argument first
        const betvalShow = document.getElementById('inputBetVal');
        var betval = betvalShow.value;
        const betvalMod = document.getElementById('inputBetMod');
        var betmod = betvalMod.value;
        if (!betval || !betmod) {
            alert("invalid input");
            return;
        }
        var _betval = parseInt(betval)
        var _betmod = parseInt(betmod)

        if (isNaN(_betval) || isNaN(_betmod)) {
            alert("invalid input");
            return;
        }
        if (_betval > _betmod || _betmod < 1 || _betmod > 12) {
            alert("invalid input");
            return;
        }
        console.log("value", _betval, "mod", _betmod);

        // using metamask to send transaction
        // make it cloneable
        var gameContract = gameContractShow.innerHTML;

        // stuipd data encoding
        var encoded_data = "0x6ffcc719"; // "bet(uint256,uint256)"
        var hex_value = _betval.toString(16).padStart(64, '0')
        var hex_mod = _betmod.toString(16).padStart(64, '0')
        encoded_data = encoded_data + hex_value + hex_mod

        console.log("debug, encoded data:", encoded_data);

        try {
            ethereum.request({
                method: 'eth_sendTransaction',
                params: [
                    {
                        to: gameContract,
                        from: activeAccount,
                        data: encoded_data,
                        gas: '0x493e00'
                    },
                ],
            });

            // send bet information to server
            syncBetServer(_betval, _betmod);
        } catch (error) {
            console.error(error);
        }

    };

    const onClickGetFlag = async () => {
        var syncReq = new XMLHttpRequest();
        syncReq.open("POST", "/flag");
        syncReq.responseType = 'text';
        syncReq.onload = function () {
            if (syncReq.readyState === syncReq.DONE) {
                if (syncReq.status === 200) {
                    flagShow.innerHTML = syncReq.responseText;
                }
            }
        };
        syncReq.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        var senddata = "address=" + activeAccount;
        syncReq.send(senddata);
    };

    const onClickLog = async () => {
        var syncReq = new XMLHttpRequest();
        syncReq.open("POST", "/log");
        syncReq.responseType = 'text';
        syncReq.onload = function () {
            if (syncReq.readyState === syncReq.DONE) {
                if (syncReq.status === 200) {
                    logShow.innerHTML = syncReq.responseText;
                }
            }
        };
        syncReq.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        var senddata = "address=" + activeAccount;
        syncReq.send(senddata);
    };

    const onClickGetAirDrop = async () => {
        // "airdrop()": "3884d635",
        var encoded_data = '0x3884d635';
        var gameContract = gameContractShow.innerHTML;
        try {
            ethereum.request({
                method: 'eth_sendTransaction',
                params: [
                    {
                        to: gameContract,
                        from: activeAccount,
                        data: encoded_data,
                        gas: '0x493e00'
                    },
                ],
            });
            airdropButton.disabled = true;
        } catch (error) {
            console.error(error);
        }
    };

    function initButton() {
        if (!isMetaMaskInstalled()) {
            alert("you didn't install MetaMask")
        } else {
            // onboardButton
            onboardButton.innerText = 'Connect MetaMask';
            onboardButton.onclick = onClickConnect;
            onboardButton.disabled = false;

            // startBetButton
            betButton.innerText = 'BET';
            betButton.onclick = onClickBet;
            betButton.disabled = true;

            // getFlagButton
            getFlagButton.innerText = 'GETFLAG';
            getFlagButton.onclick = onClickGetFlag;
            getFlagButton.disabled = true;

            // airdropButton
            airdropButton.innerText = 'AIRDROP'
            airdropButton.onclick = onClickGetAirDrop;
            airdropButton.disabled = true;

            // refreshButton
            refreshButton.innerText = "REFRESH";
            refreshButton.onclick = refreshStatus;
            refreshButton.disabled = true;

            downloadButton.hidden = true;
            downloadButton.innerText = "LOG"
            downloadButton.onclick = onClickLog;

        }
    }

    initButton();
}

window.addEventListener('DOMContentLoaded', initialize)