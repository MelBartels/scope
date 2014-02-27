  REM *PROGRAM PLANETS*
beg:
  DECLARE SUB BlocMaius ()
  DECLARE SUB AggiornaFile ()
  CLEAR
  COLOR 10, 1, 1
  CLS
  BlocMaius
  DIM MS(12)
  PI = 3.14159265#: K = PI / 180
  LOCATE 5, 2: PRINT " This program will calculate A.R. and Dec. of the solar system "
  LOCATE 6, 2: PRINT " objects and create the file PLANETS.DAT to be used by SCOPE."
  LOCATE 10, 2: PRINT " Do you want to use system DATE and TIME (Y/N) ? "
    Lettera$ = UCASE$(INPUT$(1))
   SELECT CASE Lettera$
    CASE "Y"
      GOTO sydate
    CASE ELSE
      GOTO datim
   END SELECT
datim:
   CLS
   LOCATE 2, 2: INPUT " Date   (day) "; G
   LOCATE 2, 22: INPUT "(month) "; MS
   LOCATE 2, 34: INPUT " (year) "; Y
   LOCATE 3, 2: INPUT " Time  (hour) "; H
   LOCATE 3, 22: INPUT "  (min) "; M
   LOCATE 3, 34: INPUT "  (sec) "; S
   GOTO start
sydate:
  G = VAL(MID$(DATE$, 4, 2))
  MS = VAL(LEFT$(DATE$, 2))
  Y = VAL(RIGHT$(DATE$, 4))

  H = VAL(LEFT$(TIME$, 2))
  M = VAL(MID$(TIME$, 4, 2))
  S = VAL(RIGHT$(TIME$, 2))
