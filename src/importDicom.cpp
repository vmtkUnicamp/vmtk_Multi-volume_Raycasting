/*
 *  importDicom.cpp: Ler um arquivo no formato DICOM, extrair a
 *                 imagem 3D e reorientá-la para que fique positivamente
 *                 orientada no referencial LPS
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
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <stdio.h>     /* for printf */
#include <stdint.h>
#include <stdlib.h>    /* for exit */
#include <string.h>
#include <limits.h>
#include <math.h>
#ifndef WIN32
#include <getopt.h>
#endif

/*============================================================
  Incluir definicoes da bilbioteca gdcm

http://www.creatis.insa-lyon.fr/software/public/Gdcm/
  ===========================================================*/
#include "gdcmReader.h"
#include "gdcmImageReader.h"
#include "gdcmMediaStorage.h"
#include "gdcmFile.h"
#include "gdcmDataSet.h"
#include "gdcmUIDs.h"
#include "gdcmGlobal.h"
#include "gdcmModules.h"
#include "gdcmDefs.h"
#include "gdcmOrientation.h"
#include "gdcmVersion.h"
#include "gdcmMD5.h"
#include "gdcmSystem.h"
#include "gdcmDirectory.h"
#include "gdcmSorter.h"
#include "gdcmScanner.h"
#include "gdcmDataSet.h"
#include "gdcmAttribute.h"

// Estrutura de dados para armazenar a informacao do volume de dados
#include "importDicom.h"

//http://nipy.sourceforge.net/nibabel/dicom/dicom_orientation.html
//Reorienta o volume de dados para que seja LPS

