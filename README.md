## VMTK: Multi-volume Raycasting

Developers

  Wu Shin-Ting (coordinator)
  Raphael Voltoline Ramos
  Wallace Souza Loos
  José Angel Iván Rubianes Silva

This program is a simplified version of the software 
available in: http://www.dca.fee.unicamp.br/projects/mtk/wu_loos_voltoline_rubianes/downloads.html
Running the executable file you should be attentive to the system requirements (programmable GPU with >=1GB graphics memory and supports OpenGL-GLSL >= 3.3)	

The raycasting algorithm was based in the work presented here: https://www.cg.tuwien.ac.at/courses/Visualisierung/2010-2011/Beispiel1/Moellinger_Ludwig/

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

executable inputVolume1  inputVolume2 registeredMatrix

executable: name of the executable
inputVolume1: reference volume
inputVolume2: float volume
registeredMatrix: co-register matrix

The volumes and the co-register matrix are provided. 
They are in the folder. The shader folder should be in the same directory of the executable.

# Instructions

## Navigation

w: rotate the volume around x axis (clockwise)
s: rotate the volume around x axis (counter-clockwise)
a: rotate the volume around z axis (clockwise)
d: rotate the volume around z axis (counter-clockwise)
q: rotate the volume around y axis (clockwise)
e: rotate the volume around y axis (counter-clockwise)

r: reset the transformation

#Threshold

+: increase threshold
-: decrease threshold

#Controling blender

b: increase reference volume
n: increase floating volume

#Sagittal Clipping

o: browse sagittal slices from left to right
p: browse sagittal slices from right to left
