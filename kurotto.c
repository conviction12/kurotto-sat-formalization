#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kurotto.h"

//Variable ID bases

static int BASE_X;
static int BASE_LEV;
static int BASE_REACH;
static int BASE_COUNT;
static int TOTAL;

void init_var_ids(int width, int height, int num_numbered_circles) {
    int WH = width * height;
    int NC = num_numbered_circles;

    BASE_X     = 1;
    BASE_LEV   = BASE_X     + WH;
    BASE_REACH = BASE_LEV   + NC * WH * (D_MAX + 1);
    BASE_COUNT = BASE_REACH + NC * WH;
    TOTAL      = BASE_COUNT + NC * (WH + 1) * (S_MAX + 1) - 1;
    if (NC == 0) TOTAL = BASE_REACH - 1;  /* no circle variables needed */
}

int total_vars(void) { return TOTAL; }

int get_x_id(int i, int j, int width) {
    return BASE_X + i * width + j;
}

int get_lev_id(int c, int i, int j, int d, int width, int height) {
    int WH = width * height;
    return BASE_LEV + c * WH * (D_MAX + 1)+ (i * width + j) * (D_MAX + 1)+ d;
}

int get_reach_id(int c, int i, int j, int width, int height) {
    int WH = width * height;
    return BASE_REACH + c * WH + i * width + j;
}

int get_count_id(int c, int p, int s, int width, int height) {
    int WH = width * height;
    return BASE_COUNT + c * (WH + 1) * (S_MAX + 1) + p * (S_MAX + 1) + s;
}


//Rule: a circle cell is WHITE
int rule_for_circle(FILE *f, int i, int j, int width) {
    fprintf(f, "-%d 0\n", get_x_id(i, j, width));
    return 1;
}


static const int DR[4] = {-1,  1,  0,  0};
static const int DC[4] = { 0,  0, -1,  1};


