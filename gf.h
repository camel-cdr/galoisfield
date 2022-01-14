#ifndef GF_H_INCLUDED

#include <inttypes.h>
#include <assert.h>
#include <stddef.h>

int gf_is_prime(size_t n);
int gf_factor(size_t n, size_t *mod, size_t *power);

/* Based on: https://lemire.me/blog/2019/02/08/faster-remainders-when-the-
 *           divisor-is-a-constant-beating-compilers-and-libdivide/ */
typedef struct {
	uint32_t c;
	uint16_t mod; /* is assumed to be prime */
} GfMod;

GfMod gf_mod_create(uint16_t mod);
uint16_t gf_mod(GfMod mod, uint16_t x);
int gf_mod_can_divide(GfMod mod, uint16_t x);
uint16_t gf_mod_neg(GfMod mod, uint16_t x);
void gf_gen_div_tbl(uint16_t *output, GfMod mod);

typedef struct {
	size_t len, cap;
	uint16_t *at;
} GfPoly;

void gf_poly_setlen(GfPoly *poly, size_t len);
void gf_poly_free(GfPoly *poly);
void gf_poly_init(GfPoly *res, size_t len, ...);
void gf_poly_copy(GfPoly *dest, GfPoly src);
void gf_poly_shrink(GfPoly *poly);
void gf_poly_shrink_mod(GfPoly *poly, GfMod mod);
void gf_poly_mod(GfPoly *poly, GfMod mod);
void gf_poly_neg(GfPoly *poly, GfMod mod);
void gf_poly_add(GfPoly *res, GfPoly poly1, GfPoly poly2);
void gf_poly_mul_full(GfPoly *res, GfPoly p1, GfPoly p2);
void gf_poly_mul(GfPoly *res, GfPoly p1, GfPoly p2, GfPoly polyMod, GfMod mod, uint16_t *div_tbl);
size_t gf_poly_to_index(GfPoly poly, GfMod mod);
void gf_poly_from_index(GfPoly *res, size_t index, GfMod mod);

typedef struct {
	size_t n, power;
	GfMod mod;
	GfPoly irreducible;
	GfPoly tmp1, tmp2, tmp3;
	uint16_t *div_tbl;
} GField;

typedef enum {
	GF_INIT_SUCCESS,
	GF_INIT_DOESNT_EXIST,
	GF_INIT_MOD_TO_BIG,
	GF_INIT_IRREDUCIBLE_INVALID
} GFieldInitStatus;


GFieldInitStatus gfield_init(GField *f, size_t n, GfPoly *irreducible);
void gfield_free(GField *f);

size_t gfield_add(GField *f, size_t i, size_t j);
size_t gfield_sub(GField *f, size_t i, size_t j);
size_t gfield_mul(GField *f, size_t i, size_t j);
size_t gfield_div(GField *f, size_t i, size_t j);

#define GF_H_INCLUDED
#endif

#ifdef GF_IMPLEMENTATION

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
gf_is_prime(size_t n)
{
	size_t i, end = sqrt(n);
	for (i = 2; i < end; ++i)
		if (n % i == 0)
			return 0;
	return 1;
}

int
gf_factor(size_t n, size_t *mod, size_t *power)
{
	size_t m, p, end = sqrt(n) + 1;

	if (n < 2)
		return 0;

	for (m = 2, p = 0; m < end; ++m) {
		if (n % m == 0) {
			do {
				n /= m;
				++p;
			} while (n % m == 0);
			break;
		}
	}

	/* n must be prime */
	if (m == end) {
		*mod = n;
		*power = 1;
		return 1;
	}

	if (!gf_is_prime(m) || (p != 0 && n != 1))
		return 0;

	*mod = m;
	*power = p;
	return 1;
}

/******************************************************************************/

GfMod
gf_mod_create(uint16_t mod)
{
	GfMod res;
#ifndef NDEBUG
	assert(gf_is_prime(mod));
#endif
	res.mod = mod;
	res.c = ~UINT32_C(0) / mod + 1;
	return res;
}

uint16_t
gf_mod(GfMod mod, uint16_t x)
{
	return ((uint64_t)(x * mod.c) * mod.mod) >> 32;
}
int
gf_mod_can_divide(GfMod mod, uint16_t x)
{
	return x * mod.c <= mod.c - 1;
}
uint16_t
gf_mod_neg(GfMod mod, uint16_t x)
{
	return mod.mod - gf_mod(mod, x);
}

