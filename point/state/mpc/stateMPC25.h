/*
FORCES - Fast interior point code generation for multistage problems.
Copyright (C) 2011-14 Alexander Domahidi [domahidi@control.ee.ethz.ch],
Automatic Control Laboratory, ETH Zurich.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __stateMPC_H__
#define __stateMPC_H__


/* DATA TYPE ------------------------------------------------------------*/
typedef double stateMPC_FLOAT;


/* SOLVER SETTINGS ------------------------------------------------------*/
/* print level */
#ifndef stateMPC_SET_PRINTLEVEL
#define stateMPC_SET_PRINTLEVEL    (0)
#endif

/* timing */
#ifndef stateMPC_SET_TIMING
#define stateMPC_SET_TIMING    (0)
#endif

/* Numeric Warnings */
/* #define PRINTNUMERICALWARNINGS */

/* maximum number of iterations  */
#define stateMPC_SET_MAXIT         (30)	

/* scaling factor of line search (affine direction) */
#define stateMPC_SET_LS_SCALE_AFF  (0.9)      

/* scaling factor of line search (combined direction) */
#define stateMPC_SET_LS_SCALE      (0.95)  

/* minimum required step size in each iteration */
#define stateMPC_SET_LS_MINSTEP    (1E-08)

/* maximum step size (combined direction) */
#define stateMPC_SET_LS_MAXSTEP    (0.995)

/* desired relative duality gap */
#define stateMPC_SET_ACC_RDGAP     (0.0001)

/* desired maximum residual on equality constraints */
#define stateMPC_SET_ACC_RESEQ     (1E-06)

/* desired maximum residual on inequality constraints */
#define stateMPC_SET_ACC_RESINEQ   (1E-06)

/* desired maximum violation of complementarity */
#define stateMPC_SET_ACC_KKTCOMPL  (1E-06)


/* RETURN CODES----------------------------------------------------------*/
/* solver has converged within desired accuracy */
#define stateMPC_OPTIMAL      (1)

/* maximum number of iterations has been reached */
#define stateMPC_MAXITREACHED (0)

/* no progress in line search possible */
#define stateMPC_NOPROGRESS   (-7)




