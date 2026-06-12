# EmbedKit_Apoorva

**Author:** Apoorva  
**Assignment:** Embedded Developer Fresher | Embed Square Solutions Pvt. Ltd.

---

## Modules

| File | Description |
|------|-------------|
| `ringbuf.c` | Circular (ring) buffer for `uint8_t` data with fixed capacity of 8 bytes |

---

## Build Instructions

You need `gcc` installed (Linux/macOS) or MinGW/WSL on Windows.

```bash
gcc -Wall -std=c99 Ringbuf.c -o ringbuf
./ringbuf
```

Zero warnings, zero errors -- verified on gcc 11+.

---

## Expected Output

```
[WRITE] 0x41 -> OK (count=1)
[WRITE] 0x42 -> OK (count=2)
[WRITE] 0x43 -> OK (count=3)
[WRITE] 0x44 -> OK (count=4)
[WRITE] 0x45 -> OK (count=5)
[WRITE] 0x46 -> OK (count=6)
[WRITE] 0x47 -> OK (count=7)
[WRITE] 0x48 -> OK (count=8) FULL
[WRITE] 0x99 -> FAIL (buffer full)
[READ]  -> 0x41 (count=7)
[READ]  -> 0x42 (count=6)
[READ]  -> 0x43 (count=5)
[WRITE] 0x49 -> OK (count=6)
[WRITE] 0x4A -> OK (count=7)
[WRITE] 0x4B -> OK (count=8) FULL
[READ]  -> 0x44 (count=7)
[READ]  -> 0x45 (count=6)
[READ]  -> 0x46 (count=5)
[READ]  -> 0x47 (count=4)
[READ]  -> 0x48 (count=3)
[READ]  -> 0x49 (count=2)
[READ]  -> 0x4A (count=1)
[READ]  -> 0x4B (count=0) EMPTY
[READ] (empty) -> FAIL (buffer empty)
```

---

## Notes

- Uses `& (BUFFER_SIZE - 1)` instead of `% BUFFER_SIZE` for head/tail wrap-around (bonus).  
  See comment in `ringbuf.c` for the explanation.
- All fixed-width types use `uint8_t` from `<stdint.h>` -- no plain `int` or `char` for data.
- Constants defined with `#define` -- no magic numbers in logic.
