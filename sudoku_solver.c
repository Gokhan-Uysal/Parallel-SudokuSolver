#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>

#define EMPTY 0

#define MAX_SIZE 25
#define THREAD_NUM 9

int cutoff = 10;
bool isSolved = false;

int findEmptyLocation(int matrix[MAX_SIZE][MAX_SIZE], int *row, int *col, int box_size);

int canBeFilled(int matrix[MAX_SIZE][MAX_SIZE], int row, int col, int num, int box_size, int grid_sz);

void printMatrix(int matrix[MAX_SIZE][MAX_SIZE], int box_sz)
{
	printf("solution matrix\n");
	int row, col;
	for (row = 0; row < box_sz; row++)
	{
		for (col = 0; col < box_sz; col++)
			printf("%d ", matrix[row][col]);
		printf("\n");
	}

	fflush(stdout);
}

int solveSudoku(int row, int col, int matrix[MAX_SIZE][MAX_SIZE], int box_sz, int grid_sz)
{
	if (col > (box_sz - 1))
	{
		col = 0;
		row++;
	}
	if (row > (box_sz - 1))
	{
		return 1;
	}
	if (matrix[row][col] != EMPTY)
	{
		if (solveSudoku(row, col + 1, matrix, box_sz, grid_sz))
		{
			printMatrix(matrix, box_sz);
		}
	}
	else
	{
		int num;
		for (num = 1; num <= box_sz; num++)
		{
			int result = canBeFilled(matrix, row, col, num, box_sz, grid_sz);
			if (result)
			{
				matrix[row][col] = num;

				if (solveSudoku(row, col + 1, matrix, box_sz, grid_sz))
				{
					printMatrix(matrix, box_sz);
				}
				matrix[row][col] = EMPTY;
			}
		}
	}

	return 0;
}

int solveSudokuEarly(int row, int col, int matrix[MAX_SIZE][MAX_SIZE], int box_sz, int grid_sz)
{
	if (isSolved)
	{
		return 0;
	}

	if (col > (box_sz - 1))
	{
		col = 0;
		row++;
	}
	if (row > (box_sz - 1))
	{
		printf("Solved!\n");
		return 1;
	}
	if (matrix[row][col] != EMPTY)
	{
		if (solveSudokuEarly(row, col + 1, matrix, box_sz, grid_sz))
		{
			printMatrix(matrix, box_sz);
		}
	}
	else
	{
		int num;
		for (num = 1; num <= box_sz; num++)
		{
			int result = canBeFilled(matrix, row, col, num, box_sz, grid_sz);
			if (result)
			{
				matrix[row][col] = num;

				if (solveSudokuEarly(row, col + 1, matrix, box_sz, grid_sz))
				{
					printMatrix(matrix, box_sz);
					isSolved = true;
				}
				matrix[row][col] = EMPTY;
			}
		}
	}

	return 0;
}

int solveSudokuParallel(int row, int col, int matrix[MAX_SIZE][MAX_SIZE], int box_sz, int grid_sz)
{
	if (col > (box_sz - 1))
	{
		col = 0;
		row++;
	}
	if (row > (box_sz - 1))
	{
		return 1;
	}
	if (matrix[row][col] != EMPTY)
	{
		if (solveSudokuParallel(row, col + 1, matrix, box_sz, grid_sz))
		{
			printMatrix(matrix, box_sz);
		}
	}
	else
	{
		int num;
#pragma omp parallel private(num) shared(box_sz, grid_sz, matrix, row, col)
		{
#pragma omp for nowait
			for (num = 1; num <= box_sz; num++)
			{
				if (canBeFilled(matrix, row, col, num, box_sz, grid_sz))
				{
					int matrix_copy[MAX_SIZE][MAX_SIZE];
#pragma omp critical
					{
						memcpy(matrix_copy, matrix, sizeof(int) * pow(MAX_SIZE, 2));
					}
					matrix_copy[row][col] = num;

					if (solveSudokuParallel(row, col + 1, matrix_copy, box_sz, grid_sz))
					{
						printMatrix(matrix_copy, box_sz);
					}
					matrix_copy[row][col] = EMPTY;
				}
			}
		}
	}

	return 0;
}

int solveSudokuParallelEarly(int row, int col, int matrix[MAX_SIZE][MAX_SIZE], int box_sz, int grid_sz)
{
	if (isSolved)
	{
		return 0;
	}

	if (col > (box_sz - 1))
	{
		col = 0;
		row++;
	}
	if (row > (box_sz - 1))
	{
		return 1;
	}
	if (matrix[row][col] != EMPTY)
	{
		if (solveSudokuParallelEarly(row, col + 1, matrix, box_sz, grid_sz))
		{
			printMatrix(matrix, box_sz);
		}
	}
	else
	{
		int num;
#pragma omp parallel private(num) shared(box_sz, grid_sz, matrix, row, col) num_threads(THREAD_NUM)
		{
#pragma omp for nowait
			for (num = 1; num <= box_sz; num++)
			{
				int matrix_copy[MAX_SIZE][MAX_SIZE];
				memcpy(matrix_copy, matrix, sizeof(int) * pow(MAX_SIZE, 2));

				if (canBeFilled(matrix_copy, row, col, num, box_sz, grid_sz))
				{
					matrix_copy[row][col] = num;

					if (solveSudokuParallelEarly(row, col + 1, matrix_copy, box_sz, grid_sz))
					{
						printMatrix(matrix_copy, box_sz);
#pragma omp critical
						{
							isSolved = true;
						}
					}
					matrix_copy[row][col] = EMPTY;
				}
			}
		}
	}

	return 0;
}

