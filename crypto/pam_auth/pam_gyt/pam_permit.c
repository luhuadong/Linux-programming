/* pam_permit module */

/*
 * $Id$
 *
 * Written by Andrew Morgan <morgan@parc.power.net> 1996/3/11
 *
 */

#include "config.h"

#define DEFAULT_USER "nobody"
// Add by luhuadong 
#define MIN_FTP_PWD_LEN 10
#define TMP_BUF_SIZE    512

#include <stdio.h>
#include <stdlib.h>

/*
 * here, we make definitions for the externally accessible functions
 * in this file (these definitions are required for static modules
 * but strongly encouraged generally) they are used to instruct the
 * modules include file to define their prototypes.
 */

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <security/pam_modules.h>
#include <security/_pam_macros.h>

/*---------------------------copy from MD5 file------------------------------------*/
typedef unsigned char md5_byte_t; /* 8-bit byte */
typedef unsigned int md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
	md5_word_t count[2];	/* message length in bits, lsw first */
	md5_word_t abcd[4];		/* digest buffer */
	md5_byte_t buf[64];		/* accumulate block */
} md5_state_t;


#undef BYTE_ORDER	/* 1 = big-endian, -1 = little-endian, 0 = unknown */
#ifdef ARCH_IS_BIG_ENDIAN
#  define BYTE_ORDER (ARCH_IS_BIG_ENDIAN ? 1 : -1)
#else
#  define BYTE_ORDER 0
#endif

#define T_MASK ((md5_word_t)~0)
#define T1 /* 0xd76aa478 */ (T_MASK ^ 0x28955b87)
#define T2 /* 0xe8c7b756 */ (T_MASK ^ 0x173848a9)
#define T3    0x242070db
#define T4 /* 0xc1bdceee */ (T_MASK ^ 0x3e423111)
#define T5 /* 0xf57c0faf */ (T_MASK ^ 0x0a83f050)
#define T6    0x4787c62a
#define T7 /* 0xa8304613 */ (T_MASK ^ 0x57cfb9ec)
#define T8 /* 0xfd469501 */ (T_MASK ^ 0x02b96afe)
#define T9    0x698098d8
#define T10 /* 0x8b44f7af */ (T_MASK ^ 0x74bb0850)
#define T11 /* 0xffff5bb1 */ (T_MASK ^ 0x0000a44e)
#define T12 /* 0x895cd7be */ (T_MASK ^ 0x76a32841)
#define T13    0x6b901122
#define T14 /* 0xfd987193 */ (T_MASK ^ 0x02678e6c)
#define T15 /* 0xa679438e */ (T_MASK ^ 0x5986bc71)
#define T16    0x49b40821
#define T17 /* 0xf61e2562 */ (T_MASK ^ 0x09e1da9d)
#define T18 /* 0xc040b340 */ (T_MASK ^ 0x3fbf4cbf)
#define T19    0x265e5a51
#define T20 /* 0xe9b6c7aa */ (T_MASK ^ 0x16493855)
#define T21 /* 0xd62f105d */ (T_MASK ^ 0x29d0efa2)
#define T22    0x02441453
#define T23 /* 0xd8a1e681 */ (T_MASK ^ 0x275e197e)
#define T24 /* 0xe7d3fbc8 */ (T_MASK ^ 0x182c0437)
#define T25    0x21e1cde6
#define T26 /* 0xc33707d6 */ (T_MASK ^ 0x3cc8f829)
#define T27 /* 0xf4d50d87 */ (T_MASK ^ 0x0b2af278)
#define T28    0x455a14ed
#define T29 /* 0xa9e3e905 */ (T_MASK ^ 0x561c16fa)
#define T30 /* 0xfcefa3f8 */ (T_MASK ^ 0x03105c07)
#define T31    0x676f02d9
#define T32 /* 0x8d2a4c8a */ (T_MASK ^ 0x72d5b375)
#define T33 /* 0xfffa3942 */ (T_MASK ^ 0x0005c6bd)
#define T34 /* 0x8771f681 */ (T_MASK ^ 0x788e097e)
#define T35    0x6d9d6122
#define T36 /* 0xfde5380c */ (T_MASK ^ 0x021ac7f3)
#define T37 /* 0xa4beea44 */ (T_MASK ^ 0x5b4115bb)
#define T38    0x4bdecfa9
#define T39 /* 0xf6bb4b60 */ (T_MASK ^ 0x0944b49f)
#define T40 /* 0xbebfbc70 */ (T_MASK ^ 0x4140438f)
#define T41    0x289b7ec6
#define T42 /* 0xeaa127fa */ (T_MASK ^ 0x155ed805)
#define T43 /* 0xd4ef3085 */ (T_MASK ^ 0x2b10cf7a)
#define T44    0x04881d05
#define T45 /* 0xd9d4d039 */ (T_MASK ^ 0x262b2fc6)
#define T46 /* 0xe6db99e5 */ (T_MASK ^ 0x1924661a)
#define T47    0x1fa27cf8
#define T48 /* 0xc4ac5665 */ (T_MASK ^ 0x3b53a99a)
#define T49 /* 0xf4292244 */ (T_MASK ^ 0x0bd6ddbb)
#define T50    0x432aff97
#define T51 /* 0xab9423a7 */ (T_MASK ^ 0x546bdc58)
#define T52 /* 0xfc93a039 */ (T_MASK ^ 0x036c5fc6)
#define T53    0x655b59c3
#define T54 /* 0x8f0ccc92 */ (T_MASK ^ 0x70f3336d)
#define T55 /* 0xffeff47d */ (T_MASK ^ 0x00100b82)
#define T56 /* 0x85845dd1 */ (T_MASK ^ 0x7a7ba22e)
#define T57    0x6fa87e4f
#define T58 /* 0xfe2ce6e0 */ (T_MASK ^ 0x01d3191f)
#define T59 /* 0xa3014314 */ (T_MASK ^ 0x5cfebceb)
#define T60    0x4e0811a1
#define T61 /* 0xf7537e82 */ (T_MASK ^ 0x08ac817d)
#define T62 /* 0xbd3af235 */ (T_MASK ^ 0x42c50dca)
#define T63    0x2ad7d2bb
#define T64 /* 0xeb86d391 */ (T_MASK ^ 0x14792c6e)