bool Import::LPS_ReorientImageVolume(ImgFormat *volData)
{
    unsigned int ix, iy, iz, idx, idy, idz;
    unsigned int dimx, dimy, dimz;
    float sx, sy, sz;
    unsigned char flag;
    unsigned char code;

    unsigned char *tmp = new unsigned char[2*volData->dims[0]*volData->dims[1]];

    std::cout << "Reorient the samples in LPS-reference ..." << std::endl;

    code = 0;

    // Reflection
    // X-axis
    flag = 0;
    if (fabs(volData->dircos[0][0]) > fabs(volData->dircos[0][1]) &&
            fabs(volData->dircos[0][0]) > fabs(volData->dircos[0][2])) {
        if (volData->dircos[0][0] < 0) {  // RL (21/05/2013 - Ting)
            flag = 1;
            volData->dircos[0][0] *= (-1);  // LR
        }
        code += 0;
    } else if (fabs(volData->dircos[0][1]) > fabs(volData->dircos[0][0])
               && fabs(volData->dircos[0][1]) > fabs(volData->dircos[0][2])) {
        if (volData->dircos[0][1] < 0) {  // PA
            flag = 1;
            volData->dircos[0][1] *= (-1);  // AP
        }
        code += 16;
    } else {
        if (volData->dircos[0][2] < 0) {  // HF
            flag = 1;
            volData->dircos[0][2] *= (-1);  // FH
        }
        code += 32;
    }
    if (flag) {
        switch (volData->nbitsalloc) {
        case 8:
            for (idz = 0; idz < volData->dims[2]; idz++) {
                for (idy = 0; idy < volData->dims[1]; idy++) {
                    for (ix=0,idx = volData->dims[0]-1; ix < idx; idx--,ix++) {
                        ///Swap the elements
                        tmp[0]=volData->buffer[
                                idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[0] + idx];
                        volData->buffer[idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[0] + idx] =
                                volData->buffer[idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[0] + ix];
                        volData->buffer[idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[0] + ix] = tmp[0];
                    }
                }
            }
            break;
        case 16:
            for (idz = 0; idz < volData->dims[2]; idz++) {
                for (idy = 0; idy < volData->dims[1]; idy++) {
                    for (ix=0,idx = volData->dims[0]-1; ix < idx; idx=idx--,ix=ix++) {
                        ///Swap the elements
                        memcpy (tmp,
                                &(volData->buffer[2*(idz * volData->dims[0] *
                                volData->dims[1] + idy * volData->dims[0] + idx)]),
                                sizeof(unsigned char)*2);
                        memcpy (&(volData->buffer[
                                  2*(idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[0] + idx)]),
                                &(volData->buffer[
                                  2*(idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[0] + ix)]),
                                sizeof(unsigned char)*2);
                        memcpy(&(volData->buffer[
                                 2*(idz * volData->dims[0] * volData->dims[1] +
                                idy * volData->dims[1] + ix)]), tmp,
                                sizeof(unsigned char)*2);
                    }
                }
            }
            break;
        }
    }

    /// Y-axis
    flag = 0;
    if (fabs(volData->dircos[1][0]) > fabs(volData->dircos[1][1])
            &&
            fabs(volData->dircos[1][0]) > fabs(volData->dircos[1][2])) {
        if (volData->dircos[1][0] < 0) {   // RL (21/05/2013 - Ting)
            flag = 1;
            volData->dircos[1][0] *= (-1);   // LR
        }
        code += 0;
    } else if (fabs(volData->dircos[1][1]) > fabs(volData->dircos[1][0])
               &&
               fabs(volData->dircos[1][1]) > fabs(volData->dircos[1][2])) {
        if (volData->dircos[1][1] < 0) {   // PA
            flag = 1;
            volData->dircos[1][1] *= (-1);   // AP
        }
        code += 4;
    } else {
        if (volData->dircos[1][2] < 0) {   // HF
            flag = 1;
            volData->dircos[1][2] *= (-1);   // FH
        }
        code += 8;
    }
    if (flag) {
        switch (volData->nbitsalloc) {
        case 8:
            for (idz = 0; idz < volData->dims[2]; idz++) {
                for (iy=0,idy = volData->dims[1]-1; iy < idy; idy--,iy++) {
                    memcpy (tmp, &(volData->buffer[idz * volData->dims[0] *
                            volData->dims[1] + idy * volData->dims[0]]),
                            sizeof(unsigned char)*volData->dims[0]);
                    memcpy(&(volData->buffer[idz * volData->dims[0] *
                            volData->dims[1] + idy * volData->dims[0]]),
                            &(volData->buffer[idz * volData->dims[0] *
                            volData->dims[1] + iy * volData->dims[0]]),
                            sizeof(unsigned char)*volData->dims[0]);
                    memcpy(&(volData->buffer[idz * volData->dims[0] *
                            volData->dims[1] + iy * volData->dims[0]]), tmp,
                            sizeof(unsigned char)*volData->dims[0]);
                }
            }
            break;
        case 16:
            for (idz = 0; idz < volData->dims[2]; idz++) {
                for (iy=0,idy = volData->dims[1]-1; iy < idy; idy--,iy++) {
                    memcpy (tmp, &(volData->buffer[2*(idz*volData->dims[0] *
                            volData->dims[1] + idy*volData->dims[0])]),
                            sizeof(unsigned char)*2*volData->dims[0]);
                    memcpy(&(volData->buffer[2*(idz * volData->dims[0] *
                           volData->dims[1] + idy * volData->dims[0])]),
                            &(volData->buffer[2*(idz * volData->dims[0] *
                            volData->dims[1] + iy * volData->dims[0])]),
                            sizeof(unsigned char)*2*volData->dims[0]);
                    memcpy(&(volData->buffer[2*(idz * volData->dims[0] *
                           volData->dims[1] + iy * volData->dims[0])]), tmp,
                            sizeof(unsigned char)*2*volData->dims[0]);
                }
            }
            break;
        }
    }

    /// Depth
    flag = 0;
    // Recompute the normal direction vector
    if (fabs(volData->dircos[2][0]) > fabs(volData->dircos[2][1])
            &&
            fabs(volData->dircos[2][0]) > fabs(volData->dircos[2][2])) {
        if (volData->dircos[2][0] < 0) {   // RL  (21/05/2013 - Ting)
            flag = 1;
            volData->dircos[2][0] *= (-1);   // LR
        }
        code += 0;
    } else if (fabs(volData->dircos[2][1]) > fabs(volData->dircos[2][0])
               &&
               fabs(volData->dircos[2][1]) > fabs(volData->dircos[2][2])) {
        if (volData->dircos[2][1] < 0) {   // PA
            flag = 1;
            volData->dircos[2][1] *= (-1);   // AP
        }
        code += 1;
    } else {
        if (volData->dircos[2][2] < 0) {   // HF
            flag = 1;
            volData->dircos[2][2] *= (-1);   // FH
        }
        code += 2;
    }
    if (flag) {
        switch (volData->nbitsalloc) {
        case 8:
            for (iz=0,idz = volData->dims[2]-1; iz < idz; idz-=1,iz+=1) {
                memcpy (tmp, &(volData->buffer[idz * volData->dims[0] * volData->dims[1]]),
                        sizeof(unsigned char)*(volData->dims[0] * volData->dims[1]));
                memcpy (&(volData->buffer[idz * volData->dims[0] * volData->dims[1]]),
                        &(volData->buffer[iz * volData->dims[0] * volData->dims[1]]),
                        sizeof(unsigned char)*(volData->dims[0] * volData->dims[1]));
                memcpy(&(volData->buffer[iz * volData->dims[0] * volData->dims[1]]), tmp,
                        sizeof(unsigned char)*(volData->dims[0] * volData->dims[1]));
            }
            break;
        case 16:
            for (iz=0,idz = volData->dims[2]-1; iz < idz; idz-=1,iz+=1) {
                memcpy (tmp, &(volData->buffer[idz * 2 * volData->dims[0] * volData->dims[1]]),
                        sizeof(unsigned char)*2*(volData->dims[0] * volData->dims[1]));
                memcpy (&(volData->buffer[idz * 2 * volData->dims[0] * volData->dims[1]]),
                        &(volData->buffer[iz * 2* volData->dims[0] * volData->dims[1]]),
                        sizeof(unsigned char)*2*(volData->dims[0] * volData->dims[1]));
                memcpy (&(volData->buffer[iz * 2 * volData->dims[0] * volData->dims[1]]), tmp,
                        sizeof(unsigned char)*2*(volData->dims[0] * volData->dims[1]));
            }
            break;
        }
    }

    delete[] tmp;

    std::cout << "code = " << (int)code << std::endl;

    if (code == 6) return true;

    // Rotation to LPS
    {
        double dir[3][3];

        switch (volData->nbitsalloc) {
        case 8:
            tmp = new unsigned char[volData->dims[0]*volData->dims[1]*volData->dims[2]];
            memcpy (tmp,volData->buffer,
                    sizeof(unsigned char)*(volData->dims[0] * volData->dims[1] * volData->dims[2]));
            memset(volData->buffer, 0x00, volData->dims[0]*volData->dims[1]*volData->dims[2]);

            if (code == 24) { // PSL
                std::cout << "PSL" << std::endl;
                for (ix = 0, idy = 0; idy < volData->dims[1]; idy++) {
                    for (idx = 0; idx < volData->dims[0]; idx++) {
                        for (idz=0; idz < volData->dims[2]; idz++) {
                            volData->buffer[ix++] =
                                    tmp[idz * volData->dims[0] * volData->dims[1] +
                                    idy * volData->dims[0] + idx];
                        }
                    }
                }
                sx = volData->space[2]; sy = volData->space[0]; sz = volData->space[1];
                dimx = volData->dims[2]; dimy = volData->dims[0]; dimz = volData->dims[1];
                dir[0][0] = volData->dircos[2][0];
                dir[0][1] = volData->dircos[2][1];
                dir[0][2] = volData->dircos[2][2];
                dir[1][0] = volData->dircos[0][0];
                dir[1][1] = volData->dircos[0][1];
                dir[1][2] = volData->dircos[0][2];
                dir[2][0] = volData->dircos[1][0];
                dir[2][1] = volData->dircos[1][1];
                dir[2][2] = volData->dircos[1][2];
            } else if (code == 33) { // SLP
                std::cout << "SLP" << std::endl;
                for (ix = 0, idx = 0; idx < volData->dims[0]; idx++) {
                    for (idz = 0; idz < volData->dims[2]; idz++) {
                        for (idy=0; idy < volData->dims[1]; idy++) {
                            volData->buffer[ix++] =
                                    tmp[idz * volData->dims[0] * volData->dims[1] +
                                    idy * volData->dims[0] + idx];
                        }
                    }
                }
                sx = volData->space[1]; sy = volData->space[2]; sz = volData->space[0];
                dimx = volData->dims[1]; dimy = volData->dims[2]; dimz = volData->dims[0];
                dir[0][0] = volData->dircos[1][0];
                dir[0][1] = volData->dircos[1][1];
                dir[0][2] = volData->dircos[1][2];
                dir[1][0] = volData->dircos[2][0];
                dir[1][1] = volData->dircos[2][1];
                dir[1][2] = volData->dircos[2][2];
                dir[2][0] = volData->dircos[0][0];
                dir[2][1] = volData->dircos[0][1];
                dir[2][2] = volData->dircos[0][2];
            }
            break;
        case 16:
            tmp = new unsigned char[2*volData->dims[0]*volData->dims[1]*volData->dims[2]];
            memcpy (tmp,volData->buffer,
                    sizeof(unsigned char)*(2*volData->dims[0]*volData->dims[1]*volData->dims[2]));
            memset(volData->buffer, 0x00, 2*volData->dims[0]*volData->dims[1]*volData->dims[2]);
            if (code == 24) { // PSL
                std::cout << "PSL" << std::endl;
                for (ix=0, idy = 0; idy < volData->dims[1]; idy++) { //PLS
                    for (idx = 0; idx < volData->dims[0]; idx++) {
                        for (idz = 0; idz < volData->dims[2]; idz++) {
                            memcpy(&(volData->buffer[2*ix]),&(tmp[2*(idz*volData->dims[0]*volData->dims[1]+idy*volData->dims[0]+idx)]), sizeof(unsigned short));
                            ix++;
                        }
                    }
                }
                sx = volData->space[2]; sy = volData->space[0]; sz = volData->space[1];
                dimx = volData->dims[2]; dimy = volData->dims[0]; dimz = volData->dims[1];
                dir[0][0] = volData->dircos[2][0];
                dir[0][1] = volData->dircos[2][1];
                dir[0][2] = volData->dircos[2][2];
                dir[1][0] = volData->dircos[0][0];
                dir[1][1] = volData->dircos[0][1];
                dir[1][2] = volData->dircos[0][2];
                dir[2][0] = volData->dircos[1][0];
                dir[2][1] = volData->dircos[1][1];
                dir[2][2] = volData->dircos[1][2];
            } else if (code == 33) { // SLP
                std::cout << "SLP" << std::endl;
                for (ix = 0, idx = 0; idx < volData->dims[0]; idx++) {
                    for (idz = 0; idz < volData->dims[2]; idz++) {
                        for (idy=0; idy < volData->dims[1]; idy++) {
                            memcpy(&(volData->buffer[2*ix]),&(tmp[2*(idz*volData->dims[0]*volData->dims[1]+idy*volData->dims[0]+idx)]), sizeof(unsigned short));
                            ix++;
                        }
                    }
                }
                sx = volData->space[1]; sy = volData->space[2]; sz = volData->space[0];
                dimx = volData->dims[1]; dimy = volData->dims[2]; dimz = volData->dims[0];
                dir[0][0] = volData->dircos[1][0];
                dir[0][1] = volData->dircos[1][1];
                dir[0][2] = volData->dircos[1][2];
                dir[1][0] = volData->dircos[2][0];
                dir[1][1] = volData->dircos[2][1];
                dir[1][2] = volData->dircos[2][2];
                dir[2][0] = volData->dircos[0][0];
                dir[2][1] = volData->dircos[0][1];
                dir[2][2] = volData->dircos[0][2];
            }
            break;
        }

        // Update the dimensions, spacings and directions
        volData->space[0]=sx; volData->space[1]=sy; volData->space[2]=sz;
        volData->dims[0]=dimx; volData->dims[1]=dimy; volData->dims[2]=dimz;
        volData->dircos[0][0] = dir[0][0]; volData->dircos[0][1] = dir[0][1]; volData->dircos[0][2] = dir[0][2];
        volData->dircos[1][0] = dir[1][0]; volData->dircos[1][1] = dir[1][1]; volData->dircos[1][2] = dir[1][2];
        volData->dircos[2][0] = dir[2][0]; volData->dircos[2][1] = dir[2][1]; volData->dircos[2][2] = dir[2][2];

        delete [] tmp;
    }

    return true;
}