int encode_numbered_circle(FILE *f, int ci, int cj, int k, int c, int width, int height) {
    int cnt = 0;
    int WH  = width * height;


    int nb_i[4], nb_j[4], n_nb = 0;
    for (int d = 0; d < 4; d++) {
        int ni = ci + DR[d], nj = cj + DC[d];
        if (ni >= 0 && ni < height && nj >= 0 && nj < width) {
            nb_i[n_nb] = ni; nb_j[n_nb] = nj; n_nb++;
        }
    }

    /* ==========================================================
     *  LEV and REACH encoding
     *  ----------------------
     *  (A1) LEV_{c,i,j,d}  ->  X_{i,j}               (black req'd)
     *  (A2a) if (i,j) is a neighbour of the circle:
     *            X_{i,j}  ->  LEV_{c,i,j,0}
     *  (A2b) if (i,j) is NOT a neighbour of the circle:
     *            -LEV_{c,i,j,0}
     *  (A3)  d >= 1:  LEV_d  ->  OR_{nbr} LEV_{nbr, d-1}
     *  (A4)  d >= 1:  X_{i,j} AND X_{nbr} AND LEV_{nbr,d-1}
     *                      -> LEV_{i,j,d}       (forward propagation)
     *  (R1)  LEV_d  ->  REACH
     *  (R2)  REACH  ->  OR_d LEV_d
     * ========================================================== */

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int xid = get_x_id(i, j, width);
            int rid = get_reach_id(c, i, j, width, height);

            int is_nbr = 0;
            for (int t = 0; t < n_nb; t++)
                if (nb_i[t] == i && nb_j[t] == j) { is_nbr = 1; break; }

            /* (A1) LEV_d -> X  for every d */
            for (int d = 0; d <= D_MAX; d++) {
                int lid = get_lev_id(c, i, j, d, width, height);
                fprintf(f, "-%d %d 0\n", lid, xid);
                cnt++;
            }

            /* (A2) base case d = 0 */
            int l0 = get_lev_id(c, i, j, 0, width, height);
            if (is_nbr) {
                fprintf(f, "-%d %d 0\n", xid, l0);   /* X -> LEV_0 */
                cnt++;
            } else {
                fprintf(f, "-%d 0\n", l0);            /* LEV_0 false */
                cnt++;
            }

            /* (A3) d >= 1 : LEV_d -> OR_{nbr} LEV_{nbr,d-1} */
            for (int d = 1; d <= D_MAX; d++) {
                int ld = get_lev_id(c, i, j, d, width, height);
                fprintf(f, "-%d", ld);
                for (int e = 0; e < 4; e++) {
                    int ni = i + DR[e], nj = j + DC[e];
                    if (ni < 0 || ni >= height || nj < 0 || nj >= width) continue;
                    fprintf(f, " %d", get_lev_id(c, ni, nj, d-1, width, height));
                }
                fprintf(f, " 0\n");
                cnt++;
            }

            /* (A4) forward propagation */
            for (int d = 1; d <= D_MAX; d++) {
                int ld = get_lev_id(c, i, j, d, width, height);
                for (int e = 0; e < 4; e++) {
                    int ni = i + DR[e], nj = j + DC[e];
                    if (ni < 0 || ni >= height || nj < 0 || nj >= width) continue;
                    int xnbr = get_x_id(ni, nj, width);
                    int lprev = get_lev_id(c, ni, nj, d-1, width, height);
                    fprintf(f, "-%d -%d -%d %d 0\n", xid, xnbr, lprev, ld);
                    cnt++;
                }
            }

            /* (R1) LEV_d -> REACH */
            for (int d = 0; d <= D_MAX; d++) {
                int ld = get_lev_id(c, i, j, d, width, height);
                fprintf(f, "-%d %d 0\n", ld, rid);
                cnt++;
            }

            /* (R2) REACH -> OR_d LEV_d */
            fprintf(f, "-%d", rid);
            for (int d = 0; d <= D_MAX; d++) {
                fprintf(f, " %d", get_lev_id(c, i, j, d, width, height));
            }
            fprintf(f, " 0\n");
            cnt++;
        }
    }

    /* ==========================================================
     *  Sequential counter for sum_{(i,j)} REACH == k
     *  --------------------------------------------
     *  COUNT_{p,s} <=> after first p cells, exactly s have REACH=1.
     *  s in [0, S_MAX];  S_MAX is a saturating slot meaning ">= S_MAX".
     *
     *  Transition (A = REACH of current cell):
     *     COUNT_{p,s} <-> ( A AND COUNT_{p-1,s-1} ) OR
     *                     ( -A AND COUNT_{p-1,s} )
     *  with saturation at s = S_MAX.
     * ========================================================== */

    /* initial state: sum = 0 after 0 cells */
    fprintf(f, "%d 0\n", get_count_id(c, 0, 0, width, height));
    cnt++;
    for (int s = 1; s <= S_MAX; s++) {
        fprintf(f, "-%d 0\n", get_count_id(c, 0, s, width, height));
        cnt++;
    }

    /* transitions, p = 1..WH */
    for (int p = 1; p <= WH; p++) {
        int idx = p - 1;
        int ri = idx / width;
        int rj = idx % width;
        int A = get_reach_id(c, ri, rj, width, height);

        for (int s = 0; s <= S_MAX; s++) {
            int Dv = get_count_id(c, p, s, width, height);

            if (s == 0) {
                /* COUNT_{p,0} <-> -A AND COUNT_{p-1,0} */
                int Cv = get_count_id(c, p-1, 0, width, height);
                /* -A AND C -> D  :  A OR -C OR D */
                fprintf(f, "%d -%d %d 0\n", A, Cv, Dv);  cnt++;
                /* D -> -A        :  -D OR -A */
                fprintf(f, "-%d -%d 0\n", Dv, A);        cnt++;
                /* D -> C         :  -D OR  C */
                fprintf(f, "-%d %d 0\n", Dv, Cv);        cnt++;
            } else if (s < S_MAX) {
                int Bv = get_count_id(c, p-1, s-1, width, height);
                int Cv = get_count_id(c, p-1, s,   width, height);
                /*  A AND B -> D   :  -A OR -B OR D */
                fprintf(f, "-%d -%d %d 0\n", A, Bv, Dv); cnt++;
                /* -A AND C -> D   :   A OR -C OR D */
                fprintf(f, "%d -%d %d 0\n", A, Cv, Dv);  cnt++;
                /*  D AND A -> B   :  -D OR -A OR B */
                fprintf(f, "-%d -%d %d 0\n", Dv, A, Bv); cnt++;
                /*  D AND -A -> C  :  -D OR  A OR C */
                fprintf(f, "-%d %d %d 0\n", Dv, A, Cv);  cnt++;
            } else {
                /* s == S_MAX, saturating:
                 *   COUNT_{p,S} <-> COUNT_{p-1,S} OR (A AND COUNT_{p-1,S-1})
                 */
                int Bv = get_count_id(c, p-1, S_MAX - 1, width, height);
                int Cv = get_count_id(c, p-1, S_MAX,     width, height);
                /* C -> D */
                fprintf(f, "-%d %d 0\n", Cv, Dv);         cnt++;
                /* A AND B -> D */
                fprintf(f, "-%d -%d %d 0\n", A, Bv, Dv);  cnt++;
                /* D -> C OR A */
                fprintf(f, "-%d %d %d 0\n", Dv, Cv, A);   cnt++;
                /* D -> C OR B */
                fprintf(f, "-%d %d %d 0\n", Dv, Cv, Bv);  cnt++;
            }
        }
    }

    /* final: sum must be exactly k */
    fprintf(f, "%d 0\n", get_count_id(c, WH, k, width, height));
    cnt++;

    return cnt;
}
