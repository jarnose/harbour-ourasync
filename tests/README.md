# Tests

Everything under `src/ring/` and `src/analysis/` is Qt-free on purpose, so it
compiles and runs with plain `g++` on a development machine. A protocol decoder
is exactly the kind of code where "it compiles" proves nothing, and the phone is
a slow place to find out.

```
tests/run.sh
```

builds and runs each suite twice, in strict `c++17` and in `gnu++17` (the mode
the Sailfish build uses, where `unix` is a macro).

| suite | what it checks |
|---|---|
| `aes128` | FIPS-197 vectors, and open_oura's real ring nonce → ciphertext pair |
| `protocol` | request byte layouts, framing, history batches, cursor validation, every event decoder |
| `session` | authentication, key install, reads and the history drain against a scripted ring |
| `daysummary` | clock epochs and anchors, a synthetic night end to end, scoring, calendar maths |

## The vectors

Nothing here is a capture from the author's ring. The vectors are the ones
[open_oura](https://github.com/Th0rgal/open_oura) published from a Ring 3
Horizon and a Ring 5, plus request shapes re-read in the Oura Android app
7.27.0 (`docs/apk-findings.md`). The synthetic night in `test_daysummary.cpp`
is built byte by byte in the documented layouts.

Real captures are personal data (sleep, heart rate, the ring's serial) and stay
out of the repository: `tests/fixtures/` and `captures/` are git-ignored.

## What these tests cannot tell you

Whether a real ring behaves as the vectors say. The session tests prove the
conversation logic against a scripted ring; the first sync on hardware is the
test of the script. `docs/design.md` lists the assumptions that only a ring can
confirm.