/******************************************************************************/

/* output: should have mod.mod elements
 * mod: mod.mod should be >= 2 */
void
gf_gen_div_tbl(uint16_t *output, GfMod mod)
{
	size_t i;
	output[0] = output[1] = 1;
	for (i = 2; i < mod.mod; ++i)
		output[i] = output[mod.mod % i] *
		            gf_mod(mod, mod.mod - mod.mod / i);
}

/******************************************************************************/

void
gf_poly_setlen(GfPoly *poly, size_t len)
{
	poly->len = len;
	if (poly->cap < len) {
		poly->cap = len;
		poly->at = realloc(poly->at, poly->cap * sizeof *poly->at);
	}
}

void
gf_poly_free(GfPoly *poly)
{
	free(poly->at);
	poly->at = 0;
	poly->len = poly->cap = 0;
}

void
gf_poly_init(GfPoly *res, size_t len, ...)
{
	va_list args;
	va_start(args, len);
	gf_poly_setlen(res, len);
	while (len--)
		res->at[len] = va_arg(args, int);
	va_end(args);
}

void
gf_poly_copy(GfPoly *dest, GfPoly src)
{
	gf_poly_setlen(dest, src.len);
	memcpy(dest->at, src.at, src.len * sizeof *src.at);
}

void
gf_poly_shrink(GfPoly *poly)
{
	while (poly->len && poly->at[poly->len-1] == 0)
		--poly->len;
}

void
gf_poly_shrink_mod(GfPoly *poly, GfMod mod)
{
	while (poly->len && gf_mod_can_divide(mod, poly->at[poly->len-1]))
		--poly->len;
}

void
gf_poly_mod(GfPoly *poly, GfMod mod)
{
	size_t i;
	for (i = 0; i < poly->len; ++i)
		poly->at[i] = gf_mod(mod, poly->at[i]);
	gf_poly_shrink(poly);
}

void
gf_poly_neg(GfPoly *poly, GfMod mod)
{
	size_t i;
	for (i = 0; i < poly->len; ++i)
		poly->at[i] = gf_mod_neg(mod, poly->at[i]);
	gf_poly_shrink(poly);
}


void
gf_poly_add(GfPoly *res, GfPoly poly1, GfPoly poly2)
{
	size_t i;
	size_t len1, len2;
	uint16_t *r, *p1, *p2;

	gf_poly_setlen(res, poly1.len > poly2.len ? poly1.len : poly2.len);
	r = res->at;

	if (poly1.len < poly2.len) {
		len1 = poly1.len, p1 = poly1.at;
		len2 = poly2.len, p2 = poly2.at;
	} else {
		len1 = poly2.len, p1 = poly2.at;
		len2 = poly1.len, p2 = poly1.at;
	}

	for (i = 0; i < len1; ++i)
		*r++ = *p1++ + *p2++;
	for (; i < len2; ++i)
		*r++ = *p2++;

	gf_poly_shrink(res);
}

void
gf_poly_mul_full(GfPoly *res, GfPoly p1, GfPoly p2)
{
	size_t i, j;
	gf_poly_setlen(res, p1.len + p2.len);
	memset(res->at, 0, res->len * sizeof *res->at);

	for (i = 0; i < p1.len; ++i)
		for (j = 0; j < p2.len; ++j)
			res->at[i + j] += p1.at[i] * p2.at[j];
}


void
gf_poly_mul(GfPoly *res, GfPoly p1, GfPoly p2, GfPoly polyMod, GfMod mod, uint16_t *div_tbl)
{
	gf_poly_mul_full(res, p1, p2);
	gf_poly_shrink_mod(res, mod);

	/* division using the school method */
	while (res->len >= polyMod.len) {
		size_t i, j;
		const uint16_t mul = res->at[res->len-1] *
		                     div_tbl[gf_mod(mod,polyMod.at[polyMod.len - 1])];

		for (i = res->len, j = polyMod.len; i--, j--;)
			res->at[i] = res->at[i] + gf_mod_neg(mod, polyMod.at[j] * mul);

		/* the degree will always be reduced by at least one */
		--res->len;
		gf_poly_shrink_mod(res, mod);
	}
}


