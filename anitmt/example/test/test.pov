#include "dummy.inc"
#include "inc_dummy.inc"

#declare testval = 5;
#declare test_s = 1;

background { rgb <0,0,(testval-1)/2> }

// spline control points
sphere { <  -1,   3,  0> 0.2 pigment{color rgb <1,0,0>} }
sphere { < 1.5,   2, -1> 0.2 pigment{color rgb <1,0,0>} }
sphere { < 2.5,   4,  0> 0.2 pigment{color rgb <1,0,0>} }
sphere { <   0,   4,  2> 0.2 pigment{color rgb <1,0,0>} }
sphere { <   2, 5.5,  0> 0.2 pigment{color rgb <1,0,0>} }

union { // testobj 
  box { 
    <-1,-1,-1> 
    <1,1,1> 
    pigment{color rgb <0,1,0>} 
  }
  box {
    <-1,-1,-1>
    <1,1,1> 
    pigment{color rgb <0,0,1>} 
    scale 0.4
    translate <0,1,0>
  }
  box {
    <-1,-1,-1>
    <1,1,1>
    pigment{color rgb <1,0,0>} 
    scale 0.4
    translate <0,0,1>
  }
  scale 0.4
}

camera{ 
  location <0,0,0>
  look_at <1,4,0>
}

light_source { < -2, -2, 10> color rgb <1,1,1> }
light_source { <   2, -3, -10> color rgb <1,1,1> }
light_source { <  20, -10, -3> color rgb <1,1,1> }
light_source { < -20, -8, 3> color rgb <1,1,1> }

/*#warning test_s*/
