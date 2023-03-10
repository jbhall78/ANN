#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "perceptron.h"

#define LEARNING_CONSTANT 1.0

#define E 2.71828183

#define DROPOUT 0

#define TANH_SIGMOID 0

static real
_perceptron_initial_weight(int bias)
{
	int s;

/*	if (bias)
		s = 1;
	else */
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

/*
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
*/


void
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

void
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

	//perceptron->n_inputs++;
	perceptron->n_inputs = n;
}

void
perceptron_link(perceptron_t *perceptron, perceptron_t *link)
{
	perceptron_t **inputs, **outputs;
	real *weights, *input;
	int i;

	// add a dendrite to the axon
	/*
	outputs = (perceptron_t **)malloc(sizeof(perceptron_t *) * (perceptron->n_outputs + 1));
	for (i = 0; i < perceptron->n_outputs; i++)
		outputs[i] = perceptron->outputs[i];
	outputs[i] = link;
	if (perceptron->outputs != NULL)
		free(perceptron->outputs);
	perceptron->outputs = outputs;

	perceptron->n_outputs++;
	*/
	perceptron_outputs_resize(perceptron, perceptron->n_outputs + 1);
	perceptron->outputs[perceptron->n_outputs - 1] = link;
	
	// add an input dendrite to the link for back propagation
	perceptron_inputs_resize(link, link->n_inputs + 1);
	int last = link->n_inputs - 1;
	link->inputs[last] = perceptron;
	link->weights[last] = _perceptron_initial_weight(perceptron->type == PERCEPTRON_BIAS ? 1 : 0);
	link->input[last] = 0.0;
	
	/*
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
	*/
}

real
sigmoid(real sum)
{
#if TANH_SIGMOID == 1
	//return tanh(sum);
	//return sin(sum);
	return (1.0 - exp(-sum)) / (1.0 + exp(-sum));
	//return (2.0 / 1.0 + exp(-2.0 * sum)) - 1.0;
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
	return 1.0 - x*x;
	//return 1.0 - tanh(x)*tanh(x);
	//return 1.0 - ((pow(exp(x) - exp(-x), 2.0) / pow(exp(x) + exp(-x), 2.0)));
#else
	return x * (1 - x);
#endif
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
		perceptron->output = sigmoid(sum);
		//perceptron->output = 1.0 / (1.0 + exp(-sum));
		//perceptron->output = 1.0 / (1.0 + exp(-2 * sum));
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
/*			for (k = 0; k < p2->n_inputs; k++)
				if (p2->inputs[k] == p1)
					break;
					*/
			k = i;
			p2->input[k] = p1->output;
		}
	}
}

real
cost(real answer, real result)
{
	return answer - result;
	//return pow(answer - result, 2.0) / 10;
}

#if 1
void
mlp_layer_train(mlp_layer_t *layer, real *answers)
{
		perceptron_t *output = layer->perceptrons[0];

		static real *output_deltas = NULL;
	
		if (output_deltas == NULL)
			output_deltas = malloc(layer->n_perceptrons * sizeof(real));

		// compute derivitive of sigmoid function multiplied by the delta of the output for later use
		for (int i = 0; i < layer->n_perceptrons; i++) {
			real r = layer->perceptrons[i]->output;		
			output_deltas[i] = sigmoid_prime(r) * cost(answers[i], r);
		}

#if DROPOUT == 1
		int dropout = rand() % output->n_inputs;
#endif

		// loop over hidden layer
		for (int i = 0; i < output->n_inputs; i++) {
			perceptron_t *hidden = output->inputs[i];

			if (hidden->type == PERCEPTRON_BIAS)
				continue;

#if DROPOUT == 1
			if (i == dropout)
				continue;
#endif
		
			real sum = 0;
			// sum of all connections from output layer to us
			for (int x = 0; x < layer->n_perceptrons; x++) {
				real r = layer->perceptrons[x]->output;
				// why are these two lines not functionally equivilent? 
				//sum += layer->perceptrons[x]->weights[i] * sigmoid_prime(r) * cost(answers[x], r);
				sum += layer->perceptrons[x]->weights[i] * output_deltas[x];
			}
							
			for (int j = 0; j < hidden->n_inputs; j++) {
				hidden->weights[j] += LEARNING_CONSTANT * sigmoid_prime(hidden->output) * sum * hidden->input[j];
			}
		}

	// update weights of output layer
	for (int i = 0; i < layer->n_perceptrons; i++) {
		perceptron_t *output = layer->perceptrons[i];

		for (int j = 0; j < output->n_inputs; j++) {
			output->weights[j] += LEARNING_CONSTANT * output_deltas[i] * output->input[j];
		}
	}
}
#else
 /* 93% */
