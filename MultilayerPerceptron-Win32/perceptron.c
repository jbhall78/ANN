#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "perceptron.h"

#define LEARNING_CONSTANT 0.5

#define E 2.71828183

static real
_perceptron_initial_weight(int bias)
{
	int s;

	if (bias)
		s = 1;
	else
		s = (rand() % 2 == 0) ? 1 : -1;

	return (real)rand() / (real)RAND_MAX * (real)s;
}

perceptron_t *
perceptron_new(perceptron_type_t type)
{
	perceptron_t *p = (perceptron_t *)malloc(sizeof(perceptron_t));
	memset(p, 0, sizeof(perceptron_t));
	p->type = type;

	if (type == PERCEPTRON_RECEPTOR) { // allocate a single input
		p->input = (real *)malloc(sizeof(real));
	//	p->weights = (real *)malloc(sizeof(real));
	//	p->weights[0] = _perceptron_initial_weight();
		p->n_inputs++;
		p->inputs = (perceptron_t **)malloc(sizeof(perceptron_t *));
		p->inputs[0] = NULL;
	}
	return p;
}

void
perceptron_link(perceptron_t *perceptron, perceptron_t *link)
{
	perceptron_t **inputs, **outputs;
	real *weights, *input;
	int i;

	// add a dendrite to the axon
	outputs = (perceptron_t **)malloc(sizeof(perceptron_t *) * (perceptron->n_outputs + 1));
	for (i = 0; i < perceptron->n_outputs; i++)
		outputs[i] = perceptron->outputs[i];
	outputs[i] = link;
	if (perceptron->outputs != NULL)
		free(perceptron->outputs);
	perceptron->outputs = outputs;

	perceptron->n_outputs++;

	// add an input dendrite to the link for back propagation
	inputs = (perceptron_t **)malloc(sizeof(perceptron_t *) * (link->n_inputs + 1));
	for (i = 0; i < link->n_inputs; i++)
		inputs[i] = link->inputs[i];
	inputs[i] = perceptron;
	if (link->inputs != NULL)
		free(link->inputs);
	link->inputs = inputs;

	link->n_inputs++;

	// add a weight to the nucleus of the link
	weights = (real *)malloc(sizeof(real) * link->n_inputs);
	for (i = 0; i < link->n_inputs - 1; i++)
		weights[i] = link->weights[i];
	weights[i] = _perceptron_initial_weight(perceptron->type == PERCEPTRON_BIAS ? 1 : 0);
	if (link->weights != NULL)
		free(link->weights);
	link->weights = weights;

	// allocate temporary input storage to the link
	input = (real *)malloc(sizeof(real) * link->n_inputs);
	for (i = 0; i < link->n_inputs - 1; i++)
		input[i] = link->input[i];
	input[i] = 0.0;
	if (link->input != NULL)
		free(link->input);
	link->input = input;

}

void
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

		// sigmoid function
		//perceptron->output = 1.0 / (1.0 + exp(-sum));
		perceptron->output = 1.0 / (1.0 + exp(-2 * sum));
	}
}

mlp_layer_t *
mlp_layer_new(void)
{
	mlp_layer_t *layer = (mlp_layer_t *)malloc(sizeof(mlp_layer_t));
	memset(layer, 0, sizeof(mlp_layer_t));
	return layer;
}

void
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

void
mlp_layer_compute(mlp_layer_t *layer)
{
	for (int i = 0; i < layer->n_perceptrons; i++) {
		perceptron_compute(layer->perceptrons[i]);
	}
}

