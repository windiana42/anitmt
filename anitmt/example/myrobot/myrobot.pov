#version 3.1;

/* ------ Include-Dateien -------------- */
#include "colors.inc"
#include "spacetextur.inc"
#include "myrobottex.inc"
#include "myrobot.inc"

/* ------------------------------------- Meldung ---------------------- */
#render "---------------------------------------------------------------\n"
#render "Beginne mit MyRobot.Pov\n"

/* ------ Kamera-Definitionen ---------- */

//camera { location< 10   , 10 , -35 > look_at < -10  , 5 ,  -10> } // Interessanter Blick
//camera { location< -16   , 10.2 , -32  > look_at < -10  , 6 , - 15> } // Interessanter Blick

/*
#declare Kameradreh = -115;
camera { // Kamera
  location< 0 , 0 , 0 >
  look_at x
  rotate < 0, 0, -6>
  rotate < 0, Kameradreh,0>
  translate < 10, 10, -35 >
}
*/

#declare Kameradreh2 = 120;
#declare Kameraentf2 = 100;
camera { // Kamera
  location < Kameraentf2, 12 , 0 >
  look_at < 0 , 5 , 0 >
  rotate < 0, Kameradreh2, 0 >
}

/* ------ Licht-Quellen ---------------- */
//light_source { <0, 10, -9> color White area_light < 3, 0, 0>, <0, 0, 3>, 2, 2 }
light_source { <   0,  60, -40> color LightGray }
//light_source { <   3, 30, -20> color White }
light_source { <  -6, 120, -50> color LightGray }
light_source { < -40,  80, -60> color LightGray }


/* ------ Hintergrund ------------------ */
background {MySkyBlue}
//background {Red}


/* ------ Objekte ---------------------- */

#declare Rob_Boden = plane {
  y, 0
  texture { FlorTexture }
}

#declare Rob_Drehgelenk_W1      =   45;
#declare Rob_Armteil1_W1        =   60;
#declare Rob_Armteil2_W1        =  -70;
#declare Rob_Tele_L1            = 19.25;
// min Rob_Tele1_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L
// max Rob_Tele1_L + Rob_Tele2_L + Rob_Tele3_L + Rob_Tele4_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L + 
// Hier : min 7.25      max 19.25
#declare Rob_Armteil3_W1        =  10;
#declare Finger1_Winkel1        =  45; 
#declare Finger2_Winkel1        =  90; 

#declare Rob_Drehgelenk_W2      =   45;
#declare Rob_Armteil1_W2        =   60;
#declare Rob_Armteil2_W2        =  -70;
#declare Rob_Tele_L2            = 19.25;
// min Rob_Tele1_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L
// max Rob_Tele1_L + Rob_Tele2_L + Rob_Tele3_L + Rob_Tele4_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L + 
// Hier : min 7.25      max 19.25
#declare Rob_Armteil3_W2        =  10;
#declare Finger1_Winkel2        =  45; 
#declare Finger2_Winkel2        =  90; 

#declare Rob_Drehgelenk_W3      =   45;
#declare Rob_Armteil1_W3        =   60;
#declare Rob_Armteil2_W3        =  -70;
#declare Rob_Tele_L3            = 19.25;
// min Rob_Tele1_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L
// max Rob_Tele1_L + Rob_Tele2_L + Rob_Tele3_L + Rob_Tele4_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L + 
// Hier : min 7.25      max 19.25
#declare Rob_Armteil3_W3        =  10;
#declare Finger1_Winkel3        =  45; 
#declare Finger2_Winkel3        =  90; 

#declare Rob_Drehgelenk_W4      =   45;
#declare Rob_Armteil1_W4        =   60;
#declare Rob_Armteil2_W4        =  -70;
#declare Rob_Tele_L4            = 19.25;
// min Rob_Tele1_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L
// max Rob_Tele1_L + Rob_Tele2_L + Rob_Tele3_L + Rob_Tele4_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L + 
// Hier : min 7.25      max 19.25
#declare Rob_Armteil3_W4        =  10;
#declare Finger1_Winkel4        =  45; 
#declare Finger2_Winkel4        =  90; 

object {
  MyRobot (Rob_Drehgelenk_W1,Rob_Armteil1_W1,Rob_Armteil2_W1,Rob_Tele_L1,Rob_Armteil3_W1,Finger1_Winkel1,Finger2_Winkel1) 
  rotate < 0, 0, 0>
  translate < -25, 0, -25>
}

object {
  MyRobot (Rob_Drehgelenk_W2,Rob_Armteil1_W2,Rob_Armteil2_W2,Rob_Tele_L2,Rob_Armteil3_W2,Finger1_Winkel2,Finger2_Winkel2) 
  rotate < 0, 0, 0>
  translate < -25, 0, +25>
}

object {
  MyRobot (Rob_Drehgelenk_W3,Rob_Armteil1_W3,Rob_Armteil2_W3,Rob_Tele_L3,Rob_Armteil3_W3,Finger1_Winkel3,Finger2_Winkel3) 
  rotate < 0, 0, 0>
  translate < +25, 0, -25>
}

object {
  MyRobot (Rob_Drehgelenk_W4,Rob_Armteil1_W4,Rob_Armteil2_W4,Rob_Tele_L4,Rob_Armteil3_W4,Finger1_Winkel4,Finger2_Winkel4) 
  rotate < 0, 0, 0>
  translate < +25, 0, +25>
}

object {
  Rob_Boden
  rotate < 0, 0, 0>
  translate < 0, 0, 0>
}


sky_sphere {
  pigment {        
    gradient y
    pigment_map {
      [0.0 SkyBlue ]
      [0.2 Blue_Sky_Clouds scale 0.3]
      [1.0 Blue_Sky_Clouds scale 0.3]
    }
  }  
}   

/* ------ Meldung ---------------------- */
#render "SpaceShip.pov beendet\n"
#render "---------------------------------------------------------------\n"
