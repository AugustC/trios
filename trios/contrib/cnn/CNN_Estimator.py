import tensorflow as tf
import math

import numpy as np
import sys
import os
from multiprocessing import Process
from CNN_TFClassifier import CNN_TFClassifier
# from subprocess import Popen, PIPE, STDOUT
import pickle
import glob

class CNN_Estimator():

	def __init__(self, input_shape, num_classes, learning_rates=[1e-4], 
		conv_layers=[([5, 5, 3, 32], [5, 5, 32, 64])], conv_strides=[[1, 1, 1, 1]], 
		model_dir='cnns', multilabel=False, num_epochs=50, batch_size=50, dropout_prob=0.5, 
		patience=3, num_proc=2):
		self.input_shape = input_shape
		self.conv_layers = conv_layers
		self.num_epochs = num_epochs
		self.batch_size = batch_size
		self.learning_rates = learning_rates
		self.dropout_prob = dropout_prob
		self.num_classes = num_classes
		self.model_dir = model_dir
		self.patience = patience
		self.multilabel = multilabel
		self.conv_strides = conv_strides
		self.num_proc = num_proc

	def cnn_dir(self, lr, conv_layers, conv_strides):
		cnn_dir = self.model_dir + "/" + ("cnn_lr_%lr" % lr) #+ str(conv_layers) +\
		 # "_conv_strides_" + str(conv_strides)
		for a_layer in conv_layers:
			cnn_dir += "_conv"
			for dim in a_layer:
				cnn_dir += "_" + str(dim)
				
		cnn_dir += "_strides"
		for dim in conv_strides:
			cnn_dir += "_" + str(dim)
		return cnn_dir

		return cnn_dir

	def summary_dir(self):
		return self.model_dir + '/summary.txt'


	def fit_cnn(self, x_train, y_train,  x_val, y_val, lr, conv_layers, conv_strides):
		cnn_dir = self.cnn_dir(lr, conv_layers, conv_strides)
		cnn = CNN_TFClassifier(self.input_shape, self.num_classes, model_dir=cnn_dir,  
			first_conv=conv_layers[0], second_conv=conv_layers[1],\
		num_epochs=self.num_epochs, batch_size=self.batch_size, learning_rate=lr, dropout_prob=self.dropout_prob, 
		patience=self.patience, conv_strides=conv_strides)
		score = cnn.fit(x_train, y_train, x_val, y_val)
		with open(self.summary_dir(), 'a') as f:
			f.write("%s\t%f\n" % (cnn_dir, score))

		with open(cnn_dir + '/my_cnn.pkl', 'wb') as output:
			pickle.dump(cnn, output, -1)
		# self.scores.append((cnn_dir, score))


	def fit(self, x_train, y_train, x_val, y_val):
		if not os.path.exists(self.model_dir):
			os.makedirs(self.model_dir)
		args = []
		self.scores = []
		for lr in self.learning_rates:
			for conv_layers in self.conv_layers:
				for conv_strides in self.conv_strides:
					args.append((x_train, y_train, x_val, y_val, lr, conv_layers, conv_strides))

		for i in range(0, len(args), self.num_proc):
			last_proc = min(i + self.num_proc, len(args))
			processes = []
			print("running procceses from %d to %d: " % (i, last_proc))
			for j in range(i, last_proc):
				p = Process(target=self.fit_cnn, args=args[j])
				p.start()
				processes.append(p)
			for p in processes:
				p.join() 

		self.delete_not_best_cnns()

		_, best_acc = self.best_cnn_dir_and_score()
		return best_acc

	def delete_not_best_cnns(self):
		best_dir, best_acc = self.best_cnn_dir_and_score()
		# summary_file = self.summary_dir()
		# cnn_dirs =  np.genfromtxt(summary_file, dtype=str, delimiter="\t", usecols=(0))
		for score in  self.scores:
			a_dir = score[1]
			if a_dir != best_dir:
				os.remove(a_dir + "/checkpoint")
				for cnn_file in glob.glob(a_dir + "/model*"):
					os.remove(cnn_file)


	def best_cnn_dir_and_score(self):
		summary_file = self.summary_dir()
		# MIGHT GEENRATE PROBLEM WHEN accuracies is a number
		accuracies =  np.genfromtxt(summary_file, dtype=None, delimiter="\t", usecols=(1))

		max_acc, index = max((accuracies[i], i) for i in range(len(accuracies)))
		cnn_dirs =  np.genfromtxt(summary_file, dtype=str, delimiter="\t", usecols=(0))

		return cnn_dirs[index], max_acc



	def read_best_model(self):
		best_cnn_dir, _ = self.best_cnn_dir_and_score()

		return best_cnn_dir + "/my_cnn.pkl"


	def best_classifier(self):
		best_model_dir = self.read_best_model()
		with open(best_model_dir, 'rb') as f:
			cnn = pickle.load(f)
			return cnn

	def predict(self, x):
		cnn = self.best_classifier()
		return cnn.predict(x)
