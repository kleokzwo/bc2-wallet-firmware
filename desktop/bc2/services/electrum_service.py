from __future__ import annotations
import hashlib, json, socket, ssl
from dataclasses import dataclass
from PySide6.QtCore import QObject,QThread,Signal,Slot

@dataclass(frozen=True)
class BalanceResult:
    confirmed:int
    unconfirmed:int
    addresses:int
    server:str

@dataclass(frozen=True)
class Utxo:
    tx_hash:str
    tx_pos:int
    value:int
    height:int
    address:str
    derivation_index:int

CHARSET="qpzry9x8gf2tvdw0s3jn54khce6mua7l"

def _polymod(values):
    chk=1; gens=(0x3B6A57B2,0x26508E6D,0x1EA119FA,0x3D4233DD,0x2A1462B3)
    for v in values:
        top=chk>>25; chk=((chk&0x1FFFFFF)<<5)^v
        for i,g in enumerate(gens):
            if (top>>i)&1: chk^=g
    return chk

def _expand(hrp): return [ord(x)>>5 for x in hrp]+[0]+[ord(x)&31 for x in hrp]

def _convert(data,fb=5,tb=8):
    acc=bits=0; out=bytearray(); maxv=(1<<tb)-1; maxacc=(1<<(fb+tb-1))-1
    for v in data:
        if v<0 or v>>fb: raise ValueError("invalid bech32 value")
        acc=((acc<<fb)|v)&maxacc; bits+=fb
        while bits>=tb:
            bits-=tb; out.append((acc>>bits)&maxv)
    if bits>=fb or ((acc<<(tb-bits))&maxv): raise ValueError("invalid bech32 padding")
    return bytes(out)

def address_to_scriptpubkey(address:str)->bytes:
    a=address.strip()
    if not a or (a.lower()!=a and a.upper()!=a): raise ValueError("mixed-case address")
    a=a.lower(); p=a.rfind("1")
    if p<1 or p+7>len(a): raise ValueError("invalid bech32 address")
    hrp=a[:p]
    try: vals=[CHARSET.index(c) for c in a[p+1:]]
    except ValueError as e: raise ValueError("invalid bech32 character") from e
    data=vals[:-6]
    if not data: raise ValueError("missing witness data")
    ver=data[0]
    if ver>16: raise ValueError("unsupported witness version")
    expected=1 if ver==0 else 0x2BC830A3
    if _polymod(_expand(hrp)+vals)!=expected: raise ValueError("invalid bech32 checksum")
    prog=_convert(data[1:])
    if not 2<=len(prog)<=40: raise ValueError("invalid witness program")
    if ver==0 and len(prog) not in (20,32): raise ValueError("invalid v0 witness program")
    op=0 if ver==0 else 0x50+ver
    return bytes([op,len(prog)])+prog

def electrum_scripthash(address:str)->str:
    return hashlib.sha256(address_to_scriptpubkey(address)).digest()[::-1].hex()

def _rpc(f,rid,method,params):
    f.write((json.dumps({"jsonrpc":"2.0","id":rid,"method":method,"params":params},
                        separators=(",",":"))+"\n").encode()); f.flush()
    while True:
        line=f.readline()
        if not line: raise ConnectionError("Electrum Server hat die Verbindung beendet.")
        msg=json.loads(line.decode())
        if msg.get("id")!=rid: continue
        if msg.get("error"): raise RuntimeError(str(msg["error"]))
        return msg.get("result")

def fetch_balances(server,addresses,timeout=8.0):
    if ":" not in server: raise ValueError("Electrum Server muss host:port sein.")
    host,port=server.rsplit(":",1); port=int(port)
    addrs=list(dict.fromkeys(a.strip() for a in addresses if a.strip()))
    if not addrs: return BalanceResult(0,0,0,server)
    ctx=ssl.create_default_context()
    with socket.create_connection((host,port),timeout=timeout) as raw:
        with ctx.wrap_socket(raw,server_hostname=host) as sock:
            sock.settimeout(timeout)
            with sock.makefile("rwb") as f:
                _rpc(f,1,"server.version",["BC2 Cold Wallet","1.4"])
                c=u=0
                for rid,address in enumerate(addrs,10):
                    r=_rpc(f,rid,"blockchain.scripthash.get_balance",[electrum_scripthash(address)]) or {}
                    c+=int(r.get("confirmed",0)); u+=int(r.get("unconfirmed",0))
    return BalanceResult(c,u,len(addrs),server)


