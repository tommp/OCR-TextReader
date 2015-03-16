#ifndef NEURAL_NETWORK_HPP
#define NEURAL_NETWORK_HPP

/*Macro definitions*/
/*---------------------------------------------*/
/*---------------------------------------------*/

/*Included headers*/
/*---------------------------------------------*/
#include "errorlogger.hpp"
/*---------------------------------------------*/

/*Included dependencies*/
/*---------------------------------------------*/
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <cmath>
/*---------------------------------------------*/

using namespace cimg_library;

/*Header content*/
/*=============================================*/
namespace ANNconsts {
	const float eta = 0.1; // 0.0 = slow learner, 0.2 = medium learner, 1.0 = recless learner
	const float alpha = 0.5; //0.0  = no mometum, 0.5 = moderate momentum
}
class Neuron;
class Network;
typedef std::vector<Neuron> Layer;

class Connection {
public:
	float weight;
	float delta_weight;
	Connection();
};

Connection::Connection(){
	this->weight = ((float)rand()) / ((float)(RAND_MAX));
	this->delta_weight = ((float)rand()) / ((float)(RAND_MAX));
}

class Neuron{
private:
	float outputvalue;
	unsigned int neuron_index;
	float gradient;
	std::vector<Connection> output_weights;
public:
	Neuron(unsigned int number_of_outputs, unsigned int neuron_index);
	float tranfer_function(float value);
	float tranfer_function_derivative(float value);
	void set_output(float new_outputvalue) {this->outputvalue = new_outputvalue;};
	float get_output() const{return outputvalue;};
	void feed_forward(const Layer& parent_neurons);
	void calculate_output_gradients(float target_value);
	void calculate_gradients(const Layer& next_layer);
	float calculate_DOW(const Layer& next_layer) const;
	void update_input_weights(Layer& previous_layer);
};

class Network{
private:
	std::vector<Layer> network_layers;
	float error_sum;
public:
	Network(const std::vector<unsigned int>& topology);
	void feed_forward(const std::vector<float>& input_values);
	void back_propogation(const std::vector<float>& target_values);
	void get_results(std::vector<float>& result_values)const;
};

/* Neuron members */

void Neuron::update_input_weights(Layer& previous_layer) {

	for (unsigned int neuron_number = 0; neuron_number < previous_layer.size(); neuron_number++) {
		Neuron& current_neuron = previous_layer[neuron_number];

		float old_delta_weight = current_neuron.output_weights[neuron_index].delta_weight;

		float new_delta_weight = ANNconsts::eta * current_neuron.get_output() *gradient +
								ANNconsts::alpha * old_delta_weight;

		current_neuron.output_weights[neuron_index].delta_weight = new_delta_weight;
		current_neuron.output_weights[neuron_index].weight += new_delta_weight;
	}
}

float Neuron::calculate_DOW(const Layer& next_layer) const {

	float sum = 0.0;

	for (unsigned int neuron_number = 0; neuron_number < next_layer.size() - 1; neuron_number++) {
		sum += output_weights[neuron_number].weight * next_layer[neuron_number].gradient;
	}

	return sum;
}

void Neuron::calculate_gradients(const Layer& next_layer) {
	
	float dow = calculate_DOW(next_layer);
	gradient = dow * tranfer_function_derivative(outputvalue);
}

void Neuron::calculate_output_gradients(float target_value) {
	float delta = target_value - outputvalue;
	gradient = delta * tranfer_function_derivative(outputvalue);

}

float Neuron::tranfer_function(float value) {
	return tanh(value);
}


float Neuron::tranfer_function_derivative(float value) {
	return 1.0 - value*value;
}

void Neuron::feed_forward(const Layer& parent_neurons) {

	float sum = 0;

	for (unsigned int neuron_number = 0; neuron_number < parent_neurons.size(); neuron_number++) {
		sum += parent_neurons[neuron_number].get_output() * output_weights[neuron_index].weight;
	}

	outputvalue = tranfer_function(sum);
}

Neuron::Neuron(unsigned int number_of_outputs, unsigned int neuron_index) {

	for (unsigned int current_output = 0; current_output < number_of_outputs; current_output++) {
		output_weights.push_back(Connection());
	}
	this->neuron_index = neuron_index;
}

/* Network members */

void Network::get_results(std::vector<float>& result_values)const {
	result_values.clear();

	for (unsigned int output_number = 0; output_number < network_layers.back().size() - 1; output_number++) {
		result_values.push_back(network_layers.back()[output_number].get_output());
	}
}

void Network::back_propogation(const std::vector<float>& target_values) {

	Layer& output_layer = network_layers.back();
	error_sum = 0.0;

	double current_error = 0.0;

	for (unsigned int output_number = 0; output_number < output_layer.size()-1; output_number++) {
		current_error = target_values[output_number] - output_layer[output_number].get_output();
		error_sum += current_error;
	}

	error_sum /= output_layer.size() - 1;
	error_sum = sqrt(error_sum); /*RMS*/

	for (unsigned int output_number = 0; output_number < output_layer.size()-1; output_number++) {
		output_layer[output_number].calculate_output_gradients(target_values[output_number]);
	}

	for (unsigned int layer_number = network_layers.size() - 2; layer_number > 0; layer_number--) {

		Layer& current_layer = network_layers[layer_number];
		Layer& next_layer = network_layers[layer_number + 1];

		for (unsigned int neuron_number = 0; neuron_number < current_layer.size(); neuron_number++) {

			current_layer[neuron_number].calculate_gradients(next_layer);
		}
	}

	for (unsigned int layer_number = network_layers.size() - 1; layer_number > 0; layer_number--) {

		Layer& current_layer = network_layers[layer_number];
		Layer& previous_layer = network_layers[layer_number - 1];

		for (unsigned int neuron_number = 0; neuron_number < current_layer.size(); neuron_number++) {
			current_layer[neuron_number].update_input_weights(previous_layer);
		}

	}
}

void Network::feed_forward(const std::vector<float>& input_values) {

	/* Set input values */
	for (unsigned int input_number = 0; input_number < input_values.size(); input_number++) {
		network_layers[0][input_number].set_output(input_values[input_number]);
	}

	/* Feed forward */
	for (unsigned int layer_number = 1; layer_number <network_layers.size()-1; layer_number++) {
		Layer& previous_layer = network_layers[layer_number - 1];
		for (unsigned int neuron_number = 0; neuron_number < network_layers.size() - 1; neuron_number++) {
			network_layers[layer_number][neuron_number].feed_forward(previous_layer);
		}
	}
}

Network::Network(const std::vector<unsigned int>& topology) {

	unsigned int number_of_layers = topology.size();
	unsigned int number_of_output_neurons = 0;

	for (unsigned int layer_number = 0; layer_number < number_of_layers; layer_number++) {

		network_layers.push_back(Layer());

		if (layer_number == number_of_layers - 1) {
			number_of_output_neurons = 0;
		} 
		else {
			number_of_output_neurons = topology[layer_number + 1];
		}

		for (unsigned int neuron_number = 0; neuron_number <= topology[layer_number]; neuron_number++) {

			network_layers.back().push_back(Neuron(number_of_output_neurons, neuron_number));
		}

		network_layers.back().back().set_output(1.0);
	}
}

void train_network(Network& net) {
	
}

#endif