#include "colors.inc"

#declare My_Green = 0;

cone{ // My_Obj      
  <-1, 0, 0>,                  
  0.5,                         
  < 1, 0, 0>,                  
  0                            
  pigment{
    color rgb <1, My_Green, 0> 
  }
}

box{ // Collision_Obj
  <-1,-1,-1>,
  < 1, 1, 1>
  scale 0.3
  pigment {
    color rgb < 1, 0, 1>
  }
}

camera{ // My_Camera     
  location <0,1,8>             
  look_at <0,0,0>              
}
      
light_source{ 
  <1,1,5>                      
  color rgb <1,1,1>            
}
