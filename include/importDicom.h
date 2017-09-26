/*
 *  importDicom.h
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

class Import {
 public:
  typedef struct ImgFormat
  {
    unsigned short dims[3];  ///< dims[0]: nrows; dims[1]: ncols; dims[2]: nplanes
    double space[3];       ///< space[0]: width; space[1]: height; space[2]: depth
    double origin[3];        ///< origin
    double dircos[3][3];     ///< basis vectors with respect to LPS
    unsigned short samples;  ///< samples per pixel
    unsigned short nbitsalloc;  ///< bits allocated
    unsigned short nbitsstored; ///< bits stored
    unsigned short nhighbit;    ///< high bit
    float slope;                ///< rescale slope
    float intercept;            ///< rescale intercept
    unsigned int length;
    char *buffer;
    int umin, umax;
  } ImgFormat;

  /*==========================================================
    Public Methods
    ===========================================================*/
  Import() {};  ///< Constructor
  ~Import() {}; ///< Destructor

/**
* @brief load the volume
* @param [in] filename: filename of the volume.
* @param [out] imgformat: volume data
*/
  int DICOMImage (std::string &filename, ImgFormat *imgformat);

  /*==========================================================
    Private Methods
    ===========================================================*/
 private:
/**
* @brief reorient the volume to LPS.
* @param [in] volData: volume data.
* @return true when the reorientation is made
*/
  bool LPS_ReorientImageVolume (ImgFormat *volData);
  bool ReadPixelData(const gdcm::Image &image, const gdcm::DataSet &ds,
             const gdcm::PixelFormat::ScalarType & stype,
             unsigned short nbitsalloc, unsigned short nbitsstored,
             float slope, float intercept,
             unsigned int *len, char **buf,
             int *umin, int *umax);
  int ProcessOneFile( std::string const & filename,
              gdcm::Defs const & defs ,
              std::string *series_desc_str,
              std::string *patient_name_str,
              std::string *patient_code_str,
              std::string *scalar,
              gdcm::PixelFormat::ScalarType *stype,
              ImgFormat *volData);
  bool ValidateMediaStorageIsImage (std::string const & filename,
                    gdcm::Defs const & defs );

  int ImportFile(std::string &filename, const gdcm::Defs &defs,
         ImgFormat *volData);

  int ImportDirectory(std::string &filename, const gdcm::Defs &defs,
          ImgFormat *imgformat);
  static bool sortFunc(gdcm::DataSet const &ds1, gdcm::DataSet const &ds2);
  bool ValidateMediaStorageIsImage(gdcm::Defs const & defs,
          std::string filename);
  char *realloc_mem(const gdcm::PixelFormat::ScalarType & stype,
          char * buf, unsigned int old_len, unsigned int new_len);
};