int solveSudokuParallelCutoff(int row, int col, int matrix[MAX_SIZE][MAX_SIZE], int box_sz, int grid_sz, int depth)
{

	if (col > (box_sz - 1))
	{
		col = 0;
		row++;
	}
	if (row > (box_sz - 1))
	{
		return 1;
	}
	if (matrix[row][col] != EMPTY)
	{
		if (solveSudokuParallelCutoff(row, col + 1, matrix, box_sz, grid_sz, depth))
		{
			printMatrix(matrix, box_sz + 4);
		}
	}
	else
	{
		int num;
		if (depth > cutoff)
		{
			solveSudoku(row, col, matrix, box_sz, grid_sz);
		}
		else
		{
#pragma omp parallel private(num) shared(box_sz, grid_sz, matrix, row, col, depth)
			{
#pragma critical
				{
					depth++;
				}
#pragma omp for nowait
				for (num = 1; num <= box_sz; num++)
				{
					int matrix_copy[MAX_SIZE][MAX_SIZE];
					memcpy(matrix_copy, matrix, sizeof(int) * pow(MAX_SIZE, 2));

					if (canBeFilled(matrix_copy, row, col, num, box_sz, grid_sz))
					{
						matrix_copy[row][col] = num;

						if (solveSudokuParallelCutoff(row, col + 1, matrix_copy, box_sz, grid_sz, depth))
							printMatrix(matrix_copy, box_sz);
						matrix_copy[row][col] = EMPTY;
					}
				}
			}
		}
	}
	return 0;
}

int existInRow(int matrix[MAX_SIZE][MAX_SIZE], int row, int num, int box_size)
{
	int col;
	int result = 0;

	for (col = 0; col < box_size; col++)
	{
		if (matrix[row][col] == num)
			result = 1;
	}
	return result;
}

int existInColumn(int matrix[MAX_SIZE][MAX_SIZE], int col, int num, int box_size)
{
	int row;
	int result = 0;

	for (row = 0; row < box_size; row++)
		if (matrix[row][col] == num)
			result = 1;
	return result;
}

int existInGrid(int matrix[MAX_SIZE][MAX_SIZE], int gridOffsetRow, int gridOffsetColumn, int num, int grid_sz)
{
	int row, col;
	int result = 0;

	for (row = 0; row < grid_sz; row++)
		for (col = 0; col < grid_sz; col++)
			if (matrix[row + gridOffsetRow][col + gridOffsetColumn] == num)
				result = 1;
	return result;
}

int canBeFilled(int matrix[MAX_SIZE][MAX_SIZE], int row, int col, int num, int box_size, int grid_sz)
{
	return !existInRow(matrix, row, num, box_size) &&
		   !existInColumn(matrix, col, num, box_size) &&
		   !existInGrid(matrix, row - row % grid_sz, col - col % grid_sz, num, grid_sz) &&
		   matrix[row][col] == EMPTY;
}

int findCutoff(int matrix[MAX_SIZE][MAX_SIZE], int box_sz)
{
	int cutoff = 0;
	int row, col;
	for (row = 0; row < box_sz; row++)
	{
		for (col = 0; col < box_sz; col++)
		{
			if (matrix[row][col] == EMPTY)
			{
				cutoff++;
			}
		}
	}
	return cutoff / 3;
}

void readCSV(int box_sz, char *filename, int matrix[MAX_SIZE][MAX_SIZE])
{
	FILE *file;
	file = fopen(filename, "r");

	int i = 0;
	char line[4098];
	while (fgets(line, 4098, file) && (i < box_sz))
	{
		char *tmp = strdup(line);

		int j = 0;
		const char *tok;
		for (tok = strtok(line, ","); tok && *tok; j++, tok = strtok(NULL, ",\n"))
		{
			matrix[i][j] = atof(tok);
		}

		free(tmp);
		i++;
	}
}

int main(int argc, char const *argv[])
{

	if (argc < 3)
	{
		printf("Please specify matrix size and the CSV file name as inputs.\n");
		exit(0);
	}

	omp_set_num_threads(THREAD_NUM);
	printf("Number of threads: %d\n", THREAD_NUM);

	int box_sz = atoi(argv[1]); // 9
	int grid_sz = sqrt(box_sz); // 3
	char filename[256];
	strcpy(filename, argv[2]); // 3x3_easy.csv

	int matrix[MAX_SIZE][MAX_SIZE];
	readCSV(box_sz, filename, matrix);
	cutoff = findCutoff(matrix, box_sz);

	double time1 = omp_get_wtime();
	cutoff = findCutoff(matrix, box_sz);
	solveSudokuParallelCutoff(0, 0, matrix, box_sz, grid_sz, 3);
	printf("Elapsed time: %0.2lf\n", omp_get_wtime() - time1);
	return 0;
}
