/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------   circles.inc   --------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
/*                                                                                                             */
/*  macro for creating a piece of a circle with a boxlike coordinate system                                    */
/*  macro for creating a gold circle with random plates on it                                                  */
/*                                                                                                             */
/*                                                                                                             */
/*                                                                                   written by Manuel Moser   */
/*                                                                                       moser.manuel@gmx.de   */
/*                                                                                                             */
/***************************************************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ------ constant values -------------- */
#declare Nix        =      0.0002 ;

// --------------------
// ---- sort macro ----
// --------------------

#macro SortValues ( Value1, Value2 )
  #if ( Value2 < Value1 )
    #local   ValueTemp = Value1;
    #declare Value1    = Value2;
    #declare Value2    = ValueTemp;
  #end
#end

// ----------------------------------------------------------------------------------
// ---- gets a randomvalue from a specified range with a certain number of steps ----
// ----------------------------------------------------------------------------------

#macro RandomValue ( RandomStream, Minimum, Maximum, Steps )
  ( ( int(rand(RandomStream) * Steps) / (Steps - 1) ) * (Maximum - Minimum) + Minimum )
#end

// -------------------------------
// ---- cylinderblock routine ----
// -------------------------------

//  Generates a block of a ring with coordinates like for a real box
//  X represents the axis wich goes around the origin. The 0 position 
//    is in the direction of the real -Z axis. 360 are one complete round.
//    You can alos give values larger than 360 or smaller than 0. The block
//    will always go from the the smaller value to the higher value.
//  Y is the height of the block and identical to the real Y axis
//  Z is the radius of the blocks. Z has to be a positive value or 0.
//
//  Call: cylinder_block ( X1, Y1, Z1, X2, Y2, Z2 )
//
//
//                        Y   /|\ 
//                           / | \      
//                             |  
//                                                           real coordinates
//                                
//       Point 2:              |        
//       ( 180, 1, 2 )  ________    
//                     |\       ----___
//                     | \     |       --__                             Y 
//                     |  \______          --_                Z     /|\
//                __--- \  |     ---__        -_               ___ / | \ 
//         |   _--       \ |   |      --_       \             |\     |  
//         | _-           \|_____        -_      \            | \    |   
//         |/                    ---__     \      \              \   |   
//          ----            \  |      --_   \      \              \  |
//                                       -_  |      |              \ |      \  X
//          x                 \|           \ |      |               \|       \      
//                             o            /       |                o--------                
//                              \          /       /|                        /
//                                   ____--       / |                       /
//                                \ |\           /  |        
//                     Point 1:     | \        _-  /         
//                     ( 0, 0, 1 )  |  \_____--   /  
//                                   \  |        /  
//                                    \ |      _-  
//                                     \|____--     
//                          
//                                        
//                                        \   
//                                         \ |
//                                          \|  Z
//                                        ---
//


