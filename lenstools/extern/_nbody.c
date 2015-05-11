/*Python wrapper module for gadget snapshot reading;
the method used is the same as in 
http://dan.iel.fm/posts/python-c-extensions/ 

The module is called _nbody and it defines the methods below (see docstrings)
*/

#include <stdio.h>

#include <Python.h>
#include <numpy/arrayobject.h>

#include "grid.h"

//Python module docstrings
static char module_docstring[] = "This module provides a python interface for operations on Nbody simulation snapshots";
static char grid3d_docstring[] = "Put the snapshot particles on a regularly spaced grid";
static char adaptive_docstring[] = "Put the snapshot particles on a regularly spaced grid using adaptive smoothing";

//Method declarations
static PyObject * _nbody_grid3d(PyObject *self,PyObject *args);
static PyObject * _nbody_adaptive(PyObject *self,PyObject *args);

//_nbody method definitions
static PyMethodDef module_methods[] = {

	{"grid3d",_nbody_grid3d,METH_VARARGS,grid3d_docstring},
	{"adaptive",_nbody_adaptive,METH_VARARGS,adaptive_docstring},
	{NULL,NULL,0,NULL}

} ;

//_nbody constructor
PyMODINIT_FUNC init_nbody(void){

	PyObject *m = Py_InitModule3("_nbody",module_methods,module_docstring);
	if(m==NULL) return;

	/*Load numpy functionality*/
	import_array();

}

//grid3d() implementation
static PyObject *_nbody_grid3d(PyObject *self,PyObject *args){

	Py_ssize_t nargs = PyTuple_Size(args);
	PyObject *positions_obj,*bins_obj,*grid_obj;
	char inplace;

	if(nargs==2){

		//parse input tuple
		if(!PyArg_ParseTuple(args,"OO",&positions_obj,&bins_obj)){
			return NULL;
		}

		inplace = 0;
	
	} else if(nargs==3){

		//parse input tuple
		if(!PyArg_ParseTuple(args,"OOO",&positions_obj,&bins_obj,&grid_obj)){
			return NULL;
		}

		inplace = 1;

	} else{

		PyErr_SetString(PyExc_TypeError,"_grid3d takes either 2 or 3 arguments!!");
		return NULL;

	}

	//interpret parsed objects as arrays
	PyObject *positions_array = PyArray_FROM_OTF(positions_obj,NPY_FLOAT32,NPY_IN_ARRAY);
	PyObject *binsX_array = PyArray_FROM_OTF(PyTuple_GET_ITEM(bins_obj,0),NPY_DOUBLE,NPY_IN_ARRAY);
	PyObject *binsY_array = PyArray_FROM_OTF(PyTuple_GET_ITEM(bins_obj,1),NPY_DOUBLE,NPY_IN_ARRAY);
	PyObject *binsZ_array = PyArray_FROM_OTF(PyTuple_GET_ITEM(bins_obj,2),NPY_DOUBLE,NPY_IN_ARRAY);

	//check if anything failed
	if(positions_array==NULL || binsX_array==NULL || binsY_array==NULL || binsZ_array==NULL){
		
		Py_XDECREF(positions_array);
		Py_XDECREF(binsX_array);
		Py_XDECREF(binsY_array);
		Py_XDECREF(binsZ_array);

		return NULL;
	}

	//Get data pointers
	float *positions_data = (float *)PyArray_DATA(positions_array);
	double *binsX_data = (double *)PyArray_DATA(binsX_array);
	double *binsY_data = (double *)PyArray_DATA(binsY_array);
	double *binsZ_data = (double *)PyArray_DATA(binsZ_array);

	//Get info about the number of bins
	int NumPart = (int)PyArray_DIM(positions_array,0);
	int nx = (int)PyArray_DIM(binsX_array,0) - 1;
	int ny = (int)PyArray_DIM(binsY_array,0) - 1;
	int nz = (int)PyArray_DIM(binsZ_array,0) - 1;

	//Allocate the new array for the grid
	PyObject *grid_array;

	if(!inplace){
		
		npy_intp gridDims[] = {(npy_intp) nx,(npy_intp) ny,(npy_intp) nz};
		grid_array = PyArray_ZEROS(3,gridDims,NPY_FLOAT32,0);
	
	} else{

		grid_array = PyArray_FROM_OTF(grid_obj,NPY_FLOAT32,NPY_IN_ARRAY);

	}

	if(grid_array==NULL){

		Py_DECREF(positions_array);
		Py_DECREF(binsX_array);
		Py_DECREF(binsY_array);
		Py_DECREF(binsZ_array);

		return NULL;

	}

	//Get a data pointer
	float *grid_data = (float *)PyArray_DATA(grid_array);

	//Snap the particles on the grid
	grid3d(positions_data,NumPart,binsX_data[0],binsY_data[0],binsZ_data[0],binsX_data[1] - binsX_data[0],binsY_data[1] - binsY_data[0],binsZ_data[1] - binsZ_data[0],nx,ny,nz,grid_data);

	//return the grid
	Py_DECREF(positions_array);
	Py_DECREF(binsX_array);
	Py_DECREF(binsY_array);
	Py_DECREF(binsZ_array);

	//if inplace, release the reference to grid_array and return None
	if(inplace){
		
		Py_DECREF(grid_array);
		Py_RETURN_NONE;
	
	} else{
		
		return grid_array;
	
	}

}