bool Import::ReadPixelData(const gdcm::Image &image, const gdcm::DataSet &ds,
                           const gdcm::PixelFormat::ScalarType & stype,
                           unsigned short nbitsalloc, unsigned short nbitsstored,
                           float slope, float intercept,
                           unsigned int *len, char **buf,
                           int *umin, int *umax)
{
    // Get Pixeldata
    gdcm::Tag rawTag(0x7fe0, 0x0010); // Default to Pixel Data
    const gdcm::DataElement& pixel_data = ds.GetDataElement(rawTag);
    const gdcm::ByteValue *bv = pixel_data.GetByteValue();
    *len = bv->GetLength();

    //std::cout << "Image length: " << *len << std::endl;
    int j, step, nShiftBits;
    unsigned int length;
    unsigned char mask, v1;
    unsigned short ivalue;
    int value;
    char *tmpbuf8;
    unsigned short *tmpbuf16;

    switch (stype) {
    case 0:  // UINT8
    case 1:  // INT8
        tmpbuf8 = new char[*len];
        bv->GetBuffer(tmpbuf8, *len);
        for(unsigned int di = 0; di < *len; di += 1)
        {
            if (stype == 0)
                value = (int)((reinterpret_cast<unsigned char*>(tmpbuf8))[di]);
            else if (stype == 1)
                value = (int)((reinterpret_cast<char*>(tmpbuf8))[di]);

            if(*umax < value) *umax = value;
            if(*umin > value) *umin = value;
        }
        *buf = tmpbuf8;
        break;
    case 2:  // UINT12
        break;
    case 3:  // INT12
        break;
    case 4:  // UINT16
    case 5:  // INT16
        length = (*len)/2;
        nShiftBits = nbitsalloc - nbitsstored;
        step = nbitsalloc / (sizeof(char) * 8);
        mask = (255 >> nShiftBits);

        tmpbuf16 = new unsigned short[length];
        tmpbuf8 = new char[*len];
        bv->GetBuffer(reinterpret_cast<char*>(tmpbuf8), *len);

        for(unsigned int di = 0; di < *len; di += step)
        {
            // Little endian
            /// The (MS)byte to be masked by nShiftBits
            ivalue = 0;
            v1 = mask & ((unsigned char) tmpbuf8[di+step-1]);
            /// Add the remaining bytes
            for (j = step - 1; j > 0; j--) {
                ivalue += (v1 * pow(256, j));
                v1 = (unsigned char) tmpbuf8[di + j - 1];
            }
            ivalue += v1;

            tmpbuf16[di/2] = ivalue;

            if (stype == 4)
                value = ivalue;
            else if (stype == 5)
                value = (int)(*(reinterpret_cast<short *>(&ivalue)));

            if(*umax < value) *umax = value;
            if(*umin > value) *umin = value;
        }
        delete[] tmpbuf8;
        *buf = reinterpret_cast<char*>(tmpbuf16);
        break;
    case 6:  // UINT32
        break;
    case 7: // INT32
        break;
    case 8: // FLOAT16
        break;
    case 9: // FLOAT32
        break;
    case 10: // FLOAT64
        break;
    case 11: // UNKONOWN
        break;
    }
    return 1;
}

