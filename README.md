# BRT AmbiPlayer (BAMBI)

The BRT AmbiPlayer (BAMBI) is a 3D audio player based on a class structure programmed in C++. This player allows the user to reproduce and rotate sound fields, which can be either a native ambisonic sound field or a sound field created from a point source.
Ambisonic field decoding is performed using a virtual loudspeaker approach, maintaining coherence with the BRT rendering pipeline.

## Citable references

Bachelor’s thesis report describing the theory and implementation of the project:

* Liceras-Ramirez, R. (2026). *Binaural Ambisonic Audio Player* [Bachelor’s thesis, University of Málaga]. Unpublished.

https://www.diana.uma.es/brt-ambiplayer/

## Additional tools

In addition to this main repository, there are other repositories with associated tools:

* OpenFrameworks: An open-source C++ toolkit for creative coding
* ofxBRTLibrary: An OpenFrameworks addon for the BRT Library. Source code available at: https://github.com/GrupoDiana/ofxBRTLibrary
* ofxImGui: An OpenFrameworks addon that allows the use of ImGui. Source code available at: https://github.com/jvcleave/ofxImGui

## Build

To use the project, download it and place it in the `apps` folder of your OpenFrameworks installation. We are working with OpenFrameworks version **of_v20250319_vs_64_release**.

Open the Visual Studio solution file and compile the project from there.

This project has been developed in standard C++, but it has only been tested on Windows 10.

## Executable

A preconfigured project is included in Release 1.0, which only needs to be compiled. It contains a first-order ambisonic test audio file.

## Credits

**Author and main developer**
- [Rocío Liceras-Ramírez](https://github.com/rocioliceras)

**Supervisor**
- [Arcadio Reyes-Lecuona](https://github.com/areyesl) (DIANA Research Group, University of Málaga) – Contact: areyes@uma.es

**Co-supervisor**
- [Daniel Gonzalez-Toledo](https://github.com/dgonzalezt) (University of Málaga)

## Support

For support or further information, please contact Rocío Liceras-Ramírez (rocioliceras@uma.es) or Arcadio Reyes-Lecuona (areyes@uma.es).

## License

This software is distributed under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Copyright (c) for each module belongs to its respective authors, who may vary as there are contributions from different institutions in this software. This information is specified in the headers of the corresponding source files.

This library includes pieces of code from the BRT Library, shared under the GPLv3 license and copyrighted (c) by the University of Málaga (contact: areyes@uma.es). See file headers for details.

## Acknowledgement

This software has been developed at the DIANA Research Group, University of Málaga.

This project has been partially funded by the Spanish Ministerio de Ciencia e Innovación under the SONIX project (PID2023-152547NB-I00). https://www.diana.uma.es/sonix