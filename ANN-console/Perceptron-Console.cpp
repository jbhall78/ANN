// ANN-console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <random>

#include "perceptron.h"

#include "mathlib.h"

int main()
{
	float point[2];
#if 1
	Perceptron<2> *perceptron = new Perceptron<2>(0.1);
	vec2_t v;

	int total = 10000;
	int right = 0, wrong = 0, totalright = 0;
	printf("\n>>> training 2x linear problem using a 640x480 grid\n\n");
	for (int i = 0; i < total; i++) {
		////point[0] = (float)(rand() % 640) / (float)640.0;
		////point[1] = (float)(rand() % 480) / (float)480.0;
		point[0] = (float)(rand() % 640) - (float)320.0;
		point[1] = (float)(rand() % 480) - (float)240.0;
		// why doesnt this work?
		//point[0] = (float)(((float)1.0 / (float)RAND_MAX) * (float)rand()) - (float)0.5;
		//point[1] = (float)(((float)1.0 / (float)RAND_MAX) * (float)rand()) - (float)0.5;
		int answer = 1;
		if (point[1] < 2 * point[0])
		//if (point[0] < 160.0)
			answer = -1;

		int guess = perceptron->learn(point, answer);
		if (guess == answer) {
			right++;
			totalright++;
		}
		else
			wrong++;

		if (i > 0) {
			/*
			if (i < 20) {
				printf("[%d] answer = %s\n", i, (guess == answer) ? "right" : "wrong");
			}
			*/
			if (i % 1000 == 0) {
				printf("[%d]  percent correct: %.2f%%  ", i, (100.0 / 1000.0 * (float)right));
				printf("weights: [  %.2f  %.2f  %.2f  ]\n", perceptron->weights[0], perceptron->weights[1], perceptron->weights[2]);
				right = 0;
			}
		}
	}
	printf("[%d] percent correct: %.2f%%  ", total, (100.0 / 1000.0 * (float)right));
	printf("weights: [  %.2f  %.2f  %.2f  ]\n", perceptron->weights[0], perceptron->weights[1], perceptron->weights[2]);
	
	printf("\ntotal error: %.2f%%", 100.0 - (100.0 / (float)total * (float)totalright));

	printf("\n\n\n\n\n\n\n");

	printf("Press <ENTER> to continue.");
	getc(stdin);

	/*
	ann->debug = 1;
	float point[2];  
	while (1) {
		point[0] = (float)(rand() % 640) - (float)320.0;
		point[1] = (float)(rand() % 480) - (float)240.0;

		printf("guess: %d (%f, %f)\n", ann->output(point), point[0] + 320.0, point[1] + 240.0);
		Sleep(5000);
	}*/

#if 0
	point[0] = point[1] = 0.0;
	perceptron->debug = 1;
	printf("guess: %d (%f, %f)\n", perceptron->output(point), point[0], point[1]);

	point[0] = point[1] = 64.0;
	printf("guess: %d (%f, %f)\n", perceptron->output(point), point[0], point[1]);

	point[0] = point[1] = 320.0;
	printf("guess: %d (%f, %f)\n", perceptron->output(point), point[0], point[1]);
#endif
#endif

#if 1
	for (int i = 0; i < 79; i++) printf("-"); printf("\n");
	Perceptron<2> *nand = new Perceptron<2>(0.1);
	struct nand_truth_table_s {
		float point[2];
		int answer;
	} nand_truth_table[] = {
		{ { 0, 0 }, 1 },
		{ { 0, 1 }, 1 },
		{ { 1, 0 }, 1 },
		{ { 1, 1 }, -1 }
	};
	nand->debug = 0;
	int epochs = 36;
	printf(">>> NAND nueron with initial weights: [ %f, %f, %f ]\n", nand->weights[0], nand->weights[1], nand->weights[2]);
	printf(">>> ... training with %d epochs. done.\n", epochs);
	for (int j = 0; j < epochs; j++) {
		for (int i = 0; i < sizeof(nand_truth_table) / sizeof(nand_truth_table[0]); i++) {
			nand->learn(nand_truth_table[i].point, nand_truth_table[i].answer);
		}
	}
	nand->debug = 0;
	printf("\n>>> NAND final weights: [ %f, %f, %f ]\n", nand->weights[0], nand->weights[1], nand->weights[2]);
	printf("\n>>> NAND truth table:\n-------------------------\n");
	for (int i = 0; i < sizeof(nand_truth_table) / sizeof(nand_truth_table[0]); i++) {
		printf("[%d !& %d] answer=%d guess=%d\n", 
			(nand_truth_table[i].point[0] > 0) ? 1 : 0,
			(nand_truth_table[i].point[1] > 0) ? 1 : 0, 
			(nand_truth_table[i].answer > 0) ? 1 : 0, 
			(nand->output(nand_truth_table[i].point) > 0) ? 1 : 0);
	}
	
	for (int i = 0; i < 79; i++) printf("-"); printf("\n");
	printf("\n\n\n\n\n\n\n\n\n");

	printf("Press <ENTER> to continue.");
	getc(stdin);

#endif

#if 1
	for (int i = 0; i < 79; i++) printf("-"); printf("\n");
	Perceptron<2> *_and = new Perceptron<2>(0.1);
	struct _and_truth_table_s {
		float point[2];
		int answer;
	} _and_truth_table[] = {
		{ { 0, 0 }, -1 },
		{ { 0, 1 }, -1 },
		{ { 1, 0 }, -1 },
		{ { 1, 1 }, 1 }
	};
	_and->debug = 0;
	epochs = 36;
	printf(">>> AND neuron with initial weights: [ %f, %f, %f ]\n", _and->weights[0], _and->weights[1], _and->weights[2]);
	printf(">>> ... training with %d epochs. done.\n", epochs);
	for (int j = 0; j < epochs; j++) {
		for (int i = 0; i < sizeof(_and_truth_table) / sizeof(_and_truth_table[0]); i++) {
			_and->learn(_and_truth_table[i].point, _and_truth_table[i].answer);
		}
	}
	printf("\n>>> AND final weights: [ %f, %f, %f ]\n", _and->weights[0], _and->weights[1], _and->weights[2]);
	printf("\n>>> AND truth table:\n------------------------\n");
	for (int i = 0; i < sizeof(_and_truth_table) / sizeof(_and_truth_table[0]); i++) {
		printf("[%d & %d] answer=%d guess=%d\n",
			(_and_truth_table[i].point[0] > 0) ? 1 : 0, 
			(_and_truth_table[i].point[1] > 0) ? 1 : 0, 
			(_and_truth_table[i].answer > 0) ? 1 : 0,
			(_and->output(_and_truth_table[i].point) > 0) ? 1 : 0);
	}

	for (int i = 0; i < 79; i++) printf("-"); printf("\n");
	printf("\n\n\n\n\n\n\n\n\n");

	printf("Press <ENTER> to continue.");
	getc(stdin);

#endif
	
#if 1
	for (int i = 0; i < 79; i++) printf("-"); printf("\n");
	Perceptron<2> *_or = new Perceptron<2>(0.1);
	struct _or_truth_table_s {
		float point[2];
		int answer;
	} _or_truth_table[] = {
		{ { 0, 0 }, -1 },
		{ { 0, 1 }, 1 },
		{ { 1, 0 }, 1 },
		{ { 1, 1 }, 1 }
	};
	_or->debug = 0;
	epochs = 36;
	printf(">>> OR neuron with initial weights: [ %f, %f, %f ]\n", _or->weights[0], _or->weights[1], _or->weights[2]);
	printf(">>> ... training with %d epochs. done.\n", epochs);
	for (int j = 0; j < epochs; j++) {
		for (int i = 0; i < sizeof(_or_truth_table) / sizeof(_or_truth_table[0]); i++) {
			_or->learn(_or_truth_table[i].point, _or_truth_table[i].answer);
		}
	}
	printf("\n>>> OR final weights: [ %f, %f, %f ]\n", _or->weights[0], _or->weights[1], _or->weights[2]);
	printf("\n>>> OR truth table:\n------------------------\n");
	for (int i = 0; i < sizeof(_or_truth_table) / sizeof(_or_truth_table[0]); i++) {
		printf("[%d | %d] answer=%d guess=%d\n", 
			(_or_truth_table[i].point[0] > 0) ? 1 : 0, 
			(_or_truth_table[i].point[1] > 0) ? 1 : 0,
			(_or_truth_table[i].answer > 0) ? 1 : 0,
			(_or->output(_or_truth_table[i].point) > 0) ? 1 : 0);
	}

	for (int i = 0; i < 79; i++) printf("-"); printf("\n");
	printf("\n\n\n\n\n\n\n\n\n");

#endif

	printf("Press <ENTER> to continue.");
	getc(stdin);
	
    return 0;
}