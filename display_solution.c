#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kurotto.h"

/* Read the puzzle file into a grid, exactly as main.c does. */
static Cell **load_puzzle(const char *path, int *width, int *height) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return NULL; }

    if (fscanf(f, "%d %d", width, height) != 2) {
        fprintf(stderr, "bad header in %s\n", path);
        fclose(f);
        return NULL;
    }

    Cell **grid = malloc(*height * sizeof(Cell *));
    for (int i = 0; i < *height; i++) {
        grid[i] = malloc(*width * sizeof(Cell));
        for (int j = 0; j < *width; j++) {
            char value[16];
            if (fscanf(f, "%15s", value) != 1) {
                fprintf(stderr, "bad cell at (%d,%d)\n", i, j);
                fclose(f);
                return NULL;
            }
            if (strcmp(value, ".") == 0) {
                grid[i][j].is_circle = 0;
                grid[i][j].number    = -1;
            } else if (strcmp(value, "o") == 0) {
                grid[i][j].is_circle = 1;
                grid[i][j].number    = -1;
            } else {
                grid[i][j].is_circle = 1;
                grid[i][j].number    = (i8)atoi(value);
            }
        }
    }
    fclose(f);
    return grid;
}

/* Read the model file.  Returns:
 *    1  -> SAT and *values populated (index 1..num_vars, 1=true, 0=false)
 *    0  -> UNSAT
 *   -1  -> parse error
 * The caller passes num_vars so we size the array correctly.          */
static int read_model(const char *path, int num_vars, char *values) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return -1; }

    char tok[32];
    if (fscanf(f, "%31s", tok) != 1) { fclose(f); return -1; }

    if (strcmp(tok, "UNSAT") == 0 || strcmp(tok, "UNSATISFIABLE") == 0) {
        fclose(f);
        return 0;
    }
    if (strcmp(tok, "SAT") != 0 && strcmp(tok, "SATISFIABLE") != 0) {
        /* Some solvers prefix with "s " or "v " lines; fall back to
         * a tolerant scan: re-open and treat all integer tokens as
         * literals, UNSAT if none appear.                              */
        fclose(f);
        f = fopen(path, "r");
        if (!f) return -1;
    }

    for (int i = 0; i <= num_vars; i++) values[i] = 0;

    int lit;
    while (fscanf(f, "%d", &lit) == 1) {
        if (lit == 0) break;
        int v = lit < 0 ? -lit : lit;
        if (v >= 1 && v <= num_vars) values[v] = (lit > 0) ? 1 : 0;
    }
    fclose(f);
    return 1;
}

/* Every cell takes 4 columns so the board stays aligned whatever symbol is inside.                       */
static void print_solution(Cell **grid, int width, int height,
                           const char *values) {
    /* Top border */
    printf("+");
    for (int j = 0; j < width; j++) printf("----+");
    printf("\n");

    for (int i = 0; i < height; i++) {
        printf("|");
        for (int j = 0; j < width; j++) {
            int xid = get_x_id(i, j, width);
            int is_black = values[xid];

            if (grid[i][j].is_circle) {
                if (grid[i][j].number >= 0)
                    printf("(%2d)", grid[i][j].number);
                else
                    printf(" o  ");
            } else if (is_black) {
                printf("####");
            } else {
                printf(" .  ");
            }
            printf("|");
        }
        printf("\n+");
        for (int j = 0; j < width; j++) printf("----+");
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <puzzle-file> <solution-file>\n", argv[0]);
        return 1;
    }

    int width, height;
    Cell **grid = load_puzzle(argv[1], &width, &height);
    if (!grid) return 1;

    int num_numbered = 0;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            if (grid[i][j].is_circle && grid[i][j].number != -1)
                num_numbered++;

    init_var_ids(width, height, num_numbered);
    int nv = total_vars();

    char *values = calloc(nv + 1, sizeof(char));
    if (!values) { fprintf(stderr, "out of memory\n"); return 1; }

    int r = read_model(argv[2], nv, values);
    if (r < 0) {
        fprintf(stderr, "could not parse solution file\n");
        return 1;
    }
    if (r == 0) {
        printf("UNSATISFIABLE - the puzzle has no solution.\n");
        return 0;
    }

    printf("SATISFIABLE - solved grid:\n\n");
    print_solution(grid, width, height, values);

    printf("\nCompact form (# = black, . = white, o/N = circle):\n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j].is_circle) {
                if (grid[i][j].number >= 0) printf("%d ", grid[i][j].number);
                else                        printf("o ");
            } else {
                int xid = get_x_id(i, j, width);
                printf("%c ", values[xid] ? '#' : '.');
            }
        }
        printf("\n");
    }

    for (int i = 0; i < height; i++) free(grid[i]);
    free(grid);
    free(values);
    return 0;
}
