#ifndef __POINT_EXPLORE_H__
#define __POINT_EXPLORE_H__

#include <vector>
#include <cmath>

#include "../util/matrix.h"
#include "../util/utils.h"
#include "../util/logging.h"

#include <Python.h>
#include <boost/python.hpp>
#include <boost/filesystem.hpp>

namespace py = boost::python;

#define TIMESTEPS 10
#define PARTICLES 20
#define DT 1.0
#define X_DIM 2
#define U_DIM 2
#define Z_DIM 1
#define Q_DIM 2
#define R_DIM 1

const int T = TIMESTEPS;
const int M = PARTICLES;
const int TOTAL_VARS = T*X_DIM + (T-1)*U_DIM;

SymmetricMatrix<R_DIM> R;

Matrix<X_DIM> x0;
Matrix<X_DIM> xMin, xMax;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const double step = 0.0078125*0.0078125;
const double INFTY = 1e10;

const double alpha = 100;
const double max_range = 0.25;

const double alpha_control = 0;

Matrix<X_DIM> target;

namespace point_explore {

template<int _dim>
float dist(const Matrix<_dim>& a, const Matrix<_dim>& b) {
	return sqrt(tr(~(a-b)*(a-b)));
}

// notice zero noise influence
Matrix<X_DIM> dynfunc(const Matrix<X_DIM>& x, const Matrix<U_DIM>& u) {
	Matrix<X_DIM> xNew = x + u*DT;
	return xNew;
}

Matrix<Z_DIM> obsfunc(const Matrix<X_DIM>& x, const Matrix<X_DIM>& t, const Matrix<R_DIM>& r) {
	Matrix<Z_DIM> z;
	z[0] = 1.0/(1.0+exp(-alpha*(max_range-dist<X_DIM>(x, t)))) + r[0];
	return z;
}

template <int _vDim, int _sDim>
float gaussLikelihood(const Matrix<_vDim>& v, const SymmetricMatrix<_sDim>& S) {
	Matrix<_sDim,_sDim> Sf;
	chol(S, Sf);
	Matrix<_sDim,1> M = (!Sf)*v;

	Matrix<_sDim> E_exp;
	float E_exp_sum = 0;
	E_exp_sum = exp(-0.5*tr(~M*M));

	float Sf_diag_prod = 1;
	for(int i=0; i < _sDim; ++i) { Sf_diag_prod *= Sf(i,i); }
	float C = pow(2*M_PI, _vDim/2)*Sf_diag_prod;

	float w = E_exp_sum / C;
	return w;
}

std::vector<Matrix<X_DIM> > lowVarianceSampler(const std::vector<Matrix<X_DIM> >& P, const std::vector<float>& W, float r) {
	std::vector<Matrix<X_DIM> > P_sampled(M);

	float c = W[0];
	int i = 0;
	for(int m=0; m < M; ++m) {
		float u = r + (m) * (1/float(M));
		while (u > c) {
			c += W[++i];
		}
		P_sampled[m] = P[i];
	}

	return P_sampled;
}


void updateStateAndParticles(const Matrix<X_DIM>& x_t, const std::vector<Matrix<X_DIM> >& P_t, const Matrix<U_DIM>& u_t,
								Matrix<X_DIM>& x_tp1, std::vector<Matrix<X_DIM> >& P_tp1) {
	x_tp1 = dynfunc(x_t, u_t);
	// receive noisy measurement
	Matrix<Z_DIM> z_tp1 = obsfunc(x_tp1, target, sampleGaussian(zeros<R_DIM,1>(), R));

	std::vector<float> W(M);
	float W_sum = 0;
	Matrix<Z_DIM> z_particle;
	// for each particle, weight by gaussLikelihood of that measurement given particle/robot distance
	for(int m=0; m < M; ++m) {
		z_particle = obsfunc(x_tp1, P_t[m], zeros<R_DIM,1>());
		Matrix<Z_DIM> e = z_particle - z_tp1;
		W[m] = gaussLikelihood<Z_DIM,Z_DIM>(e, R);
		W_sum += W[m];
	}
	for(int m=0; m < M; ++m) { W[m] = W[m] / W_sum; }

	float sampling_noise = (1/float(M))*(rand() / float(RAND_MAX));
	P_tp1 = lowVarianceSampler(P_t, W, sampling_noise);
}


float greg_entropy(const std::vector<Matrix<X_DIM> >& X, const std::vector<Matrix<U_DIM> >& U,
					 const std::vector<Matrix<X_DIM> >& P) {
	float entropy = 0;

	std::vector<Matrix<X_DIM>> X_prop(T);
	std::vector<std::vector<Matrix<Z_DIM> > > H(T, std::vector<Matrix<Z_DIM> >(M));
	for(int t=0; t < T-1; ++t) {
		X_prop[t+1] = dynfunc(X[t], U[t]);
		for(int m=0; m < M; ++m) {
			H[t+1][m] = obsfunc(X_prop[t+1], P[m], zeros<R_DIM,1>());
		}
	}

	std::vector<std::vector<float> > W(T, std::vector<float>(M,0));
	W[0] = std::vector<float>(M, 1/float(M));
	for(int t=1; t < T; ++t) {

		float W_sum = 0;
		for(int m=0; m < M; ++m) {
			for(int n=0; n < M; ++n) {
				W[t][m] += gaussLikelihood<Z_DIM, Z_DIM>(H[t][m] - H[t][n], R);
			}
			W_sum += W[t][m];
		}
		for(int m=0; m < M; ++m) { W[t][m] = W[t][m] / W_sum; }

		// use skoglar version
		float entropy_t = 0;
		for(int m=0; m < M; ++m) {
			entropy_t += -W[t][m]*log(W[t][m]);
		}

//		// simplifies because zero particle dynamics
//		for(int m=0; m < M; ++m) {
//			entropy_t += -W[t][m]*log(W[t-1][m]);
//		}

		float sum_cross_time_weights = 0;
		for(int m=0; m < M; ++m) {
			sum_cross_time_weights += W[t-1][m]*W[t][m];
		}
		entropy_t += log(sum_cross_time_weights);

		std::cout << "entropy " << t << ": " << entropy_t << "\n";
		std::cout << "X_prop: " << ~X_prop[t] << "\n";
		entropy += entropy_t;

	}

	return entropy;
}


// obs_noise (T-1)*M
float differential_entropy_noise(const std::vector<Matrix<X_DIM> >& X, const std::vector<Matrix<U_DIM> >& U,
									 const std::vector<Matrix<X_DIM> >& P, const std::vector<Matrix<R_DIM> >& obs_noise) {
	float entropy = 0;

	std::vector<Matrix<X_DIM>> X_prop(T);
	std::vector<std::vector<Matrix<Z_DIM> > > H(T, std::vector<Matrix<Z_DIM> >(M));
	std::vector<std::vector<Matrix<Z_DIM> > > H_mle(T, std::vector<Matrix<Z_DIM> >(M));
	for(int t=0; t < T-1; ++t) {
		X_prop[t+1] = dynfunc(X[t], U[t]);
		for(int m=0; m < M; ++m) {
			H[t+1][m] = obsfunc(X_prop[t+1], P[m], obs_noise[t*M+m]);
			H_mle[t+1][m] = obsfunc(X_prop[t+1], P[m], zeros<R_DIM,1>());
		}
	}

	std::vector<std::vector<float> > W(T, std::vector<float>(M,1/float(M)));
	for(int t=1; t < T; ++t) {
		// gauss likelihood of each particle at P[t]
		// set W[t]
		float W_sum = 0;
		for(int m=0; m < M; ++m) {
			W[t][m] = gaussLikelihood<Z_DIM, Z_DIM>(H[t][m] - H_mle[t][m], R);
			W_sum += W[t][m];
		}
		for(int m=0; m < M; ++m) { W[t][m] = W[t][m] / W_sum; }


		// use skoglar version
		float entropy_t = 0;
		for(int m=0; m < M; ++m) {
			entropy_t += -W[t][m]*log(W[t][m]);
		}

		// simplifies because zero particle dynamics
		for(int m=0; m < M; ++m) {
			entropy_t += -W[t][m]*log(W[t-1][m]);
		}

		float sum_cross_time_weights = 0;
		for(int m=0; m < M; ++m) {
			sum_cross_time_weights += W[t-1][m]*W[t][m];
		}
		entropy_t += log(sum_cross_time_weights);

		entropy += entropy_t;
	}

	for(int t=0; t < T-1; ++t) {
		entropy -= alpha_control*tr(~U[t]*U[t]);
	}

	return -entropy;
}

float differential_entropy(const std::vector<Matrix<X_DIM> >& X, const std::vector<Matrix<U_DIM> >& U,
							  const std::vector<Matrix<X_DIM> >& P) {
	float avg_entropy = 0;
	int max_iter = 100;
	for(int iter=0; iter < max_iter; ++iter) {
		std::vector<Matrix<R_DIM> > obs_noise = sampleGaussianN(zeros<R_DIM,1>(), R, (T-1)*M);
		float entropy = point_explore::differential_entropy_noise(X, U, P, obs_noise);
		avg_entropy += (1/float(max_iter))*entropy;
	}
	return avg_entropy;
}

Matrix<TOTAL_VARS> grad_differential_entropy(std::vector<Matrix<X_DIM> >& X, std::vector<Matrix<U_DIM> >& U,
												const std::vector<Matrix<X_DIM> >& P) {
	Matrix<TOTAL_VARS> g_avg;

	int max_iter = 20;
	for(int iter=0; iter < max_iter; iter++) {
		Matrix<TOTAL_VARS> g;

		std::vector<Matrix<R_DIM> > obs_noise = sampleGaussianN(zeros<R_DIM,1>(), R, T*M);

		float orig, entropy_p, entropy_l;
		int index = 0;
		for(int t=0; t < T; ++t) {
			for(int i=0; i < X_DIM; ++i) {
				orig = X[t][i];

				X[t][i] = orig + step;
				entropy_p = differential_entropy_noise(X, U, P, obs_noise);

				X[t][i] = orig - step;
				entropy_l = differential_entropy_noise(X, U, P, obs_noise);

				X[t][i] = orig;
				g[index++] = (entropy_p - entropy_l)/(2*step);
			}

			if (t < T-1) {
				for(int i=0; i < U_DIM; ++i) {
					orig = U[t][i];

					U[t][i] = orig + step;
					entropy_p = differential_entropy_noise(X, U, P, obs_noise);

					U[t][i] = orig - step;
					entropy_l = differential_entropy_noise(X, U, P, obs_noise);

					U[t][i] = orig;
					g[index++] = (entropy_p - entropy_l)/(2*step);
				}
			}

			g_avg += (1/float(max_iter))*g;
		}
	}

	return g_avg;
}

void pythonDisplayStateAndParticles(const Matrix<X_DIM>& x, const std::vector<Matrix<X_DIM> >& P, const Matrix<X_DIM>& targ) {
	py::list x_list, targ_list;
	for(int i=0; i < X_DIM; ++i) {
		x_list.append(x[i]);
		targ_list.append(targ[i]);
	}

	py::list particles_list;
	for(int i=0; i < X_DIM; ++i) {
		for(int m=0; m < M; ++m) {
			particles_list.append(P[m][i]);
		}
	}

	std::string workingDir = boost::filesystem::current_path().normalize().string();

	try
	{
		Py_Initialize();
		py::object main_module = py::import("__main__");
		py::object main_namespace = main_module.attr("__dict__");
		py::exec("import sys, os", main_namespace);
		py::exec(py::str("sys.path.append('"+workingDir+"/slam')"), main_namespace);
		py::object plot_module = py::import("plot_point_explore");
		py::object plot_state_and_particles = plot_module.attr("plot_state_and_particles");

		plot_state_and_particles(x_list, particles_list, targ_list, X_DIM, M);

		LOG_INFO("Press enter to continue");
		py::exec("raw_input()",main_namespace);
	}
	catch(py::error_already_set const &)
	{
		PyErr_Print();
	}


}

}


#endif
