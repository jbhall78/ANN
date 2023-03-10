#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include <random>

template<int n_inputs>
class Perceptron {
public:
	float weights[n_inputs+1];
	float learning_constant;
	int debug = 0;
public:
	Perceptron(float learning_constant)
	{
		this->learning_constant = learning_constant;
#if 0 // initialize weights with random values
		srand(1337);
		
		for (int i = 0; i < n_inputs+1; i++) {
			weights[i] = (float)(rand() / (float)(RAND_MAX));
		}
#else // initialize weights with zero. this makes our answers simpler
		for (int i = 0; i < n_inputs + 1; i++) {
			weights[i] = 0;
		}
#endif
		/*
		printf("neuron weights initialized to: [  ");
		for (int i = 0; i < n_inputs + 1; i++) {
			printf("%f  ", weights[i]);
		}
		printf(" ]\n");
		*/
	}

	int output(float *inputs) {
		float sum = 0.0;
		int i;
		for (i = 0; i < n_inputs; i++) {
			sum += inputs[i] * weights[i];
		}
		// figure in bias
		sum += 1.0 * weights[i];
		if (debug) {
			printf("sum: %f, [", sum);
			for (i = 0; i < n_inputs; i++) {
				printf("%.2f*w:%.2f + ", inputs[i], weights[i]);
			}
			printf(" %.2f*w:%.2f]", 1.0, weights[i]);
			printf("\n");
		}
		if (sum > 0.0)
			return 1;
		else
			return -1;
	}



	int learn(float *inputs, int answer)
	{
		int i;
		int guess = this->output(inputs);
		if ((answer - guess) != 0) {
			float error = (float)(answer - guess);
			if (debug)
				printf("error: %.0f [", error);
			// determine the direction/sign of the error
			if ((answer - guess) == 0)
				error = 0;
			else if (error < 0)
				error = -1;
			else
				error = 1;
			for (i = 0; i < n_inputs; i++) {
				//weights[i] += learning_constant * error * inputs[i]; //from example

				weights[i] += learning_constant * error * inputs[i];
				if (debug) {
					printf("%f, ", weights[i]);
				}
			}

			// calculate bias
			weights[i] += learning_constant * error * 1.0;
			if (debug)
				printf("%f]\n", weights[i]);
		}
		return guess;
	}
};

#endif
