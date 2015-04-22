#ifndef NEURAL_NETWORK_HPP
#define NEURAL_NETWORK_HPP

/*Macro definitions*/
/*---------------------------------------------*/
/*---------------------------------------------*/

/*Included headers*/
/*---------------------------------------------*/
#include "errorlogger.hpp"
#include "CImg.h"
/*---------------------------------------------*/

/*Included dependencies*/
/*---------------------------------------------*/
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <assert.h>
#include <cstdlib>
#include <cassert>
#include <sstream>
/*---------------------------------------------*/

using namespace std;
using namespace cimg_library;

/*Header content*/
/*=============================================*/
namespace ANNconsts {
	const double eta = 0.1; // 0.0 = slow learner, 0.2 = medium learner, 1.0 = recless learner
	const double alpha = 0.25; //0.0  = no mometum, 0.5 = moderate momentum
	const double recent_average_smoothing_factor = 10000.0; // Number of training samples to average over
}

class Neuron;

typedef vector<Neuron> Layer;

struct Connection {
	double weight;
	double deltaWeight;
};

class Neuron {
	private:
		static double transfer_function(double x);
		static double transfer_function_derivative(double x);
		static double get_random_weight(void) { return (((double)rand() / double(RAND_MAX))/2.0); }
		double calculate_DOW(const Layer &next_layer) const;
		double m_output_value;
		vector<Connection> m_output_weights;
		unsigned m_my_index;
		double m_gradient;
	public:
		Neuron(unsigned numOutputs, unsigned my_index);
		void set_output_value(double val) { m_output_value = val; }
		double get_output_value(void) const { return m_output_value; }
		void feed_forward(const Layer &previous_layer);
		void calculate_output_gradients(double target_value);
		void calculate_hidden_gradients(const Layer &next_layer);
		void update_input_weights(Layer &previous_layer);
		vector<Connection>* get_connections() {return &m_output_weights;}
};

class Net {
	private:
		vector<Layer> m_layers;
		double m_error;
		double m_recent_average_error;
	public:
		Net(const vector<unsigned> &topology);
		void feed_forward(const vector<double> &input_values);
		void backpropogate(const vector<double> &target_values);
		void get_results(vector<double> &result_values) const;
		double get_recent_average_error(void) const { return m_recent_average_error; }
		double get_error(void) const { return m_error; }
		int store_weights(std::string filename);
		int load_weights(std::string filename);
};

void Neuron::update_input_weights(Layer &previous_layer) {
	for (unsigned n = 0; n < previous_layer.size(); ++n) {
		Neuron &neuron = previous_layer[n];
		double oldDeltaWeight = neuron.m_output_weights[m_my_index].deltaWeight;

		double newDeltaWeight =
				ANNconsts::eta * neuron.get_output_value() * m_gradient
				+ ANNconsts::alpha * oldDeltaWeight;

		neuron.m_output_weights[m_my_index].deltaWeight = newDeltaWeight;
		neuron.m_output_weights[m_my_index].weight += newDeltaWeight;
	}
}

double Neuron::calculate_DOW(const Layer &next_layer) const {
	double sum = 0.0;

	for (unsigned n = 0; n < next_layer.size() - 1; ++n) {
		sum += m_output_weights[n].weight * next_layer[n].m_gradient;
	}

	return sum;
}

void Neuron::calculate_hidden_gradients(const Layer &next_layer) {
	double dow = calculate_DOW(next_layer);
	m_gradient = dow * Neuron::transfer_function_derivative(m_output_value);
}

void Neuron::calculate_output_gradients(double target_value) {	
	double delta = target_value - m_output_value;
	m_gradient = delta * Neuron::transfer_function_derivative(m_output_value);
}

double Neuron::transfer_function(double x) {

	return tanh(x);
}

double Neuron::transfer_function_derivative(double x) {
	/* Hack for faster convergence */
	return (1.0 - (x * x * x * x * x));
}

void Neuron::feed_forward(const Layer &previous_layer) {
	double sum = 0.0;

	for (unsigned n = 0; n < previous_layer.size(); ++n) {
		sum += previous_layer[n].get_output_value() *
				previous_layer[n].m_output_weights[m_my_index].weight;
	}

	m_output_value = Neuron::transfer_function(sum);
}

Neuron::Neuron(unsigned numOutputs, unsigned my_index) {
	for (unsigned c = 0; c < numOutputs; ++c) {
		m_output_weights.push_back(Connection());
		m_output_weights.back().weight = get_random_weight();
		m_output_weights.back().deltaWeight = 0.00;
	}

	m_my_index = my_index;
}


