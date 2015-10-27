import trios
from trios.feature_extractors import RAWFeatureExtractor, RAWBitFeatureExtractor
from trios.classifiers import SKClassifier

import sklearn
from sklearn.tree import DecisionTreeClassifier

import numpy as np

import tempfile
import gzip
import nose
from nose.tools import assert_equal
import cPickle as pickle

def compare_window(win1, win2):
	assert win1.shape[0] == win2.shape[0]
	assert win1.shape[1] == win2.shape[1]
	for i in range(win1.shape[0]):
		for j in range(win1.shape[1]):
			assert win1[i, j] == win2[i, j]
			

def compare_raw_fe(ext, ext2):
	assert ext.parallel == ext2.parallel
	compare_window(ext.window, ext2.window)

def test_pickle_rawfe():
	win = np.ones((9, 2), np.uint8) 
	win[4, 1] = 0
	ext = RAWFeatureExtractor(win)
	temp = tempfile.NamedTemporaryFile(delete=False)
	name = temp.name
	temp.close()
	
	gz = gzip.GzipFile(name, mode='w+b')
	pickle.dump(ext, gz, -1)
	gz.close()
	
	gz = gzip.GzipFile(name, mode='rb')
	ext2 = pickle.load(gz)
	gz.close()
	compare_raw_fe(ext, ext2)

def test_pickle_rawbitfe():
	win = np.ones((9, 2), np.uint8) 
	win[4, 1] = 0
	ext = RAWBitFeatureExtractor(win)
	temp = tempfile.NamedTemporaryFile(delete=False)
	name = temp.name
	temp.close()
	
	gz = gzip.GzipFile(name, mode='w+b')
	pickle.dump(ext, gz, -1)
	gz.close()
	
	gz = gzip.GzipFile(name, mode='rb')
	ext2 = pickle.load(gz)
	gz.close()
	compare_raw_fe(ext, ext2)
	
def test_pickle_woperator_tree():
	win = np.ones((9, 2), np.uint8) 
	wop = trios.WOperator(win, SKClassifier(DecisionTreeClassifier()), RAWFeatureExtractor)
	
	
	temp = tempfile.NamedTemporaryFile(delete=False)
	name = temp.name
	temp.close()
	gz = gzip.GzipFile(name, mode='w+b')
	pickle.dump(wop, gz, -1)
	gz.close()
	
	gz = gzip.GzipFile(name, mode='rb')
	wop2 = pickle.load(gz)
	gz.close()

	compare_window(wop.window, wop2.window)
	compare_raw_fe(wop.extractor, wop2.extractor)
	assert_equal(wop.trained, wop2.trained)
	assert_equal(type(wop.classifier.cls), type(wop2.classifier.cls))
	