start:
  AggiornaFile
  GG = G
  MS(1) = 0: MS(2) = 31: MS(3) = 28: MS(4) = 31: MS(5) = 30: MS(6) = 31
  MS(7) = 30: MS(8) = 31: MS(9) = 31: MS(10) = 30: MS(11) = 31: MS(12) = 30
  Y1 = (Y - 1900) / 4: IF (Y1 - INT(Y1)) = 0 THEN LET MS(3) = 29
  FOR I = 1 TO MS: GG = GG + MS(I): NEXT I
  T0 = ((Y - 1900) * 365 + INT(Y1) - .5 + GG) / 36525!
  T0 = (T0 * 8640184.541999999# + .0929 * T0 ^ 2 + 45.836) / 60 ^ 2 + 6 + 38 / 60
  T0 = (T0 / 24 - INT(T0 / 24)) * 24
  TH = INT(T0): TM = INT((T0 - TH) * 60): TS = INT(((T0 - TH) * 60 - TM) * 60)
 
  OPEN "SITE.DDD" FOR INPUT AS #2
  INPUT #2, C$, LH, LM, LS, TZ
   HO = H + M / 60 + S / 3600
   AL = ABS(LH)
   LO = LH / AL * (AL + LM / 60 + LS / 3600) / 15
   ST = (HO + TZ) * (1 + .06571 / 24) - LO + T0
   ST = (ST / 24 - INT(ST / 24)) * 24
   SH = INT(ST): SM = INT((ST - SH) * 60): SS = INT(((ST - SH) * 60 - SM) * 60)
   CLS
   PRINT
   PRINT " "; C$; "   (d m Y)  : "; G; MS; Y
   PRINT " Time    (h m s)  = "; H; M; S
   PRINT " Long.  (ø ' '')  = "; LH; LM; LS
   PRINT " Sid Time 0 UT (h m s) = "; TH; TM; TS
   PRINT " Loc Sid Time  (h m s) = "; SH; SM; SS
   PRINT
   GT = (Y - 1900) * 365 + INT(Y1) + GG - .5 + (HO + TZ) / 24: T = GT / 36525!: D = GT / 10 ^ 4
   RESTORE sun
    OPEN "PLANETS.DAT" FOR OUTPUT AS #1
    READ M, M1, M2, L, L1, L2, EC, E0, EP, E1, AM
    SU$ = " Sun"
sun:
    DATA 358.475833,35999.0497522,1.494165E-04,281.220844,1.7191733
    DATA 4.52251637E-04,.01675104,4.18E-05,23.452294,.0130125,1.00000023
    M = K * (M + M1 * T - M2 * T ^ 2): M = (M / 2 / PI - INT(M / 2 / PI)) * 2 * PI
    L = K * (L + L1 * T + L2 * T ^ 2): EC = EC - E0 * T: EP = K * (EP - E1 * T)
    NU = M + (2 * EC - EC ^ 3 / 4) * SIN(M) + 5 / 4 * EC ^ 2 * SIN(2 * M) + 13 / 12 * EC ^ 3 * SIN(3 * M)
    E = 2 * ATN(TAN(NU / 2) * SQR((1 - EC) / (1 + EC))): RS = AM * (1 - EC * COS(E))
    XS = RS * COS(NU + L): YS = RS * SIN(NU + L) * COS(EP): ZS = RS * SIN(NU + L) * SIN(EP)
    SR = ATN(YS / XS)
    IF XS < 0 THEN LET SR = SR + PI: GOTO ok
    IF YS < 0 THEN LET SR = SR + 2 * PI: GOTO ok
ok:
    DE = ATN(ZS / RS / SQR(1 - (ZS / RS) ^ 2))
    SO = SR / 15 / K: SOH = INT(SO): SOM = INT((SO - SOH) * 60): SOS = INT(((SO - SOH) * 60 - SOM) * 60)
    DG = ABS(DE / K): DGG = INT(DG): DGM = INT((DG - DGG) * 60): DGS = INT(((DG - DGG) * 60 - DGM) * 60)
    ARS = SOH + SOM / 100 + SOS / 10000
    DCS = DE / ABS(DE) * (DGG + DGM / 100 + DGS / 10000)
    TR = SO - ST + HO: TR = (TR / 24 - INT(TR / 24)) * 24
    TH = INT(TR): TM = INT((TR - TH) * 60): TRS = INT(((TR - TH) * 60 - TM) * 60)
    TRA = TH + TM / 100 + TRS / 10000
    PRINT TAB(14); "A.R."; TAB(26); "DEC."; TAB(38); "D.A."; TAB(48); "MAG."; TAB(58); "TRANS."
    PRINT TAB(14); "hh.mmss"; TAB(26); "gg.ppss"; TAB(38); "sec"; TAB(58); "hh.mmss"
    PRINT
    PRINT SU$; TAB(12); USING "###.####"; ARS;
    PRINT TAB(24); USING "###.####"; DCS;
    PRINT TAB(56); USING "###.####"; TRA
    PRINT #1, SOH; TAB(5); SOM; TAB(9); SOS; TAB(15); SGN(DE) * DGG; TAB(19); DGM; TAB(23); DGS; TAB(28); SU$

    LU$ = " Moon"
moon:
    DATA 270.434164,13.1763965268,-8.5E-5,259.183275,-.0529539222,1.557E-4
    DATA 334.329556,.1114040803,-.00077,5.145396374,.054900489
    RESTORE moon
    READ L, L1, L2, O, O1, O2, GL, G1, G2, IL, EL
    L = K * (L + L1 * GT + L2 * D ^ 2): L = (L / 2 / PI - INT(L / 2 / PI)) * 2 * PI
    O = K * (O + O1 * GT + O2 * D ^ 2): O = (O / 2 / PI - INT(O / 2 / PI)) * 2 * PI
    GL = K * (GL + G1 * GT + G2 * D ^ 2): GL = (GL / 2 / PI - INT(GL / 2 / PI)) * 2 * PI
    IL = IL * K: M = L - GL: M = (M / 2 / PI - INT(M / 2 / PI)) * 2 * PI
    NL = M + (2 * EL - EL ^ 3 / 4) * SIN(M) + 5 / 4 * EL ^ 2 * SIN(2 * M) + 13 / 12 * EL ^ 3 * SIN(3 * M)
    NG = ((NL + GL) / 2 / PI - INT((NL + GL) / 2 / PI)) * 2 * PI
    BL = SIN(NG - O) * SIN(IL): BL = ATN(BL / SQR(1 - BL ^ 2))
    LA = ATN(TAN(NG - O) * COS(IL)) + O
    IF COS(NG - O) < 0 THEN LET LA = LA + PI: GOTO calc
    IF SIN(NG - O) < 0 THEN LET LA = LA + 2 * PI: GOTO calc
calc:
    LA = (LA / 2 / PI - INT(LA / 2 / PI)) * 2 * PI
    DL = COS(BL) * SIN(LA) * SIN(EP) + SIN(BL) * COS(EP)
    DL = ATN(DL / SQR(1 - DL ^ 2))
    LR = ATN((COS(BL) * SIN(LA) * COS(EP) - SIN(BL) * SIN(EP)) / (COS(BL) * COS(LA)))
    IF COS(NG) < 0 THEN LET LR = LR + PI: GOTO corr
    IF SIN(NG) < 0 THEN LET LR = LR + 2 * PI: GOTO corr
corr:
    LR = (LR / 2 / PI - INT(LR / 2 / PI)) * 2 * PI
    LU = LR / 15 / K: LUH = INT(LU): LUM = INT((LU - LUH) * 60): LUS = INT(((LU - LUH) * 60 - LUM) * 60)
    DU = ABS(DL / K): DUG = INT(DU): DUM = INT((DU - DUG) * 60): DUS = INT(((DU - DUG) * 60 - DUM) * 60)
    ARL = LUH + LUM / 100 + LUS / 10 ^ 4
    DCL = DL / K / DU * (DUG + DUM / 100 + DUS / 10 ^ 4)
    TLU = LU - ST + HO: TLU = (TLU / 24 - INT(TLU / 24)) * 24
    TLH = INT(TLU): TLM = INT((TLU - TLH) * 60): TLS = INT(((TLU - TLH) * 60 - TLM) * 60)
    TRL = TLH + TLM / 100 + TLS / 10 ^ 4
    PRINT LU$; TAB(12); USING "###.####"; ARL;
    PRINT TAB(24); USING "###.####"; DCL;
    PRINT TAB(56); USING "###.####"; TRL
    PRINT #1, LUH; TAB(5); LUM; TAB(9); LUS; TAB(15); DUG * SGN(DL); TAB(19); DUM; TAB(23); DUS; TAB(28); LU$
merc:
    DATA " Mercury"
    DATA 3.1098116,.0714254533,1.3247,7.43283e-07,.822852,5.66346e-07
    DATA .20561421#,5.585E-10,.1222233,.3870986,6.7,-.21,3.8,-3.4,2
venu:
    DATA " Venus"
    DATA 5.9824136,.027963119,2.27178745,6.72823165E-07,1.32260435,5.66346E-07
    DATA .00682069,-1.30705E-09,.05923003,.7233316,16.7,-4.1,.1,2.4,-.65
mars:
    DATA " Mars",5.126863279
    DATA .0091467673,5.990287692,8.795974E-07,.851484038,3.684146E-7
    DATA .0933129,2.520575E-09,.032294409,1.5236915,9.4,-1.36,1.5,0,0
jupi:
    DATA " Jupiter",4.159103463
    DATA .001450683628,.2218584983,7.698615077E-07,1.735519524,4.830221E-07
    DATA .0483376,4.4627E-09,.0228410269,5.202803,196.9,-9,1.48,0,0
satu:
    DATA " Saturn",4.654194
    DATA 5.8456837E-04,1.58980248,9.36178E-07,1.968445836,4.1718554E-07
    DATA .05589,-9.50034E-09,.043503786,9.538843,165.5,-8.7,1.7,0,0
uran:
    DATA " Uranus",4.2480274,2.0540274E-04,2.950245
    DATA 7.759674E-07,1.2826415,2.4396645E-07
    DATA .0470463,.07447E-09,.013486547,19.21694,71.4,-7,0,0,0
nept:
    DATA " Neptune",1.48432971,1.0504235E-04
    DATA .76353063,4.195745E-07,2.28077494,5.264257E-07
    DATA .0085285,2.1081451E-09,.031053771,30.057779,68.3,-7,0,0,0
plut:
    DATA " Pluto",1.614988159,7.01214E-05
    DATA 3.897556305,6.672E-07,1.903156305,6.672E-07
    DATA .250236,0,.29968,39.438712,8.3,-1.5,0,0,0
    RESTORE merc: GOSUB pcalc
    RESTORE venu: GOSUB pcalc
    RESTORE mars: GOSUB pcalc
    RESTORE jupi: GOSUB pcalc
    RESTORE satu: GOSUB pcalc
    RESTORE uran: GOSUB pcalc
    RESTORE nept: GOSUB pcalc
    RESTORE plut: GOSUB pcalc
    GOTO reca
pcalc:
    READ P$, L, L0, W, W0, O, O0, EC, E0, I, A, DA, MG, A0, B0, C0
    W = W + W0 * GT: M = L + L0 * GT - W: M = (M / 2 / PI - INT(M / 2 / PI)) * 2 * PI
    EC = EC + E0 * GT: O = O + O0 * GT
    NU = M + (2 * EC - EC ^ 3 / 4) * SIN(M) + 5 / 4 * EC ^ 2 * SIN(2 * M) + 13 / 12 * EC ^ 3 * SIN(3 * M)
    E = 2 * ATN(TAN(NU / 2) * SQR((1 - EC) / (1 + EC)))
    RP = A * (1 - EC * COS(E)): AL = NU + W - O
    IF COS(AL) = 0 THEN LET D = AL: GOTO new
    D = ATN(TAN(AL) * COS(I)): IF COS(AL) < 0 THEN LET D = D + PI
new:
    LH = D + O
    XP = RP * (COS(O) * COS(AL) - SIN(O) * SIN(AL) * COS(I))
    YI = RP * (SIN(O) * COS(AL) + COS(O) * SIN(AL) * COS(I))
    ZI = RP * SIN(AL) * SIN(I)
    YP = YI * COS(EP) - ZI * SIN(EP): ZP = YI * SIN(EP) + ZI * COS(EP)
    CS = XP + XS: ET = YP + YS: ZT = ZP + ZS: AR = ATN(ET / CS): R = SQR(CS ^ 2 + ET ^ 2)
    IF CS < 0 THEN LET AR = AR + PI: GOTO just
    IF ET < 0 THEN LET AR = AR + 2 * PI: GOTO just
just:
    DE = ATN(ZT * COS(AR) / CS)
    DA = DA / R: FA = ABS(AR - LH) / K: IF FA > 180 THEN LET FA = 360 - FA
    MG = MG + 5 * LOG(RP * R) * .4343 + A0 * FA / 100 + B0 * FA ^ 2 / 10 ^ 4 + C0 * FA ^ 3 / 10 ^ 6
    MG = INT(MG * 10 + .5) / 10
    AR = AR / 15 / K: DE = DE / K: DA = INT(DA * 100) / 100
    TR = AR - ST + HO: TR = (TR / 24 - INT(TR / 24)) * 24
    ARH = INT(AR): ARM = INT((AR - ARH) * 60): ARS = INT(((AR - ARH) * 60 - ARM) * 60)
    ARP = ARH + ARM / 100 + ARS / 10 ^ 4
    DP = ABS(DE): DH = INT(DP): DM = INT((DP - DH) * 60): DS = INT(((DP - DH) * 60 - DM) * 60)
    DCP = DE / DP * (DH + DM / 100 + DS / 10 ^ 4)
    TRH = INT(TR): TRM = INT((TR - TRH) * 60): TRS = INT(((TR - TRH) * 60 - TRM) * 60)
    TRP = TRH + TRM / 100 + TRS / 10 ^ 4
    PRINT P$; TAB(12); USING "###.####"; ARP;
    PRINT TAB(24); USING "###.####"; DCP;
    PRINT TAB(36); USING "###.##"; DA;
    PRINT TAB(46); USING "###.#"; MG;
    PRINT TAB(56); USING "###.####"; TRP
    PRINT #1, ARH; TAB(5); ARM; TAB(9); ARS; TAB(15); SGN(DE) * DH; TAB(19); DM; TAB(23); DS; TAB(28); P$
    RETURN
    CLOSE #1
reca:
    PRINT
    PRINT " Press 'R' to restart, any other key to exit this program"
    PRINT " and be redy to use the file PLANETS.DAT with SCOPE."
    Lettera$ = UCASE$(INPUT$(1))
   SELECT CASE Lettera$
    CASE "R"
      GOTO beg
    CASE ELSE
      GOTO fine
   END SELECT

fine:
    CLS : SYSTEM
    END

DEFDBL A-Z
SUB AggiornaFile STATIC

CLS
   OPEN "SITE.DDD" FOR INPUT AS #2
   LOCATE 2, 2
   PRINT " Actual site data are:"
   PRINT
   DO UNTIL EOF(2)
   INPUT #2, C$
   PRINT " Site name      :  "; C$
   INPUT #2, LH, LM, LS
   PRINT " Long. (ø ' '') : "; LH; LM; LS
   INPUT #2, TZ
   PRINT " TZ      (h)    :  "; TZ

   EXIT DO
   LOOP
   CLOSE #2
   PRINT
   PRINT
   LOCATE 15, 1
   PRINT
   PRINT " Press 'M' to modify the data, any other key to continue"
   Lettera$ = UCASE$(INPUT$(1))
 SELECT CASE Lettera$
   CASE "M"
     GOTO modify
  
   CASE ELSE
     GOTO vai
 END SELECT

modify:
  CLS
  OPEN "SITE.DDD" FOR OUTPUT AS #2
  
   LOCATE 4, 2: INPUT " Site name "; C$
   LOCATE 6, 2: PRINT TAB(12); "(ø)"; TAB(18); "(')"; TAB(24); "('')"
   LOCATE 7, 2: INPUT " Long.  "; LH
   LOCATE 7, 17: INPUT ; LM
   LOCATE 7, 23: INPUT ; LS
   LOCATE 8, 2: INPUT " Tz      (h)     "; TZ
   WRITE #2, C$, LH, LM, LS, TZ

   CLOSE #2
vai:
END SUB

DEFSNG A-Z
SUB BlocMaius STATIC
DEF SEG = 0
POKE &H417, PEEK(&H417) OR &H40
DEF SEG
END SUB

