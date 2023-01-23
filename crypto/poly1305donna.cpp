/*
    poly1305 implementation using 16 bit * 16 bit = 32 bit multiplication and 32 bit addition
    see: https://github.com/floodyberry/poly1305-donna
*/

#include "crypto/poly1305donna.h"

// #if defined(_MSC_VER)
// 	#define POLY1305_NOINLINE __declspec(noinline)
// #elif defined(__GNUC__)
// 	#define POLY1305_NOINLINE __attribute__((noinline))
// #else
#define POLY1305_NOINLINE
//#endif

namespace nanocrypto
{

    /* interpret two 8 bit unsigned integers as a 16 bit unsigned integer in little endian */
    static unsigned short
    U8TO16(const unsigned char *p)
    {
        return (((unsigned short)(p[0] & 0xff)) |
                ((unsigned short)(p[1] & 0xff) << 8));
    }

    /* store a 16 bit unsigned integer as two 8 bit unsigned integers in little endian */
    static void
    U16TO8(unsigned char *p, unsigned short v)
    {
        p[0] = (v)&0xff;
        p[1] = (v >> 8) & 0xff;
    }

    void
    poly1305_init(poly1305_context *ctx, const unsigned char key[32])
    {
        poly1305_state_internal_t *st = (poly1305_state_internal_t *)ctx;
        unsigned short t0, t1, t2, t3, t4, t5, t6, t7;
        size_t i;

        /* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
        t0 = U8TO16(&key[0]);
        st->r[0] = (t0)&0x1fff;
        t1 = U8TO16(&key[2]);
        st->r[1] = ((t0 >> 13) | (t1 << 3)) & 0x1fff;
        t2 = U8TO16(&key[4]);
        st->r[2] = ((t1 >> 10) | (t2 << 6)) & 0x1f03;
        t3 = U8TO16(&key[6]);
        st->r[3] = ((t2 >> 7) | (t3 << 9)) & 0x1fff;
        t4 = U8TO16(&key[8]);
        st->r[4] = ((t3 >> 4) | (t4 << 12)) & 0x00ff;
        st->r[5] = ((t4 >> 1)) & 0x1ffe;
        t5 = U8TO16(&key[10]);
        st->r[6] = ((t4 >> 14) | (t5 << 2)) & 0x1fff;
        t6 = U8TO16(&key[12]);
        st->r[7] = ((t5 >> 11) | (t6 << 5)) & 0x1f81;
        t7 = U8TO16(&key[14]);
        st->r[8] = ((t6 >> 8) | (t7 << 8)) & 0x1fff;
        st->r[9] = ((t7 >> 5)) & 0x007f;

        /* h = 0 */
        for (i = 0; i < 10; i++)
            st->h[i] = 0;

        /* save pad for later */
        for (i = 0; i < 8; i++)
            st->pad[i] = U8TO16(&key[16 + (2 * i)]);

        st->leftover = 0;
        st->final = 0;
    }

    static void
    poly1305_blocks(poly1305_state_internal_t *st, const unsigned char *m, size_t bytes)
    {
        const unsigned short hibit = (st->final) ? 0 : (1 << 11); /* 1 << 128 */
        unsigned short t0, t1, t2, t3, t4, t5, t6, t7;
        unsigned long d[10];
        unsigned long c;

        while (bytes >= poly1305_block_size)
        {
            size_t i, j;

            /* h += m[i] */
            t0 = U8TO16(&m[0]);
            st->h[0] += (t0)&0x1fff;
            t1 = U8TO16(&m[2]);
            st->h[1] += ((t0 >> 13) | (t1 << 3)) & 0x1fff;
            t2 = U8TO16(&m[4]);
            st->h[2] += ((t1 >> 10) | (t2 << 6)) & 0x1fff;
            t3 = U8TO16(&m[6]);
            st->h[3] += ((t2 >> 7) | (t3 << 9)) & 0x1fff;
            t4 = U8TO16(&m[8]);
            st->h[4] += ((t3 >> 4) | (t4 << 12)) & 0x1fff;
            st->h[5] += ((t4 >> 1)) & 0x1fff;
            t5 = U8TO16(&m[10]);
            st->h[6] += ((t4 >> 14) | (t5 << 2)) & 0x1fff;
            t6 = U8TO16(&m[12]);
            st->h[7] += ((t5 >> 11) | (t6 << 5)) & 0x1fff;
            t7 = U8TO16(&m[14]);
            st->h[8] += ((t6 >> 8) | (t7 << 8)) & 0x1fff;
            st->h[9] += ((t7 >> 5)) | hibit;

            /* h *= r, (partial) h %= p */
            for (i = 0, c = 0; i < 10; i++)
            {
                d[i] = c;
                for (j = 0; j < 10; j++)
                {
                    d[i] += (unsigned long)st->h[j] * ((j <= i) ? st->r[i - j] : (5 * st->r[i + 10 - j]));
                    /* Sum(h[i] * r[i] * 5) will overflow slightly above 6 products with an unclamped r, so carry at 5 */
                    if (j == 4)
                    {
                        c = (d[i] >> 13);
                        d[i] &= 0x1fff;
                    }
                }
                c += (d[i] >> 13);
                d[i] &= 0x1fff;
            }
            c = ((c << 2) + c); /* c *= 5 */
            c += d[0];
            d[0] = ((unsigned short)c & 0x1fff);
            c = (c >> 13);
            d[1] += c;

            for (i = 0; i < 10; i++)
                st->h[i] = (unsigned short)d[i];

            m += poly1305_block_size;
            bytes -= poly1305_block_size;
        }
    }

