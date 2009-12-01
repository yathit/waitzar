All projects in this directory were contributions (direct or indirect) from other
authors. They are licensed under separate but compatible licenses from the main
code of WaitZar. Please see the README files in each sub-directory for more information,
and the NOTICE file in the root directory for our rationale for using each project.

WaitZar includes the "contrib" folder, so including these headers only requires including 
the relevant folders, e.g.:

#include "Pulp Core/PulpCoreFont.h"

If you want to link to your own Boost install, it's easy enough; just delete the boost directory
  (and add your boost install location as an "Additional Include Directory").
All Boost includes take advantage of the case-insensitive nature of Windows files, so:

#include "boost/config.hpp"
and
#include "Boost/config.hpp"

...will both link to either your version of Boost or the one we've cached locally. Note that
we do not recommend going much below Boost 1.38.

