# FFSK - WriteUp

### 0. Intro

In the game period, only one team had solved this problem: MapleBacon, a genius team at the University of British Columbia. I’m happy about their praise, but after checking their solution I think what truly “impressive” is their creativity and persistence.

**Strongly recommend reading their awesome solution: [https://maplebacon.org/2022/06/actf-ffsk/](https://maplebacon.org/2022/06/actf-ffsk/)**

FSK = Frequency-shift keying.

FFSK = Double FSK or Fast FSK, whatever.

This problem is designed to invite participants to have a look at *the principle of communication*. 

### 1. Description

A journey to solve a misc problem always begins from a problem description. Here’s it:

 

> I’ve bought the **second commercial modem** for computers in a big city of the UK.
> 
> 
> > 激情澎湃的球迷迷恋这个地方。遇上球赛季，酒吧里的热情、呐喊、啤酒、摇滚，足球让这个城市充满活力和希望。
> 从三万英尺的云端望去，往日的生活成了一个遥远微小的地图。
> 阳光明媚的日子，开始出发，北京时间00:50 开始起飞，一个梦的距离，就可以到达荷兰阿姆斯特丹，短暂停留之后，然后转机飞往英国
> 南航的飞机配置完备，全程可以充电，还有wifi，影视屏有面前最新的电影。睡睡醒醒，在飞机上觅到一部《北京爱情故事》，让我在三万英尺的空中哭的稀里哗啦。
> > 

Just Google it, and you’ll realize what it means:

- **second commercial modem→Bell 103, corresponds with the file name “modem.wav”**
- a big city in the UK: **Manchester**, which refers to the famous coding method.
- The source of the long Chinese paragraph: [https://kknews.cc/zh-hk/travel/e6yjp34.html](https://kknews.cc/zh-hk/travel/e6yjp34.html)
    
    It describes a trip to Manchester, which is indeed a big city in the UK.
    

### 2. Bell 103

Here’s an article that shows how the Bell 103 protocol works: [https://vigrey.com/blog/emulating-bell-103-modem](https://vigrey.com/blog/emulating-bell-103-modem)

So two key points need your attention. First, characters are stored in ASCII code and are **little-endian;** second, it has 2 channels for communication: one for the server-side(2025/2225 Hz), and another for the client-side(1070/1270 Hz).

You can also find it from the spectrogram of the .wav file.

### 2. Server channel

Using the `minimodem`tool (See MapleBacon’s write-up) is functional.

Also, you can find some useful tools in GitHub: [https://github.com/laurenschneider/audiodecoder](https://github.com/laurenschneider/audiodecoder)

It may be a faster way. In fact, the `solve.py` is based on its code.

After all, you’ll see this on the server channel:

```
HINT_Hamming@ddddPdddddddPdddPdPP(20).ECCode; Content: Why do you use such
a slow method with a high Bit Error Ratio for communication? It took me a lot of
effort to correct bit-flips, making sure the communication was less
error-prone...that is 2 say, THE ORIGINAL PROTOCOL IS WRAPPED BY SOME OTHER
TRANSFORMATIONS! Fortunately, we can now communicate properly on another channel
while enjoying a vacation in this BIG CITY--I mean, IEEE 802.3.....Wait, what is
the new protocol? Guess by yourself!
```

### 3. Client channel

We can extract the bit string on this channel using the same method but just make some tweaks of frequency. You’ll get a bit string of 53640 bits.

Notice that the bit string contains only “01” “10”, that is what **Manchester is** all about. The `IEEE 802.3`mentioned in the server channel message is actually to make sure you decode in the right way: there are 2 opposite ways to map 01/10 to 1/0, but what is widely used is defined in IEEE 802.3, which says “01”→1 and “10”→0

Then the key problem is to solve Hamming code. From the given information, you’ll realize the block size is 20bits. Implement it by yourself or just Google/GitHub/StackOverflow it.

Find every “1” bit in a block, XOR their **positions**, you got the error bit(0 if no error), then flip it.

Actually, every block has, and only has an error bit: that’s an intended design to notify you that you’re on the right way :)

### 4. Final Step

Now you’ve got the cipher bit string: just applied Bell 103 decoder to it once again. 

It’ll yield a string that starts with `data:image/png;base64,`

An experienced CTFer will immediately put it into the browser (like what MapleBacon did). Or you can find a random online converter to recover this Base64-encoded image. It’s a QR Code. Scan it, and got the flag.

### 5. Hints Explanation

1. `所有人都认为，吃鸡蛋前，原始的方法是打破鸡蛋较大的一端。可是当今皇帝的祖父 时候吃鸡蛋，一次按古法打鸡蛋时碰巧将一个手指弄破了，因此他的父亲，当时的皇帝， 就下了一道敕令，命令全体臣民吃鸡蛋时打破鸡蛋较小的一端，违令者重罚。 老百姓们 对这项命令极为反感。历史告诉我们，由此曾发生过六次叛乱，其中一个皇帝送了命，另 一个丢了王位…关于这一争端，曾出版过几百本大部著作，不过大端派的书一直是受禁的 ，法律也规定该派的任何人不得做官。 ——乔纳森·斯威夫特，《格列佛游记》`
    
    It is a quote from Gulliver's Travels.  Fun fact: this paragraph is **exactly the original source of the 2 words used in modern computer science: “big-endian” & “little-endian”.**
    
    This hint is intended to guide those who are stuck because of their ignorance of the contents(especially the coding method) of the Bell 103 protocol.
    
2. `Hamming code block size: 20bits`
    
    Noticed that minimodem may yield partly corrupt text and mislead participants. This hint is to make sure they see the hint hidden at the beginning of the service-side channel message.
    
3. `Bell 103`
    
    To help those who ignored the problem description.
    

### 6. Script to Solve

```python
# goertzel.py
"""
Module to create a Goertzel filter
Original source: https://github.com/laurenschneider/audiodecoder
"""

import numpy as np

class Goertzel():

    def __init__(self, rate, freq):
        self.normalize = 0
        self.coeffs = 0
        self.sample_rate = rate
        self.target_freq = freq

    def calculate_coeff(self):
        """
        Precompute coefficients needed for filter equation.
        Coeff formulas courtesy of Prof. Massey
        """
        n = 160

        w0 = (2 * np.pi * self.target_freq) / self.sample_rate
        self.normalize = np.exp(1j * w0 * n)
        self.coeffs = np.array([np.exp((-1j) * w0 * k) for k in range(n)])

    def filter(self, samples):
        """
        Goertzel filter equation
        :param samples: array of samples
        :returns: amplitude
        """
        y = self.normalize * 160 * np.dot(self.coeffs, samples)
        ampl = np.abs(y)

        return ampl
```

```python
# decode.py
"""
Decode a wav file using a Goertzel filter.
Modified from https://github.com/laurenschneider/audiodecoder
"""

from goertzel import Goertzel
import numpy as np
import os
from scipy.io import wavfile

DATA = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(DATA, "filename.wav")

# Read sample rate and data from audio file
rate, data = wavfile.read(filepath)

message = ''
bit_string = ''
mark_freq = 2225
space_freq = 2025
mark_filter = Goertzel(rate, mark_freq)
space_filter = Goertzel(rate, space_freq)

# calculate coefficients for each filter
mark_filter.calculate_coeff()
space_filter.calculate_coeff()

for i in range(data.size + 1):

    # for each chunk of 160 samples
    if i%160 == 0 and i != 0:
        start = i - 160
        end = i
        samples = data[start:end]

        # get amplitutes of sample set
        mark_amp = mark_filter.filter(samples)
        space_amp = space_filter.filter(samples)

        if mark_amp > space_amp:
            # bit is 1
            to_add = '1'
        else:
            # bit is zero
            to_add = '0'
        bit_string = to_add + bit_string

for x in range(10, len(bit_string)+10):
    if x%10 == 0:
        start = x - 9
        end = x - 1
        message = chr(int(bit_string[start:end],2)) + message
print(message)

# set target frequencies
mark_freq = 1270
space_freq = 1070

# create two filters
mark_filter = Goertzel(rate, mark_freq)
space_filter = Goertzel(rate, space_freq)

# calculate coefficients for each filter
mark_filter.calculate_coeff()
space_filter.calculate_coeff()

bit_string = ''
message = ''
for i in range(data.size + 1):

    # for each chunk of 160 samples
    if i%160 == 0 and i != 0:
        start = i - 160
        end = i
        samples = data[start:end]

        # get amplitutes of sample set
        mark_amp = mark_filter.filter(samples)
        space_amp = space_filter.filter(samples)

        if mark_amp > space_amp:
            to_add = '1'
        else:
            to_add = '0'
        bit_string = to_add + bit_string

def HammingBolck(message):
    assert(len(message)==15)
    message=message[::-1]
    code = 0
    m_pos = 0
    for ind in range(20):
        if (ind+1)&(ind): # Not parity check bit
            if int(message[m_pos],2):
                code = code ^ (1<<ind)
                code = code ^ (((ind+1)&0b1)<<0)
                code = code ^ (((ind+1)&0b10)<<0)
                code = code ^ (((ind+1)&0b100)<<1)
                code = code ^ (((ind+1)&0b1000)<<4)
            m_pos = m_pos + 1

    code = code ^ (1<<random.randint(0,19))

    retStr = "{0:020b}".format(code)
    print("from "+message+" to "+retStr)
    
    return retStr

def HammingBolckInv(message):
    assert(len(message)==20)
    code = int(message,2)
    wrong = 0
    for ind in range(20):
        if (1<<ind)&code:
            wrong = wrong ^ (ind+1)
    if wrong:
        code = code ^ (1<<(wrong-1))
    retStr = ""
    for ind in range(20):
        if (ind+1)&(ind): # Not parity check bit
            retStr = retStr + ("1" if code&(1<<ind) else "0")
    return retStr[::-1]

def Hamming(message):
    retStr = ""
    for ind in range(0,len(message),15):
        retStr = retStr + HammingBolck(message[ind:ind+15])
    return retStr

def HammingInv(message):
    retStr = ""
    for ind in range(0,len(message),20):
        retStr = retStr + HammingBolckInv(message[ind:ind+20])
    return retStr

def manchester(message):
    retStr = ""
    for char in message:
        retStr += "01" if char=='1' else "10"
    return retStr
def manchesterInv(message):
    retStr = ""
    for ind in range(0, len(message), 2):
        char = message[ind]
        retStr += "0" if char=='1' else "1"
    return retStr

bit_string=bit_string[::-1]
bit_string = HammingInv(manchesterInv(bit_string))[::-1]

for x in range(10, len(bit_string)+10):
    if x%10 == 0:
        start = x - 9
        end = x - 1
        message = chr(int(bit_string[start:end],2)) + message
print(message)
```

### 7. Final Word

In my eyes, a good misc problem should not be an annoying puzzle. Steps to solve a misc problem have to be reasonable. For example, in this problem, Manchester coding is applied after the application of Hamming coding: that’s because the former is channel coding, and the latter is source coding. We shouldn’t just pick some random encryption and apply it to plaintext.

 I have made my best to make the solving process more natural. I hope you enjoy digging deep into the problem. You’ll earn much more fun than those who can just use tools written by others without understanding fundamental principles (like me).

ご武運を！