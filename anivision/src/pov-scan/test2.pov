// NOTE: non-functional POV file. 

//Spaten:
#macro Spaten(B)

union{ //main union
union{
}
//Befestigungen:
#if (B)
union{
difference{
  cylinder {<0.5,0.1,0>,<0.7,0.1,0>,0.4}
}
difference{
  cylinder {<4.9,0.4,0>,<5.21,0.4,0>,0.3}
}
box {<5,0.4,-0.4>,<5.2,-0.5,0.4>}
pigment {color rgb<0.5,0.4,0.1>}
}
#end
}
#end 

#macro RKAF (R_K,H_K,D_K,M_H,M_D,SK,LO) //Lenk, Kanone Heb, Dreh, MG Heb, DReh,
                               //SpiegelKlapp, bei 0 ausgeklappt, bei 1 angeklappt
//RKAF
union{//main union
union{

 //HaltegriffA, einfache Ausführung, ohne Farbe!
#macro HaltegriffA ()
union{
  intersection{
    box{<-4.61,-0.65,0>,<0,0.65,4.61>}
  }
  cylinder{<0,0,4>,<9,0,4>,0.6}
}
#end

//HaltegriffB, lange Ausführung,mit zwei Stützen, ohne Farbe!
#macro HaltegriffB ()
union{
  intersection{
  cylinder {<18.5,0,4>,<18.5,0,-2>,0.6}
}
#end

//Spiegel:
 #macro Spiegel ()
union{
 union{
  //cylinder{<13.1,2.6,-3>,<13.1,2.6,3>, 0.1 pigment {color rgb<0.3,0.25,0.01>}}
scale 0.8 translate <8.6,2.4,-1.4>}

box {<12.88,1.5,-3.15>,<13.22,1,-2.85> pigment {color rgb<0.3,0.25,0.01>}
scale 0.8 translate <8.6,2.4,-1.4>}
}
#end

object {Spiegel()}
object {Spiegel() scale -1*z}


//körper:
union{
  difference{
    intersection{
      prism{linear_spline linear_sweep -4.6,4.6,10 
      prism{linear_spline linear_sweep -0.1,4.7,11 
      prism{linear_spline linear_sweep -3,23,9 
      rotate -90*z}
    }
  //Radausschnitte:vorn
  prism {linear_spline linear_sweep -2.2,-4.7,10 <11,-0.1>,<11,1.5>,
  
  prism {linear_spline linear_sweep 2.2,4.7,10 <11,-0.1>,<11,1.5>,
  
  //Hinten
  prism {linear_spline linear_sweep -2.2,-4.7,9 <8,-0.1>,<8,1.5>,<7.25,2.5>,
  
  prism {linear_spline linear_sweep 2.2,4.7,9 <8,-0.1>,<8,1.5>,<7.25,2.5>,
  //Auspuff, innen:
    cylinder {<-0.9,0,0>,<-3.1,0,0>,0.22 rotate 30*z translate <0.25,3.9,1.6> 
  
  //Fußrasten:
  

  //Lampen:
  box{<21,1,2.2>,<23,3,4>}
  

  }


 //Schutzbleche: vorn:
 prism {linear_spline linear_sweep -2.5,-4.5,7 
 prism {linear_spline linear_sweep 2.5,4.5,7 
 
 //Hint:
 prism {linear_spline linear_sweep -2.5,-4.5,8 
 prism {linear_spline linear_sweep 2.5,4.5,8 
 
 //Gucker für Fahrer:
 prism {linear_spline linear_sweep 3,4.2,7 <15,0.5>
 finish { F_Glass5 }}
 prism {linear_spline linear_sweep 4.199,4.25,7 
 
 //Gitter hinten:
 difference{
   box{<1,4,-2.3>,<6,4.6,2.3>}
  }
  
  //Ösen, vorne:
  difference {cylinder {<-0.1,0,0>,<0.1,0,0>,0.3} 
  cylinder{<-0.11,0,0>,<0.11,0,0>,0.2}
  rotate 7.125*z translate <21.5,3.2,2>}
   difference {cylinder {<-0.1,0,0>,<0.1,0,0>,0.3} 
  object{ HaltegriffA () rotate 180*x rotate 36.8*x 
  scale 0.1 translate <-2,2.3,-0.4> }
 
 pigment {color rgb <0.59,0.45,0.01>}
  
}//Körper, Ende!




//Beschriftung der Körperseiten:
 text {
    ttf "cyrvetic.ttf" "0335-L" 0.01, 0
    pigment { color rgb <0.3,0.25,0.1>}
    rotate 180*y
    rotate -53.13*x
    scale 1.5
    translate <16,3.5,3.84>
  }

#macro RBody ()
union{
  difference{
  cylinder {<0,0,-1.52>,<0,0,-1.35>,0.9 pigment{color rgb<0.5,0.45,0.1>}}    
  }
sphere {<0,0,-1.2>,0.55 pigment{color rgb<0.5,0.45,0.1>}}
}
#end


//Reifen:Rechts:
union{
  translate <19,0,-3.5>
}
union{
  translate <13.5,0,-3.5>
}
union{
  translate <5.5,0,-3.5>
}
union{
  union{object {RBody()} rotate -R_K*y }
  translate <0,0,-3.5>
}
//Reifen:links:
union{
  translate <19,0,3.5>
}
union{
  translate <13.5,0,3.5>
}
union{
  translate <5.5,0,3.5>
}
union{
  translate <0,0,3.5>
}


//Turm:
union{
  union{
    intersection{
      prism{linear_spline linear_sweep 0.2,1.9,13 <-5,2.3>,
      prism{linear_spline linear_sweep -3.49,3.49,5 <-5,0.2>,
      prism{linear_spline linear_sweep -6,6,5 <-0.201,3.5>,
    }
  object {HaltegriffB()
  rotate 180*x rotate 40*x
  scale 0.08 translate <-3.2,1.2,-2.2>}
  
 
  //hinten:
  object {HaltegriffB()

  

  
 pigment {color rgb <0.6,0.49,0.01>}
 
 //Kanone:
 union{
   difference{
     union{
   }
   difference{
     cylinder {<0,0,0.58>,<1.5,0,0.58>,0.18}
   }
   //  @fubar(äääää)
   difference{
    union{
    cylinder {<0,0,-0.58>,<2.6,0,-0.58>,0.08 pigment {color rgb<0.2,0.2,0.01>}}
   }
   
 pigment {color rgb <0.5,0.45,0.01>}
    pigment{color rgb <0.6,0.6,0.6>}
}//main union
#end

object {FunkantenneKW ()  scale 1 translate <-2.5,1.65,-1>}
    box {<-0.2,-0.2,-0.15>,<0.25,0.15,0.15> pigment {color rgb<0.4,0.38,0.01>}}
  }



}//main union
#end