/* PARAMETERS -----------------------------------------------------------*/
/* fill this with data before calling the solver! */
typedef struct stateMPC_params
{
    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H1[4];

    /* vector of size 4 */
    stateMPC_FLOAT f1[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb1[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub1[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C1[8];

    /* vector of size 2 */
    stateMPC_FLOAT e1[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H2[4];

    /* vector of size 4 */
    stateMPC_FLOAT f2[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb2[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub2[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C2[8];

    /* vector of size 2 */
    stateMPC_FLOAT e2[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H3[4];

    /* vector of size 4 */
    stateMPC_FLOAT f3[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb3[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub3[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C3[8];

    /* vector of size 2 */
    stateMPC_FLOAT e3[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H4[4];

    /* vector of size 4 */
    stateMPC_FLOAT f4[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb4[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub4[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C4[8];

    /* vector of size 2 */
    stateMPC_FLOAT e4[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H5[4];

    /* vector of size 4 */
    stateMPC_FLOAT f5[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb5[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub5[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C5[8];

    /* vector of size 2 */
    stateMPC_FLOAT e5[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H6[4];

    /* vector of size 4 */
    stateMPC_FLOAT f6[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb6[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub6[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C6[8];

    /* vector of size 2 */
    stateMPC_FLOAT e6[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H7[4];

    /* vector of size 4 */
    stateMPC_FLOAT f7[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb7[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub7[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C7[8];

    /* vector of size 2 */
    stateMPC_FLOAT e7[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H8[4];

    /* vector of size 4 */
    stateMPC_FLOAT f8[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb8[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub8[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C8[8];

    /* vector of size 2 */
    stateMPC_FLOAT e8[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H9[4];

    /* vector of size 4 */
    stateMPC_FLOAT f9[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb9[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub9[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C9[8];

    /* vector of size 2 */
    stateMPC_FLOAT e9[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H10[4];

    /* vector of size 4 */
    stateMPC_FLOAT f10[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb10[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub10[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C10[8];

    /* vector of size 2 */
    stateMPC_FLOAT e10[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H11[4];

    /* vector of size 4 */
    stateMPC_FLOAT f11[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb11[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub11[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C11[8];

    /* vector of size 2 */
    stateMPC_FLOAT e11[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H12[4];

    /* vector of size 4 */
    stateMPC_FLOAT f12[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb12[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub12[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C12[8];

    /* vector of size 2 */
    stateMPC_FLOAT e12[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H13[4];

    /* vector of size 4 */
    stateMPC_FLOAT f13[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb13[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub13[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C13[8];

    /* vector of size 2 */
    stateMPC_FLOAT e13[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H14[4];

    /* vector of size 4 */
    stateMPC_FLOAT f14[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb14[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub14[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C14[8];

    /* vector of size 2 */
    stateMPC_FLOAT e14[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H15[4];

    /* vector of size 4 */
    stateMPC_FLOAT f15[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb15[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub15[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C15[8];

    /* vector of size 2 */
    stateMPC_FLOAT e15[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H16[4];

    /* vector of size 4 */
    stateMPC_FLOAT f16[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb16[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub16[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C16[8];

    /* vector of size 2 */
    stateMPC_FLOAT e16[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H17[4];

    /* vector of size 4 */
    stateMPC_FLOAT f17[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb17[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub17[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C17[8];

    /* vector of size 2 */
    stateMPC_FLOAT e17[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H18[4];

    /* vector of size 4 */
    stateMPC_FLOAT f18[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb18[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub18[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C18[8];

    /* vector of size 2 */
    stateMPC_FLOAT e18[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H19[4];

    /* vector of size 4 */
    stateMPC_FLOAT f19[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb19[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub19[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C19[8];

    /* vector of size 2 */
    stateMPC_FLOAT e19[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H20[4];

    /* vector of size 4 */
    stateMPC_FLOAT f20[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb20[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub20[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C20[8];

    /* vector of size 2 */
    stateMPC_FLOAT e20[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H21[4];

    /* vector of size 4 */
    stateMPC_FLOAT f21[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb21[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub21[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C21[8];

    /* vector of size 2 */
    stateMPC_FLOAT e21[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H22[4];

    /* vector of size 4 */
    stateMPC_FLOAT f22[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb22[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub22[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C22[8];

    /* vector of size 2 */
    stateMPC_FLOAT e22[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H23[4];

    /* vector of size 4 */
    stateMPC_FLOAT f23[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb23[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub23[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C23[8];

    /* vector of size 2 */
    stateMPC_FLOAT e23[2];

    /* diagonal matrix of size [4 x 4] (only the diagonal is stored) */
    stateMPC_FLOAT H24[4];

    /* vector of size 4 */
    stateMPC_FLOAT f24[4];

    /* vector of size 4 */
    stateMPC_FLOAT lb24[4];

    /* vector of size 4 */
    stateMPC_FLOAT ub24[4];

    /* matrix of size [2 x 4] (column major format) */
    stateMPC_FLOAT C24[8];

    /* vector of size 2 */
    stateMPC_FLOAT e24[2];

    /* diagonal matrix of size [2 x 2] (only the diagonal is stored) */
    stateMPC_FLOAT H25[2];

    /* vector of size 2 */
    stateMPC_FLOAT f25[2];

    /* vector of size 2 */
    stateMPC_FLOAT lb25[2];

    /* vector of size 2 */
    stateMPC_FLOAT ub25[2];

    /* vector of size 2 */
    stateMPC_FLOAT e25[2];

} stateMPC_params;


/* OUTPUTS --------------------------------------------------------------*/
/* the desired variables are put here by the solver */
typedef struct stateMPC_output
{
    /* vector of size 4 */
    stateMPC_FLOAT z1[4];

    /* vector of size 4 */
    stateMPC_FLOAT z2[4];

    /* vector of size 4 */
    stateMPC_FLOAT z3[4];

    /* vector of size 4 */
    stateMPC_FLOAT z4[4];

    /* vector of size 4 */
    stateMPC_FLOAT z5[4];

    /* vector of size 4 */
    stateMPC_FLOAT z6[4];

    /* vector of size 4 */
    stateMPC_FLOAT z7[4];

    /* vector of size 4 */
    stateMPC_FLOAT z8[4];

    /* vector of size 4 */
    stateMPC_FLOAT z9[4];

    /* vector of size 4 */
    stateMPC_FLOAT z10[4];

    /* vector of size 4 */
    stateMPC_FLOAT z11[4];

    /* vector of size 4 */
    stateMPC_FLOAT z12[4];

    /* vector of size 4 */
    stateMPC_FLOAT z13[4];

    /* vector of size 4 */
    stateMPC_FLOAT z14[4];

    /* vector of size 4 */
    stateMPC_FLOAT z15[4];

    /* vector of size 4 */
    stateMPC_FLOAT z16[4];

    /* vector of size 4 */
    stateMPC_FLOAT z17[4];

    /* vector of size 4 */
    stateMPC_FLOAT z18[4];

    /* vector of size 4 */
    stateMPC_FLOAT z19[4];

    /* vector of size 4 */
    stateMPC_FLOAT z20[4];

    /* vector of size 4 */
    stateMPC_FLOAT z21[4];

    /* vector of size 4 */
    stateMPC_FLOAT z22[4];

    /* vector of size 4 */
    stateMPC_FLOAT z23[4];

    /* vector of size 4 */
    stateMPC_FLOAT z24[4];

    /* vector of size 2 */
    stateMPC_FLOAT z25[2];

} stateMPC_output;


/* SOLVER INFO ----------------------------------------------------------*/
/* diagnostic data from last interior point step */
typedef struct stateMPC_info
{
    /* iteration number */
    int it;
	
    /* inf-norm of equality constraint residuals */
    stateMPC_FLOAT res_eq;
	
    /* inf-norm of inequality constraint residuals */
    stateMPC_FLOAT res_ineq;

    /* primal objective */
    stateMPC_FLOAT pobj;	
	
    /* dual objective */
    stateMPC_FLOAT dobj;	

    /* duality gap := pobj - dobj */
    stateMPC_FLOAT dgap;		
	
    /* relative duality gap := |dgap / pobj | */
    stateMPC_FLOAT rdgap;		

    /* duality measure */
    stateMPC_FLOAT mu;

	/* duality measure (after affine step) */
    stateMPC_FLOAT mu_aff;
	
    /* centering parameter */
    stateMPC_FLOAT sigma;
	
    /* number of backtracking line search steps (affine direction) */
    int lsit_aff;
    
    /* number of backtracking line search steps (combined direction) */
    int lsit_cc;
    
    /* step size (affine direction) */
    stateMPC_FLOAT step_aff;
    
    /* step size (combined direction) */
    stateMPC_FLOAT step_cc;    

	/* solvertime */
	stateMPC_FLOAT solvetime;   

} stateMPC_info;


/* SOLVER FUNCTION DEFINITION -------------------------------------------*/
/* examine exitflag before using the result! */
int stateMPC_solve(stateMPC_params* params, stateMPC_output* output, stateMPC_info* info);


#endif