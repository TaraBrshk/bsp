#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>

#include "../point.h"
#include "../point-adouble.h"

#include "util/amatrix.h"
#include "util/utils.h"
#include "util/Timer.h"
#include "util/logging.h"
//#include "util/adolcsupport.h"

#include <Python.h>
//#include <pythonrun.h>
#include <boost/python.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/filesystem.hpp>

namespace py = boost::python;

aMatrix<X_DIM> x0;
aMatrix<X_DIM,X_DIM> SqrtSigma0;
aMatrix<X_DIM> xGoal;
aMatrix<X_DIM> xMin, xMax;
aMatrix<U_DIM> uMin, uMax;


#include <adolc/adolc.h>
#include <adolc/sparse/sparsedrivers.h>


extern "C" {
#include "lpMPC.h"
}

/*namespace cfg {
const double improve_ratio_threshold = .1;
const double min_approx_improve = 1e-2;
const double min_trust_box_size = 1e-2;
const double trust_shrink_ratio = .1;
const double trust_expand_ratio = 1.5;
}*/
namespace cfg {
const double improve_ratio_threshold = .1;
const double min_approx_improve = 1e-2;
const double min_trust_box_size = 1e-3;
const double trust_shrink_ratio = .1;
const double trust_expand_ratio = 1.5;
}

// lpMPC vars
lpMPC_FLOAT **f, **lb, **ub, **C, **e, **z;



void setupLpMPCVars(lpMPC_params& problem, lpMPC_output& output)
{
	// inputs
	f = new lpMPC_FLOAT*[T];
	lb = new lpMPC_FLOAT*[T];
	ub = new lpMPC_FLOAT*[T];
	C = new lpMPC_FLOAT*[T-1];
	e = new lpMPC_FLOAT*[T];

	// output
	z = new lpMPC_FLOAT*[T];

#define SET_VARS(n)    \
		f[ BOOST_PP_SUB(n,1) ] = problem.f##n ;  \
		lb[ BOOST_PP_SUB(n,1) ] = problem.lb##n ;	\
		ub[ BOOST_PP_SUB(n,1) ] = problem.ub##n ;	\
		C[ BOOST_PP_SUB(n,1) ] = problem.C##n ; \
		e[ BOOST_PP_SUB(n,1) ] = problem.e##n ;  \
		z[ BOOST_PP_SUB(n,1) ] = output.z##n ;

#define BOOST_PP_LOCAL_MACRO(n) SET_VARS(n)
#define BOOST_PP_LOCAL_LIMITS (1, TIMESTEPS-1)
#include BOOST_PP_LOCAL_ITERATE()

#define SET_LAST_VARS(n)    \
		f[ BOOST_PP_SUB(n,1) ] = problem.f##n ;  \
		lb[ BOOST_PP_SUB(n,1) ] = problem.lb##n ;	\
		ub[ BOOST_PP_SUB(n,1) ] = problem.ub##n ;	\
		e[ BOOST_PP_SUB(n,1) ] = problem.e##n ;  \
		z[ BOOST_PP_SUB(n,1) ] = output.z##n ;

#define BOOST_PP_LOCAL_MACRO(n) SET_LAST_VARS(n)
#define BOOST_PP_LOCAL_LIMITS (TIMESTEPS, TIMESTEPS)
#include BOOST_PP_LOCAL_ITERATE()

}

void cleanup()
{
	delete[] inputVars;
	delete[] vars;

	delete[] f;
	delete[] lb;
	delete[] ub;
	delete[] C;
	delete[] e;
	delete[] z;
}

adouble costfunc_a(const std::vector<aMatrix<X_DIM> >& X, const std::vector<aMatrix<U_DIM>>& U, const aMatrix<X_DIM,X_DIM>& SqrtSigma_0)
{
	adouble cost = 0;

	aMatrix<X_DIM> x_tp1;
	aMatrix<B_DIM> b, b_tp1;
	aMatrix<X_DIM,X_DIM> SqrtSigma_t, SqrtSigma_tp1;
	SqrtSigma_t = SqrtSigma_0;

	for (int t = 0; t < T - 1; ++t)
	{

		cost += alpha_belief*aTr(SqrtSigma_t*SqrtSigma_t) + alpha_control*aTr(~U[t]*U[t]);

		vec_a(X[t], SqrtSigma_t, b);
		b_tp1 = beliefDynamics_a(b, U[t]);
		unVec_a(b_tp1, x_tp1, SqrtSigma_t);
	}

	cost += alpha_belief*aTr(SqrtSigma_t*SqrtSigma_t);

	return cost;
}


