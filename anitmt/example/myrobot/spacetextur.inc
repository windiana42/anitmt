#ifdef(SpaceTextur_Inc_Temp)
/* do nothing */
#else
#declare SpaceTextur_Inc_Temp = version;
#version 3.0;

#ifdef(View_POV_Include_Stack)
#   debug "including SpaceTextur.inc\n"
#end


#declare Licht = 1.0 // 0.25

#declare Gras = texture {
   pigment {
     granite
     color_map {
       [ 0.000  color rgb<0.193, 0.407, 0.000> ]
       [ 1.000  color rgb<0.365, 0.611, 0.000> ]
     }
   }
   normal {
     bumps 1.0000
     turbulence 0.200
     omega 2.000
     lambda 0.500
     scale     0.1000
   }
   finish {
     diffuse 0.500
     ambient 0.200
     specular 0.180
     roughness 0.0010
   }

}


#declare MySkyBlue = rgbt <0.3*Licht, 0.5*Licht, 0.8*Licht, 1.0000>

// Bright Blue Sky w/ very white clouds.
#declare Blue_Sky_Clouds = 
pigment {
    bozo
    turbulence 0.65
    color_map {
        [0.0 rgb <0.3, 0.5, 0.80>*Licht]
        [0.4 rgb <0.4, 0.6, 0.82>*Licht]
        [0.7 rgb 0.9*Licht]
        [1.0 rgb 0.7*Licht]
    }
}

// Bright Blue Sky w/ very white clouds.
#declare Grey_Clouds = 
pigment {
    bozo
    turbulence 0.65
    color_map {
        [0.0 rgbt <0.9, 0.9, 0.9, 1.0000>]
        [0.4 rgbt <0.9, 0.9, 0.9, 0.8333>]
        [0.7 rgbt <0.9, 0.9, 0.9, 0.0000>]
        [1.0 rgbt <0.7, 0.7, 0.7, 0.0000>]
    }
}

// Bright Blue Sky w/ very white clouds.
#declare Blue_Sky_Clouds_2 = 
pigment {
    bozo
    turbulence 0.65
    color_map {
        [0.0 rgb <0.3, 0.5, 0.80 >]
        [0.2 rgb <0.3, 0.5, 0.80 >]
        [0.6 rgb <0.4, 0.6, 0.82>]
        [0.9 rgb 0.9]
        [1.0 rgb 0.7]
    }
}

// Bright Blue Sky w/ very white clouds.
#declare Grey_Clouds_2 = 
pigment {
    bozo
    turbulence 0.65
    color_map {
        [0.0 rgbt <0.9, 0.9, 0.9, 1.0000>]
        [0.2 rgbt <0.9, 0.9, 0.9, 1.0000>]
        [0.6 rgbt <0.9, 0.9, 0.9, 0.8333>]
        [0.9 rgbt <0.9, 0.9, 0.9, 0.0000>]
        [1.0 rgbt <0.7, 0.7, 0.7, 0.0000>]
    }
}

#declare Glas = material{
    texture {
        pigment {color rgbf<0.8,1,1,0.8>}
        finish {
            specular 0.5 roughness 0.08
            reflection 0.3
        }
    } 
    interior {
        ior 1.5
        caustics 1
    }
     
}

#declare GlasEinfach = material{
    texture {
        pigment {color rgb< 0.4, 0.5, 0.5 > }
        finish {
            specular 0.5 roughness 0.08
            reflection 0.3
        }
    } 
  /*  interior {
        ior 1.5
        caustics 1
    } */
     
}


#declare Granitplatten = 
   texture {      
       brick
       texture {
           pigment {
               color rgb <0.6, 0.6, 0.6 >
           }
       }
       texture {
           pigment {
               granite
               color_map {
                   [ 0.0     rgbft <0.4375, 0.239567, 0.239567, 0.0, 0.0> ]
                   [ 1.0     rgbft <0.656233, 0.552067, 0.4479, 0.0, 0.0> ]
               }
//               ramp_wave
           }
//           normal {
//               granite , 0.2
//               scale  0.2
//           }
       }      
       brick_size <0.5, 0.5, 0.5>
       mortar 0.02
   }
 
#declare Metallfinish = finish { metallic reflection 0.5 brilliance 4 }
/*
    brilliance 4
    diffuse 0.5
    metallic
    specular 0.80
    roughness 1/80
    reflection 0.5
*/


#declare Granit_1 = texture {      
    pigment {
        granite
        color_map {
            [ 0.0     rgbft <0.50, 0.40, 0.40, 0.0, 0.0> ]
            [ 1.0     rgbft <0.64, 0.61, 0.52, 0.0, 0.0> ]
        }
        turbulence 0.3
        ramp_wave
    }      
    normal {
        granite , 0.2
        scale  0.2
    }
}

#declare Granit_2 = texture {      
    pigment {
        granite
        color_map {
            [ 0.0     rgbft <0.40, 0.40, 0.44, 0.0, 0.0> ]
            [ 1.0     rgbft <0.64, 0.61, 0.62, 0.0, 0.0> ]
        }
        turbulence 0.4
        ramp_wave
    }      
    normal {
        granite , 0.2
        scale  0.2
    }
}


