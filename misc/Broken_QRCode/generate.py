from qrcode import *

rick = [
    "We're no strangers to love",
    "You know the rules and so do I",
    "A full commitment's what I'm thinking of",
    "You wouldn't get this from any other guy",
    "I just wanna tell you how I'm feeling",
    "Gotta make you understand",
    "Never gonna give you up",
    "Never gonna let you down",
    "Never gonna run around and desert you",
    "Never gonna make you cry",
    "Never gonna say goodbye",
    "Never gonna tell a lie and hurt you",
]

for i, each in enumerate(rick):
    qr = QRCode(
        version=5,
        error_correction=ERROR_CORRECT_H,
    )
    qr.add_data(each)
    qr.make_image().save(f"qrs/{i}.png")


part2 = "0100000100000011000101001100011110010101111101101011011011100011000001110111010111110101000101010010010000110110111101100100001100110111110100001110110000011000011111000010001010011100010011010100101001000100" # 1Ly_kn0w_QRCod3}
hex_value = ""

for i in range(0, len(part2), 8):
    part = part2[i:i+8]
    hex_value += f"{hex(int(part, 2))[2:]:>02}"

print(hex_value)
# 410314c795f6b6e30775f5152436f64337d0ec187c229c4d4a44