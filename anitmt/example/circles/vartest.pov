camera { 
  location <    0 ,  0 ,  -10 >
  look_at  <    0 ,  0 ,    0 >
}

light_source { 
  <   -100,  300, -200>
  color rgb < 0.8, 0.8, 0.8 >
}

#macro ZeigeVariabale ( DerText, Variable, Stellen, Komma, Position )
  text {
    ttf "arial.ttf"
    concat( DerText, str(Variable, Stellen, Komma ) )
    0.1,
    < 0, 0, 0 >
    translate Position
    pigment { 
      color rgb < 1, 1, 0 >
    }
  }
#end

#macro ZeigeLineal( StartPos, EndPos, Startwert, Endwert, Markieren, Wert )
  cylinder {
    StartPos,
    EndPos,
    0.2
    pigment {
      color rgb < 0, 0, 1 >
    }
  }
  #local Var_I = Startwert;
  #while ( Var_I <= Endwert )
    sphere {
      (Var_I - Startwert) / (Endwert - Startwert) * EndPos + (1 - (Var_I - Startwert) / (Endwert - Startwert)) * StartPos, 0.205
      pigment { 
        color rgb < 1, 1, 0 >
      }
    }
    #local Var_I = Var_I + Markieren;
  #end
  sphere {
    (Wert - Startwert) / (Endwert - Startwert) * EndPos + (1 - (Wert - Startwert) / (Endwert - Startwert)) * StartPos, 0.21
    pigment { 
      color rgb < 1, 0, 0 >
    }
  }
  
#end

#declare Rotation = 0;
#declare PlasmaOffset = 0;
#declare TimeVar = 0;

ZeigeVariabale ( "TimeVar:  ", TimeVar,      9, 3, < - 5, 4, 0 > )
ZeigeVariabale ( "Rotation: ", Rotation,     9, 3, < - 5, 2, 0 > )
ZeigeLineal    ( < -5, 1, 0 >, < +5, 1, 0 >, -10, 360, 10, Rotation )
ZeigeVariabale ( "Plasma:   ", PlasmaOffset, 9, 3, < - 5, 0, 0 > )

  
