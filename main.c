#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kurotto.h"


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <puzzle-file>\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) { perror(argv[1]); return 1; }

    int width, height;
    if (fscanf(input, "%d %d", &width, &height) != 2) {
        fprintf(stderr, "bad header\n"); fclose(input); return 1;
    }

    /* Load the grid */
    Cell **grid = malloc(height * sizeof(Cell *));
    for (int i = 0; i < height; i++) {
        grid[i] = malloc(width * sizeof(Cell));
        for (int j = 0; j < width; j++) {
            char value[16];
            if (fscanf(input, "%15s", value) != 1) {
                fprintf(stderr, "bad cell at (%d,%d)\n", i, j);
                fclose(input); return 1;
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
    fclose(input);

    /* --- First pass: count numbered circles -------------------- */
    int num_numbered = 0;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            if (grid[i][j].is_circle && grid[i][j].number != -1)
                num_numbered++;

    init_var_ids(width, height, num_numbered);

    /* --- Open output CNF file ---------------------------------- */
    char output_name[512];
    snprintf(output_name, sizeof(output_name), "%s.cnf", argv[1]);
    FILE *output = fopen(output_name, "w");
    if (!output) { perror(output_name); return 1; }

    fprintf(output, "p cnf %10d %10d\n", total_vars(), 0);

    int clause_count = 0;
    int circle_idx   = 0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j].is_circle) {
                /* every circle cell (numbered or not) is white */
                clause_count += rule_for_circle(output, i, j, width);
                if (grid[i][j].number != -1) {
                    clause_count += encode_numbered_circle(
                        output, i, j, grid[i][j].number, circle_idx,
                        width, height);
                    circle_idx++;
                }
            }

        }
    }

    /* Rewind to the header and overwrite it with the real counts */
    fseek(output, 0, SEEK_SET);
    fprintf(output, "p cnf %10d %10d\n", total_vars(), clause_count);

    fclose(output);

    printf("CNF written to %s\n", output_name);
    printf("  variables : %d\n", total_vars());
    printf("  clauses   : %d\n", clause_count);
    printf("  numbered circles : %d\n", circle_idx);

    for (int i = 0; i < height; i++) free(grid[i]);
    free(grid);

    return 0;
}