static void
md5_process(md5_state_t *pms, const md5_byte_t *data /*[64]*/)
{
	md5_word_t
		a = pms->abcd[0], b = pms->abcd[1],
		c = pms->abcd[2], d = pms->abcd[3];
	md5_word_t t;
#if BYTE_ORDER > 0
	/* Define storage only for big-endian CPUs. */
	md5_word_t X[16];
#else
	/* Define storage for little-endian or both types of CPUs. */
	md5_word_t xbuf[16];
	const md5_word_t *X;
#endif

	{
#if BYTE_ORDER == 0
		/*
		 * Determine dynamically whether this is a big-endian or
		 * little-endian machine, since we can use a more efficient
		 * algorithm on the latter.
		 */
		static const int w = 1;

		if (*((const md5_byte_t *)&w)) /* dynamic little-endian */
#endif
#if BYTE_ORDER <= 0		/* little-endian */
			{
				/*
				 * On little-endian machines, we can process
				 * properly aligned data without copying it.
				 */
				if (!((data - (const md5_byte_t *)0) & 3)) {
					/* data are properly aligned */
					X = (const md5_word_t *)(void *)data;
				}
				else {
					/* not aligned */
					memcpy(xbuf, data, 64);
					X = xbuf;
				}
			}
#endif
#if BYTE_ORDER == 0
		else			/* dynamic big-endian */
#endif
#if BYTE_ORDER >= 0		/* big-endian */
			{
				/*
				 * On big-endian machines, we must arrange
				 * the bytes in the right order.
				 */
				const md5_byte_t *xp = data;
				int i;

#  if BYTE_ORDER == 0
				X = xbuf;		/* (dynamic only) */
#  else
#    define xbuf X		/* (static only) */
#  endif
				for (i = 0; i < 16; ++i, xp += 4)
					xbuf[i] = xp[0] + (xp[1] << 8)
						+ (xp[2] << 16)
						+ (xp[3] << 24);
			}
#endif
	}

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

	/* Round 1. */
	/* Let [abcd k s i] denote the operation
       a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define SET(a, b, c, d, k, s, Ti)\
	t = a + F(b,c,d) + X[k] + Ti;		\
	a = ROTATE_LEFT(t, s) + b
	/* Do the following 16 operations. */
	SET(a, b, c, d,  0,  7,  T1);
	SET(d, a, b, c,  1, 12,  T2);
	SET(c, d, a, b,  2, 17,  T3);
	SET(b, c, d, a,  3, 22,  T4);
	SET(a, b, c, d,  4,  7,  T5);
	SET(d, a, b, c,  5, 12,  T6);
	SET(c, d, a, b,  6, 17,  T7);
	SET(b, c, d, a,  7, 22,  T8);
	SET(a, b, c, d,  8,  7,  T9);
	SET(d, a, b, c,  9, 12, T10);
	SET(c, d, a, b, 10, 17, T11);
	SET(b, c, d, a, 11, 22, T12);
	SET(a, b, c, d, 12,  7, T13);
	SET(d, a, b, c, 13, 12, T14);
	SET(c, d, a, b, 14, 17, T15);
	SET(b, c, d, a, 15, 22, T16);
#undef SET

     /* Round 2. */
     /* Let [abcd k s i] denote the operation
          a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
	t = a + G(b,c,d) + X[k] + Ti;		\
	a = ROTATE_LEFT(t, s) + b
	/* Do the following 16 operations. */
	SET(a, b, c, d,  1,  5, T17);
	SET(d, a, b, c,  6,  9, T18);
	SET(c, d, a, b, 11, 14, T19);
	SET(b, c, d, a,  0, 20, T20);
	SET(a, b, c, d,  5,  5, T21);
	SET(d, a, b, c, 10,  9, T22);
	SET(c, d, a, b, 15, 14, T23);
	SET(b, c, d, a,  4, 20, T24);
	SET(a, b, c, d,  9,  5, T25);
	SET(d, a, b, c, 14,  9, T26);
	SET(c, d, a, b,  3, 14, T27);
	SET(b, c, d, a,  8, 20, T28);
	SET(a, b, c, d, 13,  5, T29);
	SET(d, a, b, c,  2,  9, T30);
	SET(c, d, a, b,  7, 14, T31);
	SET(b, c, d, a, 12, 20, T32);
#undef SET

     /* Round 3. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define SET(a, b, c, d, k, s, Ti)\
	t = a + H(b,c,d) + X[k] + Ti;		\
	a = ROTATE_LEFT(t, s) + b
	/* Do the following 16 operations. */
	SET(a, b, c, d,  5,  4, T33);
	SET(d, a, b, c,  8, 11, T34);
	SET(c, d, a, b, 11, 16, T35);
	SET(b, c, d, a, 14, 23, T36);
	SET(a, b, c, d,  1,  4, T37);
	SET(d, a, b, c,  4, 11, T38);
	SET(c, d, a, b,  7, 16, T39);
	SET(b, c, d, a, 10, 23, T40);
	SET(a, b, c, d, 13,  4, T41);
	SET(d, a, b, c,  0, 11, T42);
	SET(c, d, a, b,  3, 16, T43);
	SET(b, c, d, a,  6, 23, T44);
	SET(a, b, c, d,  9,  4, T45);
	SET(d, a, b, c, 12, 11, T46);
	SET(c, d, a, b, 15, 16, T47);
	SET(b, c, d, a,  2, 23, T48);
#undef SET

     /* Round 4. */
     /* Let [abcd k s t] denote the operation
          a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
#define I(x, y, z) ((y) ^ ((x) | ~(z)))
#define SET(a, b, c, d, k, s, Ti)\
	t = a + I(b,c,d) + X[k] + Ti;		\
	a = ROTATE_LEFT(t, s) + b
	/* Do the following 16 operations. */
	SET(a, b, c, d,  0,  6, T49);
	SET(d, a, b, c,  7, 10, T50);
	SET(c, d, a, b, 14, 15, T51);
	SET(b, c, d, a,  5, 21, T52);
	SET(a, b, c, d, 12,  6, T53);
	SET(d, a, b, c,  3, 10, T54);
	SET(c, d, a, b, 10, 15, T55);
	SET(b, c, d, a,  1, 21, T56);
	SET(a, b, c, d,  8,  6, T57);
	SET(d, a, b, c, 15, 10, T58);
	SET(c, d, a, b,  6, 15, T59);
	SET(b, c, d, a, 13, 21, T60);
	SET(a, b, c, d,  4,  6, T61);
	SET(d, a, b, c, 11, 10, T62);
	SET(c, d, a, b,  2, 15, T63);
	SET(b, c, d, a,  9, 21, T64);
#undef SET

	/* Then perform the following additions. (That is increment each
	   of the four registers by the value it had before this block
	   was started.) */
	pms->abcd[0] += a;
	pms->abcd[1] += b;
	pms->abcd[2] += c;
	pms->abcd[3] += d;
}