void // probably shouldn't name this transfer
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
			for (k = 0; k < p2->n_inputs; k++)
				if (p2->inputs[k] == p1)
					break;

			p2->input[k] = p1->output;
		}
	}
}
/*
void
perceptron_train(perceptron_t *output, real answer)
{
	real result = output->output;
	// derivative of sigmoid output function multiplied by the delta of the output
	real delta = 2 * result * (1 - result) * (answer - result);


	for (int i = 0; i < output->n_inputs; i++) {
		perceptron_t *hidden = output->inputs[i];

		// i think this should be a sum of all the outputs with a connection to us
		real sum = 0;
		//for (int j = 0; j < output->n_inputs; j++) // if this line is uncommented total network error is reduced for some reason
			sum += output->weights[i] * delta;       // perhaps because it is altering the weights faster
													 // the weights appear to be more correct (smaller) with it commented though 

		// adjust connection weights for connection from input neuron to hidden neuron
		for (int j = 0; j < hidden->n_inputs; j++) {
			//hidden->weights[j] += LEARNING_CONSTANT * hidden->output * (1 - hidden->output) * sum * hidden->inputs[j]->output;
			hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->input[j] * (1 - hidden->input[j])) * sum * hidden->output;				
		}
	}

	// for each hidden neuron
	for (int i = 0; i < output->n_inputs; i++) {
		//perceptron_t *hidden = output->inputs[i];

		// adjust the weight of the connection from the hidden neuron to the output neuron
		output->weights[i] += LEARNING_CONSTANT * output->input[i] * delta;
	}

}
*/

void
perceptron_train(perceptron_t *output, real answer)
{
	real result = output->output;
	// derivative of sigmoid output function multiplied by the delta of the output
	real delta = result * (1 - result) * (answer - result);

	// for each hidden neuron
	for (int i = 0; i < output->n_inputs; i++) {
		perceptron_t *hidden = output->inputs[i];

		// adjust the weight of the connection from the hidden neuron to the output neuron
		output->weights[i] += LEARNING_CONSTANT * hidden->output * delta;
	}

	for (int i = 0; i < output->n_inputs; i++) {
		perceptron_t *hidden = output->inputs[i];

		// i think this should be a sum of all the outputs with a connection to us
		real sum = 0;
		//for (int j = 0; j < output->n_inputs; j++) // if this line is uncommented total network error is reduced for some reason
		sum += output->weights[i] * delta;       // perhaps because it is altering the weights faster
												 // the weights appear to be more correct (smaller) with it commented though 

												 // adjust connection weights for connection from input neuron to hidden neuron
		for (int j = 0; j < hidden->n_inputs; j++) {
			//hidden->weights[j] += LEARNING_CONSTANT * hidden->output * (1 - hidden->output) * sum * hidden->inputs[j]->output;
			hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->output * (1 - hidden->output)) * sum * hidden->inputs[j]->output;
		}
	}
}

mlp_layer_t **
mlp_net_new(int inputs, int hidden, int outputs)
{
	mlp_layer_t **layers = (mlp_layer_t **)malloc(sizeof(mlp_layer_t *) * 3);
	int i, j, k;
	// XXX - add bias neurons
	// create layers
	for (i = 0; i < 3; i++)
		layers[i] = mlp_layer_new();
	
	// create neurons and add them to layers
	for (i = 0; i < inputs; i++) {
		perceptron_t *p = perceptron_new(PERCEPTRON_RECEPTOR);
		mlp_layer_add(layers[0], p);
	}

	for (i = 0; i < hidden; i++) {
		perceptron_t *p = perceptron_new(PERCEPTRON_NEURON);
		mlp_layer_add(layers[1], p);
	}
	
	for (i = 0; i < outputs; i++) {
		perceptron_t *p = perceptron_new(PERCEPTRON_OUTPUT);
		mlp_layer_add(layers[2], p);
	}

	// link input neurons to hidden neurons
	for (i = 0; i < layers[0]->n_perceptrons; i++) {
		for (j = 0; j < layers[1]->n_perceptrons; i++) {
			perceptron_link(layers[0]->perceptrons[i], layers[1]->perceptrons[j]);
			// link hidden neurons to output neurons
			for (k = 0; k < layers[2]->n_perceptrons; k++)
				perceptron_link(layers[1]->perceptrons[j], layers[2]->perceptrons[k]);
		}
	}
}

void
mlp_net_compute(mlp_layer_t **layers, int *output)
{
	mlp_layer_compute(layers[0]);
	mlp_layer_transfer(layers[0]);
	mlp_layer_compute(layers[1]);
	mlp_layer_transfer(layers[1]);
	mlp_layer_compute(layers[2]);

	for (int i = 0; i < layers[2]->n_perceptrons; i++)
		output[i] = layers[2]->perceptrons[i]->output;
}
