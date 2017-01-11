from trios.feature_extractors import RAWFeatureExtractor
from trios import FeatureExtractor
from sklearn.metrics.pairwise import rbf_kernel
from numpy.linalg import eigh
import numpy as np
import numpy.random

from libc.math cimport pow as cpow
from cython.parallel import prange
cimport cython
cimport numpy as np

@cython.boundscheck(False)
@cython.wraparound(False)
@cython.cdivision(True)
cpdef np.ndarray fast_poly_kernel(double[:,:] X, double[:,:] Y, int degree=3):
    cdef int total_pix = X.shape[0] * Y.shape[0]
    cdef int i, j, k
    cdef double[:,:] mat = np.dot(X, Y.T)

    with nogil:
        for k in prange(total_pix):
            i = k / Y.shape[0]
            j = k % Y.shape[0]
            mat[i, j] = cpow(mat[i, j]/X.shape[1] + 1, degree)
    #mat /= X.shape[1]
    #mat += 1
    #mat **= degree

    return np.asarray(mat)


class NystroemFeatures(FeatureExtractor):
    def __init__(self, window=None, imgset=None, n_components=100, kernel='poly', degree=3, gamma=1, **kw):
        super().__init__(window, **kw)
        self.n_components = n_components
        self.kernel = kernel
        self.degree = degree
        self.gamma = gamma
        self.dtype = np.float32

        if self.window is not None:
            self.estimate_phi(imgset)


    def estimate_phi(self, imgset):
        self.rawfext = RAWFeatureExtractor(self.window, batch_size=self.n_components)
        self.rawpat = self.rawfext.temp_feature_vector()
        self.cached_rawbatch = None
        self.scaled = np.zeros(self.rawpat.shape[0], np.float32)

        Xtr, y = self.rawfext.extract_dataset(imgset, ordered=True)

        self.mean = Xtr.mean(axis=0)
        self.std = Xtr.std(axis=0)
        self.std[self.std == 0] = 1e-12

        # get n_components random rows from Xtr
        idx = np.random.permutation(Xtr.shape[0])
        Xbasis = (Xtr[idx[:self.n_components]] - self.mean)/self.std
        # compute X_mm (kernel betweem Xtr elements)
        if self.kernel == 'poly':
            K_mm = fast_poly_kernel(Xbasis, Xbasis, degree=self.degree)
        else:
            K_mm = rbf_kernel(Xbasis, gamma=self.gamma)
        # Compute eigenvalues/vectors of X_mm
        l, v = eigh(K_mm)
        l = np.maximum(l, 1e-12)
        # Compute approx feature map
        self.phi_map = np.diag(1/np.sqrt(l)).dot(v.T).astype(self.dtype)
        self.basis = Xbasis
        self.idx = idx

    def write_state(self, obj_dict):
        super().write_state(obj_dict)
        obj_dict['n_components'] = self.n_components
        obj_dict['kernel'] = self.kernel
        obj_dict['degree'] = self.degree
        obj_dict['gamma'] = self.gamma

        obj_dict['basis'] = self.basis
        obj_dict['mean'] = self.mean
        obj_dict['std'] = self.std
        obj_dict['phi'] = self.phi_map

    def set_state(self, obj_dict):
        super().set_state(obj_dict)
        self.n_components = obj_dict['n_components']
        self.kernel = obj_dict['kernel']
        self.degree = obj_dict['degree']
        self.gamma = obj_dict['gamma']

        self.basis = obj_dict['basis']
        self.mean =obj_dict['mean']
        self.std = obj_dict['std']
        self.phi_map = obj_dict['phi']

        self.rawfext = RAWFeatureExtractor(self.window, batch_size=10)
        self.rawpat = self.rawfext.temp_feature_vector()
        self.cached_rawbatch = None


    def __len__(self):
        return self.n_components

    def extract(self, img, i, j, pat):
        self.rawfext.extract(img, i, j, self.rawpat)
        self.scaled = (self.rawpat - self.mean)/self.std
        # compute kernel between self.rawpat and Xtr
        if self.kernel == 'poly':
            #K_y = polynomial_kernel(self.basis, self.scaled.reshape(1, -1), degree=self.degree).astype(np.float32)
            K_y = fast_poly_kernel(self.basis, self.scaled.reshape(1, -1), degree=self.degree).astype(self.dtype)
        else:
            K_y = rbf_kernel(self.basis, self.scaled.reshape(1, -1), gamma=self.gamma).astype(self.dtype)
        p = self.phi_map.dot(K_y)

        pat[:] = p.reshape(-1)

    @cython.profile(True)
    def extract_batch(self, inp, idx_i, idx_j, batch_X):
        if self.cached_rawbatch is None or self.cached_rawbatch.shape[0] < batch_X.shape[0]:
            self.cached_rawbatch = np.zeros((batch_X.shape[0], len(self.rawfext)),
                self.rawfext.dtype)
            rawbatch = self.cached_rawbatch
        else:
            rawbatch = self.cached_rawbatch[:idx_i.shape[0]]
        self.rawfext.extract_batch(inp, idx_i, idx_j, rawbatch)

        Xscaled = rawbatch.astype(np.float64)
        Xscaled -= self.mean
        Xscaled /= self.std
        #Xscaled = (Xscaled - self.mean)/self.std
        if self.kernel == 'poly':
            #K_y2 = polynomial_kernel(Xscaled, self.basis, degree=self.degree).astype(np.float32)
            K_y = fast_poly_kernel(Xscaled, self.basis, degree=self.degree).astype(self.dtype)
        else:
            K_y = rbf_kernel(Xscaled, self.basis, gamma=self.gamma).astype(self.dtype)

        batch_X[:] = self.phi_map.dot(K_y.T).T
