#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "perceptron.h"

#define LEARNING_CONSTANT 1.0

#define TANH_SIGMOID 0

static real
_perceptron_initial_weight(int bias)
{
	int s;

/*	if (bias)
		s = 1;
	else */
	s = (rand() % 2 == 0) ? 1 : -1;
#if TANH_SIGMOID == 1
	return (real)rand() / (real)RAND_MAX * (real)s;
#else
	return (real)rand() / (real)RAND_MAX * (real)s;
#endif


}

static perceptron_t *
perceptron_new(perceptron_type_t type)
{
	perceptron_t *p = (perceptron_t *)malloc(sizeof(perceptron_t));
	memset(p, 0, sizeof(perceptron_t));
	p->type = type;

	if (type == PERCEPTRON_RECEPTOR) { // allocate a single input
		p->input = (real *)malloc(sizeof(real));
		p->n_inputs++;
		p->inputs = (perceptron_t **)malloc(sizeof(perceptron_t *));
		p->inputs[0] = NULL;
	}
	return p;
}

static void
perceptron_outputs_resize(perceptron_t *perceptron, int n)
{
	perceptron_t **outputs;

	if (n > perceptron->outputs_alloc) {
		outputs = (perceptron_t **)malloc(sizeof(perceptron_t *) * (n + 1000));
		perceptron->outputs_alloc = n + 1000;
		for (int i = 0; i < perceptron->n_outputs; i++)
			outputs[i] = perceptron->outputs[i];
		
		if (perceptron->outputs != NULL)
			free(perceptron->outputs);
		perceptron->outputs = outputs;
	}
	
	perceptron->n_outputs++;
}

static void
perceptron_inputs_resize(perceptron_t *perceptron, int n)
{
	perceptron_t **inputs;
	real *weights, *input;
	int i;

	if (n > perceptron->inputs_alloc) {
		inputs = (perceptron_t **)malloc(sizeof(perceptron_t *) * (n + 1000));
		perceptron->inputs_alloc = n + 1000;

		for (i = 0; i < perceptron->n_inputs; i++)
			inputs[i] = perceptron->inputs[i];

		if (perceptron->inputs != NULL)
			free(perceptron->inputs);
		perceptron->inputs = inputs;

		// add a weight to the nucleus of the link
		weights = (real *)malloc(sizeof(real) * perceptron->inputs_alloc);
		for (i = 0; i < perceptron->n_inputs; i++)
			weights[i] = perceptron->weights[i];
		
		if (perceptron->weights != NULL)
			free(perceptron->weights);
		perceptron->weights = weights;

		// allocate temporary input storage to the link
		input = (real *)malloc(sizeof(real) * perceptron->inputs_alloc);
		for (i = 0; i < perceptron->n_inputs; i++)
			input[i] = perceptron->input[i];
		
		if (perceptron->input != NULL)
			free(perceptron->input);
		perceptron->input = input;
	}

	perceptron->n_inputs = n;
}

static void
perceptron_link(perceptron_t *perceptron, perceptron_t *link)
{
	perceptron_t **inputs, **outputs;
	real *weights, *input;
	int i;

	// add a dendrite to the axon
	perceptron_outputs_resize(perceptron, perceptron->n_outputs + 1);
	perceptron->outputs[perceptron->n_outputs - 1] = link;
	
	// add an input dendrite to the link for back propagation
	perceptron_inputs_resize(link, link->n_inputs + 1);
	int last = link->n_inputs - 1;
	link->inputs[last] = perceptron;
	link->weights[last] = _perceptron_initial_weight(perceptron->type == PERCEPTRON_BIAS ? 1 : 0);
	link->input[last] = 0.0;
}

real
sigmoid(real sum)
{
#if TANH_SIGMOID == 1
	return sinh(sum) / cosh(sum);
	//return tanh(sum);
	//return sin(sum);
	//return (1.0 - exp(-sum)) / (1.0 + exp(-sum));
	//return (2.0 / (1.0 + exp(-2.0 * sum))) - 1.0;
	//return (exp(sum) - exp(-sum)) / (exp(sum) + exp(-sum));
#else
	return 1.0 / (1.0 + exp(-sum));
#endif
	//return 1.0 / (1.0 + exp(-2 * sum));
}

real
sigmoid_prime(real x)
{
#if TANH_SIGMOID == 1
	//return cos(x);
	//return 1.0 / cosh(x);
	return 1.0 - x*x;
	//return 1.0 - tanh(x)*tanh(x);
	//return 1.0 - ((pow(exp(x) - exp(-x), 2.0) / pow(exp(x) + exp(-x), 2.0)));
#else
	//return sigmoid(x) * (1 - sigmoid(x));
	return x * (1 - x);
#endif
}

