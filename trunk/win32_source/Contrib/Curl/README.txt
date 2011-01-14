libCurl
Version: 7.21.3
Copyright: Daniel Stenberg, 1996 - 2010
License: MIT-X/derivative (see LICENSE.txt)

Source Code Location: Contrib\Curl
How to Include:  #include "Curl/curl.h" (Usually no need for other files)

Changes made for WaitZar:
   * Removed dllimport/dllexport imperatives
   * Added to the project file manually.
   * Generally removed all macros geared towards making a "library", since we're including it statically.

