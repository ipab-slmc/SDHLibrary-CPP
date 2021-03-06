//======================================================================
/*!
  \addtogroup sdh_library_cpp_demo_programs_group
  @{
*/
/*!

    \file
     \section sdhlibrary_cpp_sdh_demo_simple_general General file information

       \author   Dirk Osswald
       \date     2007-01-18

     \brief
       Very simple demonstration program using the SDHLibrary-CPP: Make an attached %SDH move one finger
       with "pose" controller type (coordinated position control).
       See \ref demo_simple__help__ "__help__" and online help ("-h" or "--help") for available options.


       This code contains only the very basicst use of the features
       provided by the SDHLibrary-CPP. For more sophisticated
       applications see the other demo-*.cpp programms, or of course
       the html/pdf documentation.

     \section sdhlibrary_cpp_sdh_demo_simple_copyright Copyright

     Copyright (c) 2007 SCHUNK GmbH & Co. KG

     <HR>
     \internal

       \subsection sdhlibrary_cpp_sdh_demo_simple_details SVN related, detailed file specific information:
         $LastChangedBy: Osswald2 $
         $LastChangedDate: 2013-11-27 16:12:49 +0100 (Wed, 27 Nov 2013) $
         \par SVN file revision:
           $Id: demo-simple.cpp 11045 2013-11-27 15:12:49Z Osswald2 $

     \subsection sdhlibrary_cpp_sdh_demo_simple_changelog Changelog of this file:
         \include demo-simple.cpp.log

*/
/*!
  @}
*/
//======================================================================

#include <iostream>
#include <vector>


// Include the cSDH interface
#include "sdh/sdh.h"
#include "sdh/util.h"
#include "sdh/sdhlibrary_settings.h"
#include "sdh/basisdef.h"
#include "sdhoptions.h"

USING_NAMESPACE_SDH

/*!
    \anchor sdhlibrary_cpp_demo_simple_cpp_vars
    \name   Some informative variables

    @{
*/
//! \anchor demo_simple__help__
char const* __help__      =
    "Move proximal and distal joints of finger 1 three times by 10 degrees.\n"
    "(C++ demo application using the SDHLibrary-CPP library.)\n"
    "\n"
    "- Example usage:\n"
    "  - Make SDH connected via Ethernet move.\n"
    "    The SDH has IP-Address 192.168.1.42 and is attached to TCP port 23.\n"
    "    (Requires at least SDH-firmware v0.0.3.1)\n"
    "    > demo-simple --tcp=192.168.1.42:23\n"
    "     \n"
    "  - Make SDH connected to port 2 = COM3 move:\n"
    "    > demo-simple -p 2\n"
    "     \n"
    "  - Make SDH connected to USB to RS232 converter 0 move:\n"
    "    > demo-simple --sdh_rs_device=/dev/ttyUSB0 \n"
    "     \n"
    "  - Get the version info of both the joint controllers and the tactile \n"
    "    sensor firmware from an SDH connected via Ethernet.\n"
    "    The joint controllers and the tactile sensors have a common IP-Address,\n"
    "    here 192.168.1.42. The SDH controller is attached to the \n"
    "    default TCP port 23 and the tactile sensors to the default TCP port 13000.\n"
    "    (Requires at least SDH-firmware v0.0.3.2)\n"
    "    > demo-simple --tcp=192.168.1.42 --dsa_tcp -v\n"
    "     \n"
    "  - Get the version info of an SDH connected to port 2 = COM3 \n"
    "    > demo-simple --port=2 -v\n";
char const* __author__    = "Dirk Osswald: dirk.osswald@de.schunk.com";
char const* __url__       = "http://www.schunk.com";
char const* __version__   = "$Id: demo-simple.cpp 11045 2013-11-27 15:12:49Z Osswald2 $";
char const* __copyright__ = "Copyright (c) 2007 SCHUNK GmbH & Co. KG";

//  end of doxygen name group sdhlibrary_cpp_demo_simple_cpp_vars
//  @}

char const* usage =
  "usage: demo-simple [options]\n"
  ;


int main( int argc, char** argv )
{
    SDH_ASSERT_TYPESIZES();

    //---------------------
    // handle command line options: set defaults first then overwrite by parsing actual command line
    cSDHOptions options;

    options.Parse( argc, argv, __help__, "demo-simple", __version__, cSDH::GetLibraryName(), cSDH::GetLibraryRelease() );
    //
    //---------------------

    //---------------------
    // initialize debug message printing:
    cDBG cdbg( options.debug_level > 0, "red", options.debuglog );
    g_sdh_debug_log = options.debuglog;

    cdbg << "Debug messages of " << argv[0] << " are printed like this.\n";

    // reduce debug level for subsystems
    options.debug_level-=1;
    //---------------------

    int    iFinger        = 0;     // The index of the finger to move

    try
    {
        // Create an instance "hand" of the class cSDH:
        cSDH hand( false, options.use_fahrenheit, options.debug_level );
        cdbg << "Successfully created cSDH instance\n";

        // Open configured communication to the SDH device
        options.OpenCommunication( hand );
        cdbg << "Successfully opened communication to SDH\n";

        // Switch to "pose" controller mode and set default velocities first:
        hand.SetController( hand.eCT_POSE );
        hand.SetAxisTargetVelocity( hand.All, 40.0 );

        // Now perform some action:
        //   get the current actual axis angles of finger iFinger:
        std::vector<double> faa = hand.GetFingerActualAngle( iFinger );

        //   sometimes the actual angles are reported slightly out of range
        //   (Like -0.001 for axis 0 ). So limit the angles to the allowed range:
        ToRange( faa, hand.GetFingerMinAngle( iFinger ), hand.GetFingerMaxAngle( iFinger ) );

        //   modify faa by decrementing the proximal and the distal axis angles
        //   (make a copy fta of faa and modify that to keep actual pose available)
        std::vector<double> fta = faa;

        fta[1] -= 10.0;
        fta[2] -= 10.0;

        //   keep fta in range too:
        ToRange( fta, hand.GetFingerMinAngle( iFinger ), hand.GetFingerMaxAngle( iFinger ) );

        std::cout << "Moving finger " << iFinger << " between faa=" << faa << " and fta=" << fta << "\n";

        //   now move for 3 times between these two poses:
        for (int i=0; i<3; i++ )
        {
            // set a new target angles
            hand.SetFingerTargetAngle( iFinger, fta );

            // and make the finger move there:
            hand.MoveFinger( iFinger );


            // set a new target angles
            hand.SetFingerTargetAngle( iFinger, faa );

            // and make the finger move there:
            hand.MoveFinger( iFinger );

        }


        // Finally close connection to SDH again, this switches the axis controllers off
        hand.Close();
    }
    catch (cSDHLibraryException* e)
    {
        std::cerr << "demo-simple main(): An exception was caught: " << e->what() << "\n";
        delete e;
    }
}
//----------------------------------------------------------------------


//======================================================================
/*
  Here are some settings for the emacs/xemacs editor (and can be safely ignored)
  (e.g. to explicitely set C++ mode for *.h header files)

  Local Variables:
  mode:C++
  mode:ELSE
  End:
*/
//======================================================================]
