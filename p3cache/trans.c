/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	int r, c, temp, index, row, col;
    if (N == 32)
    {
        col = 0;
        while(col < N)
        {
            row = 0;
            while(row < M)
            {
                r = row;
                while(r < (row+8))
                {
                    c = col; 
                    while(c < (col+8))
                    {
                        if (c == r)
                        {
                            temp = A[r][c];
                            index = r;
                        }
                        else
                            B[c][r] = A[r][c];
                        c++;
                    }
                    if (col == row)
                        B[index][index] = temp;
                    r++;
                }
                row+=8;
            }
           col+=8;
        }
    } 

    else if ((M == 32) & (N == 64)) 
    {
        row = 0;
        while(row<64)
        {
            col = 0;
            while(col<32)
            {
                r = row;
                while(r<row+8)
                {
                    c = col;
                    while(c<col+4)
                    {
                        temp = A[r][c];
                        B[c][r] = temp;
                        c++;
                    }
                    r++;
                }
                col+=4;
            }
            row+=8;
        }
    }
        

	else
	{
		col = 0;
		while(col < M)
		{
			row = 0;
			while(row < N)
			{
				r = row;
				while(r < (row+16) && r<N)	
				{
					c = col;
					while(c<(col+16) && c<M)
					{	
						if(r!=c)
							B[c][r] = A[r][c];
						else
						{
							temp = A[r][c];
							index = r;
						}
						c++;
					}
					if(row==col)
						B[index][index] = temp;
					r++;
				 }
				row+=16;
			}
			col+=16;
		}
	}
}





/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */

char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