//adaptive() implementation
static PyObject * _nbody_adaptive(PyObject *self,PyObject *args){

	PyObject *positions_obj,*rp_obj,*binning_obj,*projectAll;
	double center;
	int direction0,direction1,normal;

	//Parse argument tuple
	if(!PyArg_ParseTuple(args,"OOOdiiiO",&positions_obj,&rp_obj,&binning_obj,&center,&direction0,&direction1,&normal,&projectAll)){
		return NULL;
	}

	//Parse arrays
	PyObject *positions_array = PyArray_FROM_OTF(positions_obj,NPY_FLOAT32,NPY_IN_ARRAY);
	PyObject *rp_array = PyArray_FROM_OTF(rp_obj,NPY_DOUBLE,NPY_IN_ARRAY);
	PyObject *binning0_array = PyArray_FROM_OTF(PyList_GetItem(binning_obj,0),NPY_DOUBLE,NPY_IN_ARRAY);
	PyObject *binning1_array = PyArray_FROM_OTF(PyList_GetItem(binning_obj,1),NPY_DOUBLE,NPY_IN_ARRAY);

	//Check if anything went wrong
	if(positions_array==NULL || rp_array==NULL || binning0_array==NULL || binning1_array==NULL){
		
		Py_XDECREF(positions_array);
		Py_XDECREF(rp_array);
		Py_XDECREF(binning0_array);
		Py_XDECREF(binning1_array);

		return NULL;
	}

	//Compute the number of particles
	int NumPart = (int)PyArray_DIM(positions_array,0);

	//Allocate space for lensing plane
	npy_intp dims[] =  {PyArray_DIM(binning0_array,0)-1,PyArray_DIM(binning1_array,0)-1};
	int size0 = (int)dims[0];
	int size1 = (int)dims[1];
	
	PyObject *lensingPlane_array = PyArray_ZEROS(2,dims,NPY_DOUBLE,0);
	
	if(lensingPlane_array==NULL){

		Py_DECREF(positions_array);
		Py_DECREF(rp_array);
		Py_DECREF(binning0_array);
		Py_DECREF(binning1_array);

		return NULL;

	}

	//Get data pointers
	float *positions = (float *)PyArray_DATA(positions_array);
	double *rp = (double *)PyArray_DATA(rp_array);
	double *binning0 = (double *)PyArray_DATA(binning0_array);
	double *binning1 = (double *)PyArray_DATA(binning1_array);
	double *lensingPlane = (double *)PyArray_DATA(lensingPlane_array);

	//Compute the adaptive smoothing using C backend
	adaptiveSmoothing(NumPart,positions,rp,binning0,binning1,center,direction0,direction1,normal,size0,size1,PyObject_IsTrue(projectAll),lensingPlane);

	//Cleanup
	Py_DECREF(positions_array);
	Py_DECREF(rp_array);
	Py_DECREF(binning0_array);
	Py_DECREF(binning1_array);

	//Return
	return lensingPlane_array;


}