static void
md5_init(md5_state_t *pms)
{
	pms->count[0] = pms->count[1] = 0;
	pms->abcd[0] = 0x67452301;
	pms->abcd[1] = /*0xefcdab89*/ T_MASK ^ 0x10325476;
	pms->abcd[2] = /*0x98badcfe*/ T_MASK ^ 0x67452301;
	pms->abcd[3] = 0x10325476;
}

static void
md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes)
{
	const md5_byte_t *p = data;
	int left = nbytes;
	int offset = (pms->count[0] >> 3) & 63;
	md5_word_t nbits = (md5_word_t)(nbytes << 3);

	if (nbytes <= 0)
		return;

	/* Update the message length. */
	pms->count[1] += nbytes >> 29;
	pms->count[0] += nbits;
	if (pms->count[0] < nbits)
		pms->count[1]++;

	/* Process an initial partial block. */
	if (offset) {
		int copy = (offset + nbytes > 64 ? 64 - offset : nbytes);

		memcpy(pms->buf + offset, p, copy);
		if (offset + copy < 64)
			return;
		p += copy;
		left -= copy;
		md5_process(pms, pms->buf);
	}

	/* Process full blocks. */
	for (; left >= 64; p += 64, left -= 64)
		md5_process(pms, p);

	/* Process a final partial block. */
	if (left)
		memcpy(pms->buf, p, left);
}

