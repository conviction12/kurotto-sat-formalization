#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include <stdint.h>
typedef int32_t i32;
typedef uint32_t ui32;
typedef int8_t i8;
typedef uint8_t ui8;


typedef struct {
    i32 *literals; // Array of literals in the clause
    ui32 num_literals; // Number of literals in the clause
} Clause;

typedef struct {
    Clause *clauses; // Array of clauses in the formula
    ui32 num_clauses; // Number of clauses in the formula
    ui32 num_variables; // Number of variables in the formula
    i8  *assignments; // Array of variable assignments (0 for false, 1 for true, -1 for unassigned)
} Formula;

void create_formula(Formula *F);

void read_cnf_file(FILE *file, Formula *F);

void display_formula(Formula *F);

i8 check_formula_status(Clause *c, i8 *assignments);

int dpll(Formula *F);

#endif // SAT_SOLVER_H
