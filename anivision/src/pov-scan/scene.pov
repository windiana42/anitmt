//@fileID=scene
#include "colors.inc"

global_settings {
	assumed_gamma 2.1
}

light_source { <0, 100, 0> color White }

plane { y,-3 
	pigment { checker color Green color White }
}
plane { -z,-20
	pigment { rgb 0 }
	finish { reflection 0.8 }
}

camera {
	location -30*z+ 9*y +5*x
	up y
	sky y
	look_at 0
	angle 60
}

/*
camera {
	perspective
	
	location 0   // default=0
	direction z  // "front"; default=z
	right image_widht/image_height*x   // default=1.3333*x
	up y         // default=y
	
	matrix ...
	
	angle ...
}
*/

/*@object(macro=Fighter,  
	params={pintens},
	defs={testme=pintens+1},
	include={"colors.inc","colors.inc","metals"+"."+"inc"},
	// Note that you can actually use expressios for include={} 
	// like for defs={} and params={}
	type="object",
	// NOTE: type="xxx" means that the object is enclosed in a scope 
	//       named xxx, hence you may use "camera" for 
	//          camera  { <name>(...) matrix ... }
	//       You may use NULL for no scope at all (i.e. without even 
	//       the braces. I do not know if here are cases where that 
	//       is useful. 
	//       Default: "object", of course. 
	append_raw="")
	// append_raw: to be appended after the matrix and before the closing 
	// brace in raw form; normally a string value. 
	// Note again that you can include expressions evalutated just before 
	//      the POV file gets written (e.g. "angle "+strof(my_angle) )
*/
#macro Fighter(xlen)
union {
	cylinder { 0,-xlen*x,0.9 }
	sphere { 0,1.7 }
	sphere { 1.2*x+1.2*y,0.8 }
	cylinder { -3*x,3*x,0.2 }
	cylinder { -3*y,3*y,0.2 }
	cylinder { -3*z,3*z,0.2 }
	pigment { color Red }
}
#end

//@-object(macro=Fighter,   // actually a camera test
//    type="camera",
//    append_raw="angle "+strof(50+0*pintens))
/*#macro Fighter()
// camera {   <-- added outside
	perspective
	
	location 0   // default=0
	direction x  // "front"; default=z
	right image_width/image_height*z   // default=1.3333*x
	up y         // default=y
	
	//matrix ...  <-- automagically
	
	//angle ...  <-- via append_raw
// }  <-- added outside
#end*/


//@object(macro=something,defs={},include={"colors.inc"}
//)
#macro something()
union{
	sphere { 2,0.0000001 }
	sphere { 2,0.0000001 }
	pigment { rgbf 1 }
}
#end

// For testing: 
#ifndef(AniVision)
	object { Fighter(0) }
#end
