# -*- coding: utf-8 -*-
"""
Created on Wed Mar 25 14:40:19 2015

@author: igordsm
"""
from __future__ import print_function
from __future__ import division


from trios.wop_matrix_ops import process_image, apply_loop, compare_images, compare_images_binary, process_image_ordered, process_one_image, count_pixels_mask

from trios.serializable import Serializable

import trios
import trios.shortcuts.persistence as p

import numpy as np
import scipy as sp
import scipy.ndimage

import inspect
import sys

import math

import multiprocessing
from functools import partial
import itertools

import logging
logger = logging.getLogger('trios.woperator')


def worker_eval(t):
    self, window, imgset, nprocs, procnumber, bin = t
    if not window is None:
        x_border = window.shape[1]//2
        y_border = window.shape[0]//2
    else:
        x_border = y_border = 0
    idx = [ i  for i in range(len(imgset)) if i % nprocs == procnumber]
    errs = []
    for i in idx:
        inp, out, msk = imgset[i]
        if trios.show_eval_progress:
            print('Testing', i, inp, out, file=sys.stderr)

        inp = sp.ndimage.imread(inp, mode='L')
        out = sp.ndimage.imread(out, mode='L')
        msk = p.load_mask_image(msk, inp.shape, self.window)
        res = self.apply(inp, msk)
        if bin:
            errs.append(compare_images_binary(out, msk, res, x_border, y_border))
        else:
            errs.append(compare_images(out, msk, res, x_border, y_border))
        del inp, out, msk, res
    return errs


class WOperator(Serializable):
    '''
    The WOperator class contains all the elements necessary to train and
    apply a local image operator to an image. The most important parts of
    a WOperator are:

    (i) its neighborhood (Window) of the operator;
    (ii) the classifier used (defined in the Classifier class).
    '''
    def __init__(self, window=None, cls=None, fext=None, batch=True):
        self.window = window
        if inspect.isclass(cls):
            self.classifier = cls(window)
        else:
            self.classifier = cls

        if inspect.isclass(fext):
            self.extractor = fext(window)
        else:
            self.extractor = fext

        self.trained = False
        self.batch = batch

    def train(self, imgset, kw={}):
        if self.classifier.partial:
            for i in range(1): #niters
                X, y = self.extractor.extract_dataset_batch(imgset)
                self.classifier.partial_train(X , y, kw)
        else:
            dataset = self.extractor.extract_dataset(imgset, self.classifier.ordered)
            self.classifier.train(dataset, kw)
        self.trained = True

    def apply(self, image, mask):
        if self.batch:
            if mask is None:
                mask = np.ones_like(image)
            res = np.zeros(image.shape, np.uint8)
            ww2 = max(self.window.shape[1]//2, 1)
            hh2 = max(self.window.shape[0]//2, 1)
            y, x = np.nonzero(mask[hh2:-hh2, ww2:-ww2])
            temp = self.extractor.temp_feature_vector()
            X = np.zeros((len(y), len(self.extractor)), temp.dtype)
            self.extractor.extract_batch(image[hh2:-hh2, ww2:-ww2], y, x, X)
            ypred = self.classifier.apply_batch(X)
            res[y+hh2,x+ww2] = ypred
            return res
        else:
            return apply_loop(self.window, image, mask, self.classifier, self.extractor)

    def eval(self, imgset, window=None, per_image=False, binary=False, procs=2):
        errors = []
        ll = list(zip(itertools.repeat(self), itertools.repeat(window), itertools.repeat(imgset), itertools.repeat(procs), range(procs), itertools.repeat(binary)))
        if trios.mp_support and procs > 1:
            ctx = multiprocessing.get_context('forkserver')
            errors_p = ctx.Pool(processes=procs)
            errors = errors_p.map(worker_eval, ll)
            errors_p.close()
            errors_p.join()
            del errors_p
            errors = list(itertools.chain(*errors))
        else:
            errors = worker_eval((self, window, imgset, 1, 0, binary))
        if binary:
            TP = sum([err[0] for err in errors])
            TN = sum([err[1] for err in errors])
            FP = sum([err[2] for err in errors])
            FN = sum([err[3] for err in errors])
            return TP, TN, FP, FN
        else:
            total_pixels = sum([err[1] for err in errors])
            total_errors = sum([err[0] for err in errors])
            if per_image:
                return float(total_errors)/total_pixels, errors
            return float(total_errors)/total_pixels


    def write_state(self, obj_dict):
        obj_dict['classifier'] = self.classifier.write(None)
        obj_dict['extractor'] = self.extractor.write(None)

        obj_dict['window'] = np.asarray(self.window)
        obj_dict['trained'] = self.trained
        obj_dict['batch'] = self.batch

    def set_state(self, obj_dict):
        self.classifier = Serializable.read(obj_dict['classifier'])
        self.extractor = Serializable.read(obj_dict['extractor'])

        self.window = obj_dict['window']
        if 'trained' in obj_dict:
            self.trained = obj_dict['trained']
        else:
            self.trained = True

        if 'batch' in obj_dict:
            self.batch = obj_dict['batch']
        else:
            self.batch = True

        if len(self.window.shape) == 1:
            self.window = self.window.reshape((self.window.shape[0], 1))



