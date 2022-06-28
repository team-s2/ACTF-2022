from asyncio.log import logger
import socket
import random
import selectors
import traceback
import logging
import json
import logging.handlers as handlers
from time import sleep, time
import threading
from key import GenerateTK,GenerateKey
from util import Message

HOST = '127.0.0.1'
PORT = 1018
LOG_FILE = 'AP.log'
MAX_RAMDOM_NUM = 0x0fffffff
MAX_RETRIES = 233
TIME_OUT = 1


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


def resend():
    while True:
        sleep(2)
        t = sel._fd_to_key.copy()
        for key in t:
            data = sel._fd_to_key[key].data
            if data is not None and data.last_handshake_send is not None and data.last_handshake_time is not None:
                if data.handshake_retry > MAX_RETRIES:
                    logger.info("Max Retries to %s",data.addr)
                    data.close()
                elif(data.handshake != 4 and time()-data.last_handshake_time > TIME_OUT):
                    data.handshake_retry += 1
                    data.last_handshake_time = time()
                    data.last_handshake_send["r"] += 1
                    data.write(data.last_handshake_send)
            


def main():
    global logger, sel
    logger = logger_init(LOG_FILE)
    # open listening socket
    sel = server_init()
    thred = threading.Thread(target=resend)
    thred.start()
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
                else:
                    # need to receive
                    msg = key.data
                    try:
                        if mask & selectors.EVENT_READ:
                            packet = msg.read()
                            if packet is None : continue
                            if msg.handshake==0 and packet["Message"]=="Request":
                                msg.ANonce = random.randint(100, MAX_RAMDOM_NUM)
                                packet = {"type":"step1","Message": msg.ANonce, "r": random.randint(100, MAX_RAMDOM_NUM)}
                                msg.write(packet)
                                msg.last_handshake_send = packet
                                msg.last_handshake_time = time()
                                msg.handshake += 1
                            elif msg.handshake==1 and packet["r"]==msg.last_handshake_send["r"]:
                                if(type(packet["Message"])!=int and packet["type"]!="step2"): continue
                                msg.CNonce = packet["Message"] 
                                packet = {"type":"step3", "Message": "CNonce Received", "r": msg.last_handshake_send["r"]+1}
                                msg.write(packet)
                                msg.last_handshake_send = packet
                                msg.last_handshake_time = time()
                                msg.handshake += 2
                            elif msg.handshake==3 and packet["r"]==msg.last_handshake_send["r"]:
                                if(packet["Message"]=="Complete"and packet["type"]=="step4"):
                                    msg.handshake += 1
                                    msg.TK = GenerateTK(msg.ANonce,msg.CNonce)
                                    msg.Nonce = 0
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
