%Calibrate value
Rs = 24575.00;
%Excpected air values
Benceno = .034;
Tolueno = .035;
Fenol = .97;
Amonio = .005;
MonoxidoDeCarbono = .1;
DioxidoDeCarbono = 400;
disp("Calibración de mq135")

Ro_Benceno = Rs/((Benceno/37.89)^-(1/3.165))
Ro_Tolueno =  Rs/((Tolueno/47.36)^-(1/3.292))
Ro_Fenol =  Rs/((Fenol/79.77)^-(1/3.005))
Ro_Amonio =  Rs/((Amonio/101)^-(1/2.495))
Ro_MonoxidoDeCarbono =  Rs/((MonoxidoDeCarbono/763.7)^-(1/4.541))
Ro_DioxidoDeCarbono =  Rs/((DioxidoDeCarbono/110.8)^-(1/2.729))