double costfunc(const std::vector<aMatrix<X_DIM> >& aX, const std::vector<aMatrix<U_DIM>>& aU, const aMatrix<X_DIM,X_DIM>& aSqrtSigma_0)
{
	std::vector<Matrix<X_DIM> > X(T);
	std::vector<Matrix<U_DIM>> U(T-1);
	Matrix<X_DIM,X_DIM> SqrtSigma_0;

	for(int t=0; t < T-1; ++t) {
		for(int i=0; i < X_DIM; ++i) {
			X[t][i] = aX[t][i].value();
		}
		for(int i=0; i < U_DIM; ++i) {
			U[t][i] = aU[t][i].value();
		}
	}
	for(int i=0; i < X_DIM; ++i) {
		X[T-1][i] = aX[T-1][i].value();
	}

	for(int i=0; i < X_DIM*X_DIM; ++i) {
		SqrtSigma_0[i] = aSqrtSigma_0[i].value();
	}

	double cost = 0;

	Matrix<X_DIM> x_tp1;
	Matrix<B_DIM> b, b_tp1;
	Matrix<X_DIM,X_DIM> SqrtSigma_t, SqrtSigma_tp1;
	SqrtSigma_t = SqrtSigma_0;

	for (int t = 0; t < T - 1; ++t)
	{

		cost += alpha_belief*tr(SqrtSigma_t*SqrtSigma_t) + alpha_control*tr(~U[t]*U[t]);

		vec(X[t], SqrtSigma_t, b);
		b_tp1 = beliefDynamics(b, U[t]);
		unVec(b_tp1, x_tp1, SqrtSigma_t);
	}

	cost += alpha_belief*tr(SqrtSigma_t*SqrtSigma_t);

	return cost;
}



void initXUAdolcMatrixes(std::vector<aMatrix<X_DIM>>& X, std::vector<aMatrix<U_DIM>>& U) {
	for(int t=0; t < T-1; ++t) {
		for(int i=0; i < X_DIM; ++i) {
			X[t][i] <<= X[t][i].value();
		}
		for(int i=0; i < U_DIM; ++i) {
			U[t][i] <<= U[t][i].value();
		}
	}
	for(int i=0; i < X_DIM; ++i) {
		X[T-1][i] <<= X[T-1][i].value();
	}
}


void initXUVectorArray(std::vector<aMatrix<X_DIM>>& X, std::vector<aMatrix<U_DIM>>& U, double* XU) {
	int idx = 0;
	for(int t=0; t < T-1; ++t) {
		for(int i=0; i < X_DIM; ++i) {
			XU[idx++] = X[t][i].value();
		}
		for(int i=0; i < U_DIM; ++i) {
			XU[idx++] = U[t][i].value();
		}
	}
	for(int i=0; i < X_DIM; ++i) {
		XU[idx++] = X[T-1][i].value();
	}
}


