import secret, hashlib
assert hashlib.sha256((hex(secret.bob_pub[0]) + hex(secret.bob_pub[1])).encode()).hexdigest() == "3a66fb0241a28b7439ed71b0a202bd8a904afb8175c1a3247f6335814ccb4969"
assert hashlib.sha256((hex(secret.carol_pub[0]) + hex(secret.carol_pub[1])).encode()).hexdigest() == "eca9746a961fdb403df08880668ebd5322883addb84d3d9d385d54fcfd337dea"