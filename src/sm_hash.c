/* SM.EXEC
   anton.bondarenko@gmail.com 
   Hash function by Bob Jenkins: 
   http://burtleburtle.net/bob/c/lookup3.c 
   https://en.wikipedia.org/wiki/Jenkins_hash_function */

#include "sm_hash.h"

#define bj_rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define bj_mix(a,b,c) { \
  a -= c;  a ^= bj_rot(c, 4);  c += b; \
  b -= a;  b ^= bj_rot(a, 6);  a += c; \
  c -= b;  c ^= bj_rot(b, 8);  b += a; \
  a -= c;  a ^= bj_rot(c,16);  c += b; \
  b -= a;  b ^= bj_rot(a,19);  a += c; \
  c -= b;  c ^= bj_rot(b, 4);  b += a; \
}

#define bj_final(a,b,c) { \
  c ^= b; c -= bj_rot(b,14); \
  a ^= c; a -= bj_rot(c,11); \
  b ^= a; b -= bj_rot(a,25); \
  c ^= b; c -= bj_rot(b,16); \
  a ^= c; a -= bj_rot(c,4);  \
  b ^= a; b -= bj_rot(a,14); \
  c ^= b; c -= bj_rot(b,24); \
}

uint32_t sm_hash_bj_little( const void *key, size_t length, uint32_t initval) {
    uint32_t a,b,c;
    a = b = c = 0xdeadbeef + ((uint32_t)length) + initval;
    const uint32_t *k = (const uint32_t *)key;
    const uint8_t  *k8; (void)k8;
    while (length > 12) {
		a += k[0];
      	b += k[1];
      	c += k[2];
      	bj_mix(a,b,c);
      	length -= 12;
      	k += 3;
    }
    switch(length) {
		case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
		case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
		case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
		case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
		case 8 : b+=k[1]; a+=k[0]; break;
		case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
		case 6 : b+=k[1]&0xffff; a+=k[0]; break;
		case 5 : b+=k[1]&0xff; a+=k[0]; break;
		case 4 : a+=k[0]; break;
		case 3 : a+=k[0]&0xffffff; break;
		case 2 : a+=k[0]&0xffff; break;
		case 1 : a+=k[0]&0xff; break;
		case 0 : return c;
    }
    bj_final(a,b,c);
    return c;
}