double lpCollocation(std::vector< aMatrix<X_DIM> >& X, std::vector< aMatrix<U_DIM> >& U, lpMPC_params& problem, lpMPC_output& output, lpMPC_info& info)
{
	util::Timer tapeTimer, gradTimer, forcesTimer, costTimer;
	double tapeTime = 0, gradTime = 0, forcesTime = 0, costTime = 0;

	int maxIter = 100, idx;
	double Xeps = 1;
	double Ueps = 1;

	// box constraint around goal
	double delta = 0.01;
	aMatrix<X_DIM,1> x0 = X[0];

	double prevcost = INFTY, optcost;
	double merit, model_merit, new_merit;
	double approx_merit_improve, exact_merit_improve, merit_improve_ratio;
	double constant_cost, jac_constant;

	prevcost = costfunc(X, U, SqrtSigma0);

	LOG_DEBUG("Initialization trajectory cost: %4.10f", prevcost);


	std::vector<aMatrix<X_DIM> > Xopt(T);
	std::vector<aMatrix<U_DIM> > Uopt(T-1);

	bool solution_accepted = true;
	bool done_trace_already = false;

	// adol-c initializing
	short int tag = 0, keep = 1, repeat = 0;
	adouble meritAdolc;

	double* grad_arr = new double[X_DIM*T + U_DIM*(T-1)];
	double* XU_arr = new double[X_DIM*T + U_DIM*(T-1)];

	for(int it = 0; it < maxIter; ++it)
	{
		LOG_DEBUG("\nIter: %d", it);

		if (solution_accepted) {

			if (done_trace_already) {
				util::Timer_tic(&costTimer);
				merit = costfunc(X, U, SqrtSigma0);
				costTime += util::Timer_toc(&costTimer);
			} else {
				util::Timer_tic(&tapeTimer);
				done_trace_already = true;
				trace_on(tag, keep);
				initXUAdolcMatrixes(X,U);
				meritAdolc = costfunc_a(X, U, SqrtSigma0);
				meritAdolc >>= merit;
				trace_off();
				tapeTime += util::Timer_toc(&tapeTimer);
			}


			// compute gradient
			initXUVectorArray(X,U,XU_arr);

			util::Timer_tic(&gradTimer);
			gradient(tag, X_DIM*T + U_DIM*(T-1), XU_arr, grad_arr); // return value is 0
			gradTime += util::Timer_toc(&gradTimer);



			// evaluate constant cost term (omitted from optimization)
			// Need to compute:
			// ~z_bar * H * z_bar (easy to compute since H is diagonal)
			// -f * z_bar
			constant_cost = 0;
			jac_constant = 0;


			util::Timer_tic(&forcesTimer);


			idx = 0;
			for (int t = 0; t < T-1; ++t)
			{
				for (int i = 0; i < (X_DIM+U_DIM); ++i) {
					f[t][i] = grad_arr[idx++];
				}

				for(int i = 0; i < X_DIM; ++i) {
					jac_constant += -(f[t][i]*X[t][i].value());
				}
				for(int i = 0; i < U_DIM; ++i) {
					jac_constant += -(f[t][i+X_DIM]*U[t][i].value());
				}

				// TODO: move following CMat code outside, is same every iteration
				Matrix<X_DIM,X_DIM+U_DIM> CMat;

				CMat.insert<X_DIM,X_DIM>(0,0,identity<X_DIM>());
				CMat.insert<X_DIM,U_DIM>(0,X_DIM,DT*identity<U_DIM>());
				int idx = 0;
				for(int c = 0; c < (X_DIM+U_DIM); ++c) {
					for(int r = 0; r < X_DIM; ++r) {
						C[t][idx++] = CMat[c + r*(X_DIM+U_DIM)];
					}
				}

				if (t == 0) {
					e[t][0] = x0[0].value(); e[t][1] = x0[1].value();
				} else {
					e[t][0] = 0; e[t][1] = 0;
				}


			} //setting up problem

			for (int i = 0; i < X_DIM; ++i) {
				f[T-1][i] = grad_arr[idx++];
			}

			for(int i = 0; i < X_DIM; ++i) {
				jac_constant += -(f[T-1][i]*X[T-1][i].value());
			}

			e[T-1][0] = 0; e[T-1][1] = 0;

			constant_cost = jac_constant + merit;

		} // end solution_accepted


		// set trust region bounds based on current trust region size
		for (int t = 0; t < T-1; ++t)
		{
			aMatrix<X_DIM>& xt = X[t];
			aMatrix<U_DIM>& ut = U[t];

			// Fill in lb, ub, C, e
			lb[t][0] = MAX(xMin[0].value(), xt[0].value() - Xeps);
			lb[t][1] = MAX(xMin[1].value(), xt[1].value() - Xeps);
			lb[t][2] = MAX(uMin[0].value(), ut[0].value() - Ueps);
			lb[t][3] = MAX(uMin[1].value(), ut[1].value() - Ueps);

			ub[t][0] = MIN(xMax[0].value(), xt[0].value() + Xeps);
			ub[t][1] = MIN(xMax[1].value(), xt[1].value() + Xeps);
			ub[t][2] = MIN(uMax[0].value(), ut[0].value() + Ueps);
			ub[t][3] = MIN(uMax[1].value(), ut[1].value() + Ueps);
		} //setting up problem

		// Fill in lb, ub, C, e
		aMatrix<X_DIM>& xT = X[T-1];
		lb[T-1][0] = MAX(xGoal[0].value() - delta, xT[0].value() - Xeps);
		lb[T-1][1] = MAX(xGoal[1].value() - delta, xT[1].value() - Xeps);

		ub[T-1][0] = MIN(xGoal[0].value() + delta, xT[0].value() + Xeps);
		ub[T-1][1] = MIN(xGoal[1].value() + delta, xT[1].value() + Xeps);

		// Verify problem inputs

		//if (!isValidInputs(result)) {
		//	std::cout << "Inputs are not valid!" << std::endl;
		//	exit(0);
		//}

		LOG_DEBUG("Solving with FORCES");
		forcesTime += util::Timer_toc(&forcesTimer);
		int exitflag = lpMPC_solve(&problem, &output, &info);
		if (exitflag == 1) {
			for(int t = 0; t < T-1; ++t) {
				aMatrix<X_DIM>& xt = Xopt[t];
				aMatrix<U_DIM>& ut = Uopt[t];

				for(int i = 0; i < X_DIM; ++i) {
					xt[i] = z[t][i];
				}
				for(int i = 0; i < U_DIM; ++i) {
					ut[i] = z[t][X_DIM+i];
				}
				optcost = info.pobj;
			}
			aMatrix<X_DIM>& xt = Xopt[T-1];
			xt[0] = z[T-1][0]; xt[1] = z[T-1][1];
		}
		else {
			LOG_FATAL("Some problem in solver");
			std::exit(-1);
		}

		model_merit = optcost + constant_cost; // need to add constant terms that were dropped

		util::Timer_tic(&costTimer);
		new_merit = costfunc(Xopt, Uopt, SqrtSigma0);
		costTime += util::Timer_toc(&costTimer);

		LOG_DEBUG("merit: %f", merit);
		LOG_DEBUG("model_merit: %f", model_merit);
		LOG_DEBUG("new_merit: %f", new_merit);
		LOG_DEBUG("constant cost term: %f", constant_cost);

		approx_merit_improve = merit - model_merit;
		exact_merit_improve = merit - new_merit;
		merit_improve_ratio = exact_merit_improve / approx_merit_improve;

		LOG_DEBUG("approx_merit_improve: %f", approx_merit_improve);
		LOG_DEBUG("exact_merit_improve: %f", exact_merit_improve);
		LOG_DEBUG("merit_improve_ratio: %f", merit_improve_ratio);

		if (approx_merit_improve < -1e-5) {
			LOG_ERROR("Approximate merit function got worse: %f", approx_merit_improve);
			LOG_ERROR("Failure!");
			return INFTY;
		} else if (approx_merit_improve < cfg::min_approx_improve) {
			LOG_DEBUG("Converged: improvement small enough");
			X = Xopt; U = Uopt;
			solution_accepted = true;
			break;
		} else if ((exact_merit_improve < 0) || (merit_improve_ratio < cfg::improve_ratio_threshold)) {
			LOG_DEBUG("Shrinking trust region size to: %2.6f %2.6f", Xeps, Ueps);
			Xeps *= cfg::trust_shrink_ratio;
			Ueps *= cfg::trust_shrink_ratio;
			solution_accepted = false;
		} else {
			LOG_DEBUG("Accepted, Increasing trust region size to:  %2.6f %2.6f", Xeps, Ueps);
			// expand Xeps and Ueps
			Xeps *= cfg::trust_expand_ratio;
			Ueps *= cfg::trust_expand_ratio;
			X = Xopt; U = Uopt;
			prevcost = optcost;
			solution_accepted = true;
		}

		if (Xeps < cfg::min_trust_box_size && Ueps < cfg::min_trust_box_size) {
			LOG_DEBUG("Converged: x tolerance");
			break;
		}
	}

	optcost = costfunc(X, U, SqrtSigma0);

	std::cout << "tape time: " << tapeTime*1000 << "ms" << std::endl;
	std::cout << "grad time: " << gradTime*1000 << "ms" << std::endl;
	std::cout << "forces time: " << forcesTime*1000 << "ms" << std::endl;
	std::cout << "cost time: " << costTime*1000 << "ms" << std::endl;

	return optcost;

}

