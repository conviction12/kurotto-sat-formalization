#include <stdio.h>
#include <stdlib.h>
#include "sat_solver.h"
#include <math.h>
#define MAX_LEN  100000

void create_formula(Formula *F){
    F->clauses = NULL;
    F->assignments = NULL;
    F->num_clauses = 0;
    F->num_variables = 0;
}

void read_cnf_file(FILE *file, Formula *F) {
    if(!file) return;
    char buffer[MAX_LEN];

    while( fgets(buffer, sizeof(buffer), file)){
        if(buffer[0] == 'c') continue;
        if(buffer[0] == 'p'){
            sscanf(buffer, "p cnf %d %d", &F->num_variables, &F->num_clauses);
            break;
        }
    }
    F->clauses = (Clause *)malloc(F->num_clauses * sizeof(Clause));
    F->assignments = (i8 *)calloc(F->num_variables + 1, sizeof(i8));
    i32 lit;
    for( ui32 i = 0; i< F->num_clauses; ++i){
        ui32 temp_capacity = 4;
        F->clauses[i].literals = (i32*)malloc(temp_capacity * sizeof(i32));
        F->clauses[i].num_literals = 0;

        while(fscanf(file, "%d", &lit) == 1 && lit != 0){
            if( F->clauses[i].num_literals >= temp_capacity){
                temp_capacity *=2;
                F->clauses[i].literals = realloc(F->clauses[i].literals, temp_capacity *sizeof(i32) );
            }
            F->clauses[i].literals[F->clauses[i].num_literals] = lit;
            F->clauses[i].num_literals++;
        }
        if (F->clauses[i].num_literals > 0) {
            F->clauses[i].literals = realloc(F->clauses[i].literals, F->clauses[i].num_literals * sizeof(i32));

        }
    }
}

void display_formula(Formula *F) {
    if (F == NULL || F->clauses == NULL) {
        fprintf(stderr, "Formula is empty or not initialized.\n");
        return;
    }

    fprintf(stderr, "--- Show DIMACS Formula ---\n");
    fprintf(stderr, "Variables: %u | Clause: %u\n", F->num_variables, F->num_clauses);
    fprintf(stderr, "------------------------------\n");

    for (ui32 i = 0; i < F->num_clauses; i++) {
        fprintf(stderr, "Clause %u: ", i + 1);

        for (ui32 j = 0; j < F->clauses[i].num_literals; j++) {

            fprintf(stderr, "%d ", F->clauses[i].literals[j]);
        }

        fprintf(stderr, "0\n");
    }
    fprintf(stderr, "------------------------------\n");
}

i8 check_formula_status(Clause *c, i8 *assignments){
    ui32 false_counter = 0;
    ui32 unassigned_counter = 0;

    for( ui32 i = 0; i < c->num_literals; ++i){
        i32 lit = c->literals[i];
        i32 var = abs(lit);
        i8 assign = assignments[var];

        if (assign == 0){
            unassigned_counter++;
            continue;
        }

        if ((lit > 0 && assign == 1) || (lit < 0 && assign == -1)) return 1;


        false_counter ++;

    }

        if(  false_counter ==  c->num_literals) return -1;
        if (unassigned_counter == 1) return 0;
        // Here u might have a question, it is like this because if all are false, then there is a conflict,
        //where as, if one is missing then is a unitary clause

        return 2;//This is for the case when there are more unassigned literals

}

int dpll(Formula *F){
        /*Inspiration from the Wikipedia algorithm
https://en.wikipedia.org/wiki/DPLL_algorithm*/
   // 1. UNIT PROPAGATION
   for(ui32 i = 0; i < F->num_clauses; ++i){
        if(check_formula_status(&F->clauses[i], F->assignments) ==  0){
            for(ui32 j = 0; j < F -> clauses[i].num_literals; ++j ){
                i32 lit = F->clauses[i].literals[j];
                if(F->assignments[abs(lit)] == 0){
                    F->assignments[abs(lit)] = (lit > 0) ? 1 : -1;
                    int res = dpll(F);
                    if( res == 0 ){
                        F->assignments[abs(lit)] = 0;
                        return 0;
                    }
                    return res;
                }
            }
        }
   }

    // 2. STOPPING CONDITIONS
    int all_sat = 1;
    for( ui32 i = 0; i < F->num_clauses; i++){
        i8 status = check_formula_status(&F->clauses[i], F->assignments);
        if( status == -1) return 0; //UNSAT
        if( status != 1) all_sat = 0;
    }
    if (all_sat) return 1;


   // 3. CHOOSE LITERAL
   ui32 var = 0;
   for (ui32 i = 1; i <= F->num_variables; ++ i){
    if( F->assignments[i] == 0){var =  i; break;}
   }
   if (var == 0) return 1;


   // 4. BRANCHING

   F->assignments[var] = 1;
   if( dpll(F)) return 1;

   F->assignments[var] = -1;
   if(dpll(F)) return 1;

   F->assignments[var] = 0;
   return 0;

}

/* ---------------------------------------------------------------------
 *  Write the model in MiniSat-compatible format:
 *     SAT
 *     1 -2 3 -4 ... 0
 *
 *  Unassigned variables are written as negative (don't-care = false).
 * ------------------------------------------------------------------ */
static void write_model(FILE *out, Formula *F) {
    fprintf(out, "SAT\n");
    for (ui32 v = 1; v <= F->num_variables; ++v) {
        i8 a = F->assignments[v];
        if (a == 1)        fprintf(out, "%u ", v);
        else               fprintf(out, "-%u ", v);   /* -1 or 0 */
    }
    fprintf(out, "0\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <input.cnf> [output.sol]\n", argv[0]);
        fprintf(stderr, "  Without output.sol the result is written to stdout.\n");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    FILE *out = stdout;
    if (argc == 3) {
        out = fopen(argv[2], "w");
        if (!out) { perror(argv[2]); fclose(file); return EXIT_FAILURE; }
    }

    Formula CNF_formula;
    create_formula(&CNF_formula);
    read_cnf_file(file, &CNF_formula);
    fclose(file);

    /* Uncomment the next line for verbose debug of the parsed formula.
       Output goes to stderr so it never pollutes the model on stdout. */
    /* display_formula(&CNF_formula); */

    if (dpll(&CNF_formula)) {
        write_model(out, &CNF_formula);
        fprintf(stderr, "SATISFIABLE\n");
    } else {
        fprintf(out, "UNSAT\n");
        fprintf(stderr, "UNSATISFIABLE\n");
    }

    if (out != stdout) fclose(out);

    /* Free memory */
    for (ui32 i = 0; i < CNF_formula.num_clauses; ++i)
        free(CNF_formula.clauses[i].literals);
    free(CNF_formula.clauses);
    free(CNF_formula.assignments);

    return 0;
}