void
mlp_layer_train(mlp_layer_t *layer, real *answers)
{

	
	//for (int k = 0; k < layer->n_perceptrons; k++) {
	for (int k = 0; k < 1; k++) {
		perceptron_t *output = layer->perceptrons[k];

#if DROPOUT == 1
		int dropout = rand() % output->n_inputs;
#endif

		for (int i = 0; i < output->n_inputs; i++) {
			perceptron_t *hidden = output->inputs[i];

#if DROPOUT == 1
			if (i == dropout)
				continue;
#endif

	
			real sum = 0;

			// sum of all connections to us
			for (int x = 0; x < layer->n_perceptrons; x++) {
				for (int y = 0; y < layer->perceptrons[x]->n_inputs; y++) {
					if (layer->perceptrons[x]->inputs[y] == hidden) {
						real r = layer->perceptrons[x]->output;
						//sum += layer->perceptrons[x]->weights[y] * 2 * r * (1 - r) * (answers[x] - r); //delta;
						//sum += layer->perceptrons[x]->weights[y] * r * (1 - r) * (answers[x] - r); //delta;
						sum += layer->perceptrons[x]->weights[y] * sigmoid_prime(r) * (answers[x] - r); //delta;
						break;
					}
				}
			}
			
			
			for (int j = 0; j < hidden->n_inputs; j++) {
				//hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->output * (1 - hidden->output)) * sum * hidden->input[j]; //hidden->inputs[j]->output;

				//hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->input[j] * (1 - hidden->input[j])) * sum * hidden->output;
				//hidden->weights[j] += LEARNING_CONSTANT * (hidden->input[j] * (1 - hidden->input[j])) * sum * hidden->output;
				
				
				//hidden->weights[j] += LEARNING_CONSTANT * sigmoid_prime(hidden->input[j]) * sum * hidden->output;
				hidden->weights[j] += LEARNING_CONSTANT * sigmoid_prime(hidden->output) * sum * hidden->input[j];
			}
		}

	}

	for (int k = 0; k < layer->n_perceptrons; k++) {
		perceptron_t *output = layer->perceptrons[k];

		real result = output->output;

		// partial derivative of sigmoid output function multiplied by the delta of the output
		//real delta = 2 * result * (1 - result) * (answers[k] - result);
		//real delta = result * (1 - result) * (answers[k] - result);
		real delta = sigmoid_prime(result) * (answers[k] - result);


		for (int i = 0; i < output->n_inputs; i++) {
			//perceptron_t *hidden = output->inputs[i];
			//real r = hidden->output;
			//real delta = r * (1 - r);// *(answers[k] - r);
			
			// adjust the weight of the connection from the hidden neuron to the output neuron
			//output->weights[i] += LEARNING_CONSTANT * hidden->output * delta;
			output->weights[i] += LEARNING_CONSTANT * output->input[i] * delta;// *result;
			//output->weights[i] += LEARNING_CONSTANT * delta *result;
			//output->weights[i] += LEARNING_CONSTANT * delta * (answers[k] - result);

																			   //output->weights[i] += LEARNING_CONSTANT * delta * result;
		}
	}
}
#endif

