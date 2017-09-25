/*  
 *  transferFunction.h 
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

#ifndef _transferFunction_h
#define _transferFunction_h

#include <fstream>
#include <vector>


class TransferFunction {
 public:
  void GetGrayScaleTF (int tag, int min, int max, int *elemSize, unsigned char **ptr);
  void GetGrayScaleTF (int tag, int min, int max, int *elemSize, unsigned char **ptr, unsigned short nbtsalloc);
  void GetRGBTF (int tag, int min, int max, int *elemSize, unsigned char **ptr);
  
 private:
  void GetGrayScaleTFE (int min, int max, int *elemSize, unsigned char **ptr);
  void GetGrayScaleTFL (int min, int max, int *elemSize, unsigned char **ptr);
  void GetGrayScaleTFS (int min, int max, int *elemSize, unsigned char **ptr);
  void GetGrayScaleTFC (int min, int max, int *elemSize, unsigned char **ptr);
  void GetGrayScaleTFD (int min, int max, int *elemSize, unsigned char **ptr, unsigned short nbitsalloc);
};
#endif