static void
perceptron_compute(perceptron_t *perceptron)
{
	real sum = 0.0;

	switch (perceptron->type) {
	case PERCEPTRON_BIAS:
		perceptron->output = 1.0;
		break;
	case PERCEPTRON_RECEPTOR:
		perceptron->output = perceptron->input[0];
		//perceptron->output = 1.0 / (1.0 + exp(-perceptron->input[0]));
		break;
	default:
		for (int i = 0; i < perceptron->n_inputs; i++)
			sum += perceptron->input[i] * perceptron->weights[i];

		//if (perceptron->type == PERCEPTRON_OUTPUT)
		//	perceptron->output = sum;
		//else
			perceptron->output = sigmoid(sum);
		//perceptron->output = 1.0 / (1.0 + exp(-sum));
		//perceptron->output = 1.0 / (1.0 + exp(-2 * sum));
	}
}


static mlp_layer_t *
mlp_layer_new(void)
{
	mlp_layer_t *layer = (mlp_layer_t *)malloc(sizeof(mlp_layer_t));
	memset(layer, 0, sizeof(mlp_layer_t));
	return layer;
}

static void
mlp_layer_add(mlp_layer_t *layer, perceptron_t *perceptron)
{
	perceptron_t **perceptrons = (perceptron_t **)malloc(sizeof(perceptron_t *) * (layer->n_perceptrons + 1));
	int i;
	for (i = 0; i < layer->n_perceptrons; i++)
		perceptrons[i] = layer->perceptrons[i];
	perceptrons[i] = perceptron;
	if (layer->perceptrons != NULL)
		free(layer->perceptrons);
	layer->perceptrons = perceptrons;
	layer->n_perceptrons++;
}

static void
mlp_layer_compute(mlp_layer_t *layer)
{
	for (int i = 0; i < layer->n_perceptrons; i++) {
		perceptron_compute(layer->perceptrons[i]);
	}
	// compute softmax on outputs
#ifdef SOFTMAX
	if (layer->perceptrons[0]->type == PERCEPTRON_OUTPUT) {
		static real *outputs = NULL;
		if (outputs == NULL)
			outputs = (real *)malloc(layer->n_perceptrons * sizeof(real));
		for (int i = 0; i < layer->n_perceptrons; i++) {
			outputs[i] = exp(layer->perceptrons[i]->output);
		}
		real sum = 0;
		for (int i = 0; i < layer->n_perceptrons; i++)
			sum += outputs[i];
		for (int i = 0; i < layer->n_perceptrons; i++)
			layer->perceptrons[i]->output = outputs[i] / sum;
	}
#endif
}

void
mlp_layer_compute_backwards(mlp_net_t *net, int layer)
{
	if (layer >= net->n_layers - 1)
		return;

	int next = layer + 1;

	for (int i = 0; i < net->layers[layer]->n_perceptrons; i++) {
		real sum = 0;
		for (int j = 0; j < net->layers[next]->n_perceptrons; j++) {
			if (net->layers[next]->perceptrons[j]->type == PERCEPTRON_BIAS)
				continue;
			sum += net->layers[next]->perceptrons[j]->output * net->layers[next]->perceptrons[j]->weights[i];
		}
		//net->layers[layer]->perceptrons[i]->output = sigmoid(sum);
		net->layers[layer]->perceptrons[i]->output = sum;
	}
}

static void // probably shouldn't name this transfer
mlp_layer_transfer(mlp_layer_t *src)
{
	for (int i = 0; i < src->n_perceptrons; i++) {
		// perceptron with our output
		perceptron_t *p1 = src->perceptrons[i];

		// loop over each its outputs
		for (int j = 0; j < p1->n_outputs; j++) {
			perceptron_t *p2 = p1->outputs[j];
			int k;

			// find which input slot we are in
/*			for (k = 0; k < p2->n_inputs; k++)
				if (p2->inputs[k] == p1)
					break;
					*/
			k = i; // just so happens our index is always our index in the layer
			p2->input[k] = p1->output;
		}
	}
}


static real
cost(real answer, real result)
{
#if TANH_SIGMOID == 1
	return answer - result;
#else
	return answer - result;
#endif
	//return pow(answer - result, 2.0) / 10;
}