#if 0
void
mlp_layer_train(mlp_layer_t *layer, real *answers)
{
	for (int k = 0; k < layer->n_perceptrons; k++) {
		perceptron_t *output = layer->perceptrons[k];

		for (int i = 0; i < output->n_inputs; i++) {
			perceptron_t *hidden = output->inputs[i];

			real sum = 0;

			// sum of all connections to us
			/*
			for (int x = 0; x < layer->n_perceptrons; x++) {
				for (int y = 0; y < layer->perceptrons[x]->n_inputs; y++) {
					if (layer->perceptrons[x]->inputs[y] == hidden) {
						real r = layer->perceptrons[x]->output;
						sum += layer->perceptrons[x]->weights[y] * 2 * r * (1 - r) * (answers[x] - r); //delta;
					}
				}
			}
			*/

			for (int j = 0; j < hidden->n_inputs; j++) {
				//hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->output * (1 - hidden->output)) * sum * hidden->input[j]; //hidden->inputs[j]->output;
				
				hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->input[j] * (1 - hidden->input[j])) * sum * hidden->output;
				//hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->input[j] * (1 - hidden->input[j])) * sum;
			}
		}
		
	}

	for (int k = 0; k < layer->n_perceptrons; k++) {
		perceptron_t *output = layer->perceptrons[k];

		real result = output->output;

		// partial derivative of sigmoid output function multiplied by the delta of the output
		real delta = 2 * result * (1 - result) * (answers[k] - result);
		

		for (int i = 0; i < output->n_inputs; i++) {
			perceptron_t *hidden = output->inputs[i];
			//real r = hidden->output;
			//real delta = 2 * r * (1 - r) * (answers[k] - r);

			// adjust the weight of the connection from the hidden neuron to the output neuron
			output->weights[i] += LEARNING_CONSTANT * hidden->output * delta;
			//output->weights[i] += LEARNING_CONSTANT * delta * result;
		}
	}
}
#endif


/*
void
mlp_layer_train(mlp_layer_t *layer, real *answers)
{
	for (int k = 0; k < layer->n_perceptrons; k++) {
		perceptron_t *output = layer->perceptrons[k];

		real result = output->output;

		// partial derivative of sigmoid output function multiplied by the delta of the output
		real delta = 2 * result * (1 - result) * (answers[k] - result);

		// update output weights??

		for (int i = 0; i < output->n_inputs; i++) {
			perceptron_t *hidden = output->inputs[i];

			real sum = 0;

			// sum of all connections to us
			for (int x = 0; x < layer->n_perceptrons; x++) {
				for (int y = 0; y < layer->perceptrons[x]->n_inputs; y++) {
					if (layer->perceptrons[x]->inputs[y] == hidden)
						sum += layer->perceptrons[x]->weights[y] * delta;
				}
			}

			for (int j = 0; j < hidden->n_inputs; j++) {
				//hidden->weights[j] += LEARNING_CONSTANT * hidden->output * (1 - hidden->output) * sum * hidden->inputs[j]->output;
				hidden->weights[j] += LEARNING_CONSTANT * (2 * hidden->output * (1 - hidden->output)) * sum * hidden->inputs[j]->output;
			}
		}

		// update output weights (not sure if i should do this before or after we sum them above)
		for (int i = 0; i < output->n_inputs; i++) {
			perceptron_t *hidden = output->inputs[i];

			// adjust the weight of the connection from the hidden neuron to the output neuron
			output->weights[i] += LEARNING_CONSTANT * hidden->output * delta;
		}



	}



}
*/
/*
void
perceptron_train(perceptron_t *output, real answer)
{
	real result = output->output;
	// derivative of sigmoid output function multiplied by the delta of the output
	real delta = 2 * result * (1 - result) * (answer - result);
	//real delta = sigmoid_prime(result) * (answer - result);

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
			//hidden->weights[j] += LEARNING_CONSTANT * sigmoid_prime(hidden->output) * sum * hidden->inputs[j]->output;
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
*/