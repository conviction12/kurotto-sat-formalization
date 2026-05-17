#ifndef KUROTTO_H
#define KUROTTO_H

#include <stdio.h>
#include <stdint.h>

typedef int32_t i32;
typedef uint32_t ui32;
typedef int8_t i8;
typedef uint8_t ui8;

typedef struct {
    i8 number;
    ui8 is_circle;
} Cell;

/* Maximum value of a circle number (i.e. maximum total black neighbours sum). */
#define MAX_VAL 10

/* Maximum distance from a circle-neighbour inside a component (component size - 1). */
#define D_MAX (MAX_VAL - 1)

/* Saturating upper bound for the prefix-sum counter (sum cannot exceed this). */
#define S_MAX (MAX_VAL + 1)


/* ---------------------------------------------------------------------
 *  Variable ID layout
 *
 *   BASE_X     :  X_{i,j}                     -> WH vars
 *   BASE_LEV   :  LEV_{c,i,j,d}               -> NC * WH * (D_MAX+1) vars
 *   BASE_REACH :  REACH_{c,i,j}               -> NC * WH vars
 *   BASE_COUNT :  COUNT_{c,p,s}               -> NC * (WH+1) * (S_MAX+1) vars
 *
 *   WH = width * height, NC = number of *numbered* circles.
 *
 *   init_var_ids() must be called once, AFTER counting numbered circles
 *   and BEFORE any get_*_id() call.
 * ------------------------------------------------------------------ */

void init_var_ids(int width, int height, int num_numbered_circles);
int  total_vars(void);   /* largest allocated variable id */


/* X_{i,j} = true  <=>  cell (i,j) is BLACK. */
int get_x_id(int i, int j, int width);

/* LEV_{c,i,j,d} = true  <=>  there is a walk of length d in the black
 * subgraph from some black neighbour of circle c to cell (i,j).
 * In particular LEV_d => X_{i,j}. */
int get_lev_id(int c, int i, int j, int d, int width, int height);

/* REACH_{c,i,j} = true  <=>  cell (i,j) is in the same connected black
 * component as at least one black neighbour of circle c. */
int get_reach_id(int c, int i, int j, int width, int height);

/* COUNT_{c,p,s} = true  <=>  after scanning the first p cells in
 * row-major order, exactly s of them had REACH_{c,.,.} = true.
 * Value s saturates at S_MAX (meaning "sum >= S_MAX"). */
int get_count_id(int c, int p, int s, int width, int height);


/* ---------------------------------------------------------------------
 *  Clause emitters. Each returns the number of clauses written.
 * ------------------------------------------------------------------ */

/* Circle cells are always white. Call for every is_circle==1 cell,
 * numbered or not. */
int rule_for_circle(FILE *f, int i, int j, int width);

/* Emit the full "sum of touching black-group sizes == k" constraint for
 * a numbered circle at (ci, cj) with number k. `c` is the 0-indexed
 * id of this circle among numbered circles. */
int encode_numbered_circle(FILE *f, int ci, int cj, int k, int c, int width, int height);

#endif
