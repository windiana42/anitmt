/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/***************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------   plasma.inc   ---------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------*/
/*                                                                                                             */
/*  contains macros and declarations for medias                                                                */
/*                                                                                                             */
/*                                                                                   written by Manuel Moser   */
/*                                                                                       moser.manuel@gmx.de   */
/*                                                                                                             */
/***************************************************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// -----------------------------------------------
// ----   color maps for densities in media   ----
// -----------------------------------------------

#declare PlasmaRed = color_map {
  [0.0 color rgb <0.0, 0.0, 0.0>]
  [0.3 color rgb <0.9, 0.1, 0.1>]
  [1.0 color rgb <1.0, 0.8, 0.8>]
}
#declare PlasmaGreen = color_map {
  [0.0 color rgb <0.0, 0.0, 0.0>]
  [0.3 color rgb <0.1, 0.9, 0.1>]
  [1.0 color rgb <0.8, 1.0, 0.8>]
}
#declare PlasmaBlue = color_map {
  [0.0 color rgb <0.0, 0.0, 0.0>]
  [0.3 color rgb <0.1, 0.1, 0.9>]
  [1.0 color rgb <0.8, 0.8, 1.0>]
}
#declare PlasmaYellow = color_map {
  [0.0 color rgb <0.0, 0.0, 0.0>]
  [0.3 color rgb <0.9, 0.9, 0.1>]
  [1.0 color rgb <1.0, 1.0, 0.8>]
}
#declare PlasmaCyan = color_map {
  [0.0 color rgb <0.0, 0.0, 0.0>]
  [0.3 color rgb <0.1, 0.9, 0.9>]
  [1.0 color rgb <0.8, 1.0, 1.0>]
}
#declare PlasmaMagenta = color_map {
  [0.0 color rgb <0.0, 0.0, 0.0>]
  [0.3 color rgb <0.9, 0.1, 0.9>]
  [1.0 color rgb <1.0, 0.8, 1.0>]
}

// -----------------------------------------------
// ----   a media cylinder along the y axis   ----
// -----------------------------------------------

#macro PlasmaCylinder ( PlasmaLength, PlasmaOffset, TheColorMap )
  cylinder {
    < 0,            0, 0 >,
    < 0, PlasmaLength, 0 >,
    1.0
    pigment { 
      color rgbf <1, 1, 1, 1> 
    }
    finish { 
      ambient 0
      diffuse 0
    }
    interior { 
      media {
        emission 0.40
        //intervals 5
        samples 1, 10
        confidence 0.9999
        variance 1/1000
        density {
          cylindrical
          ramp_wave
          color_map {
            TheColorMap
          } 
          turbulence 0.4
          scale 0.8
          translate < 0, PlasmaOffset, 0 >
        }
      } 
    }
    hollow 
  }
#end

// ----------------------------
// ----   a media sphere   ----
// ----------------------------

#macro PlasmaSphere ( TheColorMap )
  sphere { 0, 1
    pigment { 
      color rgbf <1, 1, 1, 1> 
    }
    finish { 
      ambient 0
      diffuse 0
    }
    interior { 
      media {
        emission 0.40
        //intervals 5
        samples 1, 10
        confidence 0.9999
        variance 1/1000
        density {
          spherical
          ramp_wave
          color_map {
            TheColorMap
          } 
          turbulence 0.4
          scale 0.8
          translate < 0, PlasmaOffset, 0 >
        }
      } 
    }
    hollow 
  }
#end

// ----------------------------------------------------------------------------------------
// ----   media connection with the shape of a cylinder with two spheres at each end   ----
// ----------------------------------------------------------------------------------------

#macro MediaConnection ( X1, Y1, Z1, X2, Y2, Z2, PlasmaOffset, PlasmaOffsetDir, TheColorMap )
  #local Xd = X2 - X1;
  #local Yd = Y2 - Y1;
  #local Zd = Z2 - Z1;
  media {
    emission 0.80 // 0.6
    //intervals 5
    samples 1, 10
    confidence 0.9999
    variance 1/1000
    density {
      planar
      density_map {
        [ 
          0 
          planar
          density_map {
            [ 
              0
              spherical 
              color_map { TheColorMap }
              scale < 1, 1 / sqrt(pow(Xd,2) + pow(Yd,2) + pow(Zd,2)), 1 >
              translate < 0, 2, 0 >
            ]
            [ 
              0
              spherical 
              scale < 1, 1 / sqrt(pow(Xd,2) + pow(Yd,2) + pow(Zd,2)), 1 >
              color_map { TheColorMap }
            ]
            [ 
              1
              spherical 
              scale < 1, 1 / sqrt(pow(Xd,2) + pow(Yd,2) + pow(Zd,2)), 1 >
              color_map { TheColorMap }
              
            ]
          }
          translate < 0, -1, 0 >
        ]
        [ 0 cylindrical color_map { TheColorMap } ]
        [ 1 cylindrical color_map { TheColorMap } ]
      }
      translate < 0, 1, 0 >
      scale 0.5
      scale < 1, sqrt(pow(Xd,2) + pow(Yd,2) + pow(Zd,2)), 1 >
      rotate < 0, 0, - degrees(atan2( sqrt(pow(Xd,2) + pow(Zd,2)), Yd )) >
      #if (!( (Xd = 0) & (Zd = 0) ) )
        rotate < 0, - degrees(atan2( Zd, Xd ) ), 0 >
      #end
      translate < X1, Y1, Z1 >
      translate + PlasmaOffsetDir * PlasmaOffset
      warp { turbulence 0.4 }
      translate - PlasmaOffsetDir * PlasmaOffset
    }
  }
#end

#macro AniTMTPlasma ( PlasmaOffset )
  box {
    < - 1, -1, -1 >,
    <  21,  4, +1 >
    pigment { 
      color rgbf <1, 1, 1, 1> 
    }
    finish { 
      ambient 0
      diffuse 0
    }
    interior { 
      // ------- A --------
      MediaConnection (  0  ,  0  ,  0,  3  ,  3  ,  0, PlasmaOffset, z, PlasmaBlue )
      MediaConnection (  1  ,  1  ,  0,  3  ,  1  ,  0, PlasmaOffset, z, PlasmaBlue )
      MediaConnection (  3  ,  0  ,  0,  3  ,  3  ,  0, PlasmaOffset, z, PlasmaBlue )
      // ------- N --------                          
      MediaConnection (  4  ,  0  ,  0,  4  ,  3  ,  0, PlasmaOffset, z, PlasmaBlue )
      MediaConnection (  4  ,  3  ,  0,  7  ,  0  ,  0, PlasmaOffset, z, PlasmaBlue )
      MediaConnection (  7  ,  0  ,  0,  7  ,  3  ,  0, PlasmaOffset, z, PlasmaBlue )
      // ------- I --------                            
      MediaConnection (  8  ,  0  ,  0,  8  ,  3  ,  0, PlasmaOffset, z, PlasmaBlue )
      // ------- T --------               
      MediaConnection (  9  ,  3  ,  0, 12  ,  3  ,  0, PlasmaOffset, z, PlasmaRed )
      MediaConnection ( 10.5,  0  ,  0, 10.5,  3  ,  0, PlasmaOffset, z, PlasmaRed )
      // ------- M --------                         
      MediaConnection ( 13  ,  0  ,  0, 13  ,  3  ,  0, PlasmaOffset, z, PlasmaYellow )
      MediaConnection ( 13  ,  3  ,  0, 14.5,  1.5,  0, PlasmaOffset, z, PlasmaYellow )
      MediaConnection ( 14.5,  1.5,  0, 16  ,  3  ,  0, PlasmaOffset, z, PlasmaYellow )
      MediaConnection ( 16  ,  3  ,  0, 16  ,  0  ,  0, PlasmaOffset, z, PlasmaYellow )
      // ------- T --------                    
      MediaConnection ( 17  ,  3  ,  0, 20  ,  3  ,  0, PlasmaOffset, z, PlasmaRed )
      MediaConnection ( 18.5,  0  ,  0, 18.5,  3  ,  0, PlasmaOffset, z, PlasmaRed )
    }
    hollow
  }
#end