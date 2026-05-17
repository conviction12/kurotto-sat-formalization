# Kurotto SAT Formalization

This project implements a complete SAT-based formalization and solving pipeline for the logic puzzle **Kurotto**.

The program translates a Kurotto grid into a Boolean satisfiability problem, generates a DIMACS CNF file, solves it using a custom SAT Solver, and displays the final solution in a human-readable format.

## Overview

Kurotto is a grid-based logic puzzle where each cell must be colored either black or white.

Cells containing circles must always remain white. Numbered circles impose constraints on the total size of adjacent connected black components. If a black connected component touches a numbered circle, its size contributes to the value required by that circle.

This project models the puzzle rules using propositional logic and encodes them into the standard **DIMACS CNF** format. The generated formula is then solved using a custom SAT Solver implemented in C.

## Main Features

- Formalization of the Kurotto puzzle as a SAT problem

- DIMACS CNF generation

- Support for numbered and empty circles

- Reachability-based modeling of connected black components

- Sequential counter for exact cardinality constraints

- Custom SAT Solver based on the DPLL algorithm

- Unit propagation and recursive backtracking

- SAT / UNSAT detection

- Human-readable display of solved grids

- Automated build and execution pipeline using a Makefile

## Technologies Used

- C

- Makefile

- DIMACS CNF

- Propositional Logic

- SAT Solving

- DPLL Algorithm

- Sequential Counter

- Graph Reachability

## Project Structure

```text

.

├── main.c                 # Encoder entry point: puzzle grid -> DIMACS CNF

├── kurotto.c              # Kurotto encoding logic and constraint generation

├── kurotto.h              # Header for Kurotto encoding functions

├── sat_solver.c           # Custom SAT Solver implementation

├── sat_solver.h           # SAT Solver data structures and functions

├── display_solution.c     # Displays the solved puzzle in a readable format

├── Makefile               # Build system and execution pipeline

├── demo6x6.txt            # Example satisfiable puzzle

└── unsat_demo.txt         # Example unsatisfiable puzzle

```

## Build System

The project includes a `Makefile` that compiles three executables:

```text

encoder      # Converts a Kurotto puzzle into a DIMACS CNF file

sat_solver   # Solves the generated CNF file

display      # Displays the solution in a readable grid format

```

The Makefile uses `gcc` as the compiler and builds the project with debugging and warning flags.

## Compilation

To compile the full project, run:

```bash

make

```

This creates the following executables:

```text

encoder

sat_solver

display

```

## Running the Full Pipeline

The easiest way to run the project is to use the `solve` target from the Makefile:

```bash

make solve FILE=demo6x6.txt

```

This command automatically executes the full solving pipeline:

```bash

./encoder demo6x6.txt

./sat_solver demo6x6.txt.cnf demo6x6.txt.sol

./display demo6x6.txt demo6x6.txt.sol

```

## Pipeline Explanation

The project works in three main stages.

### 1. Encoding

```bash

./encoder demo6x6.txt

```

The encoder reads the Kurotto puzzle and generates a DIMACS CNF file:

```text

demo6x6.txt.cnf

```

This file contains the Boolean variables and clauses representing the puzzle constraints.

### 2. Solving

```bash

./sat_solver demo6x6.txt.cnf demo6x6.txt.sol

```

The SAT Solver reads the generated CNF file and determines whether the formula is satisfiable.

It produces a solution file:

```text

demo6x6.txt.sol

```

The result is either:

```text

SATISFIABLE

```

or:

```text

UNSATISFIABLE

```

### 3. Displaying

```bash

./display demo6x6.txt demo6x6.txt.sol

```

The display program maps the SAT assignment back to the original Kurotto grid and prints the solved puzzle in a readable format.

## Example Usage

Run:

```bash

make solve FILE=demo6x6.txt

```

Example output:

```text

========== Running the full pipeline on demo6x6.txt ==========

./encoder demo6x6.txt

CNF written to demo6x6.txt.cnf

variables : 5916

clauses : 27314

numbered circles : 7

./sat_solver demo6x6.txt.cnf demo6x6.txt.sol

SATISFIABLE

./display demo6x6.txt demo6x6.txt.sol

SATISFIABLE - solved grid:

Compact form (# = black, . = white, o/N = circle):

. 0 . # # #

# 1 . # # #

. . # . . .

# . # 2 0 0

# # . . . .

# # . # 1 0

```

## Unsatisfiable Example

The project can also detect impossible Kurotto puzzles.

For example, a `2 x 2` puzzle with a numbered circle requiring 5 adjacent black cells is unsatisfiable, because the grid does not contain enough available cells.

Run:

```bash

make solve FILE=unsat_demo.txt

```

Expected result:

```text

UNSATISFIABLE

```

## Cleaning Generated Files

To remove compiled binaries and generated intermediate files, run:

```bash

make clean

```

This removes files such as:

```text

encoder

sat_solver

display

*.o

*.cnf

*.sol

```

## SAT Encoding

The encoding uses several types of Boolean variables.

### Color Variables

Color variables represent whether each cell is black or white.

A cell is black if its corresponding variable is true, and white otherwise.

### Reachability Variables

Reachability variables represent whether a black cell belongs to a connected component adjacent to a numbered circle.

This is necessary because Kurotto requires counting the size of connected black components touching numbered circles.

### Counter Variables

Counter variables are used to count the exact number of reached black cells.

The project uses a sequential counter to enforce exact cardinality constraints.

## Main Logical Constraints

The generated CNF formula includes the following constraints:

- Circle cells must always be white.

- A numbered circle must count the total size of adjacent connected black components.

- Reachability is propagated through orthogonally adjacent black cells.

- Isolated black components that do not touch the circle are not counted.

- Cardinality constraints are enforced using a sequential counter.

- The final count for each numbered circle must match the number inside the circle.

## Custom SAT Solver

The project includes a custom SAT Solver implemented in C.

The solver is based on the classical **DPLL algorithm** and includes:

- DIMACS CNF parsing

- Clause evaluation

- Unit propagation

- Recursive branching

- Backtracking

- SAT / UNSAT detection

## DPLL Algorithm

The DPLL algorithm works by recursively assigning truth values to variables while simplifying the formula.

The implementation includes three main steps:

1. **Unit Propagation**  

   If a clause contains only one unassigned literal and all other literals are false, that literal must be forced to true.

2. **Stopping Conditions**  

   The solver stops when all clauses are satisfied or when a contradiction is detected.

3. **Branching and Backtracking**  

   If no forced move is available, the solver chooses an unassigned variable, tries assigning it to true, then false if needed.

## Input Format

Puzzle files describe the Kurotto grid.

The encoder reads the input file and produces a `.cnf` file in DIMACS format.

Example generated files:

```text

demo6x6.txt.cnf

demo6x6.txt.sol

```

## Output Format

If the puzzle is satisfiable, the program prints a solved grid.

Symbols used in the compact display:

```text

#   black cell

.   white cell

o   empty circle

N   numbered circle

```

## Authors

- Daniel Nunu

- Evgeniia Kotlova

- Sherif Robov

## Academic Context

This project was developed as part of an academic assignment in logic and automated reasoning.

It combines propositional logic, SAT reduction, graph reachability, cardinality constraints, DIMACS encoding, and SAT solving algorithms.

## License

This project is currently published without a license.

All rights are reserved by the authors unless a license is added later.
