/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------   circles.pov   --------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
/*                                                                                                             */
/*  Four strange looking and turning circles with a text in the middle made with media                         */
/*  Created for animation with AniTMT                                                                          */
/*                                                                                                             */
/*                                                                                                             */
/*                                                                                   written by Manuel Moser   */
/*                                                                                       moser.manuel@gmx.de   */
/*                                                                                                             */
/***************************************************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 3.1;

#declare Vartest = off;
#if (Vartest = on)
  #include "vartest.pov"
#else

/* ------ Include-files -------------- */
#include "colors.inc"
//#include "golds.inc"
#include "metals.inc"
#include "circles.inc"
#include "plasma.inc"
#include "mosertex.inc"

/* ------------------------------------- Meldung ---------------------- */
#render "---------------------------------------------------------------\n"
#render "starting with Kreise.pov\n"
 
/* ------ Konstante Werte -------------- */
#declare def        =      2      ;
#declare Nix        =      0.0002 ;
#declare One        =      1      ;
#declare Unendlich  = 100000      ;

/* ------ Kamera-Definitionen ---------- */
//camera { location <      0 ,    5 ,  -20 > look_at  <      0  ,      0 ,     0 > } // Text
//camera { location <      0 ,   20 ,  -15 > look_at  <      0  ,      0 ,     0 > } // Testobjekte
  camera { location <      0 ,   40 ,  -50 > look_at  <      0  ,      0 ,     0 > } // Richtig
//camera { location <     40 ,   40 ,  -40 > look_at  <      0  ,      0 ,     0 > } // Richtig schräg
//camera { location <     50 ,   10 ,  -20 > look_at  <     20  ,      0 ,   -10 > } // connections
                                                          
/* ------ Hilfs-Licht-Quellen ---------- */
//light_source { <   - 100,  3000, - 200> color LightGrey }
  light_source { <   -1100,  3000, -2000> color LightGrey }
//light_source { <  2000,  3000, -1100> color LightGrey }
//light_source { <  1100,   600, -2100> color LightGrey }
//light_source { <   + 200,  1650,  1400> color LightGrey }
//light_source { <   -  20,  1950,   100> color LightGrey }

/* ------ Hintergrund ------------------ */

sky_sphere {
  pigment {        
    gradient y
    pigment_map {
      [0.0 SkyBlue ]
      [0.2 Blue_Sky_Clouds_mod scale 0.3]
      [1.0 Blue_Sky_Clouds_mod scale 0.3]
    }
  }  
}   

/* ------ Objekte ---------------------- */
// Testobjects

/*
object { cylinder_block ( -22.5, -1,  2, 337.5,  1,  0 ) pigment { color rgb < 1  , 1  , 1   > } } // 360  
object { cylinder_block (   0  ,  0,  3, 315  ,  2,  4 ) pigment { color rgb < 0  , 1  , 1   > } } // 315  
object { cylinder_block ( 337.5,  2,  6, 607.5,  0,  5 ) pigment { color rgb < 1  , 0  , 1   > } } // 270  
object { cylinder_block ( -45  ,  2,  7, 180  ,  3,  8 ) pigment { color rgb < 1  , 0  , 0   > } } // 225 
object { cylinder_block ( 202.5,  3,  9, 382.5,  2, 10 ) pigment { color rgb < 0  , 1  , 0   > } } // 180  
object { cylinder_block (  45  ,  2, 10, 180  ,  4,  9 ) pigment { color rgb < 0.5, 0.5, 0.5 > } } // 135
object { cylinder_block ( 202.5,  3,  8, 292.5,  2,  7 ) pigment { color rgb < 0  , 0  , 1   > } } //  90
object { cylinder_block ( 270  ,  0,  5, 315  ,  2,  6 ) pigment { color rgb < 1  , 1  , 0   > } } //  45  

//object { cylinder_block ( 210,  4, 13, 255,  2, 13 ) pigment { color rgb < 1, 1, 1 > } } //  45  
*/


#declare Rotation = 15;
#declare PlasmaOffset = 0;

#local Max_Recursion_Deep = 8;
#declare Circle_D = 5;
#declare Inner_Circle_R = 15;
#declare Outer_Plasma_Length = 2000;
#declare InitalSeed = 4713;
#declare RandomStream = seed ( InitalSeed );
#declare Rotate_Every_Four_Against = true;

#declare Rotate_Together = true;



#if (Rotate_Together = true)
  #declare Rotation_Value = array[Max_Recursion_Deep]
  #declare Var_I = 0;
  #while ( Var_I < Max_Recursion_Deep )
    #declare Rotation_Value[Var_I] = Rotation;
    #declare Var_I = Var_I + 1;
  #end
#end