int Import::ProcessOneFile( std::string const & filename,
                            gdcm::Defs const & defs ,
                            std::string *series_desc_str,
                            std::string *patient_name_str,
                            std::string *patient_code_str,
                            std::string *scalar,
                            gdcm::PixelFormat::ScalarType *stype,
                            ImgFormat *volData)
{
    gdcm::ImageReader reader;
    reader.SetFileName( filename.c_str() );
    if( !reader.Read() )
    {
        std::cerr << "Could not read image from: "
                  << filename << std::endl;
        return 0;
    }
    const gdcm::File &file = reader.GetFile();
    const gdcm::DataSet &ds = file.GetDataSet();
    const gdcm::Image &image = reader.GetImage();

    // Get Series Description
    gdcm::Tag att(0x0008,0x103e);
    if( ds.FindDataElement(att) ) {
        const gdcm::DataElement& series_desc = ds.GetDataElement( att );
        //const gdcm::ByteValue * bv = series_desc.GetByteValue();
        (*series_desc_str) += (series_desc.GetByteValue()->GetPointer());
        (*series_desc_str).resize(series_desc.GetVL());
        std::cout << "Serie Description: " << (*series_desc_str)
                  << std::endl;
    }
    // Get Patient Name
    if( ds.FindDataElement(gdcm::Tag(0x0010, 0x0010)) ) {
        const gdcm::DataElement& patient_name =
                ds.GetDataElement( gdcm::Tag(0x0010, 0x0010) );
        (*patient_name_str) += (patient_name.GetByteValue()->GetPointer());
        (*patient_name_str).resize(patient_name.GetVL());
        std::cout << "Patient Name: " << (*patient_name_str) << std::endl;
    }
    // Get Patient ID
    if( ds.FindDataElement(gdcm::Tag(0x0010, 0x0020)) ) {
        const gdcm::DataElement& patient_code =
                ds.GetDataElement(gdcm::Tag(0x0010, 0x0020) );
        (*patient_code_str) += (patient_code.GetByteValue()->GetPointer());
        (*patient_code_str).resize(patient_code.GetVL());
        std::cout << "Patient Code: " << (*patient_code_str)
                  << std::endl;
    }

    // Read data set information for each file
    // Get Slice Location (<0x0020,0x1041>)
    if(ds.FindDataElement(gdcm::Tag(0x0020, 0x1041)) ) {
        const gdcm::DataElement& location_ptr=
                ds.GetDataElement( gdcm::Tag(0x0020, 0x1041) );
        std::string location_str
                (location_ptr.GetByteValue()->GetPointer());
        location_str.resize(location_ptr.GetVL());
        const float location = atof(location_str.c_str());
        std::cout << "Location :  " << location << std::endl ;
    }
    // Get Dimension Sizes
    const unsigned int ndim = image.GetNumberOfDimensions();
    std::cout << "Number Of Dimensions: " << ndim << std::endl;

    const unsigned int *dims=image.GetDimensions();
    volData->dims[0] = dims[1];
    volData->dims[1] = dims[0];
    if (ndim == 3)
        volData->dims[2] = dims[2];
    else if (ndim == 2)
        volData->dims[2] = 0;
    std::cout << "Dimensions:  (" << volData->dims[0] << ","
              << volData->dims[1] << ","
              << volData->dims[2] << ")" << std::endl ;
    // Get Origin (<0x0020,0x0032>)
    const double *origin = image.GetOrigin();

    volData->origin[0] = origin[0];
    volData->origin[1] = origin[1];
    volData->origin[2] = origin[2];
    std::cout << "Origin:  (";
    int i;
    for(i = 0; i < 2; ++i)
    {
        std::cout << origin[i] << ",";
    }
    std::cout << origin[i] << ")" << std::endl;
    // Get Pixel Spacing (<0x0028, 0x0030>)
    volData->space[2] = 0.0;
    const double *space = image.GetSpacing();
    volData->space[0] = space[0];
    volData->space[1] = space[1];
    volData->space[2] = space[2];    // (21/05/2013 - Ting)
    // Get Slice Thickness
    if (fabs(volData->space[2]) < 1.e-7) { // Bug fixed - 21/05/2013
        const gdcm::DataElement& slice_thickness =
                ds.GetDataElement(gdcm::Tag(0x0018, 0x0050));
        const gdcm::DataElement& spacing_between_slices =
                ds.GetDataElement(gdcm::Tag(0x0018, 0x0088));
        if(!slice_thickness.IsEmpty()) {
            std::string slice_thickness_str
                    (slice_thickness.GetByteValue()->GetPointer());
            slice_thickness_str.resize(slice_thickness.GetVL());
            volData->space[2] = atof(slice_thickness_str.c_str());
        } else if (spacing_between_slices.IsEmpty()) {
            std::string spacing_between_slices_str
                    (spacing_between_slices.GetByteValue()->GetPointer());
            spacing_between_slices_str.resize(spacing_between_slices.GetVL());
            volData->space[2] = atof(spacing_between_slices_str.c_str());
        }
    }
    std::cout << "Spacing:  (" << volData->space[0] << ","
              << volData->space[1] << ","
              << volData->space[2] << ")" << std::endl;
    // Get Pixel Format
    const gdcm::PixelFormat &ptype=image.GetPixelFormat();
    // Get Samples Per Pixel (<0x0028,0x0002>)
    volData->samples = ptype.GetSamplesPerPixel();
    std::cout << "Samples per pixel: " << volData->samples << std::endl;
    // Get Bits Allocated (<0x0028,0x0100>)
    volData->nbitsalloc = ptype.GetBitsAllocated();
    std::cout << "Bits Allocated: " << volData->nbitsalloc << std::endl;
    // Get Bits Stored (<0x0028,0x0101>)
    volData->nbitsstored = ptype.GetBitsStored();
    std::cout << "Bits Stored: " << volData->nbitsstored << std::endl;
    // Get High Bit (<0x0028,0x0102>)
    volData->nhighbit = ptype.GetHighBit();
    std::cout << "High Bit: " << volData->nhighbit << std::endl;
    // Get Pixel Representation (<0x0028,0x0103>)
    const unsigned short pixelRep = ptype.GetPixelRepresentation();
    std::cout << "Pixel Representation: " << pixelRep << std::endl;
    // Get Scalar Type (???)
    *stype = ptype.GetScalarType();
    const char *stypestring = ptype.GetScalarTypeAsString();
    *scalar += stypestring;
    std::cout << "Scalar Type: " << *stype << ", " << *scalar << std::endl;
    // Get Orientation (<0x0020,0x0035> ou <0x0020,0x0037>)
    const double *dircos = image.GetDirectionCosines();
    volData->dircos[0][0] = dircos[0];
    volData->dircos[0][1] = dircos[1];
    volData->dircos[0][2] = dircos[2];
    volData->dircos[1][0] = dircos[3];
    volData->dircos[1][1] = dircos[4];
    volData->dircos[1][2] = dircos[5];
    volData->dircos[2][0] = dircos[1]*dircos[5]-dircos[2]*dircos[4];
    volData->dircos[2][1] = dircos[2]*dircos[3]-dircos[0]*dircos[5];
    volData->dircos[2][2] = dircos[0]*dircos[4]-dircos[1]*dircos[3];
    std::cout << "X: (" << volData->dircos[0][0] << "," << volData->dircos[0][1] << "," << volData->dircos[0][2] << ")" << std::endl;
    std::cout << "Y: (" << volData->dircos[1][0] << "," << volData->dircos[1][1] << "," << volData->dircos[1][2] << ")" << std::endl;
    std::cout << "Z: (" << volData->dircos[2][0] << "," << volData->dircos[2][1] << "," << volData->dircos[2][2] << ")" << std::endl;

    gdcm::Orientation::OrientationType type =
            gdcm::Orientation::GetType(dircos);
    std::cout << "Type : " << type << std::endl;
    const char *label = gdcm::Orientation::GetLabel( type );
    std::cout << "Orientation Label: " << label << std::endl;
    // Get Photometric Interpretation (<0x0028,0x0004>)
    const gdcm::PhotometricInterpretation &pi =
            image.GetPhotometricInterpretation();
    std::cout << "PhotometricInterpretation: " << pi << std::endl;

    // Get Rescale Slope (<0x0028,0x1053>)
    float slope, intercept;

    if(ds.FindDataElement(gdcm::Tag(0x0028, 0x1053)) ) {
        const gdcm::DataElement& slope_ptr=
                ds.GetDataElement( gdcm::Tag(0x0028, 0x1053) );
        std::string slope_str (slope_ptr.GetByteValue()->GetPointer());
        slope_str.resize(slope_ptr.GetVL());
        volData->slope = atof(slope_str.c_str());
    } else
        volData->slope = image.GetSlope();

    // Get Rescale Intercept (<0x0028,0x1052>)
    if(ds.FindDataElement(gdcm::Tag(0x0028, 0x1052)) ) {
        const gdcm::DataElement& intercept_ptr=
                ds.GetDataElement( gdcm::Tag(0x0028, 0x1052) );
        std::string intercept_str
                (intercept_ptr.GetByteValue()->GetPointer());
        intercept_str.resize(intercept_ptr.GetVL());
        volData->intercept = atof(intercept_str.c_str());
    } else
        volData->intercept = image.GetIntercept();

    ReadPixelData(image, ds, *stype, volData->nbitsalloc,
                  volData->nbitsstored, volData->slope,
                  volData->intercept, &volData->length, &volData->buffer,
                  &volData->umin,
                  &volData->umax);

    return 1;
}