#macro cylinder_block ( X1, Y1, Z1, X2, Y2, Z2 )
  // -------- fixing given values --------
  SortValues ( X1, X2 )
  SortValues ( Y1, Y2 )
  //SortValues ( Z1, Z2 )
  #if ( X2 > X1 + 360 )
    #warning concat ( "cylinder block: More than one round given. X1: ", str ( X1, 6, 2 ), " X2: ", str ( X2, 6, 2 ), " (fixing X2 to X1 + 360)\n" ) 
    #local X2 = X1 + 360;
  #end
  #if ( Z1 < 0 )
    #warning concat ( "cylinder block: negative Value for Z1: ", str ( Z2, 6, 2 ), " (fixing Z1)\n" ) 
    #local Z1 = abs(Z1);
  #end
  #if ( Z2 < 0 )
    #warning concat ( "cylinder block: negative Value for Z2: ", str ( Z2, 6, 2 ), " (fixing Z2)\n" ) 
    #local Z2 = abs(Z2);
  #end
  SortValues ( Z1, Z2 )

  // -------- cylinders --------
  #local cylinderobject = 
  #if (!(( X1 = X2 )|( Y1 = Y2 )|( Z1 = Z2 )))
    #if ( Z1 = 0 )
      cylinder {
        < 0, Y1, 0 >,
        < 0, Y2, 0 >,
        Z2
      }
    #else
      difference {
        cylinder {
          < 0, Y1, 0 >,
          < 0, Y2, 0 >,
          Z2
        }
        cylinder {
          < 0, Y1 - Nix, 0 >,
          < 0, Y2 + Nix, 0 >,
          Z1
        }
      }
    #end
  #end
  
  // -------- final object --------
  #if (( X1 = X2 )|( Y1 = Y2 )|( Z1 = Z2 ))
    // block not visible 
    //#warning concat ( "cylinder block is invisible\n" ) 
    box { 0, 0 pigment { color rgbt < 0,0,0,1 > } }
  #else
    // block is visible 
    #if ( X2 < X1 + 180 )
      // block is smaller than 180°
      #if ( X2 < X1 + 90 )
        // block is smaller than 90°
        intersection {
          object { cylinderobject }
          // with exact boxes
          box {
            <                                  0, Y1 - Nix, - cos(radians(X2 - X1)) * Z1      + Nix >,
            < + sin(radians(X2 - X1)) * Z2 + Nix, Y2 + Nix,                              - Z2 - Nix > 
            rotate < 0, - X1, 0 >
          }
          box {
            <                                  0, Y1 - Nix, - cos(radians(X2 - X1)) * Z1      + Nix >,
            < - sin(radians(X2 - X1)) * Z2 - Nix, Y2 + Nix,                              - Z2 - Nix > 
            rotate < 0, - X2, 0 >
          }
        }
      #else
        // block is is smaller than 180° but not smaller than 90°
        #if ( X2 = X1 + 90 )
          // block is exactly 90° 
          intersection {
            object { cylinderobject }
            // with quarter boxes
            box {
              <          0, Y1 - Nix,          0 >,
              < + Z2 + Nix, Y2 + Nix, - Z2 - Nix > 
              rotate < 0, - X1, 0 >
            }
          }
        #else
          // block larger than 90° and smaller than 180°
          intersection {
            object { cylinderobject }
            // with half boxes
            box {
              <          0, Y1 - Nix, - cos(radians(X2 - X1)) * Z2 + Nix >,
              < + Z2 + Nix, Y2 + Nix,                         - Z2 - Nix > 
              rotate < 0, - X1, 0 >
            }
            box {
              <          0, Y1 - Nix, - cos(radians(X2 - X1)) * Z2 + Nix >,
              < - Z2 - Nix, Y2 + Nix,                         - Z2 - Nix > 
              rotate < 0, - X2, 0 >
            }
          }
        #end
      #end
    #else
      // block is not smaller than 180°
      #if ( X2 = X1 + 180 )
        // block ist exactly 180° 
        intersection {
          object { cylinderobject }
          // with half boxes
          box {
            <          0, Y1 - Nix, + Z2 + Nix >,
            < + Z2 + Nix, Y2 + Nix, - Z2 - Nix > 
            rotate < 0, - X1, 0 >
          }
        }
      #else
        // block is larger than 180°
        #if ( X2 = X1 + 360 )
          // block is exactly 360° (a complete ring)
          object { cylinderobject }
        #else 
          // block is larger than 180° but not 360°
          intersection {
            object { cylinderobject }
            union {
              // with half boxes
              box {
                <          0, Y1 - Nix, + Z2 + Nix >,
                < + Z2 + Nix, Y2 + Nix, - Z2 - Nix > 
                rotate < 0, - X1, 0 >
              }
              box {
                <          0, Y1 - Nix, + Z2 + Nix >,
                < - Z2 - Nix, Y2 + Nix, - Z2 - Nix > 
                rotate < 0, - X2, 0 >
              }
            }
          }
        #end
      #end
    #end
  #end
#end

// -----------------------------------------------
// ---- gold circle with random silver plates ----
// -----------------------------------------------

#macro MyCircle ( MainRadius, RandomInit )
  #local Goldtexture = T_Gold_1C
  #local Silvertexture = T_Silver_1C
  #local SizeZ = 3;
  #local SizeY = 2;
  #local DeepZ = 0.2;
  #local DeepY = 0.2;
  
  #local RandNumber = 200;
  #local RandStream = seed (RandomInit);
  
  union {
    object {
      cylinder_block (
          0, - SizeY / 2 + DeepY, MainRadius - SizeZ / 2 + DeepZ, 
        360, + SizeY / 2 - DeepY, MainRadius + SizeZ / 2 - DeepZ  
      )
      texture { Goldtexture } 
    }
    
    // -------- own use of values --------
    #macro special_cylinder_block ( X1, Y1, Z1, X2, Y2, Z2 )
      cylinder_block ( X1, Y1 * SizeY / 2, MainRadius + Z1 * SizeZ / 2, X2, Y2 * SizeY / 2, MainRadius + Z2 * SizeZ / 2 )
    #end
    
    union {
      #local Var_R = 0;
      #while ( Var_R < RandNumber )

        #local X1 =      RandomValue ( RandStream,  1, 360, 360 );
        #local Y1 =      RandomValue ( RandStream, -1,   1,   5 );
        #local Z1 =      RandomValue ( RandStream, -1,   1,   5 );
        #local X2 = X1 + RandomValue ( RandStream,  1,  20,  20 );
        #local Y2 =      RandomValue ( RandStream, -1,   1,   5 );
        #local Z2 =      RandomValue ( RandStream, -1,   1,   5 );
        
        // ------- Debugg Output --------
        //#render concat ( "special_cylinder_block",
        //                 " X1: ", str (X1, 7, 3),
        //                 " Y1: ", str (Y1, 7, 3),
        //                 " Z1: ", str (Z1, 7, 3),
        //                 " X2: ", str (X2, 7, 3),
        //                 " Y2: ", str (Y2, 7, 3),
        //                 " Z2: ", str (Z2, 7, 3),
        //                 "\n" )
        #if ( (Y1 = +1)|(Y1 = -1)|(Y2 = +1)|(Y2 = -1)|(Z1 = +1)|(Z1 = -1)|(Z2 = +1)|(Z2 = -1) )
          special_cylinder_block ( X1, Y1, Z1, X2, Y2, Z2 )
        //#else
          // ------- Debugg Output --------
          //#render concat ( "Invisible Block eliminated, Radius: ", str ( MainRadius, 5, 2 ), " Nr: ", str ( Var_R, 5, 0 ), "\n" ) 
        #end
        #local Var_R = Var_R + 1;
      #end
      
      texture { Silvertexture }
    }
  }
#end