int main(int argc, char* argv[])
{
	x0[0] = -3.5; x0[1] = 2;
	SqrtSigma0 = aIdentity<X_DIM>();
	xGoal[0] = -3.5; xGoal[1] = -2;

	xMin[0] = -5; xMin[1] = -3;
	xMax[0] = 5; xMax[1] = 3;
	uMin[0] = -1; uMin[1] = -1;
	uMax[0] = 1; uMax[1] = 1;

	aMatrix<U_DIM> uinit;
	uinit[0] = (xGoal[0] - x0[0]) / (T-1);
	uinit[1] = (xGoal[1] - x0[1]) / (T-1);

	std::vector<aMatrix<U_DIM> > U(T-1, uinit);
	std::vector<aMatrix<X_DIM> > X(T);

	X[0] = x0;
	for (int t = 0; t < T-1; ++t) {
		X[t+1] = dynfunc_a(X[t], U[t], aZeros<Q_DIM,1>());
	}

	//setupDstarInterface(std::string(getMask()));


	lpMPC_params problem;
	lpMPC_output output;
	lpMPC_info info;

	setupLpMPCVars(problem, output);

	util::Timer solveTimer;
	util::Timer_tic(&solveTimer);

	// compute cost for the trajectory
	/*
	for(int i = 0; i < 100000; ++i) {
		for (int t = 0; t < T-1; ++t) {
			X[t+1] = dynfunc(X[t], U[t], zeros<Q_DIM,1>());
		}
	}
	*/

	double cost = lpCollocation(X, U, problem, output, info);

	double solvetime = util::Timer_toc(&solveTimer);
	LOG_INFO("Cost: %4.10f", cost);
	LOG_INFO("Solve time: %5.3f ms", solvetime*1000);

	aMatrix<B_DIM> binit = aZeros<B_DIM>();
	std::vector<aMatrix<B_DIM> > B(T, binit);

	vec_a(X[0], SqrtSigma0, B[0]);
	for (size_t t = 0; t < T-1; ++t) {
		B[t+1] = beliefDynamics_a(B[t], U[t]);
	}


	// had to copy in python display because Matrix uses adouble
#define PYTHON_PLOT
#ifdef PYTHON_PLOT
	py::list Bvec;
	for(int j=0; j < B_DIM; j++) {
		for(int i=0; i < T; i++) {
			Bvec.append(B[i][j].value());
		}
	}

	py::list Uvec;
		for(int j=0; j < U_DIM; j++) {
			for(int i=0; i < T-1; i++) {
			Uvec.append(U[i][j].value());
		}
	}

	py::list x0_list, xGoal_list;
	for(int i=0; i < X_DIM; i++) {
		x0_list.append(x0[i].value());
		xGoal_list.append(xGoal[i].value());
	}

	std::string workingDir = boost::filesystem::current_path().normalize().string();
	std::string bspDir = workingDir.substr(0,workingDir.find("bsp"));

	try
	{
		Py_Initialize();
		py::object main_module = py::import("__main__");
		py::object main_namespace = main_module.attr("__dict__");
		py::exec("import sys, os", main_namespace);
		py::exec(py::str("sys.path.append('"+bspDir+"bsp/python')"), main_namespace);
		py::exec("from bsp_light_dark import LightDarkModel", main_namespace);
		py::object model = py::eval("LightDarkModel()", main_namespace);
		py::object plot_mod = py::import("plot");
		py::object plot_traj = plot_mod.attr("plot_belief_trajectory_cpp");

		plot_traj(Bvec, Uvec, model, x0_list, xGoal_list, T);
	}
	catch(py::error_already_set const &)
	{
		PyErr_Print();
	}

	int k;
	std::cin >> k;
#endif

	cleanup();

	//CAL_End();
	return 0;
}
