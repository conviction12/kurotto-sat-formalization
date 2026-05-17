#############################################################################
# Makefile - Kurotto SAT project
#############################################################################
# Targets:
#   make              compiles encoder, sat_solver and display
#   make solve FILE=puzzle.txt   runs the full pipeline on FILE
#   make clean        removes the binaries and intermediate files
#############################################################################

CC          = gcc
INCDIR      = .
LIBDIR      = .
LDOPTS      = -L$(LIBDIR) -lm
INCLUDEOPTS = -I$(INCDIR)
COMPILOPTS  = -g -Wall $(INCLUDEOPTS)

EXECUTABLES = encoder sat_solver display

#############################################################################
# default
all : $(EXECUTABLES)

#############################################################################
# generic rule for .o files that have a matching .h
%.o : %.c %.h
	@echo ""
	@echo "---------------------------------------------"
	@echo "Compiling module "$*
	@echo "---------------------------------------------"
	$(CC) -c $(COMPILOPTS) $<

# main.c doesn't have a main.h; give it an explicit rule
main.o : main.c kurotto.h
	@echo ""
	@echo "---------------------------------------------"
	@echo "Compiling main"
	@echo "---------------------------------------------"
	$(CC) -c $(COMPILOPTS) main.c

display_solution.o : display_solution.c kurotto.h
	@echo ""
	@echo "---------------------------------------------"
	@echo "Compiling display_solution"
	@echo "---------------------------------------------"
	$(CC) -c $(COMPILOPTS) display_solution.c

#############################################################################
# executables

# encoder : puzzle -> DIMACS CNF
encoder : main.o kurotto.o
	@echo ""
	@echo "---------------------------------------------"
	@echo "Linking executable $@"
	@echo "---------------------------------------------"
	$(CC) $^ $(LDOPTS) -o $@

# sat_solver : DIMACS CNF -> SAT model (MiniSat format)
sat_solver : sat_solver.o
	@echo ""
	@echo "---------------------------------------------"
	@echo "Linking executable $@"
	@echo "---------------------------------------------"
	$(CC) $^ $(LDOPTS) -o $@

# display : puzzle + SAT model -> human-readable solved grid
display : display_solution.o kurotto.o
	@echo ""
	@echo "---------------------------------------------"
	@echo "Linking executable $@"
	@echo "---------------------------------------------"
	$(CC) $^ $(LDOPTS) -o $@

#############################################################################
# one-shot pipeline: make solve FILE=puzzles/demo.txt
FILE ?= puzzle.txt

solve : all
	@echo ""
	@echo "==========  Running the full pipeline on $(FILE)  =========="
	./encoder $(FILE)
	./sat_solver $(FILE).cnf $(FILE).sol
	./display $(FILE) $(FILE).sol

#############################################################################
clean:
	rm -fR $(EXECUTABLES) *.o *.cnf *.sol
