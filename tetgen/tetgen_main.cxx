///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TetGen                                                                    //
//                                                                           //
// A Quality Tetrahedral Mesh Generator and A 3D Delaunay Triangulator       //
//                                                                           //
// Version 1.5                                                               //
// November 4, 2013                                                          //
//                                                                           //
// TetGen is freely available through the website: http://www.tetgen.org.    //
//   It may be copied, modified, and redistributed for non-commercial use.   //
//   Please consult the file LICENSE for the detailed copyright notices.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "tetgen.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// main()    The command line interface of TetGen.                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  tetgenbehavior b;

  tetgenio in, addin, bgmin;

  if (!b.parse_commandline(argc, argv)) {
    terminatetetgen(NULL, 10);
  }

  // Read input files.
  if (b.refine) { // -r
    if (!in.load_tetmesh(b.infilename, (int) b.object)) {
      terminatetetgen(NULL, 10);
    }
  } else { // -p
    if (!in.load_plc(b.infilename, (int) b.object)) {
      terminatetetgen(NULL, 10);
    }
  }
  if (b.insertaddpoints) { // -i
    // Try to read a .a.node file.
    addin.load_node(b.addinfilename);
  }
  if (b.metric) { // -m
    // Try to read a background mesh in files .b.node, .b.ele.
    bgmin.load_tetmesh(b.bgmeshfilename, (int) b.object);
  }

  tetrahedralize(&b, &in, NULL, &addin, &bgmin);

  return 0;
}

////                                                                       ////
////                                                                       ////
//// main_cxx /////////////////////////////////////////////////////////////////

