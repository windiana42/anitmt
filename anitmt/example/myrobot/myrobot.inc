#ifdef(MyRobot_Inc_Temp)
  /* do nothing */
#else
#declare MyRobot_Inc_Temp = version;
#version 3.1;
   
#ifdef(View_POV_Include_Stack)
  #debug "including MyRobot.inc\n"
#end

//#include "colors.inc"
//#include "spacetextur.inc"
//#include "myrobottex.inc"

/* --------------------------- Meldung --------------------------------- */
#render "---------------------------------------------------------------\n"
#render "Beginne mit MyRobot.inc\n"

// --------- Konstante Werte --------------------------
#declare Nix        =    0.000001;


#macro MyRobot (Rob_Drehgelenk_W,Rob_Armteil1_W,Rob_Armteil2_W,Rob_Tele_L,Rob_Armteil3_W,Finger1_Winkel,Finger2_Winkel)

  /*
  #declare Rob_Drehgelenk_W       =   45;
  #declare Rob_Armteil1_W         =   60;
  #declare Rob_Armteil2_W         =  -70;
  #declare Rob_Tele_L             = 19.25;
  */
  // min Rob_Tele1_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L
  // max Rob_Tele1_L + Rob_Tele2_L + Rob_Tele3_L + Rob_Tele4_L + Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L + 
  // Hier : min 7.25      max 19.25
  /*
  #declare Rob_Armteil3_W         =  10;
  #declare Finger1_Winkel         =  45; 
  #declare Finger2_Winkel         =  90; 
  */


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /************************************************************************************************************/
  /*----------------------------------------------------------------------------------------------------------*/
  /*--------------------------------------------  MyRobot       ----------------------------------------------*/
  /*----------------------------------------------------------------------------------------------------------*/
  /*                                                                                                          */
  #declare Rob_Sockel_B           = 12;      /*                                                               */
  #declare Rob_Sockel_H           =  1.5;    /*                                                               */
  #declare Rob_Drehgelenk_R       =  5;      /*                                                               */
  #declare Rob_Drehgelenk_H       =  2;      /*                                                               */
  #declare Rob_HaupAchseHalt_H    =  2.5;    /*                                                               */
  #declare Rob_HaupAchseHalt_R    =  2.0;    /*                                                               */
  #declare Rob_HaupAchseHalt_B    =  1.0;    /*                                                               */
  #declare Rob_HaupAchse_L        =  4;      /*                                                               */
  #declare Rob_HaupAchse_R        =  1.75;   /*                                                               */
  #declare Rob_HaupAchse_U        =  1.5 ;   /*                                                               */
  #declare Rob_Gelenk1_R          =  2.0 ;   /*                                                               */
  #declare Rob_Gelenk1_L          =  3.0 ;   /*                                                               */
  #declare Rob_Arm1_L             =  8.0 ;   /*                                                               */
  #declare Rob_Arm1_R             =  1.4 ;   /*                                                               */
  #declare Rob_Achse2Halt_H       =  1.7 ;   /*                                                               */
  #declare Rob_Achse2Halt_B       =  0.75;   /*                                                               */
  #declare Rob_Achse2Halt_R       =  1.5 ;   /*                                                               */
  #declare Rob_Achse2Halt_D       =  0.5 ;   /*                                                               */
  #declare Rob_Achse2_L           =  3.0 ;   /*                                                               */
  #declare Rob_Achse2_R           =  1.2 ;   /*                                                               */
  #declare Rob_Achse2_U           =  1.0 ;   /*                                                               */
  #declare Rob_Gelenk2_R          =  1.5 ;   /*                                                               */
  #declare Rob_Gelenk2_L          =  2.0 ;   /*                                                               */
  #declare Rob_Tele1_L            =  6.5 ;   /*                                                               */
  #declare Rob_Tele1_R            =  0.9 ;   /*                                                               */
  #declare Rob_Tele1Ring_L        =  0.25;   /*                                                               */
  #declare Rob_Tele1Ring_R        =  1.0 ;   /*                                                               */
  #declare Rob_Tele2_L            =  4.5 ;   /*                                                               */
  #declare Rob_Tele2_R            =  0.7 ;   /*                                                               */
  #declare Rob_Tele2Ring_L        =  0.25;   /*                                                               */
  #declare Rob_Tele2Ring_R        =  0.8 ;   /*                                                               */
  #declare Rob_Tele3_L            =  4.0 ;   /*                                                               */
  #declare Rob_Tele3_R            =  0.5 ;   /*                                                               */
  #declare Rob_Tele3Ring_L        =  0.25;   /*                                                               */
  #declare Rob_Tele3Ring_R        =  0.6 ;   /*                                                               */
  #declare Rob_Tele4_L            =  3.5 ;   /*                                                               */
  #declare Rob_Tele4_R            =  0.3 ;   /*                                                               */
  #declare Rob_Achse3Halt_H       =  0.4 ;   /*                                                               */
  #declare Rob_Achse3Halt_B       =  0.1 ;   /*                                                               */
  #declare Rob_Achse3Halt_R       =  0.3 ;   /*                                                               */
  #declare Rob_Achse3Halt_D       =  0.1 ;   /*                                                               */
  #declare Rob_Achse3_L           =  1.0 ;   /*                                                               */
  #declare Rob_Achse3_R           =  0.25;   /*                                                               */
  #declare Rob_Achse3_U           =  0.2 ;   /*                                                               */
  #declare Rob_Gelenk3_R          =  0.3 ;   /*                                                               */
  #declare Rob_Gelenk3_L          =  0.5 ;   /*                                                               */
  #declare Rob_Arm3_L             =  2.0 ;   /*                                                               */
  #declare Rob_Arm3_R             =  0.2 ;   /*                                                               */
  #declare Rob_Achse4Halt_H       =  0.3 ;   /*                                                               */
  #declare Rob_Achse4Halt_B       =  0.6 ;   /*                                                               */
  #declare Rob_Achse4Halt_R       =  0.25;   /*                                                               */
  #declare Rob_Achse4_L           =  2.70;   /*                                                               */
  #declare Rob_Achse4_R           =  0.20;   /*                                                               */
  #declare Rob_FGelenk_A          =  0.5 ;   /*                                                               */
  #declare Rob_FHalt1_H           =  0.30;   /*                                                               */
  #declare Rob_FHalt1_D           =  0.10;   /*                                                               */
  #declare Rob_FHalt1_R           =  0.25;   /*                                                               */
  #declare Rob_Finger1_L          =  1.00;   /*                                                               */
  #declare Rob_Finger1_R          =  0.15;   /*                                                               */
  #declare Rob_FHalt2_H           =  0.30;   /*                                                               */
  #declare Rob_FHalt2_D           =  0.10;   /*                                                               */
  #declare Rob_FHalt2_R           =  0.25;   /*                                                               */
  #declare Rob_Achse5_L           =  0.65;   /*                                                               */
  #declare Rob_Achse5_R           =  0.20;   /*                                                               */
  #declare Rob_Finger2_L          =  1.00;   /*                                                               */
  #declare Rob_Finger2_R          =  0.15;   /*                                                               */
  #declare Rob_FKugel_R           =  0.25;   /*                                                               */
  /*                                                                                                          */
  /*                                                                                                          */
  /*                                                                                                          */
  /*                                                                                                          */
  /*                                                                                                          */
  /*                                                                                                          */
  /************************************************************************************************************/
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////


  // ------- Objekt ---------- 

  // Finger2
  #declare Finger2 = union {
    cylinder {
      < + Rob_Achse5_L / 2, 0, 0 >, 
      < - Rob_Achse5_L / 2, 0, 0 >, 
      Rob_Achse5_R
      texture { Rob_Chrome_Look } 
    }
    union { 
      cylinder {
        < - Rob_FHalt2_R    , 0, 0 >,
        < - Rob_FHalt2_R / 3, 0, 0 >,
        Rob_FHalt2_R
      }
      box {
        < - Rob_FHalt2_R    , 0           , - Rob_FHalt2_R >,
        < - Rob_FHalt2_R / 3, Rob_FHalt2_H,   Rob_FHalt2_R >
      }
      box {
        < + Rob_FHalt2_R, Rob_FHalt2_H + Rob_FHalt2_D, - Rob_FHalt2_R >
        < - Rob_FHalt2_R, Rob_FHalt2_H               ,   Rob_FHalt2_R >
      }
      texture { Rob_BodyTxtr }
    }
    cylinder {
      < 0, Rob_FHalt2_H + Rob_FHalt2_D                , 0 >,
      < 0, Rob_FHalt2_H + Rob_FHalt2_D + Rob_Finger2_L, 0 >,
      Rob_Finger2_R
      texture { Rob_Chrome_Look } 
    }
    sphere {
      < 0, Rob_FHalt2_H + Rob_FHalt2_D + Rob_Finger2_L, 0 >,
      Rob_FKugel_R
      texture { Rob_BodyTxtr }
    }
  }

  // Finger1
  #declare Finger1 = union {
    union { 
      cylinder {
        < Rob_FHalt1_R    , 0, 0 >,
        < Rob_FHalt1_R / 3, 0, 0 >,
        Rob_FHalt1_R
      }
      box {
        < Rob_FHalt1_R    , 0           , - Rob_FHalt1_R >,
        < Rob_FHalt1_R / 3, Rob_FHalt1_H,   Rob_FHalt1_R >
      }
      box {
        < + Rob_FHalt1_R, Rob_FHalt1_H + Rob_FHalt1_D, - Rob_FHalt1_R >
        < - Rob_FHalt1_R, Rob_FHalt1_H               ,   Rob_FHalt1_R >
      }
      texture { Rob_BodyTxtr }
    }
    cylinder {
      < 0, Rob_FHalt1_H + Rob_FHalt1_D                , 0 >,
      < 0, Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L, 0 >,
      Rob_Finger1_R
      texture { Rob_Chrome_Look } 
    }
    union { 
      box {
        < + Rob_FHalt2_R, Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L + Rob_FHalt2_D, - Rob_FHalt2_R >
        < - Rob_FHalt2_R, Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L               ,   Rob_FHalt2_R >
      }
      box {
        < Rob_FHalt2_R    , Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L + Rob_FHalt2_D               , - Rob_FHalt2_R >,
        < Rob_FHalt2_R / 3, Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L + Rob_FHalt2_D + Rob_FHalt2_H,   Rob_FHalt2_R >
      }
      cylinder {
        < Rob_FHalt2_R    , Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L + Rob_FHalt2_D + Rob_FHalt2_H, 0 >,
        < Rob_FHalt2_R / 3, Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L + Rob_FHalt2_D + Rob_FHalt2_H, 0 >,
        Rob_FHalt2_R
      }
      texture { Rob_BodyTxtr }
    }
    object {
      Finger2
      rotate < - Finger2_Winkel, 0, 0 >
      translate < 0, Rob_FHalt1_H + Rob_FHalt1_D + Rob_Finger1_L + Rob_FHalt2_D + Rob_FHalt2_H, 0 >
    }

  }



  // Armteil3
  #declare Armteil3 = union {
    cylinder {
      < + Rob_Achse3_L / 2 + Rob_Achse3_U, 0, 0 >,
      < - Rob_Achse3_L / 2 - Rob_Achse3_U, 0, 0 >,
      Rob_Achse3_R
      texture { Rob_Chrome_Look } 
    }
    cylinder {
      <   Rob_Gelenk3_L / 2, 0, 0 >,
      < - Rob_Gelenk3_L / 2, 0, 0 >,
      Rob_Gelenk3_R
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < 0, 0,   0          >,
      < 0, 0, - Rob_Arm3_L >,
      Rob_Arm3_R
      texture { Rob_Chrome_Look } 
    }
    box {
      < - Rob_Achse4Halt_B / 2, - Rob_Achse4Halt_R, - Rob_Arm3_L                    >,
      < + Rob_Achse4Halt_B / 2, + Rob_Achse4Halt_R, - Rob_Arm3_L - Rob_Achse4Halt_H >
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < - Rob_Achse4Halt_B / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >,
      < + Rob_Achse4Halt_B / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >,
      Rob_Achse4Halt_R
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < - Rob_Achse4_L / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >,
      < + Rob_Achse4_L / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >,
      Rob_Achse4_R
      texture { Rob_Chrome_Look }              
    }
    object {
      Finger1                
      rotate < - Finger1_Winkel, 0, 0 >
      translate <   Rob_FGelenk_A + Rob_Achse4Halt_B / 2 + Rob_FHalt1_R / 2,  0, - Rob_Arm3_L - Rob_Achse4Halt_H >
    }
    object {
      Finger1                  
      rotate < 0, 0, 180 >
      rotate <   Finger1_Winkel, 0, 0 >
      translate <   Rob_FGelenk_A + Rob_Achse4Halt_B / 2 + Rob_FHalt1_R / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >
    }
    object {
      Finger1
      rotate < - Finger1_Winkel, 0, 0 >
      translate < - Rob_FGelenk_A - Rob_Achse4Halt_B / 2 - Rob_FHalt1_R / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >
    }
    object {
      Finger1
      rotate < 0, 0, 180 >
      rotate <   Finger1_Winkel, 0, 0 >
      translate < - Rob_FGelenk_A - Rob_Achse4Halt_B / 2 - Rob_FHalt1_R / 2, 0, - Rob_Arm3_L - Rob_Achse4Halt_H >
    }
  }



  #declare Zu_Vergebende_L         = Rob_Tele_L - (Rob_Tele1Ring_L + Rob_Tele2Ring_L + Rob_Tele3Ring_L) - Rob_Tele1_L;
  // Hier:                         =            -  0.75                                                 - 6.5
  #declare Relativ_Zu_Vergebende_L = Zu_Vergebende_L / (Rob_Tele2_L + Rob_Tele3_L + Rob_Tele4_L);
  // Hier:                         =                 /  12
  #declare Rob_Tele2_AL = Relativ_Zu_Vergebende_L * Rob_Tele2_L;
  // Hier:          
  #declare Rob_Tele3_AL = Relativ_Zu_Vergebende_L * Rob_Tele3_L;
  #declare Rob_Tele4_AL = Relativ_Zu_Vergebende_L * Rob_Tele4_L;

  // Tele 4
  #declare Tele4 = union {
    cylinder {
      < 0, 0, + Rob_Tele4_L >,
      < 0, 0,   0           >,
      Rob_Tele4_R
      texture { Rob_Chrome_Look } 
    }
    box {
      < - Rob_Achse3_L / 2 - Rob_Achse3Halt_B, - Rob_Achse3Halt_R, - Rob_Achse3Halt_D >,
      < + Rob_Achse3_L / 2 + Rob_Achse3Halt_B,   Rob_Achse3Halt_R,                    >
      texture { Rob_BodyTxtr } 
    }
    box {
      < - Rob_Achse3_L / 2 - Rob_Achse3Halt_B, - Rob_Achse3Halt_R, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >,
      < - Rob_Achse3_L / 2                   ,   Rob_Achse3Halt_R, - Rob_Achse3Halt_D                    >
      texture { Rob_BodyTxtr } 
    }
    box {
      < + Rob_Achse3_L / 2 + Rob_Achse3Halt_B, - Rob_Achse3Halt_R, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >,
      < + Rob_Achse3_L / 2                   ,   Rob_Achse3Halt_R, - Rob_Achse3Halt_D                    >
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < + Rob_Achse3_L / 2 + Rob_Achse3Halt_B, 0, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >,
      < + Rob_Achse3_L / 2                   , 0, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >,
      Rob_Achse3Halt_R
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < - Rob_Achse3_L / 2 - Rob_Achse3Halt_B, 0, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >,
      < - Rob_Achse3_L / 2                   , 0, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >,
      Rob_Achse3Halt_R
      texture { Rob_BodyTxtr } 
    }
    object {
      Armteil3
      rotate < Rob_Armteil3_W, 0, 0 >
      translate < 0, 0, - Rob_Achse3Halt_D - Rob_Achse3Halt_H >
    }
  }

  // Tele 3
  #declare Tele3 = union {
    cylinder {
      < 0, 0, + Rob_Tele3_L >,
      < 0, 0,   0           >,
      Rob_Tele3_R
      texture { Rob_Chrome_Look } 
    }
    cylinder {
      < 0, 0,   0               >,
      < 0, 0, - Rob_Tele3Ring_L >,
      Rob_Tele3Ring_R
      texture { Rob_BodyTxtr } 
    }
    object {
      Tele4
      translate < 0, 0, - Rob_Tele3Ring_L - Rob_Tele4_AL >
    }
  }


  // Tele 2
  #declare Tele2 = union {
    cylinder {
      < 0, 0, + Rob_Tele2_L >,
      < 0, 0,   0           >,
      Rob_Tele2_R
      texture { Rob_Chrome_Look } 
    }
    cylinder {
      < 0, 0,   0               >,
      < 0, 0, - Rob_Tele2Ring_L >,
      Rob_Tele2Ring_R
      texture { Rob_BodyTxtr } 
    }
    object {
      Tele3
      translate < 0, 0, - Rob_Tele2Ring_L - Rob_Tele3_AL >
    }
  }

  // Armteil2 (Tele 1)
  #declare Armteil2 = union {
    cylinder {
      < + Rob_Achse2_L / 2 + Rob_Achse2_U, 0, 0 >,
      < - Rob_Achse2_L / 2 - Rob_Achse2_U, 0, 0 >,
      Rob_Achse2_R
      texture { Rob_Chrome_Look } 
    }
    cylinder {
      <   Rob_Gelenk2_L / 2, 0, 0 >,
      < - Rob_Gelenk2_L / 2, 0, 0 >,
      Rob_Gelenk2_R
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < 0, 0,   0            >,
      < 0, 0, - Rob_Tele1_L >,
      Rob_Tele1_R
      texture { Rob_Chrome_Look } 
    }
    cylinder {
      < 0, 0, - Rob_Tele1_L                   >,
      < 0, 0, - Rob_Tele1_L - Rob_Tele1Ring_L >,
      Rob_Tele1Ring_R
      texture { Rob_BodyTxtr } 
    }
    object {
      Tele2
      translate < 0, 0, - Rob_Tele1_L - Rob_Tele1Ring_L - Rob_Tele2_AL >
    }
  }

  // Armteil1
  #declare Armteil1 = union {
    cylinder {
      <   Rob_HaupAchse_L / 2 + Rob_HaupAchse_U, 0, 0 >,
      < - Rob_HaupAchse_L / 2 - Rob_HaupAchse_U, 0, 0 >,
      Rob_HaupAchse_R
      texture { Rob_Chrome_Look } 
    }
    cylinder {
      <   Rob_Gelenk1_L / 2, 0, 0 >,
      < - Rob_Gelenk1_L / 2, 0, 0 >,
      Rob_Gelenk1_R
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < 0, 0,   0          >,
      < 0, 0, - Rob_Arm1_L >,
      Rob_Arm1_R
      texture { Rob_Chrome_Look } 
    }
    box {
      < - Rob_Achse2_L / 2 - Rob_Achse2Halt_B, - Rob_Achse2Halt_R, - Rob_Arm1_L - Rob_Achse2Halt_D >,
      < + Rob_Achse2_L / 2 + Rob_Achse2Halt_B,   Rob_Achse2Halt_R, - Rob_Arm1_L                    >
      texture { Rob_BodyTxtr } 
    }
    box {
      < - Rob_Achse2_L / 2 - Rob_Achse2Halt_B, - Rob_Achse2Halt_R, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >,
      < - Rob_Achse2_L / 2                   ,   Rob_Achse2Halt_R, - Rob_Arm1_L - Rob_Achse2Halt_D                    >
      texture { Rob_BodyTxtr } 
    }
    box {
      < + Rob_Achse2_L / 2 + Rob_Achse2Halt_B, - Rob_Achse2Halt_R, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >,
      < + Rob_Achse2_L / 2                   ,   Rob_Achse2Halt_R, - Rob_Arm1_L - Rob_Achse2Halt_D                    >
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < + Rob_Achse2_L / 2 + Rob_Achse2Halt_B, 0, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >,
      < + Rob_Achse2_L / 2                   , 0, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >,
      Rob_Achse2Halt_R
      texture { Rob_BodyTxtr } 
    }
    cylinder {
      < - Rob_Achse2_L / 2 - Rob_Achse2Halt_B, 0, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >,
      < - Rob_Achse2_L / 2                   , 0, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >,
      Rob_Achse2Halt_R
      texture { Rob_BodyTxtr } 
    }
    object {
      Armteil2
      rotate < Rob_Armteil2_W, 0, 0 >
      translate < 0, 0, - Rob_Arm1_L - Rob_Achse2Halt_D - Rob_Achse2Halt_H >
    }
  }

  // Drehgelenk
  #declare Drehgelenk = union {
    union {
      cylinder {
        < 0, 0, 0 >, < 0, Rob_Drehgelenk_H, 0 >, Rob_Drehgelenk_R
      }
      box {
        <   Rob_HaupAchse_L / 2 + Rob_HaupAchseHalt_B, Rob_Drehgelenk_H                      ,   Rob_HaupAchseHalt_R >,
        <   Rob_HaupAchse_L / 2                      , Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, - Rob_HaupAchseHalt_R >
      }
      box {
        < - Rob_HaupAchse_L / 2 - Rob_HaupAchseHalt_B, Rob_Drehgelenk_H                      ,   Rob_HaupAchseHalt_R >,
        < - Rob_HaupAchse_L / 2                      , Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, - Rob_HaupAchseHalt_R >
      }
      cylinder {
        <   Rob_HaupAchse_L / 2 + Rob_HaupAchseHalt_B, Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, 0 >,
        <   Rob_HaupAchse_L / 2                      , Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, 0 >,
        Rob_HaupAchseHalt_R
      }    
      cylinder {
        < - Rob_HaupAchse_L / 2 - Rob_HaupAchseHalt_B, Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, 0 >,
        < - Rob_HaupAchse_L / 2                      , Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, 0 >,
        Rob_HaupAchseHalt_R
      }    
      texture { Rob_BodyTxtr }
    }
    object {
      Armteil1
      rotate < Rob_Armteil1_W, 0, 0 >
      translate < 0, Rob_Drehgelenk_H + Rob_HaupAchseHalt_H, 0 >
    }
  }

  // Sockel
  #declare Sockel = box {
    < - Rob_Sockel_B / 2, 0, - Rob_Sockel_B / 2 >, < Rob_Sockel_B / 2, Rob_Sockel_H, Rob_Sockel_B / 2 > 
    texture { Rob_BodyTxtr }
  }

  // Roboter
  #declare TheMyRobot = union {
    object { Sockel }
    object {
      Drehgelenk 
      rotate < 0, Rob_Drehgelenk_W, 0 >
      translate < 0, Rob_Sockel_H, 0 >
    }  
  }
  
  object { TheMyRobot }
 
#end

/* ------ Meldung ---------------------- */
#render "MyRobot.inc beendet\n"


#version MyRobot_Inc_Temp;
#end


//----ERROR---->ALL RIGHT<----                No objects in scene?


