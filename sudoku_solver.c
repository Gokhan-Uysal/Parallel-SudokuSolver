#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define EMPTY 0

#define MAX_SIZE 25
#define THREAD_NUM 6

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
					printMatrix(matrix, box_sz);
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
		if (solveSudoku(row, col + 1, matrix, box_sz, grid_sz))
		{
			printMatrix(matrix, box_sz);
		}
	}
	else
	{
		int num;
		int result = EMPTY;

#pragma omp parallel private(num) shared(box_sz, grid_sz, matrix, row, col)
		{
#pragma omp for
			for (num = 1; num <= box_sz; num++)
			{
				if (canBeFilled(matrix, row, col, num, box_sz, grid_sz))
				{
					int matrix_copy[MAX_SIZE][MAX_SIZE];
					memcpy(matrix_copy, matrix, sizeof(int) * pow(MAX_SIZE, 2));

#pragma omp critical
					matrix[row][col] = num;

					if (solveSudokuParallel(row, col + 1, matrix_copy, box_sz, grid_sz))
						printMatrix(matrix, box_sz);
#pragma omp critical
					matrix[row][col] = EMPTY;
				}
			}
		}

#pragma omp barrier
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

	omp_set_dynamic(0);
	omp_set_num_threads(THREAD_NUM);
	printf("Threads using: %d\n", omp_get_num_threads());

	int box_sz = atoi(argv[1]); // 9
	int grid_sz = sqrt(box_sz); // 3
	char filename[256];
	strcpy(filename, argv[2]); // 3x3_easy.csv

	int matrix[MAX_SIZE][MAX_SIZE];

	readCSV(box_sz, filename, matrix); // 9 3x3_easy.csv matrix[][]

	double time1 = omp_get_wtime();
	solveSudokuParallel(0, 0, matrix, box_sz, grid_sz); // 0 0 matrix[][] 9 3
	printf("Elapsed time: %0.2lf\n", omp_get_wtime() - time1);
	return 0;
}
