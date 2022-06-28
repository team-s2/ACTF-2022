from qrcode import *
from PIL import Image, ImageOps, ImageChops

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

for i, content in enumerate(rick):
    img1 = Image.open(f"qrcodes/{i}.jpg").convert("RGB")
    img2 = make(content, version=5, error_correction=ERROR_CORRECT_H).convert("RGB")
    cropped_img1 = img1.crop(ImageOps.invert(img1).getbbox())
    cropped_img2 = img2.crop(ImageOps.invert(img2).getbbox()).resize(cropped_img1.size)
    ImageChops.difference(cropped_img1, cropped_img2).save(f"diff/{i}.jpg")