    POLY1305_NOINLINE void
    poly1305_finish(poly1305_context *ctx, unsigned char mac[16])
    {
        poly1305_state_internal_t *st = (poly1305_state_internal_t *)ctx;
        unsigned short c;
        unsigned short g[10];
        unsigned short mask;
        unsigned long f;
        size_t i;

        /* process the remaining block */
        if (st->leftover)
        {
            size_t i = st->leftover;
            st->buffer[i++] = 1;
            for (; i < poly1305_block_size; i++)
                st->buffer[i] = 0;
            st->final = 1;
            poly1305_blocks(st, st->buffer, poly1305_block_size);
        }

        /* fully carry h */
        c = st->h[1] >> 13;
        st->h[1] &= 0x1fff;
        for (i = 2; i < 10; i++)
        {
            st->h[i] += c;
            c = st->h[i] >> 13;
            st->h[i] &= 0x1fff;
        }
        st->h[0] += (c * 5);
        c = st->h[0] >> 13;
        st->h[0] &= 0x1fff;
        st->h[1] += c;
        c = st->h[1] >> 13;
        st->h[1] &= 0x1fff;
        st->h[2] += c;

        /* compute h + -p */
        g[0] = st->h[0] + 5;
        c = g[0] >> 13;
        g[0] &= 0x1fff;
        for (i = 1; i < 10; i++)
        {
            g[i] = st->h[i] + c;
            c = g[i] >> 13;
            g[i] &= 0x1fff;
        }

        /* select h if h < p, or h + -p if h >= p */
        mask = (c ^ 1) - 1;
        for (i = 0; i < 10; i++)
            g[i] &= mask;
        mask = ~mask;
        for (i = 0; i < 10; i++)
            st->h[i] = (st->h[i] & mask) | g[i];

        /* h = h % (2^128) */
        st->h[0] = ((st->h[0]) | (st->h[1] << 13)) & 0xffff;
        st->h[1] = ((st->h[1] >> 3) | (st->h[2] << 10)) & 0xffff;
        st->h[2] = ((st->h[2] >> 6) | (st->h[3] << 7)) & 0xffff;
        st->h[3] = ((st->h[3] >> 9) | (st->h[4] << 4)) & 0xffff;
        st->h[4] = ((st->h[4] >> 12) | (st->h[5] << 1) | (st->h[6] << 14)) & 0xffff;
        st->h[5] = ((st->h[6] >> 2) | (st->h[7] << 11)) & 0xffff;
        st->h[6] = ((st->h[7] >> 5) | (st->h[8] << 8)) & 0xffff;
        st->h[7] = ((st->h[8] >> 8) | (st->h[9] << 5)) & 0xffff;

        /* mac = (h + pad) % (2^128) */
        f = (unsigned long)st->h[0] + st->pad[0];
        st->h[0] = (unsigned short)f;
        for (i = 1; i < 8; i++)
        {
            f = (unsigned long)st->h[i] + st->pad[i] + (f >> 16);
            st->h[i] = (unsigned short)f;
        }

        for (i = 0; i < 8; i++)
            U16TO8(mac + (i * 2), st->h[i]);

        /* zero out the state */
        for (i = 0; i < 10; i++)
            st->h[i] = 0;
        for (i = 0; i < 10; i++)
            st->r[i] = 0;
        for (i = 0; i < 8; i++)
            st->pad[i] = 0;
    }


