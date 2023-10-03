# DNGE
DNGE is a free and open source canvas-based graphical user interface framework for writing C++ GUI applications. CBGUI can be automatically scaled and aligned for all resolutions, but the font size of the text must be adjusted manually.

## Features
* Canvas-based
* Easy to align to different Resolutions
* Renderer agnostic
* Freetype2 Based Font Builder
* Font texture atlas
* Unicode aware
* Text Format and Multiple font supported

## Example
See Samples/Sample1
![Main_menu](https://user-images.githubusercontent.com/117200113/199342945-a33c2f13-f945-423c-b329-4abc9f21b4ec.jpg)

## License
CBGUI is licensed under MIT license. See [LICENSE](LICENSE)
For third-party licenses see [ThirdPartyLicenses.txt](ThirdParty/ThirdPartyLicenses.txt)

## How to build
1. Install latest Visual Studio
2. Get latest Freetype2 (http://freetype.org/download.html)
3. Place the Freetype2 library in the file named "ThirdParty".
4. Change the freetype2 library filename to "freetype2".	("CBGUI\ThirdParty\freetype2\include")
5. Open the solution(CBGUI.sln) then build it.
