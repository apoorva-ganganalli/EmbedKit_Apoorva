/*
 * ringbuf.c  --  circular buffer for uint8_t data
 *
 * Apoorva  |  EmbedKit Assignment  |  Embed Square Solutions
 *
 * Compile:  gcc -Wall -std=c99 ringbuf.c -o ringbuf
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  Config                                                              */
/* ------------------------------------------------------------------ */

/*
 * BUFFER_SIZE must stay a power of 2.
 * That lets us use the & trick below instead of % for index wrap-around.
 * On most Cortex-M0 / AVR MCUs there is no hardware divider, so % compiles
 * to a slow software division routine (~20-50 cycles).  A bitwise AND is
 * always a single cycle.  In a UART ISR that fires hundreds of times per
 * second, that difference actually matters.
 */
#define BUFFER_SIZE   8
#define WRAP_MASK     (BUFFER_SIZE - 1)   /* 0x07 -- works because 8 is 2^3 */

/* return codes */
#define RB_OK    0
#define RB_ERR  -1

/* ------------------------------------------------------------------ */
/*  Data structure                                                      */
/* ------------------------------------------------------------------ */

typedef struct {
    uint8_t  buf[BUFFER_SIZE];
    uint8_t  head;    /* next slot to write into */
    uint8_t  tail;    /* next slot to read from  */
    uint8_t  count;   /* bytes currently stored  */
} RingBuf;

/* ------------------------------------------------------------------ */
/*  API                                                                 */
/* ------------------------------------------------------------------ */

void ringbuf_init(RingBuf *rb)
{
    rb->head  = 0;
    rb->tail  = 0;
    rb->count = 0;
    memset(rb->buf, 0, sizeof(rb->buf));
}

/*
 * Write one byte.
 * Returns RB_OK on success, RB_ERR if the buffer is already full.
 * We never silently overwrite unread data -- the caller must handle the error.
 */
int ringbuf_write(RingBuf *rb, uint8_t byte)
{
    if (rb->count == BUFFER_SIZE)
        return RB_ERR;

    rb->buf[rb->head] = byte;
    rb->head = (uint8_t)((rb->head + 1) & WRAP_MASK);  /* wrap with AND, not % */
    rb->count++;
    return RB_OK;
}

/*
 * Read one byte into *out.
 * Returns RB_OK on success, RB_ERR if the buffer is empty.
 */
int ringbuf_read(RingBuf *rb, uint8_t *out)
{
    if (rb->count == 0)
        return RB_ERR;

    *out = rb->buf[rb->tail];
    rb->tail = (uint8_t)((rb->tail + 1) & WRAP_MASK);
    rb->count--;
    return RB_OK;
}

uint8_t ringbuf_count(const RingBuf *rb)
{
    return rb->count;
}

int ringbuf_is_full(const RingBuf *rb)
{
    return rb->count == BUFFER_SIZE;
}

int ringbuf_is_empty(const RingBuf *rb)
{
    return rb->count == 0;
}

/* ------------------------------------------------------------------ */
/*  Demo / main                                                         */
/* ------------------------------------------------------------------ */

int main(void)
{
    RingBuf rb;
    ringbuf_init(&rb);

    uint8_t byte;
    int     ret;

    /* --- Step 1: fill the buffer with 0x41 .. 0x48 --- */
    uint8_t fill_data[] = { 0x41, 0x42, 0x43, 0x44,
                             0x45, 0x46, 0x47, 0x48 };
    int fill_len = (int)(sizeof(fill_data) / sizeof(fill_data[0]));

    for (int i = 0; i < fill_len; i++) {
        ret = ringbuf_write(&rb, fill_data[i]);
        if (ret == RB_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   fill_data[i],
                   (unsigned)ringbuf_count(&rb),
                   ringbuf_is_full(&rb) ? " FULL" : "");
        }
    }

    /* --- Step 2: try to push one more byte in -- should fail --- */
    ret = ringbuf_write(&rb, 0x99);
    if (ret == RB_ERR)
        printf("[WRITE] 0x99 -> FAIL (buffer full)\n");

    /* --- Step 3: read back 3 bytes --- */
    for (int i = 0; i < 3; i++) {
        ret = ringbuf_read(&rb, &byte);
        if (ret == RB_OK)
            printf("[READ]  -> 0x%02X (count=%u)\n",
                   (unsigned)byte, (unsigned)ringbuf_count(&rb));
    }

    /* --- Step 4: write 3 new bytes into the freed slots --- */
    uint8_t new_data[] = { 0x49, 0x4A, 0x4B };
    int new_len = (int)(sizeof(new_data) / sizeof(new_data[0]));

    for (int i = 0; i < new_len; i++) {
        ret = ringbuf_write(&rb, new_data[i]);
        if (ret == RB_OK) {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n",
                   new_data[i],
                   (unsigned)ringbuf_count(&rb),
                   ringbuf_is_full(&rb) ? " FULL" : "");
        }
    }

    /* --- Step 5: drain everything out --- */
    while (!ringbuf_is_empty(&rb)) {
        ret = ringbuf_read(&rb, &byte);
        if (ret == RB_OK) {
            printf("[READ]  -> 0x%02X (count=%u)%s\n",
                   (unsigned)byte,
                   (unsigned)ringbuf_count(&rb),
                   ringbuf_is_empty(&rb) ? " EMPTY" : "");
        }
    }

    /* --- Step 6: attempt read on empty buffer --- */
    ret = ringbuf_read(&rb, &byte);
    if (ret == RB_ERR)
        printf("[READ] (empty) -> FAIL (buffer empty)\n");

    return 0;
}
