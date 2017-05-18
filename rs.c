#include <string.h>
#include <stdint.h>
//#define DEBUG 2
#if defined DEBUG
#include <stdio.h>
#endif

/* Stuff specific to the CCSDS (255,223) RS codec
 * (255,223) code over GF(256). Note: the conventional basis is still
 * used; the dual-basis mappings are performed in [en|de]code_rs_ccsds.c
 *
 * Copyright 2003 Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

static inline int mod255(int x)
{
	while (x >= 255) {
		x -= 255;
		x = (x >> 8) + (x & 255);
	}
	return x;
}

static const uint8_t CCSDS_alpha_to[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x87, 0x89, 0x95, 0xad,
	0xdd, 0x3d, 0x7a, 0xf4, 0x6f, 0xde, 0x3b, 0x76, 0xec, 0x5f, 0xbe, 0xfb,
	0x71, 0xe2, 0x43, 0x86, 0x8b, 0x91, 0xa5, 0xcd, 0x1d, 0x3a, 0x74, 0xe8,
	0x57, 0xae, 0xdb, 0x31, 0x62, 0xc4, 0x0f, 0x1e, 0x3c, 0x78, 0xf0, 0x67,
	0xce, 0x1b, 0x36, 0x6c, 0xd8, 0x37, 0x6e, 0xdc, 0x3f, 0x7e, 0xfc, 0x7f,
	0xfe, 0x7b, 0xf6, 0x6b, 0xd6, 0x2b, 0x56, 0xac, 0xdf, 0x39, 0x72, 0xe4,
	0x4f, 0x9e, 0xbb, 0xf1, 0x65, 0xca, 0x13, 0x26, 0x4c, 0x98, 0xb7, 0xe9,
	0x55, 0xaa, 0xd3, 0x21, 0x42, 0x84, 0x8f, 0x99, 0xb5, 0xed, 0x5d, 0xba,
	0xf3, 0x61, 0xc2, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x07, 0x0e,
	0x1c, 0x38, 0x70, 0xe0, 0x47, 0x8e, 0x9b, 0xb1, 0xe5, 0x4d, 0x9a, 0xb3,
	0xe1, 0x45, 0x8a, 0x93, 0xa1, 0xc5, 0x0d, 0x1a, 0x34, 0x68, 0xd0, 0x27,
	0x4e, 0x9c, 0xbf, 0xf9, 0x75, 0xea, 0x53, 0xa6, 0xcb, 0x11, 0x22, 0x44,
	0x88, 0x97, 0xa9, 0xd5, 0x2d, 0x5a, 0xb4, 0xef, 0x59, 0xb2, 0xe3, 0x41,
	0x82, 0x83, 0x81, 0x85, 0x8d, 0x9d, 0xbd, 0xfd, 0x7d, 0xfa, 0x73, 0xe6,
	0x4b, 0x96, 0xab, 0xd1, 0x25, 0x4a, 0x94, 0xaf, 0xd9, 0x35, 0x6a, 0xd4,
	0x2f, 0x5e, 0xbc, 0xff, 0x79, 0xf2, 0x63, 0xc6, 0x0b, 0x16, 0x2c, 0x58,
	0xb0, 0xe7, 0x49, 0x92, 0xa3, 0xc1, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0,
	0xc7, 0x09, 0x12, 0x24, 0x48, 0x90, 0xa7, 0xc9, 0x15, 0x2a, 0x54, 0xa8,
	0xd7, 0x29, 0x52, 0xa4, 0xcf, 0x19, 0x32, 0x64, 0xc8, 0x17, 0x2e, 0x5c,
	0xb8, 0xf7, 0x69, 0xd2, 0x23, 0x46, 0x8c, 0x9f, 0xb9, 0xf5, 0x6d, 0xda,
	0x33, 0x66, 0xcc, 0x1f, 0x3e, 0x7c, 0xf8, 0x77, 0xee, 0x5b, 0xb6, 0xeb,
	0x51, 0xa2, 0xc3, 0x00
};

static const uint8_t CCSDS_index_of[] = {
	255, 0, 1, 99, 2, 198, 100, 106, 3, 205, 199, 188, 101, 126, 107, 42, 4,
	141, 206, 78, 200, 212, 189, 225, 102, 221, 127, 49, 108, 32, 43, 243,
	5, 87, 142, 232, 207, 172, 79, 131, 201, 217, 213, 65, 190, 148, 226,
	180, 103, 39, 222, 240, 128, 177, 50, 53, 109, 69, 33, 18, 44, 13, 244,
	56, 6, 155, 88, 26, 143, 121, 233, 112, 208, 194, 173, 168, 80, 117,
	132, 72, 202, 252, 218, 138, 214, 84, 66, 36, 191, 152, 149, 249, 227,
	94, 181, 21, 104, 97, 40, 186, 223, 76, 241, 47, 129, 230, 178, 63, 51,
	238, 54, 16, 110, 24, 70, 166, 34, 136, 19, 247, 45, 184, 14, 61, 245,
	164, 57, 59, 7, 158, 156, 157, 89, 159, 27, 8, 144, 9, 122, 28, 234,
	160, 113, 90, 209, 29, 195, 123, 174, 10, 169, 145, 81, 91, 118, 114,
	133, 161, 73, 235, 203, 124, 253, 196, 219, 30, 139, 210, 215, 146, 85,
	170, 67, 11, 37, 175, 192, 115, 153, 119, 150, 92, 250, 82, 228, 236,
	95, 74, 182, 162, 22, 134, 105, 197, 98, 254, 41, 125, 187, 204, 224,
	211, 77, 140, 242, 31, 48, 220, 130, 171, 231, 86, 179, 147, 64, 216,
	52, 176, 239, 38, 55, 12, 17, 68, 111, 120, 25, 154, 71, 116, 167, 193,
	35, 83, 137, 251, 20, 93, 248, 151, 46, 75, 185, 96, 15, 237, 62, 229,
	246, 135, 165, 23, 58, 163, 60, 183
};

static const uint8_t CCSDS_poly[] = {
	0, 249, 59, 66, 4, 43, 126, 251, 97, 30, 3, 213, 50, 66, 170, 5, 24, 5,
	170, 66, 50, 213, 3, 30, 97, 251, 126, 43, 4, 66, 59, 249, 0
};

#define MODNN(x) mod255(x)

#define MM 8
#define NN 255
#define ALPHA_TO CCSDS_alpha_to
#define INDEX_OF CCSDS_index_of
#define GENPOLY CCSDS_poly
#define NROOTS 32
#define FCR 112
#define PRIM 11
#define IPRIM 116
#define PAD pad

void ccsds_rs_encode(uint8_t *data, uint8_t *parity, int pad)
{
	/* The guts of the Reed-Solomon encoder, meant to be #included
	 * into a function body with the following typedefs, macros and variables supplied
	 * according to the code parameters:

	 * uint8_t - a typedef for the data symbol
	 * uint8_t data[] - array of NN-NROOTS-PAD and type uint8_t to be encoded
	 * uint8_t parity[] - an array of NROOTS and type uint8_t to be written with parity symbols
	 * NROOTS - the number of roots in the RS code generator polynomial,
	 *          which is the same as the number of parity symbols in a block.
	 Integer variable or literal.
	 * 
	 * NN - the total number of symbols in a RS block. Integer variable or literal.
	 * PAD - the number of pad symbols in a block. Integer variable or literal.
	 * ALPHA_TO - The address of an array of NN elements to convert Galois field
	 *            elements in index (log) form to polynomial form. Read only.
	 * INDEX_OF - The address of an array of NN elements to convert Galois field
	 *            elements in polynomial form to index (log) form. Read only.
	 * MODNN - a function to reduce its argument modulo NN. May be inline or a macro.
	 * GENPOLY - an array of NROOTS+1 elements containing the generator polynomial in index form

	 * The memset() and memmove() functions are used. The appropriate header
	 * file declaring these functions (usually <string.h>) must be included by the calling
	 * program.

	 * Copyright 2004, Phil Karn, KA9Q
	 * May be used under the terms of the GNU Lesser General Public License (LGPL)
	 */


