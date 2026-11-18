#!/usr/bin/env python3
"""
Writes sjson_vectors.json: documents and the packets the reference Python
SJSON library (https://github.com/jnaujok/sjson) produces for them, so the
C++ port can check that it writes identical bytes and reads Python packets.

    pip install sjson   # or: PYTHONPATH=/path/to/sjson/src
    python3 make_vectors.py > sjson_vectors.json

"exact" sequences are encoded with the LZ4 string form switched off, since
LZ4 output may differ between library versions; "lz4" sequences are only
checked by decoding.
"""

import json
import random
import struct
import sys

from sjson.sjson import SJSON
from sjson.string_node import StringNode

SENDER = 1


def sequences():
    unicode_text = ["café", "日本語のテキスト", "emoji 🎉 party", "a日", "ÿ" * 40, "Ωmega"]
    strings = [
        "", "x", "xyz", "abc", "hello there", "deadbeef00112233", "DEADBEEF00112233",
        "0123456789", "2026-11-18T10:20:30Z", "2026-11-18T10:20:30.123456+05:30",
        "2026-11-18", "2026-11-18T23:59:59.5-08:00", "aGVsbG8gd29ybGQgdGhpcyBpcyBiYXNlNjQh",
        "550e8400-e29b-41d4-a716-446655440000", "550E8400E29B41D4A716446655440000",
        "550e8400-E29B-41d4-a716-446655440000", "2026-13-45", "QUJD", "x" * 300,
        "The quick brown fox jumps over the lazy dog", "line\nbreak\ttab \"quoted\" \\",
    ]
    numbers = [0, 1, -1, 7, 15, 16, 255, -256, 65535, 2**31, -(2**31), 2**32 - 1, 2**32,
               -(2**40), 9007199254740993, 12345678901234567890123, -98765432109876543210,
               0.0, -0.0, 1.0, -2.5, 0.15625, 0.1, 123.45, 1e-7, 1.23e17, 1e16, 1e15,
               0.0001, 3.4028234663852886e38, 1e300, 5e-324, 2.0**60, 123456789.123456789]
    yield "scalars", [None, True, False] + numbers + strings + unicode_text
    yield "strings in objects", [{"s": s} for s in strings + unicode_text]
    yield "numbers in arrays", [numbers, [[1, [2, [3, []]]], {}], []]
    yield "nesting", [
        {"name": "Standard_test", "nested": {"deeper": {"deepest": [1, {"x": None}]}}},
        {"a": [], "b": {}, "c": [{}], "d": [[], [[]]], "": "empty key"},
        {"ünïcödé kéy": 1, "日本": [True, False, None]},
    ]
    yield "dictionary growth", [{f"k{i}": i for i in range(300)}, {f"k{i}": -i for i in range(0, 300, 7)}]
    yield "urls", [
        {"link": "https://example.com/a", "home": "http://example.org/"},
        {"link": "https://example.com/a", "home": "http://example.org/", "new": "https://x.test/1"},
        {"again": "https://example.com/a"},
    ]
    yield "order", [{
        "orderId": "A1B2C3D4", "created": "2026-11-18T10:20:30Z", "status": "SHIPPED",
        "total": 129.99, "qty": 3,
        "items": [{"sku": "X-100", "price": 19.99, "count": 2}, {"sku": "Y-200", "price": 90.01, "count": 1}],
        "customer": {"id": 48213, "email": "pat@example.com", "vip": False},
    }] * 3


def encode(documents):
    sender = SJSON(sender_id=SENDER)
    packets = [sender.to_binary(d).tobytes().hex() for d in documents]
    nak = sender.nak_dictionary(SENDER, 0, 0).tobytes().hex()
    return packets, nak


def stream_sequences():
    tracks = []
    for step in range(30):
        for track in range(4):
            tracks.append({
                "id": f"6f9619ff-8b86-4011-b42d-00c04fc96{track:03d}",
                "seq": step,
                "time": f"2026-11-18T16:00:{step:02d}.{(step * 37 + track) % 1000:03d}Z",
                "lat": 0.6 + step * 1e-5 + track,
                "lon": -1.2 - step * 2e-5,
                "label": ["ALPHA ONE", "BRAVO TWO", "CHARLIE 3", "DELTA FOUR"][track],
                "note": "a fairly long free-text note about track %d" % track,
                "url": "https://example.com/tracks/%d/%d" % (track, step % 3),
                "flags": [True, step % 2 == 0, None],
                "nested": {"same": {"x": 1, "y": [1, 2]}, "n": step * 2, "big": 2**70 + step},
            })
    yield "tracks", tracks
    for name, docs in sequences():
        yield name, docs


def encode_stream(documents):
    sender = SJSON(sender_id=SENDER, stateful=True)
    return [sender.to_binary(d).tobytes().hex() for d in documents]


def main():
    out = {"sender_id": SENDER, "version": SJSON.VERSION, "exact": [], "lz4": [], "stream": []}
    for name, docs in stream_sequences():
        out["stream"].append({"name": name, "documents": docs, "packets": encode_stream(docs)})
    # repr() of doubles (as IEEE-754 bit patterns), for the C++ float formatter.
    rng = random.Random(42)
    floats = [0.1, 1e16, 1e15, 1e-4, 1e-5, 123.456, 1.5e300, 5e-324, 2.2250738585072014e-308,
              1.7976931348623157e308, 0.3, 2.0 / 3, 100.0, 12345678901234567.0]
    floats += [struct.unpack(">d", struct.pack(">Q", rng.getrandbits(64)))[0] for _ in range(3000)]
    floats += [rng.uniform(-1e6, 1e6) for _ in range(1000)] + [round(rng.uniform(-100, 100), rng.randint(0, 6)) for _ in range(1000)]
    out["float_reprs"] = [[struct.pack(">d", f).hex(), repr(f)] for f in floats if f == f and f not in (float("inf"), float("-inf"))]
    lz4_min = StringNode.LZ4_MIN_BYTES
    for name, docs in sequences():
        StringNode.LZ4_MIN_BYTES = 10**9
        packets, nak = encode(docs)
        out["exact"].append({"name": name, "documents": docs, "packets": packets, "nak": nak})
        StringNode.LZ4_MIN_BYTES = lz4_min
        packets, nak = encode(docs)
        out["lz4"].append({"name": name, "documents": docs, "packets": packets, "nak": nak})
    json.dump(out, sys.stdout, ensure_ascii=False, indent=0)


if __name__ == "__main__":
    main()
