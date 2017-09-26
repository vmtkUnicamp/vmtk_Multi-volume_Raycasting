/*  
 *  transferFunction.cpp: constrói diferentes funcoes de transferencia 
 *                 que mapeiam os valores escalares da imagem 3D em 
 *                 propriedades opticas (niveis de cinza/cores e 
 *                 e opacidade)
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
#include <iostream>
#include <string.h>
#include "transferFunction.h"
#include <cmath>

#ifndef SQR
#define SQR(a) ((a) * (a))
#endif
#ifndef CUB
#define CUB(a) ((a) * (a) * (a))
#endif

/// Estimate logarithmic transfer function from image char data values
/// entry 0 is reserved to background intensity 0
void TransferFunction::GetGrayScaleTFE (int min, int max, int *elemSize, unsigned char **ptr) {
  double aux;

  *elemSize = max-min+2;

  *ptr = new unsigned char[*elemSize];
  aux = 1./(max-min+1)*9;

  (*ptr)[0] = 0;
  for (int i = 1 ; i < *elemSize; i++) 
    (*ptr)[i] = (unsigned char)(log(i*aux+1.)*255);
}

/// Estimate linear transfer function from image char data values
void TransferFunction::GetGrayScaleTFL (int min, int max, int *elemSize, unsigned char **ptr) {
  double aux;

  *elemSize = max-min+2;

  *ptr = new unsigned char[*elemSize];
  aux = 1./(max-min+1);

  (*ptr)[0] = 0;
  for (int i = 1 ; i < *elemSize; i++) 
    (*ptr)[i] = (unsigned char)(i*aux*255);
}

void TransferFunction::GetGrayScaleTFS (int min, int max, int *elemSize, unsigned char **ptr) {
  double aux;

  *elemSize = max-min+2;

  *ptr = new unsigned char[*elemSize];
  aux = 1./(max-min+1);

  (*ptr)[0] = 0;
  for (int i = 1 ; i < *elemSize; i++) 
    (*ptr)[i] = (unsigned char)(SQR(i*aux)*255);
}

void TransferFunction::GetGrayScaleTFC (int min, int max, int *elemSize, unsigned char **ptr) {
  double aux;

  *elemSize = max-min+2;

  *ptr = new unsigned char[*elemSize];
  aux = 1./(max-min+1);

  (*ptr)[0] = 0;
  for (int i = 1 ; i < *elemSize; i++) 
    (*ptr)[i] = (unsigned char)(CUB(i*aux)*255);
}

// Constroi uma funcao de transferencia que mapeia em [0,255] os valores
// do intervalo [umin,umax] do dominio [0,2^{nbitsalloc}]
void TransferFunction::GetGrayScaleTFD (int min, int max, int *elemSize, unsigned char **ptr, unsigned short nbitsalloc) {
  double aux;
  int node1, node2;

  *elemSize = 256;

  *ptr = new unsigned char[*elemSize];

  aux = (1.*pow(2,8))/pow(2,nbitsalloc);

  node1 = (int)(min*aux);
  node2 = (int)(max*aux);

  for (int i = 0 ; i < node1; i++) 
    (*ptr)[i] = 0;
  for (int i = node1 ; i < node2; i++)
    (*ptr)[i] = (unsigned char)((1.0*(i-node1))/(node2-node1)*255);
  for (int i = node2; i < *elemSize; i++)
    (*ptr)[i] = 0;
}

void TransferFunction::GetGrayScaleTF (int tag, int min, int max, 
  int *elemSize, unsigned char **ptr) {

 switch (tag) {
 case 0:
   GetGrayScaleTFE (min, max, elemSize, ptr);
   break;
 case 1:
   GetGrayScaleTFL (min, max, elemSize, ptr);
   break;
 case 2:
   GetGrayScaleTFS (min, max, elemSize, ptr);
   break;
 case 3:
   GetGrayScaleTFC (min, max, elemSize, ptr);
   break;
 }
}

void TransferFunction::GetGrayScaleTF (int tag, int min, int max, 
  int *elemSize, unsigned char **ptr, unsigned short nbitsalloc) {

 switch (tag) {
 case 4:
   GetGrayScaleTFD (min, max, elemSize, ptr, nbitsalloc);
   break;
 }
}

void TransferFunction::GetRGBTF (int tag, int min, int max, 
  int *elemSize, unsigned char **ptr) {

  // Variavel tag reservado para extensoes futuras

  unsigned char tabelaCor [] = {
    0, 0, 0,
    51, 0, 0,
    102, 0, 0,
    153, 0, 0,
    204, 0, 0,
    255, 0 , 0,
    255, 51, 0,
    255, 102, 0,
    255, 153, 0,
    255, 204, 0,
    255, 255, 0,
    204, 255, 0,
    153, 255, 0,
    102, 255, 0,
    51, 255, 0,
    0, 255, 0,
    0, 255, 51,
    0, 255, 102,
    0, 255, 153,
    0, 255, 204,
    0, 255, 255,
    0, 204, 255,
    0, 153, 255,
    0, 102, 255,
    0, 51, 255,
    0, 0, 255,
    51, 0, 255,
    102, 0, 255,
    153, 0, 255,
    204, 0, 255,
    255, 0, 255,
    255, 51, 255,
    255, 102, 255,
    255, 153, 255,
    255, 204, 244,
    255, 255, 255
  };

  *elemSize = sizeof(tabelaCor);

  *ptr = new unsigned char[*elemSize];

  memcpy (*ptr, tabelaCor, *elemSize);
}
