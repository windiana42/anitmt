// NOTE: non-functional POV file. 

//@fileID=pass_any_name
#include "colors.inc"

#include "test.pov"
#include "../../../anivision/src/pov-scan/test.pov"
#include "../anivision/src/pov-scan/test.pov"

//@object(macro=wollwo,params={angle,gangle,sin(gear.angle)*2*<1,1,0>},defs={})
global_settings {
	//assumed_gamma 2.2
	assumed_gamma 1.5
}

//LICHTER
light_source { <2, 40, -3> color White }


/**///PLANE
plane { <0,1,0>, -5
  pigment { checker color White color Green }
}

//camera { location <-11,3,-5> look_at <0,1,2.1> }     //Wurzel, Flügel
//camera { location <-1,1.2,-5> look_at <-1,1,-0.8> }     //Wurzel, Flügel,vorne

//camera { location <1,1.2,7> look_at <1,1,6> }     //Spitze, Flügel, vorne
//camera { location <5,4,3> look_at <1,1,3> }     //Vorderkante
//camera { location <3,4,1> look_at <-5,0,5> }     //Vorderkante Klappen
//camera { location <3,5,2> look_at <1,4,1> }     //Kraxl


//camera { location <-11,1.2,3> look_at <-3,0,1.5> }     //Flügel, Hinterkante
//camera { location <-6.5,-0.2,-2> look_at <-6.5,-0.2,-0.5> }     //Flügel, DrAchse innen
//camera { location <2,3,7> look_at <0,1,5.5> }     //Flügel, DrAchse außen


#macro Angelflugel (IKlappe,AKlappe,Anstellung)
//neuer Flügel:
union{

//#declare IKlappe=0; // - ist nach oben
//#declare AKlappe=0; // - ist nach oben
//#declare Anstellung=0; // - ist nach oben
difference{
	  difference{
	    cone{<0,0,0>,5.6*2 <6*2,0,0>,1.7*2 scale <1,0.6,1> pigment {Yellow}}
	    cone {<-0.1,0,0>,5.6*2 <6.01*2,0,0>,1.7*2 scale <1,1,1.48> scale <1,0.5,1> pigment {Yellow}}
	  }
	  pigment {White}
	
  difference{ //Form der Rundung, Vorderkante
    prism{linear_sweep linear_spline -0.2, 1.35 ,5,<-1.2,-1.1>,
    cone{<-1.25,0.233,-1.5>,0.491 <1.5,1.07441,6.75>,0.2048-0.05 pigment {Yellow}} 
   rotate -18.3*y scale <2,1,1> rotate 18.3*y translate <0,0,0>}
	  
	  //Einrundung in der die männliche Klappenrundung läuft!
	pigment {Yellow}}

}

#include "test2.pov"



//Drehachse der Ruder:
cylinder {<-6.375,-0.57,-0.625>,<-0.375,0.77,5.375>,0.05 pigment {Red}}



//inneres Ruder
difference{
   difference{ //Rundung, männliche Gelenkzapfen des Ruders
    prism{linear_sweep linear_spline -0.9, 1.1 ,5,<-5.875,-0.125>,
    cone{<-6.375,-0.57,-0.625>,0.175 <-0.375,0.77,5.375>,0.175 pigment {Grey}} 
   intersection{ //Rundung, in die männliche Zapfen des Flügels greifen
    cone{<-6.375,-0.57,-0.625>,0.21 <-0.375,0.77,5.375>,0.21 pigment {Grey}}
    union{
    prism{linear_sweep linear_spline -0.9, 1.1 ,5,<-6.5,-0.5>,
    prism{linear_sweep linear_spline -0.9, 1.1 ,5,<-4.875,0.875>,
    prism{linear_sweep linear_spline -0.9, 1.1 ,5,<-3.5,2.5>,
    }}}
   }
}}}}


//äußeres Ruder:
difference{
	intersection{
	  difference{
	  rotate 26.05*z
	  }
	  prism{linear_sweep linear_spline -4, 3 ,10,<-4+0.05,3.5+0.05>,
	  pigment {Grey}}
 
	}

   difference{ //Rundung, männliche Gelenkzapfen des Ruders
    prism{linear_sweep linear_spline -0.9, 1.1 ,5,<-2.625,3.125>,
   }
   intersection{ //Rundung, in die männliche Zapfen des Flügels greifen
    } 
   }
translate <6.375,0.57,0.625>
}



cylinder {<-2,0.225,-0.78>,<-2,0.225,-0.9>,0.4 pigment {Yellow}}


}//union ganzer Flügel
#end

//Drehbare Flügeleinheit komplett:
#macro Flugelanlage (KlapP)
 
union{

/*   	  	@object(macro=Fighter,  
	params={ sin(this.povvalI),sqrt(povvalS),povvalV,<1,2,77>},     //7777 
	defs={FCPIntens=pintens})  
 */

//  @object(macro=something, defs={lkjdflkjf=kjshdfkh,
// jdf=ksjdhf} //hhh
// ) // hhh

//  aaaaaaaaaa 
// @object(macro=whatever) @- jhsaskjh

// @-

/*@-object(params={})*/
/*@-object(macro=Something,macro=SomethingElse)*/
/*@-object(macro=Something,params={},declare=SomethingElse)*/
/*@-object(declare=thisone,params={abc})*/

/*@object(declare=declobj)*/

//Schulterflügelansatz:
difference{
		  sphere {<0,0,0>,0.8,1 scale <1.8,1,1> rotate 20*y rotate -25*z
	  rotate -30*x}
	box {<0.9,-0.6,-0.5>,<-4.6,1.0,0.55> pigment{Yellow}}
	}



rotate KlapP*x  //von +60 bis -10

translate <0,2.9,1.2>
}//Drehbare Flügeleinheit komplett:ENDE
#end



#macro Angelbody()

union{//main union

difference{ //Körper, in Difference alles was an Löchern rein muss!!
 blob {
 
 pigment { color Yellow }
  translate <0,2,0>
 }
cone {<-3,0,0>,0.6 <-6,0,0>,1 scale <1,0.8,1> translate <0,3,0> pigment {Yellow}} //beim
//Düsenauslass oben, direkt bei Auslass
}


//Schwanz: 
difference{
  sphere_sweep {
   cubic_spline,
  8,
  }
pigment { Yellow}
}
 
//feste Bestandteile:
difference {
	union{
	translate <0,2,0>
	}
cone {<-3,0,0>,0.6 <-6,0,0>,1 scale <1,0.8,1> translate 3*y pigment {Yellow}} //rausgeschnitten unter Endrohr!

pigment{Yellow}	
}

//Haube:
sphere {<0,0,0>,0.5 scale <1.5,1.5,1> translate <1.4,2.6,0> pigment {Blue}}
//rotate 90*x
} //main union ENDE
#end
/**/
