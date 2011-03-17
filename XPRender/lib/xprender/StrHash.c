#include "StrHash.h"

#include "pstdint.h" /* Replace with <stdint.h> if appropriate */

//HSIEH algorithm http://www.azillionmonkeys.com/qed/hash.html
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
					+(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash (const char * data, int len)
{
	uint32_t hash = len, tmp;
	int rem;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (;len > 0; len--) {
		hash  += get16bits (data);
		tmp    = (get16bits (data+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		data  += 2*sizeof (uint16_t);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
		case 3: hash += get16bits (data);
				hash ^= hash << 16;
				hash ^= data[sizeof (uint16_t)] << 18;
				hash += hash >> 11;
				break;
		case 2: hash += get16bits (data);
				hash ^= hash << 11;
				hash += hash >> 17;
				break;
		case 1: hash += *data;
				hash ^= hash << 10;
				hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

XprHashCode HSIEH(const char * data, int len)
{
	return (XprHashCode)SuperFastHash(data, len);
}

#define xpr_hash_const(i) XPR_HASH_CONSTANTS[i % XPR_HASH_DEPTH]
#define xpr_hash_mult(i) XPR_HASH_MULTS[i % XPR_HASH_DEPTH]

XprHashCode XprHashStruct(const void* data, size_t len)
{
	/*
	if(1 == len) {
		return XPR_HASH_FUNCTION(XPR_HASH_CONSTANTS[0], ((char*)data)[0]);
	}
	else {
		size_t i;
		size_t last = xpr_hash_mult(len-2) * XPR_HASH_FUNCTION(xpr_hash_const(len-1), ((char*)data)[len-1]);

		for(i = len-2; i > 0; --i) {
			last = xpr_hash_mult(i-1) * XPR_HASH_FUNCTION(last, ((char*)data)[i]);
		}

		return XPR_HASH_FUNCTION(last, ((char*)data)[0]);
	}
	*/

	// http://en.wikipedia.org/wiki/Jenkins_hash_function
	uint32_t hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += ((char*)data)[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;

}