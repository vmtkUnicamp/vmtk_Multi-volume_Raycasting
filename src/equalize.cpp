/*
 *  equaliza.cpp: equaliza o histograma para aumentar a escala dinamica 
 *                das intensidades
 *
 *  Copyright (C) 2013  Wu Shin-Ting, FEEC, Unicamp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <cmath>
#include <string.h>
#include "equalize.h"

// Sugestão de leitura:
//http://en.wikipedia.org/wiki/Histogram_equalization

void Equalize::EqualizeHistogram (int dimx, int dimy, int dimz, 
            unsigned short *volume, int nbitsalloc, 
            unsigned int umax, unsigned short **histogram)
{
  // Computa o histograma do <volume>: 
  // frequencia de ocorrência de cada intensidade
  int i;
  int L = pow(2,nbitsalloc);
  int *tmp = new int[L];
  int *cdf = new int[L];

  // Initializa os vetores
  memset(tmp, 0x00, L*sizeof(int));

  // Determina a frequencia de ocorrencia de cada intensidade
  int intensidade, iz, iy, ix; 
  for (iz = 0; iz < dimz; iz++) {
    for (iy =0; iy < dimy; iy++) {
      for (ix =0; ix < dimx; ix++) {
	intensidade = (int)(volume[iz*dimx*dimy+iy*dimx+ix]);
	tmp[intensidade]++;
      }
    }
  }

  int volSize = dimx*dimy*dimz;

  // Determina a funcao de distribuicao acumulada
  // https://pt.wikipedia.org/wiki/Fun%C3%A7%C3%A3o_distribui%C3%A7%C3%A3o_acumulada
  cdf[0] = tmp[0];
  int cdfmin = -1;
  	
  if (cdf[0]) cdfmin = cdf[0];
  for(i = 1; i < L; i++) {
    cdf[i] = cdf[i-1] + tmp[i];
    if (cdfmin == -1 && cdf[i]) cdfmin = cdf[i];
  }

  delete [] tmp;

  // Mapeamento de intensidades
  *histogram = new unsigned short[umax];
  memset(*histogram, 0x00, umax*sizeof(unsigned short));

  for (i=0; i<umax; i++) {
    (*histogram)[i] = (unsigned short)(((1.0*(cdf[i]-cdfmin))/(volSize-cdfmin))*(L-1));
  }
  delete [] cdf;
}
