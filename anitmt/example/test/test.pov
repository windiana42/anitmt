#include "dummy.inc"
#include "inc_dummy.inc"

#declare testval = 5;
#declare test_s = 1;

background { rgb <0,1,(testval-1)/2> }

sphere { <0,0,0> 0.0000001 }

camera{ // testobj

}

/*#warning test_s*/