size_t
gf_poly_to_index(GfPoly poly, GfMod mod)
{
	size_t i, res = 0, power = 1;
	for (i = 0; i < poly.len; i++) {
		res += power * gf_mod(mod, poly.at[i]);
		power *= mod.mod;
	}
	return res;
}

void
gf_poly_from_index(GfPoly *res, size_t index, GfMod mod)
{
	size_t i;
	gf_poly_setlen(res, log(index + 1) / log(mod.mod) + 1);
	for (i = 0; i < res->len; i++) {
		res->at[i] = gf_mod(mod, index);
		index /= mod.mod;
	}
	gf_poly_shrink(res);
}


GFieldInitStatus
gfield_init(GField *f, size_t n, GfPoly *irreducible)
{
	size_t mod = 0, power = 0;

	if (!gf_factor(n, &mod, &power))
		return GF_INIT_DOESNT_EXIST;

	if (mod > UINT16_MAX)
		return GF_INIT_MOD_TO_BIG;

	f->n = n;
	f->power = power;
	f->mod = gf_mod_create(mod);

	if (!irreducible) {
		return GF_INIT_IRREDUCIBLE_INVALID;
	}
	gf_poly_mod(irreducible, f->mod);
	if (irreducible->len == 0) {
		return GF_INIT_IRREDUCIBLE_INVALID;
	}
	gf_poly_copy(&f->irreducible, *irreducible);
	gf_poly_mod(&f->irreducible, f->mod);

	f->div_tbl = realloc(f->div_tbl, mod * sizeof *f->div_tbl);
	gf_gen_div_tbl(f->div_tbl, f->mod);

	return GF_INIT_SUCCESS;
}

void
gfield_free(GField *f)
{
	gf_poly_free(&f->irreducible);
	gf_poly_free(&f->tmp1);
	gf_poly_free(&f->tmp2);
	gf_poly_free(&f->tmp3);
	free(f->div_tbl);
}

size_t
gfield_add(GField *f, size_t i, size_t j)
{
	i %= f->n; j %= f->n;
	gf_poly_from_index(&f->tmp1, i, f->mod);
	gf_poly_from_index(&f->tmp2, j, f->mod);
	gf_poly_add(&f->tmp3, f->tmp1, f->tmp2);
	gf_poly_mod(&f->tmp3, f->mod);
	return gf_poly_to_index(f->tmp3, f->mod);
}

size_t
gfield_sub(GField *f, size_t i, size_t j)
{
	i %= f->n; j %= f->n;
	gf_poly_from_index(&f->tmp1, i, f->mod);
	gf_poly_from_index(&f->tmp2, j, f->mod);
	gf_poly_neg(&f->tmp2, f->mod);
	gf_poly_add(&f->tmp3, f->tmp1, f->tmp2);
	gf_poly_mod(&f->tmp3, f->mod);
	return gf_poly_to_index(f->tmp3, f->mod);
}

size_t
gfield_mul(GField *f, size_t i, size_t j)
{
	i %= f->n; j %= f->n;
	gf_poly_from_index(&f->tmp1, i, f->mod);
	gf_poly_from_index(&f->tmp2, j, f->mod);
	gf_poly_mul(&f->tmp3, f->tmp1, f->tmp2, f->irreducible, f->mod, f->div_tbl);
	gf_poly_mod(&f->tmp3, f->mod);
	return gf_poly_to_index(f->tmp3, f->mod);
}

size_t
gfield_div(GField *f, size_t i, size_t j)
{
	size_t k;
	i %= f->n; j %= f->n;
	gf_poly_from_index(&f->tmp1, j, f->mod);
	for (k = 0; k < f->n; ++k) {
		size_t res;
		gf_poly_from_index(&f->tmp2, k, f->mod);
		gf_poly_mul(&f->tmp3, f->tmp1, f->tmp2, f->irreducible, f->mod, f->div_tbl);
		gf_poly_mod(&f->tmp3, f->mod);
		res = gf_poly_to_index(f->tmp3, f->mod);
		if (res == i)
			return k;
	}
	assert(0);
	return 0;
}

#undef GF_IMPLEMENTATION
#endif
