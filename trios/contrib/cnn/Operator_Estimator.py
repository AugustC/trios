import trios
from trios.feature_extractors import RAWFeatureExtractor
from sklearn.model_selection import train_test_split
import numpy as np
from CNN_Estimator import CNN_Estimator
from TFClassifier import TFClassifier
import os

from sklearn.metrics import recall_score
from sklearn.metrics import precision_score


class Operator_Estimator():

    def __init__(self, output_dir, min_mae_variance=0.001, start_win_height=15, start_win_width=15, 
        window_increment=10, max_win_side = 45):

        self.min_mae_variance = min_mae_variance
        self.start_win_height = start_win_height
        self.start_win_width = start_win_width
        self.max_win_side = max_win_side
        self.window_increment = window_increment
        self.output_dir = output_dir

    def fit(self, images_set):
        best_accuracy = 0.
        win_height = self.start_win_height
        win_width = self.start_win_width

        learning_rates = [1e-4, 1e-5]
        conv_layers = [([5, 5, 1, 32], [5, 5, 32, 64])]
        conv_strides = [[1, 2, 2, 1]]

        if not os.path.exists(self.output_dir):
            os.makedirs(self.output_dir)

        while win_height <= self.max_win_side and win_width <= self.max_win_side:  
            window = np.ones((win_height, win_width), np.uint8)
            fext = RAWFeatureExtractor(window)
            x, y = fext.extract_dataset(images_set, True)

            y = y / 255
            y = np.reshape(y, (-1, 1))
            x = np.reshape(x, (-1, win_height, win_width, 1))
            x_train, x_val, y_train, y_val = train_test_split(x, y, test_size=0.2)

            images_shape = [win_height, win_width, 1]
            num_classes = 1

            model_dir = self.output_dir + ("/win_%dx%d" % (win_height, win_width))

            cnn_estimator = CNN_Estimator(images_shape, num_classes,
                            learning_rates=learning_rates, conv_layers=conv_layers, conv_strides=conv_strides,
                            model_dir=model_dir, multilabel=False, num_proc=4, num_epochs=30, patience=3)

            cnn_estimator.fit(x_train, y_train, x_val, y_val)
            current_accuracy, rec, prec = accuracy(cnn_estimator.predict(x_val).astype(int), y_val.astype(int))            
            del x_train, x_val, y_train, y_val
            diff = current_accuracy - best_accuracy

            with open(self.output_dir+"/summary.txt", "a") as f:
                f.write("%s\t%f\t%f\t%f\t%f\t%f\n"% (model_dir, current_accuracy, rec[0], rec[1], prec[0], prec[1]))

            if diff > self.min_mae_variance:
                best_accuracy = current_accuracy
                best_estimator = cnn_estimator
            else:
                if current_accuracy > best_accuracy:
                    best_accuracy = current_accuracy
                    best_estimator = cnn_estimator
                break                
            win_height += self.window_increment
            win_width += self.window_increment
        cnn_classifier = best_estimator.best_classifier()
        best_window = np.ones((best_estimator.input_shape[0], best_estimator.input_shape[1]), np.uint8)

        op_tf = trios.WOperator(best_window, TFClassifier(cnn_classifier), RAWFeatureExtractor(best_window), batch=True)
        return op_tf



def accuracy(predictions, labels):
  return (np.sum(predictions == labels)
          / float(predictions.shape[0])) , recall_score(labels, predictions, average=None),\
  precision_score(labels, predictions, average=None)