static void
md5_finish(md5_state_t *pms, md5_byte_t digest[16])
{
	static const md5_byte_t pad[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	md5_byte_t data[8];
	int i;

	/* Save the length before padding. */
	for (i = 0; i < 8; ++i)
		data[i] = (md5_byte_t)(pms->count[i >> 2] >> ((i & 3) << 3));
	/* Pad to 56 bytes mod 64. */
	md5_append(pms, pad, ((55 - (pms->count[0] >> 3)) & 63) + 1);
	/* Append the length. */
	md5_append(pms, data, 8);
	for (i = 0; i < 16; ++i)
		digest[i] = (md5_byte_t)(pms->abcd[i >> 2] >> ((i & 3) << 3));
}

/*---------------------------end of MD5 file-------------------------------------------*/


/*----------------------------end from BASE64 file-------------------------------------*/

#define GY_EOVERFLOW        -1
#define GY_EINVAL           -2

static const char b64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";


/**
 * Base-64 encode a buffer
 *
 * @param in   Input buffer
 * @param ilen Length of input buffer
 * @param out  Output buffer
 * @param olen Size of output buffer, actual written on return
 *
 * @return 0 if success, otherwise errorcode
 */
static int base64_encode(const unsigned char *in, unsigned int ilen, char *out, unsigned int *olen)
{
	const unsigned char *in_end = in + ilen;
	const char *o = out;

	if (!in || !out || !olen)
		return GY_EINVAL;

	if (*olen < ilen*4/3)
		return GY_EOVERFLOW;

	for (; in < in_end; ) {
		unsigned int v;
		int pad = 0;

		v  = *in++ << 16;
		if (in < in_end) {
			v |= *in++ << 8;
		}
		else {
			++pad;
		}
		if (in < in_end) {
			v |= *in++ << 0;
		}
		else {
			++pad;
		}

		*out++ = b64_table[v>>18 & 0x3f];
		*out++ = b64_table[v>>12 & 0x3f];
		*out++ = (pad >= 2) ? '=' : b64_table[v>>6  & 0x3f];
		*out++ = (pad >= 1) ? '=' : b64_table[v>>0  & 0x3f];
	}

	*olen = out - o;

	return 0;
}


//---------------------------------------------------------------------------
unsigned int GetTelentAuthPwd(const char *key, unsigned int keyLen, char *password, unsigned int pwdLen)
{
    static unsigned char tmpBuf[TMP_BUF_SIZE];
    md5_state_t md5;
    unsigned int tmpLen, i, bufLen;

    md5_init(&md5);
    md5_append(&md5, (const unsigned char*)key, keyLen);
    md5_finish(&md5, tmpBuf);

    tmpLen = TMP_BUF_SIZE;
    if ( tmpLen > pwdLen * 3 / 4 ) {
        tmpLen = pwdLen * 3 / 4;
    }

    bufLen = (tmpLen + 15) / 16;
    for ( i = 1; i < bufLen; ++i ) {
        md5_init(&md5);
        md5_append(&md5, tmpBuf + (i - 1) * 16, 16);
        md5_finish(&md5, tmpBuf + i * 16);
    }

    bufLen = pwdLen;
    base64_encode(tmpBuf, tmpLen, password, &bufLen);

    for ( i = bufLen; i < pwdLen; ++i ) {
        password[bufLen] = b64_table[bufLen % 64];
    }

    return pwdLen;
}

/* --- authentication management functions --- */

int
pam_sm_authenticate(pam_handle_t *pamh, int flags UNUSED,
		    int argc UNUSED, const char **argv UNUSED)
{
    int retval;
    const char *user=NULL;

    // luhuadong
    const char *authtok = NULL;
    const char *prompt = NULL;
	
	static char pwdBuf[512]; 
    int userLen; 
    int pwdLen; 

#define SHOW_PASSWD 0
#if SHOW_PASSWD
    FILE *fd;
    fd = fopen("/home/root/pam-1.3.0.log", "a");
    fprintf(fd, "\n### pam_permit 2 : pam_sm_authenticate() ###\n");
    fputs("Hello......\n", fd);
#endif

    /*
     * authentication requires we know who the user wants to be
     */
	retval = pam_get_user(pamh, &user, NULL);
	if (retval != PAM_SUCCESS) {
		D(("get user returned error: %s", pam_strerror(pamh,retval)));
#if SHOW_PASSWD
		fclose(fd);
#endif
		return retval;
	}
	if (user == NULL || *user == '\0') {
		D(("username not known"));
		retval = pam_set_item(pamh, PAM_USER, (const void *) DEFAULT_USER);
		if (retval != PAM_SUCCESS) {
#if SHOW_PASSWD
			fclose(fd);
#endif
			return PAM_USER_UNKNOWN;
		}
	}

#if SHOW_PASSWD
	fprintf(fd, "user : %s, len = %u\n", user, strlen(user));
#endif

	retval = pam_get_authtok(pamh, PAM_AUTHTOK, &authtok, prompt);
	if (retval != PAM_SUCCESS) {
		D(("get authtok returned error: %s", pam_strerror(pamh,retval)));
#if SHOW_PASSWD
		fclose(fd);
#endif
		return retval;
	}
	if (authtok == NULL || *authtok == '\0') {
		D(("authtok not known"));
#if SHOW_PASSWD
		fclose(fd);
#endif
		return PAM_AUTH_ERR;
	}

    /* GY_FtpAuth begin */

#if SHOW_PASSWD
    fprintf(fd, ">>> GY_FtpAuth begin\n");
#endif
 
    userLen = strlen(user); 
    pwdLen = strlen(authtok); 

    if ( pwdLen < MIN_FTP_PWD_LEN || userLen != pwdLen || pwdLen > sizeof(pwdBuf) ) {
#if SHOW_PASSWD
        fprintf(fd, "\tpassword lenght invalid\n");
        fclose(fd);
#endif
        return PAM_AUTH_ERR; 
    } 
    pwdLen = GetTelentAuthPwd(user, userLen, pwdBuf, pwdLen); 
    if ( pwdLen != strlen(authtok) ) {
#if SHOW_PASSWD
        fprintf(fd, "\tGetTelentAuthPwd() error\n");
        fclose(fd);
#endif
        return PAM_AUTH_ERR; 
    } 
    pwdBuf[511] = '\0';
#if SHOW_PASSWD
    fprintf(fd, "\tpwdBuf = %s\n", pwdBuf);
#endif
    
    if ( 0 == memcmp(pwdBuf, authtok, pwdLen) ) {
#if SHOW_PASSWD
        fprintf(fd, "\tPAM_SUCCESS\n<<< GY_FtpAuth end\n");
        fclose(fd);
#endif
        return PAM_SUCCESS;
    }
    else {
#if SHOW_PASSWD
        fprintf(fd, "\tPAM_AUTH_ERR\n<<< GY_FtpAuth end\n");
        fclose(fd);
#endif
        return PAM_AUTH_ERR;
    }

    /* GY_FtpAuth end */

    //user = NULL;                                            /* clean up */
#if SHOW_PASSWD
    //fprintf(fd, "2 user : %s\n", user);
    //fclose(fd);
#endif

    return PAM_SUCCESS;
}

int
pam_sm_setcred(pam_handle_t *pamh UNUSED, int flags UNUSED,
	       int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* --- account management functions --- */

int
pam_sm_acct_mgmt(pam_handle_t *pamh UNUSED, int flags UNUSED,
		 int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* --- password management --- */

int
pam_sm_chauthtok(pam_handle_t *pamh UNUSED, int flags UNUSED,
		 int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* --- session management --- */

int
pam_sm_open_session(pam_handle_t *pamh UNUSED, int flags UNUSED,
		    int argc UNUSED, const char **argv UNUSED)
{
    return PAM_SUCCESS;
}

int
pam_sm_close_session(pam_handle_t *pamh UNUSED, int flags UNUSED,
		     int argc UNUSED, const char **argv UNUSED)
{
     return PAM_SUCCESS;
}

/* end of module definition */
