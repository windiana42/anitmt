// ------- Texturen -------

#declare Rob_BuildingBLock1 = texture {      
  pigment {
    color rgb <1.0, 0.0104, 0.0>
  }      
  finish {
    ambient 0.3453
    diffuse 0.647467
    phong_size 20.0
    specular 0.755367
    roughness 0.018404
  }
}

#declare Rob_BodyTxtr = texture {      
  pigment {
    color rgb <0.0, 0.326, 0.3>
  }      
  normal {
    bumps , 0.2
    scale  0.05
  }      
  finish {
    ambient 0.25
    diffuse 0.44
    phong_size 20.0
    specular 0.5
    roughness 0.02
  }
}

#declare Rob_Chrome_Look = texture {      
  pigment {
    color rgb <0.3, 0.3, 0.3>
  }      
  finish {
    //ambient 0.0
    //diffuse 0.0
    //roughness 0.003
    metallic 1.0
    reflection 0.7
  }
}
      

#declare FlorTexture = texture {      
  pigment {
    color rgb <0.781233, 0.666667, 0.468733>
  }      
  finish {
    ambient 0.4
    phong_size 20.0
    roughness 0.02
  }
}
      

#declare SkyTxtr = texture {      
  pigment {
    gradient z
    color_map {
      [ 0.0     rgbft <0.4, 0.2, 0.0, 0.0, 0.0> ]
      [ 0.366906  rgbft <0.7, 0.6, 0.5, 0.0, 0.0> ]
      [ 0.5     rgbft <1.0, 1.0, 1.0, 0.0, 0.0> ]
      [ 0.715827  rgbft <0.444, 0.5, 0.91, 0.0, 0.0> ]
      [ 1.0     rgbft <0.344, 0.4, 0.81, 0.0, 0.0> ]
    }
    scale  <1.0, 1.0, 30.0>
    translate -1.0 * z
  }      
  finish {
    ambient 0.4
    phong_size 20.0
    roughness 0.02
  }
}
      
