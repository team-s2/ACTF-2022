from web3 import Web3
from web3.middleware import geth_poa_middleware
import json, os, sys

force = '-f' in sys.argv

def compile_sol(name):
    path = name + '.sol'
    cache_path = name + '.cache.json'
    if not force and os.path.isfile(cache_path) and os.stat(path).st_mtime < os.stat(cache_path).st_mtime:
        return json.load(open(cache_path))
    os.system(f'./solc --abi --bin {path} -o build --optimize --optimize-runs 10000 --overwrite')
    bytecode = open(f'build/{name}.bin').read()
    abi = json.load(open(f'build/{name}.abi'))
    ret = (bytecode, abi)
    json.dump(ret, open(cache_path, 'w'))
    return ret

# connect to server
#w3 = Web3(Web3.IPCProvider('/testdata/geth.ipc'))
w3 = Web3(Web3.HTTPProvider('http://127.0.0.1:8545'))
w3.middleware_onion.inject(geth_poa_middleware, layer=0)

# unlock account
acct = w3.eth.account.create()
print('account:', acct.address)
print("pay money into it")
input()
# see balance
print('Balance:', w3.eth.get_balance(acct.address))

# constant
ADDITIONAL_GAS_LIMIT = 100000

nonceId = w3.eth.getTransactionCount(acct.address)


def sendRawTrans(caller):
    global nonceId
    gas = caller.estimateGas({
        'from': acct.address,
    })
    print('gas:', gas)
    txn = caller.buildTransaction({
        'from': acct.address,
        'nonce': nonceId,
        'gas': gas + ADDITIONAL_GAS_LIMIT,
        'gasPrice': w3.toWei(1, 'gwei')
    })
    nonceId += 1
    signed = acct.signTransaction(txn)
    return w3.eth.sendRawTransaction(signed.rawTransaction)

# depoly contract
def depoly(name, *args):
    bytecode, abi = compile_sol(name)
    option = {'from': acct.address}
    Creation = w3.eth.contract(abi=abi, bytecode=bytecode)
    callee = Creation.constructor(*args)
    option['gas'] = callee.estimateGas(option) + ADDITIONAL_GAS_LIMIT
    tx_hash = sendRawTrans(callee)
    tx_receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
    print(f'{name} contract address:', tx_receipt.contractAddress)
    contract = w3.eth.contract(address=tx_receipt.contractAddress, abi=abi)
    contract_address = tx_receipt.contractAddress
    return bytecode, abi, contract, contract_address

_, Game_abi = compile_sol('Game')

# _, _, _, Game_contract_address = depoly('Game')

Game_contract_address = Web3.toChecksumAddress(input('address of contract: '))
Game = w3.eth.contract(address=Game_contract_address, abi=Game_abi)

_, _, Solu, _ = depoly('Solu', Game_contract_address)

callee = Solu.functions.solve()

print('local:', callee.call())

tx_hash = sendRawTrans(callee)
tx_receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
print('remote:', Game.functions.isSolved().call())
