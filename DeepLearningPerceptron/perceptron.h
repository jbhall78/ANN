#ifndef _PERCEPTRON_H
#define _PERCEPTRON_H

#include "mathlib.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
		PERCEPTRON_UNKNOWN = 0,
		PERCEPTRON_NEURON = 1,
		PERCEPTRON_BIAS = 2,
		PERCEPTRON_RECEPTOR = 3,
		PERCEPTRON_OUTPUT = 4,
	} perceptron_type_t;

	typedef struct perceptron_s {
		perceptron_type_t type;
		struct perceptron_s **inputs;
		int inputs_alloc;
		struct perceptron_s **outputs;
		int outputs_alloc;
		int n_inputs;
		int n_outputs;
		real *weights;
		real *input;
		real output;
	} perceptron_t;

	typedef struct mlp_layer_s {
		perceptron_t **perceptrons;
		int n_perceptrons;
	} mlp_layer_t;

	typedef struct mlp_net_s {
		mlp_layer_t **layers;
		int n_layers;
	} mlp_net_t;

	perceptron_t *perceptron_new(perceptron_type_t type);
	void perceptron_link(perceptron_t *perceptron, perceptron_t *link);

	mlp_layer_t *mlp_layer_new(void);
	void mlp_layer_add(mlp_layer_t *layer, perceptron_t *perceptron);
	void mlp_layer_compute(mlp_layer_t *layer);
	void mlp_layer_transfer(mlp_layer_t *src);

	void mlp_net_train(mlp_net_t *net, real *answers);
	//void mlp_layer_train(mlp_layer_t **layers, int n_layers, real *answers);

	mlp_net_t *mlp_net_new(int *topology, int n_layers); // first = #input, ... = #hidden, last = #output
	void mlp_net_feedforward(mlp_net_t *net, real *inputs, real *outputs);
	void mlp_net_feedback(mlp_net_t *net, real *inputs, real *outputs);

	real sigmoid(real);

#ifdef __cplusplus
}
#endif

#endif