def fetch_utxos(server, addresses, timeout=8.0):
    if ":" not in server:
        raise ValueError("Electrum Server muss host:port sein.")
    host, port = server.rsplit(":", 1)
    port = int(port)
    addrs = list(dict.fromkeys(a.strip() for a in addresses if a.strip()))
    if not addrs:
        return []

    ctx = ssl.create_default_context()
    utxos = []
    with socket.create_connection((host, port), timeout=timeout) as raw:
        with ctx.wrap_socket(raw, server_hostname=host) as sock:
            sock.settimeout(timeout)
            with sock.makefile("rwb") as f:
                _rpc(f, 1, "server.version", ["BC2 Cold Wallet", "1.4"])
                for derivation_index, address in enumerate(addrs):
                    rid = 100 + derivation_index
                    rows = _rpc(
                        f, rid, "blockchain.scripthash.listunspent",
                        [electrum_scripthash(address)]
                    ) or []
                    for row in rows:
                        value = int(row.get("value", 0))
                        if value <= 0:
                            continue
                        utxos.append(Utxo(
                            tx_hash=str(row.get("tx_hash", "")),
                            tx_pos=int(row.get("tx_pos", 0)),
                            value=value,
                            height=int(row.get("height", 0)),
                            address=address,
                            derivation_index=derivation_index,
                        ))
    return utxos


def broadcast_transaction(server: str, raw_transaction_hex: str, timeout=15.0) -> str:
    if ":" not in server:
        raise ValueError("Electrum Server muss host:port sein.")

    raw_transaction_hex = raw_transaction_hex.strip().lower()
    if not raw_transaction_hex:
        raise ValueError("Keine signierte Transaktion vorhanden.")

    try:
        bytes.fromhex(raw_transaction_hex)
    except ValueError as exc:
        raise ValueError("Die signierte Transaktion ist kein gültiger Hex-String.") from exc

    host, port_text = server.rsplit(":", 1)
    port = int(port_text)

    ctx = ssl.create_default_context()

    with socket.create_connection((host, port), timeout=timeout) as raw:
        with ctx.wrap_socket(raw, server_hostname=host) as sock:
            sock.settimeout(timeout)

            with sock.makefile("rwb") as file:
                _rpc(
                    file,
                    1,
                    "server.version",
                    ["BC2 Cold Wallet", "1.4"],
                )
                txid = _rpc(
                    file,
                    2,
                    "blockchain.transaction.broadcast",
                    [raw_transaction_hex],
                )

    if not isinstance(txid, str) or len(txid) != 64:
        raise RuntimeError(
            f"Electrum hat eine unerwartete Broadcast-Antwort geliefert: {txid!r}"
        )

    return txid.lower()

class _Worker(QObject):
    finished=Signal(object); failed=Signal(str)
    def __init__(self,server,addresses): super().__init__(); self.s=server; self.a=addresses
    @Slot()
    def run(self):
        try: self.finished.emit(fetch_balances(self.s,self.a))
        except Exception as e: self.failed.emit(str(e))

class ElectrumService(QObject):
    started=Signal(); finished=Signal(object); failed=Signal(str)
    def __init__(self,parent=None): super().__init__(parent); self._thread=None; self._worker=None
    def sync(self,server,addresses):
        if self._thread is not None: return
        self.started.emit(); t=QThread(self); w=_Worker(server,addresses); w.moveToThread(t)
        t.started.connect(w.run); w.finished.connect(self._done); w.failed.connect(self._fail)
        w.finished.connect(t.quit); w.failed.connect(t.quit)
        t.finished.connect(w.deleteLater); t.finished.connect(t.deleteLater)
        self._thread=t; self._worker=w; t.start()
    @Slot(object)
    def _done(self,r): self._thread=None; self._worker=None; self.finished.emit(r)
    @Slot(str)
    def _fail(self,m): self._thread=None; self._worker=None; self.failed.emit(m)
