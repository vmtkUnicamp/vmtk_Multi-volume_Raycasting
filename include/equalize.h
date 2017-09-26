/*  
 *  equaliza.h 
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

#ifndef _equalize_h
#define _equalize_h

class Equalize {
 public:
	/**
	* @brief EqualizeHistogram constructor
	* @param[in] (dimx, dimy, dimz): dimension of the volume.
	* @param[in] volume: volume data.
	* @param[in] nbitsalloc: number of allocated bits for the volume.
	* @param[in] umax: maximum intensity of the volume.
	* @param [out] histogram: histogram equalized of the volume.
	*/
  void EqualizeHistogram (int dimx, int dimy, int dimz, unsigned short *volume,
        int nbitsalloc, unsigned int umax, unsigned short **histogram);
};
#endif

