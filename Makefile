gcc: sudoku_solver.c
	gcc -o sudoku_solver sudoku_solver.c -fopenmp -lm

clean:
	rm -f sudoku_solver