#declare MercedesMarmor = texture {        // MercedesMarmor
  pigment {
    agate
    agate_turb 1.0
    color_map {
      [ 0.0     rgbft <0.4479, 0.468733, 0.458333, 0.0, 0.0> ]
      [ 1.0     rgbft <1, 1, 1, 0.0, 0.0> ]
    }
  }      
  /*
  finish {
    ambient 0.1
    phong 0.4
    phong_size 20.0
  }
  */
}

 
/*
#declare Fenster_Normal = normal {
    average
    normal_map {
        [ 0.5 
                  
                  slope_map {
                      [0.05 < 2 * atan2( Fenster_B / 2 - 0.05 , Fenster_R), 0 > ]
                      [0.10 < 2 * atan2( Fenster_B / 2 - 0.10 , Fenster_R), 0 > ]
                      [0.15 < 2 * atan2( Fenster_B / 2 - 0.15 , Fenster_R), 0 > ]
                      [0.20 < 2 * atan2( Fenster_B / 2 - 0.20 , Fenster_R), 0 > ]
                      [0.25 < 2 * atan2( Fenster_B / 2 - 0.25 , Fenster_R), 0 > ]
                      [0.30 < 2 * atan2( Fenster_B / 2 - 0.30 , Fenster_R), 0 > ]
                      [0.35 < 2 * atan2( Fenster_B / 2 - 0.35 , Fenster_R), 0 > ]
                      [0.40 < 2 * atan2( Fenster_B / 2 - 0.40 , Fenster_R), 0 > ]
                      [0.45 < 2 * atan2( Fenster_B / 2 - 0.45 , Fenster_R), 0 > ]
                      [1.50 < 2 * atan2( Fenster_B / 2 - 0.50 , Fenster_R), 0 > ]     
                      [0.55 < 2 * atan2( Fenster_B / 2 - 0.55 , Fenster_R), 0 > ]
                      [0.60 < 2 * atan2( Fenster_B / 2 - 0.60 , Fenster_R), 0 > ]
                      [0.65 < 2 * atan2( Fenster_B / 2 - 0.65 , Fenster_R), 0 > ]
                      [0.70 < 2 * atan2( Fenster_B / 2 - 0.70 , Fenster_R), 0 > ]
                      [0.75 < 2 * atan2( Fenster_B / 2 - 0.75 , Fenster_R), 0 > ]
                      [0.80 < 2 * atan2( Fenster_B / 2 - 0.80 , Fenster_R), 0 > ]
                      [0.85 < 2 * atan2( Fenster_B / 2 - 0.85 , Fenster_R), 0 > ]
                      [0.90 < 2 * atan2( Fenster_B / 2 - 0.90 , Fenster_R), 0 > ]
                      [0.95 < 2 * atan2( Fenster_B / 2 - 0.95 , Fenster_R), 0 > ]
                      [1.00 < 2 * atan2( Fenster_B / 2 - 1.00 , Fenster_R), 0 > ]
                  
              }
        ]
        [ 0.5
                 
                  slope_map {
                      [0.05 < 0, 2 * atan2( Fenster_H / 2 - 0.05 , Fenster_R) > ]
                      [0.10 < 0, 2 * atan2( Fenster_H / 2 - 0.10 , Fenster_R) > ]
                      [0.15 < 0, 2 * atan2( Fenster_H / 2 - 0.15 , Fenster_R) > ]
                      [0.20 < 0, 2 * atan2( Fenster_H / 2 - 0.20 , Fenster_R) > ]
                      [0.25 < 0, 2 * atan2( Fenster_H / 2 - 0.25 , Fenster_R) > ]
                      [0.30 < 0, 2 * atan2( Fenster_H / 2 - 0.30 , Fenster_R) > ]
                      [0.35 < 0, 2 * atan2( Fenster_H / 2 - 0.35 , Fenster_R) > ]
                      [0.40 < 0, 2 * atan2( Fenster_H / 2 - 0.40 , Fenster_R) > ]
                      [0.45 < 0, 2 * atan2( Fenster_H / 2 - 0.45 , Fenster_R) > ]
                      [1.50 < 0, 2 * atan2( Fenster_H / 2 - 0.50 , Fenster_R) > ]
                      [0.55 < 0, 2 * atan2( Fenster_H / 2 - 0.55 , Fenster_R) > ]
                      [0.60 < 0, 2 * atan2( Fenster_H / 2 - 0.60 , Fenster_R) > ]
                      [0.65 < 0, 2 * atan2( Fenster_H / 2 - 0.65 , Fenster_R) > ]
                      [0.70 < 0, 2 * atan2( Fenster_H / 2 - 0.70 , Fenster_R) > ]
                      [0.75 < 0, 2 * atan2( Fenster_H / 2 - 0.75 , Fenster_R) > ]
                      [0.80 < 0, 2 * atan2( Fenster_H / 2 - 0.80 , Fenster_R) > ]
                      [0.85 < 0, 2 * atan2( Fenster_H / 2 - 0.85 , Fenster_R) > ]
                      [0.90 < 0, 2 * atan2( Fenster_H / 2 - 0.90 , Fenster_R) > ]
                      [0.95 < 0, 2 * atan2( Fenster_H / 2 - 0.95 , Fenster_R) > ]
                      [1.00 < 0, 2 * atan2( Fenster_H / 2 - 1.00 , Fenster_R) > ]
                  }
                  rotate <0,0,90>
        ]
    }     
}   
*/

#version SpaceTextur_Inc_Temp;
#end



//----ERROR---->ALL RIGHT<----                No objects in scene?
