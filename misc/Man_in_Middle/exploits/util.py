import json
import io
import selectors
import threading
'''
Message Format:
+++++++++++++++++++++++++++++++++++++++++++++++++++
++   Fixed-length Header                         ++
++   Type: 4 byte interger                       ++
+++++++++++++++++++++++++++++++++++++++++++++++++++
++   Variable-length JSON Header                 ++
++   Length: specified by fixed-length Header    ++
+++++++++++++++++++++++++++++++++++++++++++++++++++
JSON Header Format:
{
    "type"  : string,
    "Message": string or integer,
    "r": integer ,
}
'''



class Message:
    def __init__(self, selector, sock, ipaddr, logger):
        self.selector = selector
        self.logger = logger
        self.sock = sock
        self.addr = ipaddr
        self._recv_buffer = b""
        self._send_buffer = b""
        self._send_buffer_lock = threading.Lock()
        self._recv_done = False
        self._jsonheader_len = None
        self.jsonheader = None

        self.handshake = 0
        self.data = False
        self.CNonce = None
        self.ANonce = None
        self.Nonce = None
        self.Nonce_lock = threading.Lock()
        self.last_handshake_send = None
        self.last_handshake_time = None
        self.handshake_retry = 0
        self.TK = None
        self.thred = None
        self.close_thred = False
    def _read(self):
        try:
            # Should be ready to read
            data = self.sock.recv(0xffff)
        except BlockingIOError:
            # Resource temporarily unavailable (errno EWOULDBLOCK)
            pass
        else:
            if data:
                self._recv_buffer += data
            else:
                self.logger.info("Peer closed.")
                self.close()

    def _json_encode(self, obj, encoding):
        payload = json.dumps(obj, ensure_ascii=False).encode(encoding)
        return len(payload).to_bytes(4,'little') + payload

    def _json_decode(self, json_bytes, encoding):
        tiow = io.TextIOWrapper(
            io.BytesIO(json_bytes), encoding=encoding, newline=""
        )
        obj = json.load(tiow, strict=False)
        tiow.close()
        return obj
    
    def write(self,packet:str):
        with self._send_buffer_lock:
            self._send_buffer += self._json_encode(packet,"utf-8")

    # def process_events(self, mask):
    #     if mask & selectors.EVENT_READ:
    #         return self.read()
    #     if mask & selectors.EVENT_WRITE:


    def read(self):
        self._read()
        ans = None
        while not self._recv_done:
            if self._jsonheader_len is None:
                self.process_protoheader()

            if self._jsonheader_len is not None:
                if self.jsonheader is None:
                    ans = self.process_jsonheader()
        self._recv_done = False
        return ans

    def close(self):
        self.logger.info("closing connection to %s", self.addr)
        try:
            self.selector.unregister(self.sock)
        except Exception as e:
            self.logger.error(f"{self.addr}: {repr(e)}")
        try:
            if self.thred  != None:
                self.close_thred = True
                self.thred  = None
        except Exception as e:
            self.logger.error(f"{self.addr}: {repr(e)}")
        try:
            self.sock.close()
        except OSError as e:
            self.logger.error(f"{self.addr}: {repr(e)}")
        finally:
            # Delete reference to socket object for garbage collection
            self.sock = None

    def process_protoheader(self):
        hdrlen = 4
        if len(self._recv_buffer) >= hdrlen:
            self._jsonheader_len = int.from_bytes(self._recv_buffer[:hdrlen],'little')
            self._recv_buffer = self._recv_buffer[hdrlen:]
        else:
            self._recv_done = True

    def process_jsonheader(self):
        hdrlen = self._jsonheader_len
        if len(self._recv_buffer) >= hdrlen:
            try:
                self.jsonheader = self._json_decode(
                    self._recv_buffer[:hdrlen], "utf-8"
                )
            except Exception:
                self.logger.error("Invalid json header {}".format(self._recv_buffer[:hdrlen]))
                self._recv_done = True
            else:
                try:
                    for reqhdr in ("type", "Message", "r"):
                        if reqhdr not in self.jsonheader:
                            raise ValueError(f'Missing required header "{reqhdr}".')
                except Exception:
                    self.logger.error(f'Missing required header "{reqhdr}".')
                else:
                    return self.jsonheader
                finally: # always execute
                    self._jsonheader_len = None
                    self.jsonheader = None
            finally:
                self._recv_buffer = self._recv_buffer[hdrlen:]
        else:
            self._recv_done = True