    /* auto detect between 32bit / 64bit */
#define HAS_SIZEOF_INT128_64BIT (defined(__SIZEOF_INT128__) && defined(__LP64__))
#define HAS_MSVC_64BIT (defined(_MSC_VER) && defined(_M_X64))
#define HAS_GCC_4_4_64BIT (defined(__GNUC__) && defined(__LP64__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 4))))


void
poly1305_update(poly1305_context *ctx, const unsigned char *m, size_t bytes) {
	poly1305_state_internal_t *st = (poly1305_state_internal_t *)ctx;
	size_t i;

	/* handle leftover */
	if (st->leftover) {
		size_t want = (poly1305_block_size - st->leftover);
		if (want > bytes)
			want = bytes;
		for (i = 0; i < want; i++)
			st->buffer[st->leftover + i] = m[i];
		bytes -= want;
		m += want;
		st->leftover += want;
		if (st->leftover < poly1305_block_size)
			return;
		poly1305_blocks(st, st->buffer, poly1305_block_size);
		st->leftover = 0;
	}

	/* process full blocks */
	if (bytes >= poly1305_block_size) {
		size_t want = (bytes & ~(poly1305_block_size - 1));
		poly1305_blocks(st, m, want);
		m += want;
		bytes -= want;
	}

	/* store leftover */
	if (bytes) {
		for (i = 0; i < bytes; i++)
			st->buffer[st->leftover + i] = m[i];
		st->leftover += bytes;
	}
}

void
poly1305_auth(unsigned char mac[16], const unsigned char *m, size_t bytes, const unsigned char key[32]) {
	poly1305_context ctx;
	poly1305_init(&ctx, key);
	poly1305_update(&ctx, m, bytes);
	poly1305_finish(&ctx, mac);
}

int
wrong_poly1305_verify(const unsigned char mac1[16], const unsigned char mac2[16]) {
	size_t i;
	unsigned int dif = 0;
	for (i = 0; i < 16; i++)
		dif |= (mac1[i] ^ mac2[i]);
	
    // dif = (dif - 1) >> ((sizeof(unsigned int) * 8) - 1);
	// return (dif & 1);
    return (1 & ((dif - 1) >> 8)) - 1; // anything but 0 becomes 1
}

// from libsodium:
static inline int
crypto_verify_n(const unsigned char *x_, const unsigned char *y_,
                const int n)
{
    const volatile unsigned char *volatile x =
        (const volatile unsigned char *volatile) x_;
    const volatile unsigned char *volatile y =
        (const volatile unsigned char *volatile) y_;
    volatile uint_fast16_t d = 0U;
    int i;

    for (i = 0; i < n; i++) {
        d |= x[i] ^ y[i];
    }
    return (1 & ((d - 1) >> 8)) - 1;
}

int poly1305_verify(const unsigned char mac1[16], const unsigned char mac2[16]) {
    int val1 = crypto_verify_n(mac1, mac2,16);
    int val2 = wrong_poly1305_verify(mac1, mac2);
    if ( val1 != val2 ){
        return val1;
    }
    return val1;
}


/* test a few basic operations */
int
poly1305_power_on_self_test(void) {
	/* example from nacl */
	static const unsigned char PROGMEM  nacl_key[32] = {
		0xee,0xa6,0xa7,0x25,0x1c,0x1e,0x72,0x91,
		0x6d,0x11,0xc2,0xcb,0x21,0x4d,0x3c,0x25,
		0x25,0x39,0x12,0x1d,0x8e,0x23,0x4e,0x65,
		0x2d,0x65,0x1f,0xa4,0xc8,0xcf,0xf8,0x80,
	};

	static const unsigned char PROGMEM nacl_msg[131] = {
		0x8e,0x99,0x3b,0x9f,0x48,0x68,0x12,0x73,
		0xc2,0x96,0x50,0xba,0x32,0xfc,0x76,0xce,
		0x48,0x33,0x2e,0xa7,0x16,0x4d,0x96,0xa4,
		0x47,0x6f,0xb8,0xc5,0x31,0xa1,0x18,0x6a,
		0xc0,0xdf,0xc1,0x7c,0x98,0xdc,0xe8,0x7b,
		0x4d,0xa7,0xf0,0x11,0xec,0x48,0xc9,0x72,
		0x71,0xd2,0xc2,0x0f,0x9b,0x92,0x8f,0xe2,
		0x27,0x0d,0x6f,0xb8,0x63,0xd5,0x17,0x38,
		0xb4,0x8e,0xee,0xe3,0x14,0xa7,0xcc,0x8a,
		0xb9,0x32,0x16,0x45,0x48,0xe5,0x26,0xae,
		0x90,0x22,0x43,0x68,0x51,0x7a,0xcf,0xea,
		0xbd,0x6b,0xb3,0x73,0x2b,0xc0,0xe9,0xda,
		0x99,0x83,0x2b,0x61,0xca,0x01,0xb6,0xde,
		0x56,0x24,0x4a,0x9e,0x88,0xd5,0xf9,0xb3,
		0x79,0x73,0xf6,0x22,0xa4,0x3d,0x14,0xa6,
		0x59,0x9b,0x1f,0x65,0x4c,0xb4,0x5a,0x74,
		0xe3,0x55,0xa5
	};

	static const unsigned char PROGMEM nacl_mac[16] = {
		0xf3,0xff,0xc7,0x70,0x3f,0x94,0x00,0xe5,
		0x2a,0x7d,0xfb,0x4b,0x3d,0x33,0x05,0xd9
	};

	/* generates a final value of (2^130 - 2) == 3 */
	static const unsigned char PROGMEM wrap_key[32] = {
		0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};

	static const unsigned char PROGMEM wrap_msg[16] = {
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
	};

	static const unsigned char PROGMEM wrap_mac[16] = {
		0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	};

	/*
		mac of the macs of messages of length 0 to 256, where the key and messages
		have all their values set to the length
	*/
	static const unsigned char PROGMEM total_key[32] = {
		0x01,0x02,0x03,0x04,0x05,0x06,0x07,
		0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff
	};

	static const unsigned char PROGMEM total_mac[16] = {
		0x64,0xaf,0xe2,0xe8,0xd6,0xad,0x7b,0xbd,
		0xd2,0x87,0xf9,0x7c,0x44,0x62,0x3d,0x39
	};

	poly1305_context ctx;
	poly1305_context total_ctx;
	unsigned char all_key[32];
	unsigned char all_msg[256];
	unsigned char mac[16];
	size_t i, j;
	int result = 1;

	for (i = 0; i < sizeof(mac); i++)
		mac[i] = 0;
	poly1305_auth(mac, nacl_msg, sizeof(nacl_msg), nacl_key);
	result &= poly1305_verify(nacl_mac, mac);

	for (i = 0; i < sizeof(mac); i++)
		mac[i] = 0;
	poly1305_init(&ctx, nacl_key);
	poly1305_update(&ctx, nacl_msg +   0, 32);
	poly1305_update(&ctx, nacl_msg +  32, 64);
	poly1305_update(&ctx, nacl_msg +  96, 16);
	poly1305_update(&ctx, nacl_msg + 112,  8);
	poly1305_update(&ctx, nacl_msg + 120,  4);
	poly1305_update(&ctx, nacl_msg + 124,  2);
	poly1305_update(&ctx, nacl_msg + 126,  1);
	poly1305_update(&ctx, nacl_msg + 127,  1);
	poly1305_update(&ctx, nacl_msg + 128,  1);
	poly1305_update(&ctx, nacl_msg + 129,  1);
	poly1305_update(&ctx, nacl_msg + 130,  1);
	poly1305_finish(&ctx, mac);
	result &= poly1305_verify(nacl_mac, mac);

	for (i = 0; i < sizeof(mac); i++)
		mac[i] = 0;
	poly1305_auth(mac, wrap_msg, sizeof(wrap_msg), wrap_key);
	result &= poly1305_verify(wrap_mac, mac);

	poly1305_init(&total_ctx, total_key);
	for (i = 0; i < 256; i++) {
		/* set key and message to 'i,i,i..' */
		for (j = 0; j < sizeof(all_key); j++)
			all_key[j] = i;
		for (j = 0; j < i; j++)
			all_msg[j] = i;
		poly1305_auth(mac, all_msg, i, all_key);
		poly1305_update(&total_ctx, mac, 16);
	}
	poly1305_finish(&total_ctx, mac);
	result &= poly1305_verify(total_mac, mac);

	return result;
}

}