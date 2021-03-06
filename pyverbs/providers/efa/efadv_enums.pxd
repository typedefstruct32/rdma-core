# SPDX-License-Identifier: (GPL-2.0 OR Linux-OpenIB)

#cython: language_level=3


cdef extern from 'infiniband/efadv.h':

    cdef enum:
        EFADV_DEVICE_ATTR_CAPS_RDMA_READ
        EFADV_DEVICE_ATTR_CAPS_RNR_RETRY

    cdef enum:
        EFADV_QP_DRIVER_TYPE_SRD
