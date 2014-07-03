#ifndef __AZIMUTH_H
#define __AZIMUTH_H

#include <complex.h>

int azimuthal_rfft2(double _Complex *ft_map,long size_x,long size_y,double map_angle_degrees,int Nvalues,double *lvalues,double *power_l);

#endif