bool Import::ValidateMediaStorageIsImage (std::string const & filename,
                                          gdcm::Defs const & defs )
{
    gdcm::Reader reader;
    reader.SetFileName( filename.c_str() );
    if( !reader.Read() )
    {
        std::cerr << "Failed to read: " << filename << std::endl;
        return 0;
    }
    const gdcm::File &file = reader.GetFile();
    const gdcm::DataSet &ds = file.GetDataSet();

    gdcm::MediaStorage ms;
    ms.SetFromFile(file);
    /*
   * Until gdcm::MediaStorage is fixed only *compile* time constant
   * will be handled
   * see -> http://chuckhahm.com/Ischem/Zurich/XX_0134
   * which make gdcm::UIDs useless :(
   */
    if( ms.IsUndefined() )
    {
        std::cerr << "Unknown MediaStorage" << std::endl;
        return 0;
    }

    gdcm::UIDs uid;
    uid.SetFromUID( ms.GetString() );
    // std::cout << "MediaStorage is " << ms << " [" << uid.GetName()
    //	    << "]" << std::endl;
    const gdcm::TransferSyntax &ts =
            file.GetHeader().GetDataSetTransferSyntax();
    uid.SetFromUID( ts.GetString() );
    //  std::cout << "TransferSyntax is " << ts << " [" << uid.GetName()
    //	    <<  "]" << std::endl;

    return gdcm::MediaStorage::IsImage( ms );
}