#macro CircleRecursion ( Recursion_Deep )
  
  #if (Recursion_Deep = 0)
    // -------- AniTMT in the Middle --------
    /*object {
      AniTMTPlasma ( PlasmaOffset + rand(RandomStream) * 10000 )
      translate < - 10 , -1.5, 0 >
    }*/
    object {
      PlasmaCylinder ( Inner_Circle_R - 10, PlasmaOffset + rand(RandomStream) * 10000, PlasmaBlue ) 
      rotate < 0, 0, - 90 >
      translate < - Inner_Circle_R - Recursion_Deep * Circle_D, 0, 0 >
    }
    object {
      PlasmaCylinder ( Inner_Circle_R - 10, PlasmaOffset + rand(RandomStream) * 10000, PlasmaBlue ) 
      rotate < 0, 0, + 90 >
      translate < + Inner_Circle_R + Recursion_Deep * Circle_D, 0, 0 >
    }
    rotate + z * Rotation_Value[Recursion_Deep]
    rotate - y * 90 
  #else
    #if (Recursion_Deep < Max_Recursion_Deep)
      // -------- Normal Circles --------
      union {
        MyCircle ( Inner_Circle_R + (Recursion_Deep - 1) * Circle_D, rand(RandomStream) * 1000000 )
        CircleRecursion ( Recursion_Deep - 1)
      }
      object {
        PlasmaCylinder ( Circle_D, PlasmaOffset + rand(RandomStream) * 10000, PlasmaBlue ) 
        rotate < 0, 0, - 90 >
        translate < - Inner_Circle_R - Recursion_Deep * Circle_D, 0, 0 >
      }
      object {
        PlasmaCylinder ( Circle_D, PlasmaOffset + rand(RandomStream) * 10000, PlasmaBlue ) 
        rotate < 0, 0, + 90 >
        translate < + Inner_Circle_R + Recursion_Deep * Circle_D, 0, 0 >
      }
      rotate + z * Rotation_Value[Recursion_Deep]
      rotate - y * 90 
      #if ( (mod(Recursion_Deep + 1, 4) = 0) & (Rotate_Every_Four_Against = true) )
        rotate - y * 180 
      #end
    #else
      // -------- Outer Circles with long Plasma --------
      union {
        MyCircle ( Inner_Circle_R + (Recursion_Deep - 1) * Circle_D, rand(RandomStream) * 1000000 )
        CircleRecursion ( Recursion_Deep - 1)
      }
      object {
        PlasmaCylinder ( Circle_D + Outer_Plasma_Length, PlasmaOffset + rand(RandomStream) * 10000, PlasmaBlue ) 
        rotate < 0, 0, - 90 >
        translate < - Inner_Circle_R - Recursion_Deep * Circle_D - Outer_Plasma_Length, 0, 0 >
      }
      object {
        PlasmaCylinder ( Circle_D + Outer_Plasma_Length, PlasmaOffset + rand(RandomStream) * 10000, PlasmaBlue ) 
        rotate < 0, 0, + 90 >
        translate < + Inner_Circle_R + Recursion_Deep * Circle_D + Outer_Plasma_Length, 0, 0 >
      }
    #end
  #end
#end

union {
  CircleRecursion ( Max_Recursion_Deep )
  rotate y *  90 * Max_Recursion_Deep
  rotate y * 180 * div(Max_Recursion_Deep,4)
}
/*



union {
  MyCircle ( 30, 123468 )
  union {
    MyCircle ( 25, 892346 )
    union {
      MyCircle ( 20, 180342 )
      union {
        MyCircle ( 15, 734648 )
        object {
          AniTMTPlasma ( PlasmaOffset + 3564 )
          translate < - 10 , -1.5, 0 >
        }
        object {
          PlasmaCylinder ( 5, PlasmaOffset + 2334, PlasmaBlue ) 
          rotate < 0, 0, - 90 >
          translate < - 15, 0, 0 >
        }
        object {
          PlasmaCylinder ( 5, PlasmaOffset + 4329, PlasmaBlue ) 
          rotate < 0, 0, + 90 >
          translate < + 15, 0, 0 >
        }
        rotate + z * Rotation_Value_1 
      }
      object {
        PlasmaCylinder ( 5, PlasmaOffset + 3712, PlasmaBlue ) 
        rotate < + 90, 0, 0 >
        translate < 0, 0, - 20 >
      }
      object {
        PlasmaCylinder ( 5, PlasmaOffset + 8134, PlasmaBlue ) 
        rotate < - 90, 0, 0 >
        translate < 0, 0, + 20 >
      }
      rotate + x * Rotation_Value_2
    }
    object {
      PlasmaCylinder ( 5, PlasmaOffset + 5711, PlasmaBlue ) 
      rotate < 0, 0, - 90 >
      translate < - 25, 0, 0 >
    }
    object {
      PlasmaCylinder ( 5, PlasmaOffset + 9534, PlasmaBlue ) 
      rotate < 0, 0, + 90 >
      translate < + 25, 0, 0 >
    }
    rotate - z * Rotation_Value_3
  }
  object {
    PlasmaCylinder ( 5, PlasmaOffset + 7734, PlasmaBlue ) 
    rotate < + 90, 0, 0 >
    translate < 0, 0, - 30 >
  }
  object {
    PlasmaCylinder ( 5, PlasmaOffset + 9013, PlasmaBlue ) 
    rotate < - 90, 0, 0 >
    translate < 0, 0, + 30 >
  }
  rotate - x * Rotation_Value_4
}   
object {
  PlasmaCylinder ( 5 + 2000, PlasmaOffset + 3433, PlasmaBlue ) 
  rotate < 0, 0, - 90 >
  translate < - 35 - 2000, 0, 0 >
}
object {
  PlasmaCylinder ( 5 + 2000, PlasmaOffset + 6744, PlasmaBlue ) 
  rotate < 0, 0, + 90 >
  translate < + 35 + 2000, 0, 0 >
}

*/


/* ------ Meldung ---------------------- */
#render "finished with Kreise.pov\n"
#render "---------------------------------------------------------------\n"


#end
