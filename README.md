## VMTK: Multi-volume Raycasting

Developers

  - Wu Shin-Ting (coordinator)  
  - Raphael Voltoline Ramos  
  - Wallace Souza Loos  
  - José Angel Iván Rubianes Silva  

This program is a simplified version of the software 
available in: http://www.dca.fee.unicamp.br/projects/mtk/wu_loos_voltoline_rubianes/downloads.html.
Running the executable file you should be attentive to the system requirements (programmable GPU with >=1GB graphics memory and supports OpenGL-GLSL >= 3.3)	

The raycasting algorithm was based in the work presented here: https://www.cg.tuwien.ac.at/courses/Visualisierung/2010-2011/Beispiel1/Moellinger_Ludwig/

The co-register matrix, the plane equation (for multiplanar reformatting) and six [volumes](http://www.dca.fee.unicamp.br/projects/mtk/wu_loos_voltoline_rubianes/downloads/volumes.zip) : 
MT1-FLAIR, T1-T2, T1-PET, T1-SPECT/ICTAL abd T1-SPECT/INTERICTAL are provided for the test.

## This code was compiled using the follow specifications:

1) g++ version: 5.3
2) cmake version: 3.6
3) GDCM version: 2.6.7
4) Glew: 2.0
5) Freeglut 3.0
6) OpenGL >= 3.3

## Libraries and tools required

1) GDCM

Grassroots DICOM: Cross-platform DICOM implementation
https://sourceforge.net/projects/gdcm/

2) GLEW
http://glew.sourceforge.net/

3) Freeglut

http://freeglut.sourceforge.net/index.php#download

## Usage

executable inputVolume1 inputVolume2 inputVolume3 inputFolderVolume4 inputVolume5 inputVolume6 
registeredMatrix_V1-V2 registeredMatrix_V1-V3 registeredMatrix_V1-V4 registeredMatrix_V1-V5 registeredMatrix_V1-V6 equationMPR

executable: name of the executable  
inputVolume1: reference volume  
inputVolume2: float volume  
inputVolume3: float volume  
inputVolume4: float volume  
inputVolume5: float volume  
inputVolume6: float volume  
registeredMatrix_V1-V2: co-register matrix for inputVolume1 and inputVolume2  
registeredMatrix_V1-V3: co-register matrix for inputVolume1 and inputVolume3  
registeredMatrix_V1-V4: co-register matrix for inputVolume1 and inputVolume4  
registeredMatrix_V1-V5: co-register matrix for inputVolume1 and inputVolume5  
registeredMatrix_V1-V6: co-register matrix for inputVolume1 and inputVolume6  
equationMPR: plane equation for multiplanar reformatting (optional parameter)  

The volumes and the co-register matrix are provided. 
They are in the folder. The shader folder should be in the same directory of the executable.

# Instructions

w: rotate the volume around x axis (clockwise)  
s: rotate the volume around x axis (counter-clockwise)  
a: rotate the volume around z axis (clockwise)  
d: rotate the volume around z axis (counter-clockwise)  
q: rotate the volume around y axis (clockwise)  
e: rotate the volume around y axis (counter-clockwise)  
r: reset the transformation  


### Threshold

+: increase threshold  
-: decrease threshold  


### Controling blender

b: increase reference volume  
n: increase floating volume  
	

### Sagittal Clipping

o: browse sagittal slices from left to right  
p: browse sagittal slices from right to left  

	
### Multiplanar reformatting

1: Enable  
2: Disable  