void
mlp_net_train(mlp_net_t *net, real *answers)
{
	static real **deltas = NULL;

	// allocate space for deltas if required
	if (deltas == NULL) {
		deltas = (real **)malloc(sizeof(real *) * net->n_layers);
		for (int i = net->n_layers - 1; i > 0; i--) // skip the first layer since there are no weights
			deltas[i] = (real *)malloc(net->layers[i]->n_perceptrons * sizeof(real));
	}

	// work backwards through network calculating deltas

	// first compute deltas of output layer
	int output = net->n_layers - 1;
	for (int i = 0; i < net->layers[output]->n_perceptrons; i++) {
		real r = net->layers[output]->perceptrons[i]->output;
		deltas[output][i] = sigmoid_prime(r) * cost(answers[i], r);
	}

	// compute deltas of hidden layers
	for (int j = net->n_layers - 2; j > 0; j--) {
		int next = j + 1;
		int this_has_bias = 1; // we skip bias neurons
		int next_has_bias = (next < net->n_layers - 1) ? 1 : 0;

		for (int i = 0; i < net->layers[j]->n_perceptrons - this_has_bias; i++) {
			perceptron_t *hidden = net->layers[j]->perceptrons[i];

			// sum of all connections from next layer to us
			real sum = 0;
			for (int x = 0; x < net->layers[next]->n_perceptrons - next_has_bias; x++) {
				sum += net->layers[next]->perceptrons[x]->weights[i] * deltas[next][x];
			}
			deltas[j][i] = sigmoid_prime(hidden->output) * sum;
		}
	}

	// now work forward through the network applying the deltas to the weights
	for (int x = 1; x < net->n_layers; x++) { // skip the first layer as there is no weights
		int has_bias = (x < net->n_layers - 1) ? 1 : 0;
		for (int i = 0; i < net->layers[x]->n_perceptrons - has_bias; i++) {
			perceptron_t *hidden = net->layers[x]->perceptrons[i];

			for (int j = 0; j < hidden->n_inputs; j++) {
				hidden->weights[j] += LEARNING_CONSTANT * deltas[x][i] * hidden->input[j];
			}
		}
	}
}

mlp_net_t *
mlp_net_new(int *topology, int n_layers)
{
	int i, j, k;
	mlp_net_t *net;

	// create net object
	net = (mlp_net_t *)malloc(sizeof(mlp_net_t));
	net->layers = (mlp_layer_t **)malloc(sizeof(mlp_layer_t *) * n_layers);
	net->n_layers = n_layers;

	// create layers
	for (i = 0; i < n_layers; i++) {
		net->layers[i] = mlp_layer_new();
	
		if (i == 0) {
			for (j = 0; j < topology[0]; j++) {
				perceptron_t *p = perceptron_new(PERCEPTRON_RECEPTOR);
				mlp_layer_add(net->layers[0], p);
			}
		}
		else {
			perceptron_type_t type;
			if (i == n_layers - 1)
				type = PERCEPTRON_OUTPUT;
			else
				type = PERCEPTRON_NEURON;

			perceptron_t *bias = perceptron_new(PERCEPTRON_BIAS);
			perceptron_t *perceptron;

			// for each number of perceptrons per layer
			for (j = 0; j < topology[i]; j++) {
				perceptron = perceptron_new(type);
				mlp_layer_add(net->layers[i], perceptron);

				// create links from perceptrons of previous layer to this perceptron
				for (k = 0; k < net->layers[i-1]->n_perceptrons; k++)
					perceptron_link(net->layers[i-1]->perceptrons[k], perceptron);

				perceptron_link(bias, perceptron);
			}

			mlp_layer_add(net->layers[i-1], bias);
		}
	}

	return net;
}

void
mlp_net_feedforward(mlp_net_t *net, real *inputs, real *outputs)
{
	// assign the inputs
	for (int i = 0; i < net->layers[0]->n_perceptrons - 1; i++) {
		net->layers[0]->perceptrons[i]->input[0] = inputs[i];
	}

	// compute & transfer layers
	for (int i = 0; i < net->n_layers; i++) {
		mlp_layer_compute(net->layers[i]);
		if (i != net->n_layers - 1)
			mlp_layer_transfer(net->layers[i]);
	}

	// assign the outputs
	for (int i = 0; i < net->layers[net->n_layers - 1]->n_perceptrons; i++) {
		outputs[i] = net->layers[net->n_layers - 1]->perceptrons[i]->output;
	}
}

void
mlp_net_feedback(mlp_net_t *net, real *inputs, real *outputs)
{
	// assign the inputs
	for (int i = 0; i < net->layers[net->n_layers - 1]->n_perceptrons; i++) {
		net->layers[net->n_layers - 1]->perceptrons[i]->output = inputs[i];
	}

	// compute and transfer the layers backwards
	for (int i = net->n_layers - 2; i >= 0; i--) {
		mlp_layer_compute_backwards(net, i);
	}

	// assign the outputs
	for (int i = 0; i < net->layers[0]->n_perceptrons; i++) {
		outputs[i] = net->layers[0]->perceptrons[i]->output;
	}
}