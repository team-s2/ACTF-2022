#!/bin/bash

mkdir /testdata

./geth --datadir /testdata --vmdebug --nodiscover --dev --verbosity 0 &

cat > /testdata/run.js <<- EOF
const opcodes = [];
for (let i = 0; i < 256; i++) {
	let log = debug.traceCall({
		"to": null,
		"data": "0x" + (i<16?'0':'') + i.toString(16) + "000000000000000000000000000000000000000000000000000000000000000000"
	}, "latest", {"disableStorage": true, "disableMemory": true});
	let op = log.structLogs[0].op;
	if (op.includes('not defined')) continue;
	opcodes.push([i, op]);
}
console.log(JSON.stringify(opcodes));
EOF

./geth --verbosity 0 js /testdata/run.js

#kill -9 `ps aux | grep geth | grep testdata | awk '{print $2}' | head -n 1`

rm -rf /testdata