#undef A0
#define A0 (NN) /* Special reserved value encoding zero in index form */

	int i, j;
	uint8_t feedback;

	memset(parity,0,NROOTS*sizeof(uint8_t));

	for(i=0;i<NN-NROOTS-PAD;i++){
		feedback = INDEX_OF[data[i] ^ parity[0]];
		if(feedback != A0){      /* feedback term is non-zero */
#ifdef UNNORMALIZED
			/* This line is unnecessary when GENPOLY[NROOTS] is unity, as it must
			 * always be for the polynomials constructed by init_rs()
			 */
			feedback = MODNN(NN - GENPOLY[NROOTS] + feedback);
#endif
			for(j=1;j<NROOTS;j++)
				parity[j] ^= ALPHA_TO[MODNN(feedback + GENPOLY[NROOTS-j])];
		}
		/* Shift */
		memmove(&parity[0],&parity[1],sizeof(uint8_t)*(NROOTS-1));
		if(feedback != A0)
			parity[NROOTS-1] = ALPHA_TO[MODNN(feedback + GENPOLY[0])];
		else
			parity[NROOTS-1] = 0;
	}

}

int ccsds_rs_decode(uint8_t *data, int *eras_pos, int no_eras, int pad)
{
	int retval;

	if (pad < 0 || pad > 222) {
		return -1;
	}

	/* The guts of the Reed-Solomon decoder, meant to be #included
	 * into a function body with the following typedefs, macros and variables supplied
	 * according to the code parameters:

	 * uint8_t - a typedef for the data symbol
	 * uint8_t data[] - array of NN data and parity symbols to be corrected in place
	 * retval - an integer lvalue into which the decoder's return code is written
	 * NROOTS - the number of roots in the RS code generator polynomial,
	 *          which is the same as the number of parity symbols in a block.
	 Integer variable or literal.
	 * NN - the total number of symbols in a RS block. Integer variable or literal.
	 * PAD - the number of pad symbols in a block. Integer variable or literal.
	 * ALPHA_TO - The address of an array of NN elements to convert Galois field
	 *            elements in index (log) form to polynomial form. Read only.
	 * INDEX_OF - The address of an array of NN elements to convert Galois field
	 *            elements in polynomial form to index (log) form. Read only.
	 * MODNN - a function to reduce its argument modulo NN. May be inline or a macro.
	 * FCR - An integer literal or variable specifying the first consecutive root of the
	 *       Reed-Solomon generator polynomial. Integer variable or literal.
	 * PRIM - The primitive root of the generator poly. Integer variable or literal.
	 * DEBUG - If set to 1 or more, do various internal consistency checking. Leave this
	 *         undefined for production code

	 * The memset(), memmove(), and memcpy() functions are used. The appropriate header
	 * file declaring these functions (usually <string.h>) must be included by the calling
	 * program.
	 */


#if !defined(NROOTS)
#error "NROOTS not defined"
#endif

#if !defined(NN)
#error "NN not defined"
#endif

#if !defined(PAD)
#error "PAD not defined"
#endif

#if !defined(ALPHA_TO)
#error "ALPHA_TO not defined"
#endif

#if !defined(INDEX_OF)
#error "INDEX_OF not defined"
#endif

#if !defined(MODNN)
#error "MODNN not defined"
#endif

#if !defined(FCR)
#error "FCR not defined"
#endif

#if !defined(PRIM)
#error "PRIM not defined"
#endif

#if !defined(NULL)
#define NULL ((void *)0)
#endif

#undef MIN
#define	MIN(a,b)	((a) < (b) ? (a) : (b))
#undef A0
#define A0 (NN)

	int deg_lambda, el, deg_omega;
	int i, j, r,k;
	uint8_t u,q,tmp,num1,num2,den,discr_r;
	uint8_t lambda[NROOTS+1], s[NROOTS];	/* Err+Eras Locator poly and syndrome poly */
	uint8_t b[NROOTS+1], t[NROOTS+1], omega[NROOTS+1];
	uint8_t root[NROOTS], reg[NROOTS+1], loc[NROOTS];
	int syn_error, count;

	/* form the syndromes; i.e., evaluate data(x) at roots of g(x) */
	for (i = 0; i < NROOTS; i++) {
		s[i] = data[0];
	}

	for (j = 1; j < NN - PAD; j++) {
		for (i = 0; i < NROOTS; i++) {
			if (s[i] == 0) {
				s[i] = data[j];
			} else {
				s[i] = data[j] ^ ALPHA_TO[MODNN(INDEX_OF[s[i]] + (FCR + i) * PRIM)];
			}
		}
	}

	/* Convert syndromes to index form, checking for nonzero condition */
	syn_error = 0;
	for (i = 0; i < NROOTS; i++) {
		syn_error |= s[i];
		s[i] = INDEX_OF[s[i]];
	}

	if (!syn_error) {
		/* if syndrome is zero, data[] is a codeword and there are no
		 * errors to correct. So return data[] unmodified
		 */
		count = 0;
		goto finish;
	}
	memset(&lambda[1], 0, NROOTS * sizeof(lambda[0]));
	lambda[0] = 1;

	if (no_eras > 0) {
		/* Init lambda to be the erasure locator polynomial */
		lambda[1] = ALPHA_TO[MODNN(PRIM * (NN - 1 - eras_pos[0]))];
		for (i = 1; i < no_eras; i++) {
			u = MODNN(PRIM * (NN - 1 - eras_pos[i]));
			for (j = i + 1; j > 0; j--) {
				tmp = INDEX_OF[lambda[j - 1]];
				if (tmp != A0) {
					lambda[j] ^= ALPHA_TO[MODNN(u + tmp)];
				}
			}
		}

#if DEBUG >= 1
		/* Test code that verifies the erasure locator polynomial just constructed
		   Needed only for decoder debugging. */

		/* find roots of the erasure location polynomial */
		for (i = 1; i <= no_eras; i++) {
			reg[i] = INDEX_OF[lambda[i]];
		}

		count = 0;
		for (i = 1, k = IPRIM - 1; i <= NN; i++, k = MODNN(k + IPRIM)) {
			q = 1;
			for (j = 1; j <= no_eras; j++) {
				if (reg[j] != A0) {
					reg[j] = MODNN(reg[j] + j);
					q ^= ALPHA_TO[reg[j]];
				}
			}
			if (q != 0) {
				continue;
			}
			/* store root and error location number indices */
			root[count] = i;
			loc[count] = k;
			count++;
		}
		if (count != no_eras) {
			printf("count = %d no_eras = %d\n lambda(x) is WRONG\n",count,no_eras);
			count = -1;
			goto finish;
		}
#if DEBUG >= 2
		printf("\n Erasure positions as determined by roots of Eras Loc Poly:\n");
		for (i = 0; i < count; i++) {
			printf("%d ", loc[i]);
		}
		printf("\n");
#endif
#endif
	}
	for (i = 0; i < NROOTS + 1; i++) {
		b[i] = INDEX_OF[lambda[i]];
	}

	/*
	 * Begin Berlekamp-Massey algorithm to determine error+erasure
	 * locator polynomial
	 */
	r = no_eras;
	el = no_eras;
	while (++r <= NROOTS) {	/* r is the step number */
		/* Compute discrepancy at the r-th step in poly-form */
		discr_r = 0;
		for (i = 0; i < r; i++) {
			if ((lambda[i] != 0) && (s[r - i - 1] != A0)) {
				discr_r ^= ALPHA_TO[MODNN(INDEX_OF[lambda[i]] + s[r - i - 1])];
			}
		}
		discr_r = INDEX_OF[discr_r];	/* Index form */
		if (discr_r == A0) {
			/* 2 lines below: B(x) <-- x*B(x) */
			memmove(&b[1], b, NROOTS * sizeof(b[0]));
			b[0] = A0;
		} else {
			/* 7 lines below: T(x) <-- lambda(x) - discr_r*x*b(x) */
			t[0] = lambda[0];
			for (i = 0 ; i < NROOTS; i++) {
				if(b[i] != A0) {
					t[i + 1] = lambda[i + 1] ^ ALPHA_TO[MODNN(discr_r + b[i])];
				} else {
					t[i + 1] = lambda[i + 1];
				}
			}
			if (2 * el <= r + no_eras - 1) {
				el = r + no_eras - el;
				/*
				 * 2 lines below: B(x) <-- inv(discr_r) *
				 * lambda(x)
				 */
				for (i = 0; i <= NROOTS; i++) {
					b[i] = (lambda[i] == 0) ? A0 : MODNN(INDEX_OF[lambda[i]] - discr_r + NN);
				}
			} else {
				/* 2 lines below: B(x) <-- x*B(x) */
				memmove(&b[1], b, NROOTS * sizeof(b[0]));
				b[0] = A0;
			}
			memcpy(lambda, t, (NROOTS + 1) * sizeof(t[0]));
		}
	}

	/* Convert lambda to index form and compute deg(lambda(x)) */
	deg_lambda = 0;
	for (i = 0; i < NROOTS + 1; i++) {
		lambda[i] = INDEX_OF[lambda[i]];
		if (lambda[i] != A0) {
			deg_lambda = i;
		}
	}
	/* Find roots of the error+erasure locator polynomial by Chien search */
	memcpy(&reg[1], &lambda[1], NROOTS * sizeof(reg[0]));
	count = 0;		/* Number of roots of lambda(x) */
	for (i = 1, k = IPRIM - 1; i <= NN; i++, k = MODNN(k + IPRIM)) {
		q = 1; /* lambda[0] is always 0 */
		for (j = deg_lambda; j > 0; j--) {
			if (reg[j] != A0) {
				reg[j] = MODNN(reg[j] + j);
				q ^= ALPHA_TO[reg[j]];
			}
		}
		if (q != 0) {
			continue; /* Not a root */
		}
		/* store root (index-form) and error location number */
#if DEBUG>=2
		printf("count %d root %d loc %d\n", count, i, k);
#endif
		root[count] = i;
		loc[count] = k;
		/* If we've already found max possible roots,
		 * abort the search to save time
		 */
		if (++count == deg_lambda) {
			break;
		}
	}
	if (deg_lambda != count) {
		/*
		 * deg(lambda) unequal to number of roots => uncorrectable
		 * error detected
		 */
#if DEBUG>=1
		printf("count %d not equal to degree of lambda %d\n", count, deg_lambda);
#endif
		count = -1;
		goto finish;
	}
	/*
	 * Compute err+eras evaluator poly omega(x) = s(x)*lambda(x) (modulo
	 * x**NROOTS). in index form. Also find deg(omega).
	 */
	deg_omega = deg_lambda - 1;
	for (i = 0; i <= deg_omega; i++) {
		tmp = 0;
		for (j = i; j >= 0; j--) {
			if ((s[i - j] != A0) && (lambda[j] != A0)) {
				tmp ^= ALPHA_TO[MODNN(s[i - j] + lambda[j])];
			}
		}
		omega[i] = INDEX_OF[tmp];
	}

	/*
	 * Compute error values in poly-form. num1 = omega(inv(X(l))), num2 =
	 * inv(X(l))**(FCR-1) and den = lambda_pr(inv(X(l))) all in poly-form
	 */
	for (j = count - 1; j >=0; j--) {
		num1 = 0;
		for (i = deg_omega; i >= 0; i--) {
			if (omega[i] != A0) {
				num1  ^= ALPHA_TO[MODNN(omega[i] + i * root[j])];
			}
		}
		num2 = ALPHA_TO[MODNN(root[j] * (FCR - 1) + NN)];
		den = 0;

		/* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */
		for (i = MIN(deg_lambda, NROOTS - 1) & ~1; i >= 0; i -= 2) {
			if(lambda[i + 1] != A0) {
				den ^= ALPHA_TO[MODNN(lambda[i+1] + i * root[j])];
			}
		}
#if DEBUG >= 1
		if (den == 0) {
			printf("\n ERROR: denominator = 0\n");
			count = -1;
			goto finish;
		}
#endif
		/* Apply error to data */
		if (num1 != 0 && loc[j] >= PAD) {
			data[loc[j] - PAD] ^= ALPHA_TO[MODNN(INDEX_OF[num1] + INDEX_OF[num2] + NN - INDEX_OF[den])];
		}
	}
finish:
	if (eras_pos != NULL) {
		for(i = 0; i < count; i++) {
			eras_pos[i] = loc[i];
		}
	}
	retval = count;
	return retval;
}
