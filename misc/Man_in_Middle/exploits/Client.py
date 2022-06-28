from asyncio.log import logger
import base64
import socket
import random
import selectors
from struct import pack
import traceback
import logging
import json
import logging.handlers as handlers
from time import sleep, time
import threading
from key import GenerateTK,GenerateKey
from util import Message

HOST = '127.0.0.1'
PORT = 1019
LOG_FILE = 'Client.log'
MAX_RAMDOM_NUM = 0x0fffffff


def server_init():
    sel = selectors.DefaultSelector()
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Avoid bind() exception: Address already in use
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((HOST, PORT))
    server.listen(5)
    logger.info("listening on (%s, %d)", HOST, PORT)
    server.setblocking(False)
    sel.register(server, selectors.EVENT_READ |
                 selectors.EVENT_WRITE, data=None)
    return sel


def logger_init(log_file):
    logger = logging.getLogger(__name__)
    # 1 day per log
    fhandler = handlers.TimedRotatingFileHandler(
        log_file, when='D', interval=1)
    fmt = "[%(process)d] %(asctime)s %(levelname)s (%(funcName)s) %(message)s"
    formatter = logging.Formatter(fmt=fmt)
    fhandler.setFormatter(formatter)
    fhandler.setLevel(logging.INFO)
    logger.addHandler(fhandler)
    logger.setLevel(logging.INFO)
    return logger

def SendData(msg:Message):
    while True:
        try:
            with open("secret.txt","rb") as f:
                con = f.read()
        except Exception:
            msg.logger.warning("Read Secret.txt failed. Retry...")
        else:
            break
    msg.logger.info("Start sending data to (%s)",msg.addr)
    while True:
        for i in range(0,len(con),16):
            plain = con[i:i+16]+b'\n'*(16-len(con[i:i+16])) if len(con[i:i+16])<16  else con[i:i+16]
            key = GenerateKey(msg.Nonce,msg.TK)
            # msg.logger.info("Nonce:{},key:{},plain:{}".format(msg.Nonce,key,plain))
            cipher = bytes(plain[i]^key[i] for i in range(16))
            payload = base64.b64encode(cipher).decode()
            packet = {"type":"data", "Message": payload, "r": None}
            msg.write(packet)
            with msg.Nonce_lock:
                msg.Nonce += 1
            sleep(0.1)
            if(msg.close_thred):
                msg.logger.info("Closing Send Data Thread. (%s)",msg.addr)
                exit(0)

def main():
    global logger, sel
    logger = logger_init(LOG_FILE)
    # open listening socket
    sel = server_init()
    try:
        while True:
            # blocks here until there are sockets ready for I/O
            events = sel.select(timeout=None)
            for key, mask in events:
                if key.data is None:
                    # need to accept
                    sock = key.fileobj
                    conn, ipaddr = sock.accept()
                    logger.info("accept connection from %s", ipaddr)
                    conn.setblocking(False)  # prevent dead connection
                    msg = Message(sel, conn, ipaddr, logger)
                    sel.register(conn, selectors.EVENT_READ |
                                 selectors.EVENT_WRITE, data=msg)
                    msg.thred = threading.Thread(target=SendData,args=[msg])
                    packet = {"type":"step0", "Message": "Request", "r": None}            
                    msg.write(packet)
                else:
                    # need to receive
                    msg = key.data
                    try:
                        if mask & selectors.EVENT_READ:
                            packet = msg.read()
                            if packet is None : continue
                            if packet["type"] == "step1" :
                                if type(packet["Message"])!=int and type(packet["r"])!=int: continue
                                msg.ANonce = packet["Message"]
                                msg.CNonce = random.randint(100, MAX_RAMDOM_NUM)
                                send_packet = {"type":"step2", "Message": msg.CNonce, "r": packet["r"]}
                                msg.write(send_packet)
                                msg.handshake += 2
                            elif packet["type"] == "step3":
                                if (packet["Message"])!="CNonce Received" and type(packet["r"])!=int: continue
                                msg.TK = GenerateTK(msg.ANonce,msg.CNonce)
                                with msg.Nonce_lock:
                                    msg.Nonce = 0
                                msg.handshake+=1
                                if msg.thred.is_alive()==False:
                                    msg.thred.start()
                                send_packet = {"type":"step4", "Message": "Complete", "r": packet["r"]}
                                msg.write(send_packet)
                                msg.handshake += 1
                            else:
                                pass
                        if mask & selectors.EVENT_WRITE and msg._send_buffer != b'':
                            l = msg.sock.send(msg._send_buffer)
                            with msg._send_buffer_lock:
                                msg._send_buffer = msg._send_buffer[l:]

                    except Exception:
                        logger.error(traceback.format_exc())
                        msg.close()
    except Exception:
        logger.error(traceback.format_exc())
    finally:
        logger.info("Bye.")
        sel.close()


if __name__ == '__main__':
    main()