int Import::ImportFile(std::string &filename, const gdcm::Defs &defs,
                       ImgFormat *volData) {
    std::cout << "A file is processed ..." << std::endl;
    unsigned int len;
    std::string series_desc_str=""; // empty string
    std::string patient_name_str=""; // empty string
    std::string patient_code_str=""; // empty string
    std::string type = "";

    series_desc_str.clear();
    patient_name_str.clear();
    patient_code_str.clear();
    type.clear();
    gdcm::PixelFormat::ScalarType stype;

    if (ValidateMediaStorageIsImage (filename, defs)) {
        gdcm::Reader reader;
        reader.SetFileName( filename.c_str() );
        if( reader.Read() ) {
            const gdcm::File &file = reader.GetFile();
            const gdcm::DataSet &ds = file.GetDataSet();
            // Read Series Instance UID
            gdcm::Tag att(0x0020,0x000e);    // Series Instance UID
            const gdcm::DataElement& series_value= ds.GetDataElement( att );
            if( ds.FindDataElement(att) ) {
                const gdcm::ByteValue *bv = series_value.GetByteValue();
                len = 0;
                // Get Series Description
                if( ds.FindDataElement(gdcm::Tag(0x0008,0x103e)) ) {
                    const gdcm::DataElement& series_desc =
                            ds.GetDataElement(gdcm::Tag(0x0008,0x103e));
                    (series_desc_str) +=
                            (series_desc.GetByteValue()->GetPointer());
                    (series_desc_str).resize(series_desc.GetVL());
                    std::cout << "Serie Description: " <<
                                 (series_desc_str) << std::endl;
                }

                series_desc_str.clear();

                if (ProcessOneFile(filename, defs, &series_desc_str,
                                   &patient_name_str, &patient_code_str,
                                   &type, &stype, volData)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

bool Import::sortFunc(gdcm::DataSet const &ds1, gdcm::DataSet const &ds2) {
    gdcm::Attribute<0x0020, 0x0011> at1_ds1, at1_ds2; //numero da serie
    gdcm::Attribute<0x0020, 0x1041> at2_ds1, at2_ds2; //posicao do slice

    at1_ds1.Set(ds1); //numero da serie da imagem 1
    at1_ds2.Set(ds2); //numero da serie da imagem 2

    at2_ds1.Set(ds1); //posicao do paciente da imagem 1
    at2_ds2.Set(ds2); //posicao do paciente da imagem 2

    if (at1_ds1 < at1_ds2)
        return true;
    else
        return at2_ds1 < at2_ds2;
}

bool Import::ValidateMediaStorageIsImage(gdcm::Defs const & defs,
                                         std::string filename) {
    gdcm::Reader reader;
    reader.SetFileName(filename.c_str());
    if (!reader.Read()) {
        std::cerr << "Failed to read: " << filename << std::endl;
        return 0;
    }
    const gdcm::File &file = reader.GetFile();

    gdcm::MediaStorage ms;
    ms.SetFromFile(file);

    if (ms.IsUndefined()) {
        std::cerr << "Unknown MediaStorage" << std::endl;
        return 0;
    }

    gdcm::UIDs uid;
    uid.SetFromUID(ms.GetString());

    const gdcm::TransferSyntax &ts =
            file.GetHeader().GetDataSetTransferSyntax();
    uid.SetFromUID(ts.GetString());


    return gdcm::MediaStorage::IsImage(ms);
}

char *Import::realloc_mem(const gdcm::PixelFormat::ScalarType & stype,
                          char * buf, unsigned int old_len, unsigned int new_len) {
    unsigned short *tmpbuf16;
    char *tmpbuf8;

    switch (stype) {
    case 0:  // UINT8
    case 1:  // INT8
        tmpbuf8 = new char[new_len];
        std::memmove(tmpbuf8, buf, old_len);
        delete[] buf;
        return tmpbuf8;
        break;
    case 2:  // UINT12
        break;
    case 3:  // INT12
        break;
    case 4:  // UINT16
    case 5:  // INT16
        tmpbuf16 = new unsigned short[new_len / 2];
        std::memmove(reinterpret_cast<char *>(tmpbuf16), buf, old_len);
        delete[] buf;
        return reinterpret_cast<char *>(tmpbuf16);
        break;
    case 6:  // UINT32
        break;
    case 7: // INT32
        break;
    case 8: // FLOAT16
        break;
    case 9: // FLOAT32
        break;
    case 10: // FLOAT64
        break;
    case 11: // UNKONOWN
        break;
    }
}

int Import::ImportDirectory(std::string &filename, const gdcm::Defs &defs,
                            ImgFormat *imgformat) {

    gdcm::Directory d;

    // recursive loading
    unsigned int nfiles = d.Load(filename, 1);

    gdcm::Sorter sorter;

    int res = 0;

    sorter.SetSortFunction(sortFunc);

    sorter.StableSort(d.GetFilenames()); // IMPORTANT
    std::cout << "Sorter:" << std::endl;

    std::vector<std::string> const &filenames = sorter.GetFilenames();

    std::cout << "number of files:" << nfiles << std::endl;

    gdcm::Scanner s;
    s.AddTag(gdcm::Tag(0x0020, 0x000e)); // Series Instance UID

    s.Scan(sorter.GetFilenames());

    // Count how many different IPP there are:
    const gdcm::Scanner::ValuesType &values = s.GetValues();
    unsigned int nvalues = values.size();
    std::cout << "There are " << nvalues << " different volume data"
              << std::endl;

    unsigned int totalimages;
    unsigned int len;
    bool flag;
    std::string series_desc_str = "";
    std::string modality_desc_str = "";
    std::string patient_name_str = "";
    std::string patient_code_str = "";
    std::string type = "";
    for (gdcm::Scanner::ValuesType::const_iterator vit = values.begin();
         vit != values.end(); vit++) {
        std::cout << "Serie: " << (*vit).c_str() << std::endl;
        totalimages = 0;

        flag = false;
        series_desc_str.clear();
        modality_desc_str.clear();
        patient_name_str.clear();
        patient_code_str.clear();
        type.clear();
        gdcm::PixelFormat::ScalarType stype;

        for (gdcm::Directory::FilenamesType::const_iterator it =
             filenames.begin(); it != filenames.end(); it++) {
            if (ValidateMediaStorageIsImage(defs, *it)) {
                gdcm::Reader reader;
                //std::cout << "filename: " << *it << std::endl;
                reader.SetFileName((*it).c_str());
                if (reader.Read()) {
                    const gdcm::File &file = reader.GetFile();
                    const gdcm::DataSet &ds = file.GetDataSet();
                    // Read Series Instance UID
                    gdcm::Tag att(0x0020, 0x000e);    // Series Instance UID
                    const gdcm::DataElement& series_value = ds.GetDataElement(
                                att);
                    if (ds.FindDataElement(att)) {
                        const gdcm::ByteValue *bv = series_value.GetByteValue();
                        if (strncmp(bv->GetPointer(), (*vit).c_str(),
                                    (*vit).size()) == 0) {
                            // Get Series Description
                            if (ds.FindDataElement(gdcm::Tag(0x0008, 0x103e))) {
                                const gdcm::DataElement& series_desc =
                                        ds.GetDataElement(
                                            gdcm::Tag(0x0008, 0x103e));
                                (series_desc_str) +=
                                        (series_desc.GetByteValue()->GetPointer());
                                (series_desc_str).resize(series_desc.GetVL());

                            }

                            series_desc_str.clear();
                            if (!flag) {

                                if (ProcessOneFile(*it, defs, &series_desc_str,
                                                   &patient_name_str, &patient_code_str,
                                                   &type, &stype, imgformat)) {

                                    flag = true;

                                    totalimages++;
                                    res++;
                                } else
                                    break;
                            } else {
                                // Just extract the pixeldata
                                char *buf;
                                len = 0;
                                gdcm::ImageReader reader;
                                reader.SetFileName((*it).c_str());
                                if (!reader.Read()) {
                                    std::cerr << "Could not read image from: "
                                              << (*it).c_str() << std::endl;
                                } else {
                                    const gdcm::File &file = reader.GetFile();
                                    const gdcm::DataSet &ds = file.GetDataSet();
                                    const gdcm::Image &image =
                                            reader.GetImage();
                                    // Assumption: slices are equally spaced (Ting: 28/05/2013)
                                    if (totalimages == 1) {
                                        const double *center =
                                                image.GetOrigin();
                                        double space =
                                                ((center[0]
                                                 - imgformat->origin[0])
                                                * imgformat->dircos[2][0]
                                                + (center[1]
                                                - imgformat->origin[1])
                                                * imgformat->dircos[2][1]
                                                + (center[2]
                                                - imgformat->origin[2])
                                                * imgformat->dircos[2][2]);
                                        imgformat->space[2] = abs(space);
                                        std::cout << "space = " << space
                                                  << std::endl;
                                    }

                                    ReadPixelData(image, ds, stype,
                                                  imgformat->nbitsalloc,
                                                  imgformat->nbitsstored,
                                                  imgformat->slope,
                                                  imgformat->intercept, &len, &buf,
                                                  &imgformat->umin, &imgformat->umax);

                                    totalimages++;
                                    imgformat->buffer = realloc_mem(stype,
                                                                    imgformat->buffer,
                                                                    (totalimages - 1) * len,
                                                                    totalimages * len);
                                    std::memmove(
                                                &(imgformat->buffer[(totalimages
                                                                     - 1) * len]), buf, len);
                                    imgformat->length += len;
                                    res++;
                                    if (len)
                                        delete[] buf;
                                }
                            }

                        }
                    }
                }
            }
        }

        if (flag) {
            imgformat->dims[2] = totalimages;
            std::cout << "nrows: " << imgformat->dims[0] << "; ncols: "
                      << imgformat->dims[1] << "; totalimages: "
                      << imgformat->dims[2] << std::endl;
            std::cout << "spacings: (" << imgformat->space[0] << ","
                      << imgformat->space[1] << "," << imgformat->space[2] << ")"
                      << std::endl;
            std::cout << "umin : " << imgformat->umin << "; umax : "
                      << imgformat->umax << std::endl;
        }
    }

    return res;
}

/*==========================================================
  Public Methods
  ===========================================================*/
int Import::DICOMImage (std::string &filename, ImgFormat *volData)
{
    std::string xmlpath;

    if( !gdcm::System::FileExists(filename.c_str()) )
    {
        std::cerr << filename << " does not exist on the system" << std::endl;
        return 0;
    }

    // Return the sinleton instance
    gdcm::Global& g = gdcm::Global::GetInstance();

    // Locate the XML dict

    const char *xmlpathenv = getenv("GDCM_RESOURCES_PATH");
    if( xmlpathenv )
    {
        // Make sure to look for XML dict in user explicitly
        // specified dir first:
        xmlpath = xmlpathenv;
        // Prepend path at the begining of the path list
        if( !g.Prepend( xmlpath.c_str() ) )
        {
            std::cerr << "specified Resources Path is not valid: "
                      << xmlpath << std::endl;
            return 0;
        }
    }


    // Retrieve the default/internal (Part 3)
    const gdcm::Defs &defs = g.GetDefs();

    int res = 0;

    if(gdcm::System::FileIsDirectory(filename.c_str()) ) {
        std::cout << "Reading file directory is not supported ..." << std::endl;
        res = ImportDirectory(filename, defs, volData);
    } else {
        res = ImportFile (filename, defs, volData);
        LPS_ReorientImageVolume(volData);
    }

    return res;
}