void Net::get_results(vector<double> &result_values) const {
	result_values.clear();

	for (unsigned n = 0; n < m_layers.back().size() - 1; ++n) {
		result_values.push_back(m_layers.back()[n].get_output_value());
	}
}

void Net::backpropogate(const vector<double> &target_values) {

	Layer &outputLayer = m_layers.back();
	m_error = 0.0;

	for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
		double delta = target_values[n] - outputLayer[n].get_output_value();
		m_error += delta * delta;
	}
	m_error /= outputLayer.size() - 1; // get average error squared
	m_error = sqrt(m_error); // RMS

	m_recent_average_error =
			(m_recent_average_error * ANNconsts::recent_average_smoothing_factor + m_error)
			/ (ANNconsts::recent_average_smoothing_factor + 1.0);

	for (unsigned n = 0; n < outputLayer.size() - 1; ++n) {
		outputLayer[n].calculate_output_gradients(target_values[n]);
	}

	for (unsigned layerNum = m_layers.size() - 2; layerNum > 0; --layerNum) {
		Layer &hiddenLayer = m_layers[layerNum];
		Layer &next_layer = m_layers[layerNum + 1];

		for (unsigned n = 0; n < hiddenLayer.size(); ++n) {
			hiddenLayer[n].calculate_hidden_gradients(next_layer);
		}
	}

	for (unsigned layerNum = m_layers.size() - 1; layerNum > 0; --layerNum) {
		Layer &layer = m_layers[layerNum];
		Layer &previous_layer = m_layers[layerNum - 1];

		for (unsigned n = 0; n < layer.size() - 1; ++n) {
			layer[n].update_input_weights(previous_layer);
		}
	}
}

void Net::feed_forward(const vector<double> &input_values) {
	assert(input_values.size() == m_layers[0].size() - 1);

	for (unsigned i = 0; i < input_values.size(); ++i) {
		m_layers[0][i].set_output_value(input_values[i]);
	}

	for (unsigned layerNum = 1; layerNum < m_layers.size(); ++layerNum) {
		Layer &previous_layer = m_layers[layerNum - 1];
		for (unsigned n = 0; n < m_layers[layerNum].size() - 1; ++n) {
			m_layers[layerNum][n].feed_forward(previous_layer);
		}
	}
}

Net::Net(const vector<unsigned> &topology) {
	unsigned numLayers = topology.size();
	m_recent_average_error = 0;
	for (unsigned layerNum = 0; layerNum < numLayers; ++layerNum) {
		m_layers.push_back(Layer());
		unsigned numOutputs = layerNum == topology.size() - 1 ? 0 : topology[layerNum + 1];

		for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum) {
			m_layers.back().push_back(Neuron(numOutputs, neuronNum));
		}

		m_layers.back().back().set_output_value(1.0);
	}
}

int Net::store_weights(std::string filename) {
	std::ofstream weights (filename);
	if (! weights.is_open()) {
		std::cout << "Unable to open file: " << filename << std::endl;
		return -1;
	}

	weights << m_layers.size() << ' ';

	for (auto& layer : m_layers) {
		weights << layer.size() << ' ';
	}

	for (auto layer =  m_layers.begin(); layer != m_layers.end(); layer++) {
		for (auto neuron = layer->begin(); neuron != layer->end(); neuron++) {
			for (auto connection = neuron->get_connections()->begin(); connection != neuron->get_connections()->end(); connection++) {
				weights << connection->weight <<' '<< connection->deltaWeight<<' ';
			}
		}
	}
	return 0;
}
	
int Net::load_weights(std::string filename){
	std::ifstream weights (filename);

	if(!weights.good()) {
		std::cout << "File: " << filename << " does no exist!" << std::endl;
		return -1;
	}

	double weight;
	double delta_weight;

	unsigned int num_layers;
	unsigned int curr_layer_size;

	weights >> num_layers;

	if (num_layers != m_layers.size()){
		std::cout << "Layer topology not compatible with file, aborting!" << std::endl;
		return -1;
	}
	for (unsigned int i = 0; i < num_layers; i++) {
		weights >> curr_layer_size;
		if (curr_layer_size != m_layers[i].size()) {
			std::cout << "Layer topology not compatible with file, aborting!" << std::endl;
			std::cout << "Layer: " << i << " has the wrong number of neurons." << std::endl;
			return -1;
		}
	}

	for (auto layer =  m_layers.begin(); layer != m_layers.end(); layer++) {
		for (auto neuron = layer->begin(); neuron != layer->end(); neuron++) {
			for (auto connection = neuron->get_connections()->begin(); connection != neuron->get_connections()->end(); connection++) {
				weights >> weight >> delta_weight;
				connection->weight = weight;
				connection->deltaWeight = delta_weight;
			}
		}
	}
	return 0;
}

#endif