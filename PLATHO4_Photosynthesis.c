//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "xinclexp.h"
#include "Platho4.h"

int Photosynthesis_PLATHO(EXP_POINTER2,int iPlant, int it);


extern double RelativeTemperatureResponse_PLATHO(double vTemp, double vMinTemp, double vOptTemp, double vMaxTemp);

extern double Daylength(double vLatitude, int nJulianDay, int nID);

extern double HourlyTemperature_PLATHO	(double Time, double vTempMax, double vTempMin, double vTempMinNext, 
									 double vDaylength, int iSimDay);

extern double LeafAreaCum(PPLANT pPlant, double vLeafArea, double vPlantHeight, double h, int);

extern	int	GetPlantNeighbours(PPLANT, int, int*, int*, int*, int*);

extern double CalcLAICumNbg(int N, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg);

extern double CalcLA_Nbg(int iLeafLayer, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg);

extern double PhiAssimilatstau(PPLANT);

extern double AVGENERATOR(double vInput, VRESPONSE* pvResp);



int Photosynthesis_PLATHO(EXP_POINTER2,int iPlant,int it)
{
	extern double  PhiOzone(int);
    double dark_leaf_respiration_temperature(double temperature, double DeltaHaresp);
    int ReadFalgeParameters(int iPlant, PPLANT pPlant);
	extern double	PhiLeafPathogenes(int);
    extern double calc_seasonal_reduction(double Temperature);

	double DeltaT = (double)pTi->pTimeStep->fAct;	// Zeitschritt	
	double TimeDay = (double)pTi->pSimTime->fTimeDay;//Tageszeit
    double TempAct;

	double TimeBeginPs, TimeEndPs, hPs, vHour;
	double PhiN, PhiC, PhiO3, PhiTemp, PhiLfPath, PhiCO2;
	double dCO2, dCO2Cmp, dCO2PhotoCmp0, dCO2R, dCO2I, dCO2I340;
	double vNFactor, alphaN, vK;
	double dLAICum, dLAICumUpper, dLAICumLeft, dLAICumRight, dLAICumLower;
    double Amax, AmaxOpt, dEpsLUE;

	double VcmaxOpt,JmaxOpt,Alpha;
	double vDec, vSin, vCos, vRsc, vDailySin, vDailySinE, vSolarConst, vSinHeight,vDaylengthPs;
	double vAtmTrans, vDifFr;
	double PAR, PARDIR, PARDIF;
	double vExtDiffuse[LEAFLAYERS], vExtTotalDir[LEAFLAYERS], vExtDirect[LEAFLAYERS];
    double FSLLA[LEAFLAYERS];
	double FGRSH,FGRSUN,VISSUN,VISPP,FGRS,FGL;
	double SCV, SQV,REFH,REFS,VISDF,VIST,VISD,VISSHD;
	double CLUSTF[MAXPLANTS],KDIRBL[MAXPLANTS],KDIRT[MAXPLANTS];
	double RAD	= PI/180.0;

    static double PARCum;

	int	i,j;
	int	iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;
	int	N;

	int    nGauss=5;
    double xGauss[]={0.0469101, 0.2307534, 0.5000000, 0.7692465, 0.9530899};
	double wGauss[]={0.1184635, 0.2393144, 0.2844444, 0.2393144, 0.1184635};
		   
	PPLANT pPlantLeft, pPlantRight, pPlantUpper, pPlantLower;

	PBIOMASS			pBiom		= pPlant->pBiomass;
    PCANOPY             pCan        = pPlant->pCanopy;
	PPLTNITROGEN		pPltN		= pPlant->pPltNitrogen;
	PPLATHODEVELOP		pPDev		= pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHOGENOTYPE		pPGen		= pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOMORPHOLOGY	pPMorph		= pPlathoPlant[iPlant]->pPlathoMorphology;
    PPLATHOPARTITIONING pPPart      = pPlathoPlant[iPlant]->pPlathoPartitioning;
    PPLANTSTRESS		pPStress    = pPlathoPlant[iPlant]->pPlantStress;
    PFALGEPS            pFlgPS      = pPlathoPlant[iPlant]->pPlathoGenotype->pFalgePS;
    PLEAFLAYER          pLLayer;
    PPLATHOMORPHOLOGY   pPMorphRight, pPMorphLower, pPMorphLeft, pPMorphUpper;

    double   vPlantHeight,vRightPlantHeight,vLowerPlantHeight,vLeftPlantHeight,vUpperPlantHeight;
    double   vHCrownOnset,vRightHCrownOnset,vLowerHCrownOnset,vLeftHCrownOnset,vUpperHCrownOnset;

	double vGrossPs, vNettoPs, vDarkResp, vStomCond;  // vActTransp
	static double vAveGrossPs, vAveNettoPs, vAveDarkResp, vAveStomCond;  //vAveActTransp 
    double Wc,Wj, PmS, PmSh, PmSun, TempKelvin;
    double Cint1;

    double Calc_Cint(PFALGEPS pPs, double Wc,double Wj, double CO2_air, double vhumidity);
    double CalcLaminarConductivity(double TempAct, double vWind, PFALGEPS pFlgPS, PPLANT pPlant);

	// Photosynthese nur wenn Blaetter vorhanden
    TREES
    {
        SPRUCE
            goto label_99;
        else
        {
	        if((pPDev->iStagePlatho == 0)||(pPDev->iStagePlatho == 6))
	        {
			    pPlant->pPltCarbon->fGrossPhotosynR = (float)0.0;
			    pPPart->vDailyGrossPhotosynthesis = 0.0;
			    pPPart->vDailyNettoPhotosynthesis = 0.0;
			    pPPart->vDailyDarkResp            = 0.0;
			    //pPPart->vActTranspDay             = 0.0;
			    pPPart->vAveStomCondDay           = 0.0;

                pPPart->vPARabsorbedDay            = 0.0;
                PARCum = 0.0;
                          	
                pLLayer = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;
                for(N=0;N<LEAFLAYERS;N++)
                {
                    pLLayer->vGrossPsDay   = 0.0;
	                pLLayer->vNettoPsDay   = 0.0;
	                pLLayer->vDarkRespDay  = 0.0;
	                pLLayer->vStomCondDay  = 0.0;
	                //pLLayer->vActTranspDay = 0.0; 
                    pLLayer->vPARabsorbedDay = 0.0;

                    pLLayer = pLLayer->pNext;
                }

			    return 1;
	        }
        } //end BEECH, APPLE
    } //end TREES

	HERBS
	{
	    if((pPDev->iStagePlatho == 0)||(pPDev->iStagePlatho >= 5)||(pPlant->pDevelop->iDayAftEmerg == 0))
	    {
			pPlant->pPltCarbon->fGrossPhotosynR = (float)0.0;
			pPPart->vDailyGrossPhotosynthesis = 0.0;
			pPPart->vDailyNettoPhotosynthesis = 0.0;
			pPPart->vDailyDarkResp            = 0.0;
			//pPPart->vActTranspDay             = 0.0;
			pPPart->vAveStomCondDay           = 0.0;

            pPPart->vPARabsorbedDay            = 0.0;
            PARCum = 0.0;
                      	
            pLLayer = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;
            for(N=0;N<LEAFLAYERS;N++)
            {
                pLLayer->vGrossPsDay   = 0.0;
	            pLLayer->vNettoPsDay   = 0.0;
	            pLLayer->vDarkRespDay  = 0.0;
	            pLLayer->vStomCondDay  = 0.0;
	            //pLLayer->vActTranspDay = 0.0; 
                pLLayer->vPARabsorbedDay = 0.0;

                pLLayer = pLLayer->pNext;
            }

			return 1;
        }
    }  //end HERBS
        

label_99:

    if(pPlathoModules->iFlagPhotosynthesis == 2) // "Farquhar"
    {
        // Einlesen der Falge-Parameter:
        //ReadFalgeParameters(iPlant, pPlant);
        SPRUCE
        {
            // calc. the frost hardiness value K 
		    pFlgPS->K = calc_seasonal_reduction((pCl->pWeather->fTempAve>(float)-99)?
		        pCl->pWeather->fTempAve : (pCl->pWeather->fTempMin+pCl->pWeather->fTempMax)/(float)2.0);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////
    // Tageslänge, Beginn, Ende und Länge der photosynthetisch aktiven Tageszeit   //
    /////////////////////////////////////////////////////////////////////////////////

    if(pPlathoModules->iFlagLightInterception == 2)  //constant photoperiod
        vDaylengthPs = pPlathoScenario->vPhotoperiod;
    else
	    vDaylengthPs = Daylength(pLo->pFarm->fLatitude, pTi->pSimTime->iJulianDay,3);

	TimeBeginPs = (12.0 - 0.5*vDaylengthPs)/24.0;
	TimeEndPs = 1.0 - TimeBeginPs;

	//Bestimmung des relevanten Zeitintervalls in Stunden
	if(pTi->pTimeStep->fAct < (float) 1.0 )
	{
		//Nacht:
		if((TimeDay + DeltaT < TimeBeginPs)||(TimeDay > TimeEndPs))
		{
			pPlant->pPltCarbon->fGrossPhotosynR = (float)0.0;
            pPPart->vNettoPhotosynR             = 0.0;
            pPPart->vDarkRespR                  = 0.0;
            //pPPart->vActTranspR                 = 0.0;
            //pPPart->vAveStomCond                = 0.0;
            pPPart->vAveStomCond                = pFlgPS->StomCondMin;

            pLLayer = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;
            for(N=0;N<LEAFLAYERS;N++)
            {
                pLLayer->vGrossPsR   = 0.0;
                pLLayer->vNettoPsR   = 0.0;
	            pLLayer->vDarkRespR  = 0.0;
	            //pLLayer->vActTranspR = 0.0;
	            pLLayer->vCint       = 0.0;
	            //pLLayer->vStomCond   = 0.0; 
	            pLLayer->vStomCond   = pFlgPS->StomCondMin; 
 
                pLLayer = pLLayer->pNext;
           }

            return 1;
		}

		//Morgendämmerung:
		if((TimeDay < TimeBeginPs) && (TimeDay + DeltaT > TimeBeginPs))
		{
			vHour = (TimeBeginPs + 0.5*(TimeDay + DeltaT - TimeBeginPs))*24.0;
			hPs = (TimeDay + DeltaT - TimeBeginPs)*24.0;

			pPPart->vDailyGrossPhotosynthesis = 0.0;
			pPPart->vDailyNettoPhotosynthesis = 0.0;
			pPPart->vDailyDarkResp            = 0.0;
			//pPPart->vActTranspDay             = 0.0;
			pPPart->vAveStomCondDay           = 0.0;

            pPPart->vPARabsorbedDay            = 0.0;

            PARCum = 0.0;
              	        
            pLLayer = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;
            for(N=0;N<LEAFLAYERS;N++)
            {
                pLLayer->vGrossPsDay   = 0.0;
	            pLLayer->vNettoPsDay   = 0.0;
	            pLLayer->vDarkRespDay  = 0.0;
	            pLLayer->vStomCondDay  = 0.0;
	            //pLLayer->vActTranspDay = 0.0; 
                pLLayer->vPARabsorbedDay = 0.0;

                pLLayer = pLLayer->pNext;
            }
		}

		//Tag:
		if((TimeDay >= TimeBeginPs) && (TimeDay + DeltaT <= TimeEndPs))
		{
			vHour = (TimeDay + 0.5 * DeltaT)*24.0;
			hPs   = DeltaT*24.0;
		}

		//Abenddämmerung:
		if((TimeDay < TimeEndPs) && (TimeDay + DeltaT > TimeEndPs))
		{
		    vHour = (TimeDay + 0.5*(TimeEndPs - TimeDay))*24.0;
			hPs = (TimeEndPs - TimeDay)*24.0;
		}
	}
	else // daily time step
	{		
		//At the specified vHour, radiation is computed and used to compute assimilation
		vHour = 12.0+vDaylengthPs*0.5*xGauss[it-1];

		if(it==1) //Tageswert auf Null
		{
			vAveGrossPs = 0.0;
		    vAveNettoPs = 0.0;
		    vAveDarkResp = 0.0;
		    //vAveActTransp = 0.0;
		    vAveStomCond = 0.0;

			pPPart->vDailyGrossPhotosynthesis = 0.0;
			pPPart->vDailyNettoPhotosynthesis = 0.0;
			pPPart->vDailyDarkResp            = 0.0;
			//pPPart->vActTranspDay             = 0.0;
			pPPart->vAveStomCondDay           = 0.0;

            pPPart->vPARabsorbedDay            = 0.0;

            pLLayer = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;
            for(N=0;N<LEAFLAYERS;N++)
            {
                pLLayer->vGrossPsDay   = 0.0;
	            pLLayer->vNettoPsDay   = 0.0;
	            pLLayer->vDarkRespDay  = 0.0;
	            pLLayer->vStomCondDay  = 0.0;
	            //pLLayer->vActTranspDay = 0.0; 
                pLLayer->vPARabsorbedDay = 0.0;

                pLLayer = pLLayer->pNext;
            }
		}

		hPs = vDaylengthPs;
	}


	//Aktuelle Temperatur:
	if(pPlathoModules->iFlagLightInterception == 2)  //constant photoperiod
        TempAct = (double)pCl->pWeather->fTempMax;
    else
    {
	    if(pTi->pTimeStep->fAct == (float)1.0)
		    TempAct = (pCl->pWeather->fTempAve>(float)-99)?
		    (double)pCl->pWeather->fTempAve : (double)(pCl->pWeather->fTempMin+pCl->pWeather->fTempMax)/2.0;
	    else
	    {
		    if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))
			    TempAct=pPlathoClimate->vTemp;
		    else
		    {
	 		    if((int)pTi->pSimTime->fTimeAct==0)
				    TempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pCl->pWeather->fTempMax,
							    pCl->pWeather->fTempMin, pCl->pWeather->pNext->fTempMin, vDaylengthPs,1);

			    else if(pCl->pWeather->pNext==NULL)
				    TempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pCl->pWeather->fTempMax,
							    pCl->pWeather->fTempMin, pCl->pWeather->fTempMin, vDaylengthPs,0);

			    else
				    TempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pCl->pWeather->fTempMax,
							    pCl->pWeather->fTempMin, pCl->pWeather->pNext->fTempMin, vDaylengthPs,0);
		    }
	    }
    }
	
	///////////////////////////////////////////////
	// Bestimmung der Nachbarn der i-ten Pflanze. 

	GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
												&iPlantUpper, &iPlantLower);

	pPlantLeft	= 	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft;
	pPlantRight	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight;
	pPlantUpper	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper;
	pPlantLower	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower ;

    vPlantHeight       = pPlant->pCanopy->fPlantHeight;
    vRightPlantHeight  = pPlantRight->pCanopy->fPlantHeight;
	vLowerPlantHeight  = pPlantLower->pCanopy->fPlantHeight;
	vLeftPlantHeight   = pPlantLeft->pCanopy->fPlantHeight;
	vUpperPlantHeight  = pPlantUpper->pCanopy->fPlantHeight;

    pPMorphRight =  pPlathoPlant[iPlantRight]->pPlathoMorphology;
    pPMorphLower =  pPlathoPlant[iPlantLower]->pPlathoMorphology;
    pPMorphLeft  =  pPlathoPlant[iPlantLeft]->pPlathoMorphology;
    pPMorphUpper =  pPlathoPlant[iPlantUpper]->pPlathoMorphology;

    vHCrownOnset        = pPMorph->vHeightOfCrownOnset;
    vRightHCrownOnset   = pPMorphRight->vHeightOfCrownOnset;
    vLowerHCrownOnset   = pPMorphLower->vHeightOfCrownOnset;
    vLeftHCrownOnset    = pPMorphLeft->vHeightOfCrownOnset;
    vUpperHCrownOnset   = pPMorphUpper->vHeightOfCrownOnset;

	//////////////////////////////////////////////////////////////////////////////////////////
	//	Berechnung der Photosynthetisch Aktiven Strahlung (PAR) sowie des direkten (PARDIR) //
	//	und des diffusen (PARDIF) Anteils													//
	//////////////////////////////////////////////////////////////////////////////////////////

    if(pPlathoModules->iFlagLightInterception == 2) //constant light
        goto Label_PS2;

	//Declination of the sun as function of Daynumber (vDay)
	vDec = -asin( sin(23.45*RAD)*cos(2.0*PI*((double)pTi->pSimTime->iJulianDay+10.0)/365.0));
	
	//vSin, vCos and vRsc are intermediate variables
	vSin = sin(RAD*pLo->pFarm->fLatitude)*sin(vDec);
	vCos = cos(RAD*pLo->pFarm->fLatitude)*cos(vDec);
	vRsc = vSin/vCos;

 
	//Sine of solar height(vSin), integral of vDailySin(vDailySin) and integral of vDailySin 
	//with correction for lower atmospheric transmission at low solar elevations (vDailySinE)
	vDailySin  = 3600.0*(vDaylengthPs*vSin+24.0*vCos*sqrt(1.0-vRsc*vRsc)/PI); // [s]
	vDailySinE = 3600.0*(vDaylengthPs*(vSin+0.4*(vSin*vSin+vCos*vCos*0.5))
				+12.0*vCos*(2.0+3.0*0.4*vSin)*sqrt(1.0-vRsc*vRsc)/PI);  // [s]

	//Solar constant(vSolarConst) and daily extraterrestrial (vRadExt) 
	vSolarConst  = 1370.0*(1.0+0.033*cos(2.0*PI*(double)pTi->pSimTime->iJulianDay/365.0));	//[J/m^2*d]
    
    
	// ab hier Abhängig von der Uhrzeit:

	//Sine of solar elevation
	vSinHeight  = max(0.0, vSin + vCos*cos(2.0*PI*(vHour-12.0)/24.0));

	//	Achtung: Sobald der Mitttelpunkt der Sonne unter dem Horizont ist
	//	findet keine Photosynthese mehr statt (wg. sin(h)<0)!
	//	Dies ist nicht richtig, weil in der Dämmerung durchaus noch genügend
	//	diffuse Strahlung vorhanden sein kann.

	if(vSinHeight==0.0)
	{
		pPlant->pPltCarbon->fGrossPhotosynR = (float)0.0;
		return 1;
	}

Label_PS2:
    
    /////////////////////////////////////////////////////////////////////////////////////
 	// bei hochaufgelösten Klimadaten (aus *.hrc) werden Strahlungsdaten in µmol/m2/s  //
	// eingelesen (->fPAR)                                                             //
	// Umrechnung von µmol/m2/s in W/m^2:                                              //
	// 1(µmol/m2/s)*10^-6(mol/µmol)*A(Quanten/mol)*h(Js)*c(m/s)/lambda(m)              //
	// 10^-6*6.022*10^23*6.626*10^-34*2.998*10^8/(6.5*10^-7)                           //
	/////////////////////////////////////////////////////////////////////////////////////

	if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))	
    {
		PAR = pPlathoClimate->vPAR; // µmol/m2/s 
		if(pPlathoModules->iFlagPhotosynthesis == 1)  // "Goudriaan"   --> W/m2
            PAR *= 0.184; // W/m2
    }
	else
	{
		if(pPlathoModules->iFlagLightInterception == 2)  // constant light
            PAR =  pCl->pWeather->fSolRad*1E6/(vDaylengthPs*3600.0); // W/m^2   ->fSolRad = PPFD
            //PAR =  0.5*pCl->pWeather->fSolRad*1E6/(vDaylengthPs*3600.0); // W/m^2
        else        
            PAR = 0.5*pCl->pWeather->fSolRad*1E6*vSinHeight*(1.0+0.4*vSinHeight)/vDailySinE; // W/m^2	

//		PAR = pCl->pWeather->fSolRad*1E6*vSinHeight*(1.0+0.4*vSinHeight)/vDailySinE; // W/m^2	
		if(pPlathoModules->iFlagPhotosynthesis == 2)  // "Farquhar" --> µmol/m2/s 
            PAR /= 0.184; //µmol/m2/s 
	}


    //Test:
    if(pTi->pTimeStep->fAct < (float) 1.0 )
		PARCum += PAR*0.184*hPs*3600/1e6;
	else
		PARCum += PAR*0.184*wGauss[it-1]*vDaylengthPs*3600/1e6;


	
   	//Diffuse PAR (PARDIF) and direct PAR (PARDIR)
    if(pPlathoModules->iFlagLightInterception == 1) //Freiland
    {
        //vAtmTrans = Globalstrahlung/Extraterrestrische Einstrahlung
	    vAtmTrans = pCl->pWeather->fSolRad*1E6/(vSolarConst*vDailySin);

	    if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))	
		    vAtmTrans = PAR/(0.5*vSolarConst*vSinHeight);
	    else
		    vAtmTrans = pCl->pWeather->fSolRad*1E6/(vSolarConst*vDailySin);


	    if (vAtmTrans>0.75) 
		    vDifFr = 0.23;
	    else if ((vAtmTrans<=0.75)&&(vAtmTrans>0.35)) 
		    vDifFr = 1.33-1.46*vAtmTrans;
	    else if ((vAtmTrans<=0.35)&&(vAtmTrans>0.07)) 
		    vDifFr = 1.0-2.3*(vAtmTrans-0.07)*(vAtmTrans-0.07); 
	    else
		    vDifFr = 1.0;

	    //Diffuse PAR (PARDIF) and direct PAR (PARDIR)
	    PARDIF = min(PAR, vSinHeight*vDifFr*vAtmTrans*0.5*vSolarConst);
    }

	
   if(pPlathoModules->iFlagLightInterception >=2)
    {
	    PARDIF = 1.0*PAR;
	    //PARDIF = 0.8*PAR;
	    //PARDIF = (float)0.5*PAR;
    }

	PARDIR = PAR-PARDIF;

	/////////////////////////////////////////////////////////////////
	//
	// Berechnung der Extinktionscoeffizienten für diffuses und direktes Licht
	
	SCV	=	0.20; 	//Scattering coefficient of leaves for visible radiation (PAR)
	//k	= 	0.50;	//The average extinction coefficient for visible and near infrared radiation


	//===========================================================================================
	//Reflection of horizontal and spherical leaf angle distribution
	SQV  = sqrt(1.0-SCV);
	REFH = (1.0-SQV)/(1.0+SQV);
	REFS = REFH*2.0/3.0;

	//Extinction coefficient for direct radiation and total direct flux
	CLUSTF[iPlant] = pPGen->vKDiffuse / (0.8*SQV); 
	KDIRBL[iPlant] = 0.5 * CLUSTF[iPlant];
	KDIRT[iPlant]  = KDIRBL[iPlant] * SQV;

	CLUSTF[iPlantUpper] = pPlathoPlant[iPlantUpper]->pPlathoGenotype->vKDiffuse / (0.8*SQV); 
	KDIRBL[iPlantUpper] = 0.5 * CLUSTF[iPlantUpper];
	KDIRT[iPlantUpper]  = KDIRBL[iPlantUpper] * SQV;

	CLUSTF[iPlantLeft] = pPlathoPlant[iPlantLeft]->pPlathoGenotype->vKDiffuse / (0.8*SQV); 
	KDIRBL[iPlantLeft] = 0.5 * CLUSTF[iPlantLeft];
	KDIRT[iPlantLeft]  = KDIRBL[iPlantLeft] * SQV;

	CLUSTF[iPlantRight] = pPlathoPlant[iPlantRight]->pPlathoGenotype->vKDiffuse / (0.8*SQV); 
	KDIRBL[iPlantRight] = 0.5 * CLUSTF[iPlantRight];
	KDIRT[iPlantRight]  = KDIRBL[iPlantRight] * SQV;

	CLUSTF[iPlantLower] = pPlathoPlant[iPlantLower]->pPlathoGenotype->vKDiffuse / (0.8*SQV); 
	KDIRBL[iPlantLower] = 0.5 * CLUSTF[iPlantLower];
	KDIRT[iPlantLower]  = KDIRBL[iPlantLower] * SQV;

	

	////////////////////////////////////////////////////////////////////////////////////////
	//	Berechnung der maximalen Assimilationsrate bei Lichtsättigung in Abhängigkeit von 
	//	den Umweltfaktoren 
	//	Amax [kg(CO2)/ha(Blattfläche)/h]
	////////////////////////////////////////////////////////////////////////////////////////

	//	Reduktionsfaktoren:
	
	//---------------------------------------------------------------------------//
	// Wasserstress:

 /*  if(pPlathoModules->iFlagH2OEffectPhot==1)//no water deficit effect on photosynthesis
    {
	    //pPlant->pPltWater->fStressFacPhoto = (float)1.0; 
	    pPStress->dWaterShortage = (float)1.0; 
    }
    else //water deficit effect on photosynthesis included
    {
	    // Am Tag des Feldaufgangs kein Wasserstress                           
//	    if((pPlant->pDevelop->iDayAftEmerg == 1)||(pPlant->pPltWater->fPotTranspdt<=(float)0.0))
//		    pPlant->pPltWater->fStressFacPhoto = (float)1.0;
//	    else
//		    pPlant->pPltWater->fStressFacPhoto = pPlant->pPltWater->fActTranspdt/
//											    pPlant->pPltWater->fPotTranspdt;
                                                
	    if((pPlant->pDevelop->iDayAftEmerg == 1)||(pPPart->vPotTranspR<=(float)0.0))
		    pPStress->dWaterShortage = (float)1.0;
	    else
		    pPStress->dWaterShortage = min(1.0,pPPart->vActTranspR/pPPart->vPotTranspR);
		    //pPStress->dWaterShortage = min(1.0,(double)pPlant->pPltWater->fActTranspdt/
			//								    (double)pPlant->pPltWater->fPotTranspdt);
   } */        
	//---------------------------------------------------------------------------//


	//---------------------------------------------------------------------------//
	// Assimilate accumulation:
    if((pPlathoModules->iFlagCH2OEffectPhot > 1) //no effect of assimilate accumulation included
                &&(pTi->pTimeStep->fAct < (float) 1.0 ))
    	PhiC = PhiAssimilatstau(pPlant);
    else
        PhiC = (float)1.0;
	//---------------------------------------------------------------------------//


	//---------------------------------------------------------------------------//
	// Temperaturabhängigkeit:

		if(pPlant->pGenotype->PmaxTemp[0].fInput == (float)0.0)
			PhiTemp = RelativeTemperatureResponse_PLATHO(TempAct,pPlant->pGenotype->fTempMinPs,
								pPlant->pGenotype->fTempOptPs,pPlant->pGenotype->fTempMaxPs);
		else
			PhiTemp = AFGENERATOR((float)TempAct,pPlant->pGenotype->PmaxTemp);
			
	// test:
    // PhiTemp = (float)1.0;
	//---------------------------------------------------------------------------//


	//---------------------------------------------------------------------------//
	// Stickstoffabhängigkeit:
    if(pPlathoModules->iFlagNEffectPhot==2)//N effect on photosynthesis included
    {
       if(pPlathoModules->iFlagNitrogenDistribution==1)//N homogen verteilt auf Blattschichten:
	    {
        //    if(pPlathoModules->iFlagO3 <= 3) //
    	        pPltN->fNStressPhoto =(float)max(0.0,min(1.0,pow((pPltN->fLeafActConc-pPltN->fLeafMinConc)
                                                    /(pPltN->fLeafOptConc*NFAC_L-pPltN->fLeafMinConc),0.5)));
	        
           /*   if(pPlathoModules->iFlagO3 == 4) //following van Oijen
            {
                float fNConcCrit;
                
                // carboxylation velocity is direct proportional to Rubisco content (Farquhar 1980)
                // --> 
                    
                fNConcCrit = (float)0.015; // 0.02

                BEECH
                    fNConcCrit = (float)0.015; // 0.02
                SPRUCE
                    fNConcCrit = (float)0.008; // 0.02
                
                if(pPltN->fLeafActConc > fNConcCrit)
                    pPltN->fNStressPhoto =(float)1.0;
                else
                    pPltN->fNStressPhoto =(float)max(0.0,min(1.0,pPltN->fLeafActConc/fNConcCrit));
            }
         */

	        //Am Tag des Feldaufgangs kein Stickstoffstress
	        if((pPlant->pDevelop->iDayAftEmerg == 1)||(pBiom->fLeafWeight==(float)0.0))
		        pPltN->fNStressPhoto =(float)1.0;

            PhiN = pPltN->fNStressPhoto;
            // PhiN = (pPltN->fNStressPhoto+(float)1.0)/(float)2.0;
            // PhiN = (float)max(0.0,min(1.0,(pPltN->fLeafActConc-pPltN->fLeafMinConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc)));
        }
    }
    else //no N effect on photosynthesis
    {
         PhiN = 1.0;
    }

	//---------------------------------------------------------------------------//

	//---------------------------------------------------------------------------//
	// O3-effect:
    if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3)) //ozone effect on photosynthesis included
       PhiO3 = PhiOzone(iPlant); 
    else if(pPlathoModules->iFlagO3==1)//no ozone effect on photosynthesis
       PhiO3 = 1.0; 
    else    //van Oijen - O3-effect is calculated later on
       PhiO3 = 1.0; 
		 
	//---------------------------------------------------------------------------//
	
	//---------------------------------------------------------------------------//
	//  effect of leaf pathogenes on photosynthesis:
    if(pPlathoModules->iFlagLeafPathEffPhot==1)//no effect of leaf pathogenes on photosynthesis
        PhiLfPath = 1.0; 
    else
		PhiLfPath = PhiLeafPathogenes(iPlant); 
	//------------------------------------------------------------------------//

	dCO2 = (!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))?
				pPlathoClimate->vCO2:pPlathoScenario->vCO2Day;

    if(pPlathoModules->iFlagPhotosynthesis == 1) //Goudriaan
    {
   	    // CO2-Abhängigkeit:

	    //für C3-Pflanzen
	    dCO2PhotoCmp0=pPlant->pGenotype->fCO2CmpPoint; //vppm
    		
	    //Efect of Temperature
	    dCO2Cmp = dCO2PhotoCmp0*pow(2.0,(TempAct-20.0)/10.0);

	    ////////////////////////////////////////////////////////////////////////////////////
	    //	Berechnung der Initial Light Use Efficiency 
	    //	fLightUseEff [(g(CO2)/m2/h)/(J/m^2/s)]
	    ////////////////////////////////////////////////////////////////////////////////////

	    dEpsLUE	= pPlant->pGenotype->fLightUseEff*(dCO2-dCO2Cmp)/(dCO2+2*dCO2Cmp);

    	
	    //Berechnung des Effekts auf die maximale Photosyntheserate:

	    dCO2R   = pPlant->pGenotype->fCiCaRatio;

	    dCO2I 	= dCO2*dCO2R;
	    dCO2I340= dCO2R*340.0;


	    PhiCO2 = min(2.3,(dCO2I-dCO2Cmp)/(dCO2I340-dCO2Cmp)); //For C3 crops
    }
 

	/////////////////////////////////////////////////////////////////
	//	Berechnung der Photosyntheseleistung der gesamten Pflanze
	/////////////////////////////////////////////////////////////////


    //Berechnung der auf alle konkurrierenden Pflanzen bezogenen Extinktionskoeffizienten
    pLLayer = pPMorph->pLeafLayer;
	for (N=0;N<LEAFLAYERS;N++)	
	{

/*	    vExtDiffuse  = 0.0;
	    vExtTotalDir = 0.0;
	    vExtDirect   = 0.0;
*/

	    if(pPlant->pCanopy->fPlantHeight>(float)0.0)
	    {
		    dLAICum = pLLayer->vLAICum;

		    //Berechnung des kumulativen LAI über der Blattschicht gleicher Höhe des jeweiligen Nachbarn	
            dLAICumRight = CalcLAICumNbg(N, vPlantHeight, vRightPlantHeight, vHCrownOnset, vRightHCrownOnset, pPMorphRight->pLeafLayer);
            dLAICumLower = CalcLAICumNbg(N, vPlantHeight, vLowerPlantHeight, vHCrownOnset, vLowerHCrownOnset, pPMorphLower->pLeafLayer);
            dLAICumLeft  = CalcLAICumNbg(N, vPlantHeight, vLeftPlantHeight,  vHCrownOnset, vLeftHCrownOnset, pPMorphLeft->pLeafLayer);
            dLAICumUpper = CalcLAICumNbg(N, vPlantHeight, vUpperPlantHeight, vHCrownOnset, vUpperHCrownOnset, pPMorphUpper->pLeafLayer);

            pLLayer->vLeafAreaNbg[0] = CalcLA_Nbg(N, vPlantHeight, pPlantRight->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphRight->vHeightOfCrownOnset, pPMorphRight->pLeafLayer);
            pLLayer->vLeafAreaNbg[1] = CalcLA_Nbg(N, vPlantHeight, pPlantLower->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphLower->vHeightOfCrownOnset, pPMorphLower->pLeafLayer);
            pLLayer->vLeafAreaNbg[2] = CalcLA_Nbg(N, vPlantHeight, pPlantLeft->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphLeft->vHeightOfCrownOnset, pPMorphLeft->pLeafLayer);
            pLLayer->vLeafAreaNbg[3] = CalcLA_Nbg(N, vPlantHeight, pPlantUpper->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphUpper->vHeightOfCrownOnset, pPMorphUpper->pLeafLayer);
        

	        //Aussummieren der Extinktionskoeffizienten:
	        vExtDiffuse[N]  =  dLAICum * pPGen->vKDiffuse
		        + dLAICumUpper * pPlathoPlant[iPlantUpper]->pPlathoGenotype->vKDiffuse * pPMorph->AbvgCoeffComp[3]
		        + dLAICumLeft  * pPlathoPlant[iPlantLeft]->pPlathoGenotype->vKDiffuse  * pPMorph->AbvgCoeffComp[2]
		        + dLAICumRight * pPlathoPlant[iPlantRight]->pPlathoGenotype->vKDiffuse * pPMorph->AbvgCoeffComp[0]
		        + dLAICumLower * pPlathoPlant[iPlantLower]->pPlathoGenotype->vKDiffuse * pPMorph->AbvgCoeffComp[1];

	        vExtTotalDir[N] = dLAICum * KDIRT[iPlant]
		        + dLAICumUpper * KDIRT[iPlantUpper] * pPMorph->AbvgCoeffComp[3]
		        + dLAICumLeft  * KDIRT[iPlantLeft]  * pPMorph->AbvgCoeffComp[2]
		        + dLAICumRight * KDIRT[iPlantRight] * pPMorph->AbvgCoeffComp[0]
		        + dLAICumLower * KDIRT[iPlantLower] * pPMorph->AbvgCoeffComp[1];

	        vExtDirect[N]   = dLAICum * KDIRBL[iPlant]
			        + dLAICumUpper * KDIRBL[iPlantUpper] * pPMorph->AbvgCoeffComp[3]
			        + dLAICumLeft  * KDIRBL[iPlantLeft]  * pPMorph->AbvgCoeffComp[2]
			        + dLAICumRight * KDIRBL[iPlantRight] * pPMorph->AbvgCoeffComp[0]
			        + dLAICumLower * KDIRBL[iPlantLower] * pPMorph->AbvgCoeffComp[1];

       	    //Fraction sunlit leaf area (FSLLA)
            FSLLA[N] = CLUSTF[iPlant] * exp(-vExtDirect[N]);

            //Fraction of central plant leaves:
            if(pLLayer->vLeafAreaLay > 0.0)
                pLLayer->vFracCPL = pLLayer->vLeafAreaLay/(pLLayer->vLeafAreaLay
                    + pLLayer->vLeafAreaNbg[0]*pPMorphRight->AbvgCoeffComp[2]
                    + pLLayer->vLeafAreaNbg[1]*pPMorphLower->AbvgCoeffComp[3]
                    + pLLayer->vLeafAreaNbg[2]*pPMorphLeft->AbvgCoeffComp[0]
                    + pLLayer->vLeafAreaNbg[3]*pPMorphUpper->AbvgCoeffComp[1]);
            else
                pLLayer->vFracCPL = 0.0;

	    } //Ende if(fPlantHeight>0)
	    pLLayer = pLLayer->pNext;
    }
	//=======================================================================
	//Canopy assimilation is set to zero
    vGrossPs   = 0.0;
    vNettoPs   = 0.0;
    vDarkResp  = 0.0;
    //vActTransp  = 0.0;
    vStomCond  = 0.0;

    pPPart->vPARabsorbedCan = 0.0;

	//Integration über die Pflanzenhöhe
    pLLayer = pPMorph->pLeafLayer;
	for (N=0;N<LEAFLAYERS;N++)	
	{

        if(pPlathoModules->iFlagNEffectPhot==2)//N effect on photosynthesis included
        {
	        ///////////////////////////////////////////////////////////////
	        //	Berücksichtigung der N-Verteilung in den Blättern:       //
	        ///////////////////////////////////////////////////////////////
            if(pPlathoModules->iFlagNitrogenDistribution==2)//N-Verteilung auf Blattschichten:
	        {
		        // fLeafNConc = pLLayer->vLeafNConc;
		        // pPltN->fNStressPhoto =(float)max(0.0,min(1.0,pow((fLeafNConc-pPltN->fLeafMinConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc),0.5)));

		        //Zusammenhang ähnlich wie Michaelis-Menten:
		        //PhiN = fN^aN*(k+1)/(fN^aN+k)
        		
		        alphaN = 0.5;
        			
		        vK = 0.5;
		        vNFactor =max(0.0,min(1.0,pow((pLLayer->vLeafNConc-pPltN->fLeafMinConc)/(pPltN->fLeafOptConc*NFAC_L-pPltN->fLeafMinConc),alphaN)));

		        PhiN =vNFactor*(vK+1.0)/(vNFactor+vK);

		        //Am Tag des Feldaufgangs kein Stickstoffstress
		        if((pPlant->pDevelop->iDayAftEmerg == 1)||(pBiom->fLeafWeight==(float)0.0))
			        PhiN =1.0;
            }
        }// end 'N effect on photosynthesis included'
        else //no N effect on photosynthesis 
            PhiN = 1.0;
	    /////////////////////////////////////////////////////////////////////////////////////////////




			
		//Absorbed fluxes per unit leaf area: diffuse flux, total direct
		//flux, direct component of direct flux [µmol/m^2/s]

		VISDF = (1.0-REFH)*PARDIF*pPGen->vKDiffuse  *exp (-vExtDiffuse[N]);
		VIST  = (1.0-REFS)*PARDIR*KDIRT[iPlant] *exp (-vExtTotalDir[N]);
		VISD  = (1.0-SCV) *PARDIR*KDIRBL[iPlant]*exp (-vExtDirect[N]);

    if(pPlathoModules->iFlagPhotosynthesis == 1) //"Goudriaan"
    {
	    //Unterscheidung zwischen Schatten und Sonnenblättern:		
	    if(dLAICum <=2.0)
	    {
		    if (pPGen->MaxGrossPsSu[0].vInput==0.0)
			    AmaxOpt = pPlant->pGenotype->fMaxGrossPs;
		    else
			    AmaxOpt = AVGENERATOR(pPlant->pDevelop->fDevStage,pPGen->MaxGrossPsSu);
	    }
	    else
	    {
		    if (pPGen->MaxGrossPsSh[0].vInput==0.0)
			    AmaxOpt = pPlant->pGenotype->fMaxGrossPs;
		    else
			    AmaxOpt = AVGENERATOR(pPlant->pDevelop->fDevStage,pPGen->MaxGrossPsSh);
	    }



	    Amax = AmaxOpt*
			    PhiCO2*		//CO2-Abhängigkeit
			    min(pPStress->dWaterShortage,//pPlant->pPltWater->fStressFacPhoto, //Wasserstress
			    min(PhiTemp,//Temperaturabhängigkeit
			    min(PhiN,   //Stickstoffabhängigkeit
			    PhiC)))		//Assimilatstau
			    *  PhiO3	//Schädigung durch Ozon
			    * PhiLfPath;//Schäden durch Pathogene

        //Ozone-stress on carboxylation rate:
        if(pPlathoModules->iFlagO3 == 4) //following van Oijen
        {
            Amax *= min(1.0,pPStress->dRubiscoConc/(1.0*pPGen->dRubiscoConcCrit));
        }

    	
	    //Absorbed flux (J/m2(leaf)/s) for shaded leaves and assimilation of shaded leaves
		    VISSHD = max(0.0,VISDF + VIST - VISD);

		    if (Amax>0.0) 
		    FGRSH = Amax * (1.0-exp(-VISSHD*dEpsLUE/Amax));
		    else
		    FGRSH = 0.0;
    		
	    //Direct flux absorbed by leaves perpendicular on direct beam and
	    //assimilation of sunlit leaf area
            VISPP  = (1.0-SCV) * PARDIR;
            //22.02.2010 ?
			//VISPP  = (1.0-SCV) * PARDIR / vSinHeight;

            FGRSUN = 0.0;
            for (j=0;j<nGauss;j++)
            {
            VISSUN = VISSHD + VISPP * xGauss[j];
            
            if (Amax>0.0) 
                FGRS = Amax * (1.0-exp(-VISSUN*dEpsLUE/Amax));
            else
                FGRS = 0.0;
                
            FGRSUN = FGRSUN + FGRS * wGauss[j];
            }

	        // Local assimilation rate (FGL)
            FGL   = FSLLA[N]  * FGRSUN + (1.0-FSLLA[N]) * FGRSH;

        //Intercepted radiation:
        if(N+1<LEAFLAYERS)
            pLLayer->vPARabsorbed = (VISSHD + (FSLLA[N+1]-FSLLA[N])*VISPP)*pLLayer->vLeafAreaLay; // [J/s]
        else
            pLLayer->vPARabsorbed = (VISSHD + (CLUSTF[iPlant]-FSLLA[N])*VISPP)*pLLayer->vLeafAreaLay; // [J/s]

            pPPart->vPARabsorbedCan += pLLayer->vPARabsorbed*hPs*3600.0; // [J]
            pPPart->vPARabsorbedDay += pLLayer->vPARabsorbed*hPs*3600.0; // [J]
            pPPart->vPARabsorbedCum += pLLayer->vPARabsorbed*hPs*3600.0; // [J]

	    //	Integration of local assimilation rate to canopy assimilation
            vGrossPs += FGL * pLLayer->vLeafAreaLay;
    }   //end "Goudriaan" 
        
        
    if(pPlathoModules->iFlagPhotosynthesis == 2) //"Farquhar"
    {
		//Unterscheidung zwischen Schatten und Sonnenblättern:		
		if(dLAICum <=2.0)
		{
			if (pPGen->avVcmaxSu[0].vInput==0.0)
				VcmaxOpt = pPGen->vVcmaxSu;
			else
				VcmaxOpt = AVGENERATOR(pPlant->pDevelop->fDevStage,pPGen->avVcmaxSu);

			if (pPGen->avJmaxSu[0].vInput==0.0)
				JmaxOpt = pPGen->vJmaxSu;
			else
				JmaxOpt = AVGENERATOR(pPlant->pDevelop->fDevStage,pPGen->avJmaxSu);

			Alpha = pPGen->vFCalphaSu;
		}
		else
		{
			if (pPGen->avVcmaxSh[0].vInput==0.0)
				VcmaxOpt = pPGen->vVcmaxSh;
			else
				VcmaxOpt = AVGENERATOR(pPlant->pDevelop->fDevStage,pPGen->avVcmaxSh);

			if (pPGen->avJmaxSh[0].vInput==0.0)
				JmaxOpt = pPGen->vJmaxSh;
			else
				JmaxOpt = AVGENERATOR(pPlant->pDevelop->fDevStage,pPGen->avJmaxSh);

			Alpha = pPGen->vFCalphaSh;
		}

        //Ozone-stress on carboxylation rate:
        if(pPlathoModules->iFlagO3 == 4) //following van Oijen
        {
            BEECH
            {
                VcmaxOpt *= min(1.0,pPStress->dRubiscoConc/(1.0*pPGen->dRubiscoConcCrit));
                JmaxOpt *= min(1.0,pPStress->dRubiscoConc/(1.0*pPGen->dRubiscoConcCrit));
            }
            SPRUCE
            {
                VcmaxOpt *= min(1.0,pPStress->dRubiscoConc/(0.7*pPGen->dRubiscoConcCrit));
                JmaxOpt *= min(1.0,pPStress->dRubiscoConc/(0.7*pPGen->dRubiscoConcCrit));
            }
            else
            {
                VcmaxOpt *= min(1.0,pPStress->dRubiscoConc/(0.9*pPGen->dRubiscoConcCrit));
                JmaxOpt *= min(1.0,pPStress->dRubiscoConc/(0.9*pPGen->dRubiscoConcCrit));
            }
        }
        if((pPlathoModules->iFlagO3 == 2)||(pPlathoModules->iFlagO3 == 3)) //Platho
        {
            VcmaxOpt *= PhiO3;
        }
    
		//Temperature dependence 

        // recalc. GFac with soil water stress:
	    //pFlgPS->gfac = pPlant->pPltWater->fStressFacPhoto*pFlgPS->gfacbase;
	    pFlgPS->gfac = pFlgPS->gfacbase;
//	    pFlgPS->gfac = pPStress->dWaterShortageDay/(pTi->pSimTime->fTimeDay+DeltaT)*pFlgPS->gfacbase;
	    //pFlgPS->gfac = pFlgPS->gfacbase;    //15 10 2008: water shortage effect decreases directly NetPhotoMol

	    // Berechnung der unterschiedlichen Wasserdampfpartialdruecke
        pFlgPS->ewleaf  = 6.11  * exp(17.26938818 * TempAct / (237.3 + TempAct));
	    pFlgPS->ewair 	= pCl->pWeather->fHumidity/100.0 * pFlgPS->ewleaf;
	    pFlgPS->vpd   	= pFlgPS->ewleaf - pFlgPS->ewair;

	    // Berechungen im Zusammenhang mit der Temperatur
	    TempKelvin      = TempAct + 273.16;
	    pFlgPS->rtk     = TempKelvin * IDGASKONST;

        if(pPlathoModules->iFlagCi == 3)    //Falge
        {
            pFlgPS->laminarCond = CalcLaminarConductivity(TempAct, pCl->pWeather->fWindSpeed, pFlgPS, pPlant);
        } 

	    // Berechnung der Modellgleichungen, Temp abhaengigkeit
	    pFlgPS->Ko      = pFlgPS->ko298*exp(pFlgPS->DeltaHaKo*(1.0/(IDGASKONST*298.0) - 1.0/pFlgPS->rtk));
	    pFlgPS->Kc      = pFlgPS->kc298*exp(pFlgPS->DeltaHaKc*(1.0/(IDGASKONST*298.0) - 1.0/pFlgPS->rtk));
        pFlgPS->Vcmax   = pFlgPS->K * VcmaxOpt*exp(pFlgPS->DeltaHaVcMax*(1.0/(IDGASKONST*298.0) - 1.0/pFlgPS->rtk)) *
		                    (1.0 + exp((pFlgPS->DeltaSVcMax * 298.0 - pFlgPS->DeltaHdVcMax)/(IDGASKONST*298.0))) /
		                    (1.0 + exp((pFlgPS->DeltaSVcMax * TempKelvin - pFlgPS->DeltaHdVcMax)/pFlgPS->rtk)) ;
        pFlgPS->DarkResp= pFlgPS->darkresp298*dark_leaf_respiration_temperature(TempAct, pFlgPS->DeltaHaresp);
	    pFlgPS->tau     = pFlgPS->tau298*exp(pFlgPS->DeltaHatau*(1.0/(IDGASKONST*298.0) - 1.0/pFlgPS->rtk));
        pFlgPS->Pml     = pFlgPS->K * JmaxOpt/4.0*TempKelvin/298.0*exp(pFlgPS->DeltaHaPml*(1.0/(IDGASKONST*298.0) - 1.0/pFlgPS->rtk)) *
		                    (1.0 + exp((pFlgPS->DeltaSPml * 298.0 - pFlgPS->DeltaHdPml)/(IDGASKONST*298.0))) /
		                    (1.0 + exp((pFlgPS->DeltaSPml * TempKelvin - pFlgPS->DeltaHdPml)/pFlgPS->rtk)) ;
    	
        pFlgPS->fac     = 0.5;  //1.0   0.5: dark respiration in light conditions is lower than in night
	    pFlgPS->Gamma   = 0.5 * CONCO2 * 1000.0 /pFlgPS->tau;


        //Test:
        //Photokompensationspunkt mit Abhängigkeit von der Temperatur
	    //pFlgPS->Gamma = pPlant->pGenotype->dCO2CmpPoint*0.1 + 0.188*(TempAct-25.0) + 0.0036*pow((double)TempAct-25.0,2.0);

        
        //////////////////////////////////////////////////////////////////
	    // Irradiance dependence of Pm                                  //
	    //////////////////////////////////////////////////////////////////

	    //Absorbed flux (µmol/m2(leaf)/s) for shaded leaves and assimilation of shaded leaves
		    VISSHD = max(0.0,VISDF + VIST - VISD);

        if (pFlgPS->Pml>0.0) 
        {
            PmSh  =  Alpha * VISSHD / sqrt(1.0 + pow( Alpha*VISSHD/pFlgPS->Pml,2) );
        }
	    else
	        PmSh = 0.0;

        		    
	    //Direct flux absorbed by leaves perpendicular on direct beam and
	    //assimilation of sunlit leaf area
        VISPP  = (1.0-SCV) * PARDIR;

        PmSun = 0.0;
        for (j=0;j<nGauss;j++)
        {
        VISSUN = VISSHD + VISPP * xGauss[j];
        
            if (pFlgPS->Pml>0.0) 
                PmS = Alpha * VISSUN / sqrt(1.0 + pow(Alpha*VISSUN/pFlgPS->Pml,2) );
            else
                PmS = 0.0;
            
            PmSun += PmS * wGauss[j];
        }

	    //Local assimilation rate (FGL)
        FSLLA[N] = CLUSTF[iPlant] * exp(-vExtDirect[N]);
        pFlgPS->Pm   = FSLLA[N]  * PmSun + (1.0-FSLLA[N]) * PmSh;

        //Intercepted radiation:
/*        if(pLLayer->pBack!=NULL)
            pLLayer->PARabsorbed = (VISSHD + FSLLA*VISPP)*(pLLayer->pBack->LAICum -pLLayer->dLAICum)*pPMorph->vCrownArea; // [µmol/s]
        else
            pLLayer->PARabsorbed = (VISSHD + FSLLA*VISPP)*(pCan->fLAI -pLLayer->dLAICum)*pPMorph->vCrownArea; // [µmol/s]

        if(pLLayer->pBack!=NULL)
            pLLayer->PARabsorbed = (VISSHD + FSLLA*VISPP)*pLLayer->pBack->vLeafAreaCum - pLLayer->vLeafAreaCum); // [µmol/s]
        else
            pLLayer->PARabsorbed = (VISSHD + FSLLA*VISPP)*(pCan->fPlantLA -pLLayer->vLeafAreaCum); // [µmol/s]
 
        if(N+1<LEAFLAYERS)
            pLLayer->PARabsorbed = (VISSHD + (FSLLA[N+1]-FSLLA[N])*VISPP)*pLLayer->vLeafAreaLay; // [µmol/s]
        else
            pLLayer->PARabsorbed = (VISSHD + (CLUSTF[iPlant]-FSLLA[N])*VISPP)*pLLayer->vLeafAreaLay; // [µmol/s]
       

                
        if(N+1<LEAFLAYERS)
            pLLayer->PARabsorbed = VISSHD*pLLayer->vLeafAreaLay + (FSLLA[N+1]-FSLLA[N])*VISPP*pPMorph->vCrownArea*FCPL[N]; // [µmol/s]
        else
            pLLayer->PARabsorbed = VISSHD*pLLayer->vLeafAreaLay + (CLUSTF[iPlant]-FSLLA[N])*VISPP*pPMorph->vCrownArea*FCPL[N]; // [µmol/s]
 */

                
        if(N+1<LEAFLAYERS)
            pLLayer->vPARabsorbed = VISSHD*pLLayer->vLeafAreaLay + (FSLLA[N+1]-FSLLA[N])*VISPP*pPMorph->vCrownArea*pLLayer->vFracCPL; // [µmol/s]
        else
            pLLayer->vPARabsorbed = VISSHD*pLLayer->vLeafAreaLay + (CLUSTF[iPlant]-FSLLA[N])*VISPP*pPMorph->vCrownArea*pLLayer->vFracCPL; // [µmol/s]


        pLLayer->vPARabsorbedDay += pLLayer->vPARabsorbed*hPs*3600.0*0.184;

        pPPart->vPARabsorbedCan += pLLayer->vPARabsorbed*hPs*3600.0*0.184; // [J]
        pPPart->vPARabsorbedDay += pLLayer->vPARabsorbed*hPs*3600.0*0.184; // [J]
        pPPart->vPARabsorbedCum += pLLayer->vPARabsorbed*hPs*3600.0*0.184; // [J]


        if(pPlathoModules->iFlagCi <= 2)  //Falge simplified
        {
            if(pPlathoModules->iFlagCi == 1)    //constant ratio
            {
                pFlgPS->Cint= dCO2 * pPlant->pGenotype->fCiCaRatio;
            }

            if(pPlathoModules->iFlagCi == 2)    //Falge-simplified
            {
                // Berechung von Ci
	            // here really new part, simplific. begins:
	            // double facPnetRd=0.85; // describes: Pnet/(Pnet+Rd)
	            // Cint=CO2_air*(1-facPnetRd*1.6*humidity/GFac);
	            // this exercise with facPnetRd was not very "successfull",
	            // impact on pnet was very low! (max. ca. 1%)

	            //pFlgPS->Cint=dCO2*(1.0-1.6*pCl->pWeather->fHumidity/100.0/(pFlgPS->gfac+EPSILON));

	            //pFlgPS->Cint=dCO2*(0.95-0.25*pCl->pWeather->fHumidity/100.0);
	            pFlgPS->Cint=dCO2*(0.7 + 0.25*pPStress->dWaterShortage);

                //Versuche Cint direkt mit Wasserstress zu Koppeln
                //Problem: ->dWaterShortage ist immer 1, wenn PotTranp = 0;
                //Da die Photosynthese in Abhängigkeit vom Sonnenaufgang, ETpot aber immer erst ab 
                //fTimeDay > 0.3 berechnet wird entstehen hier Probleme (Sprünge!) an Tagen mit Wasserstress.
                //(-->PotTransp = 1 solange fTimeDay<=0.3, dann plötzliche sehr kleine Werte für PotTransp)

                //pFlgPS->Cint=dCO2*(1.0 - 0.5*exp(-2.3 * pPStress->dWaterShortage));
	            //pFlgPS->Cint=dCO2*(0.65 + 0.3*pPStress->dWaterShortage);

                if(pFlgPS->Cint<10.0)
                    pFlgPS->Cint = 10.0;
            }   //end iFlagCi = 2 (=Falge-simplified)

	        Wc = pFlgPS->Vcmax * pFlgPS->Cint / (pFlgPS->Cint + pFlgPS->Kc*(1.0+CONCO2/pFlgPS->Ko));
	        Wj = pFlgPS->Pm * pFlgPS->Cint/ (pFlgPS->Cint + 2.0 * pFlgPS->Gamma);

            pFlgPS->NetPhotoMol=(1.0-pFlgPS->Gamma/pFlgPS->Cint)*min(Wc,Wj) - pFlgPS->fac*pFlgPS->DarkResp;
            pFlgPS->NetPhotoMol *= min(PhiN,PhiC) * PhiLfPath;
            //pFlgPS->NetPhotoMol *= min(min(pPStress->dWaterShortage,PhiN),PhiC) * PhiLfPath;     //15 10 2008: water shortage effect decreases directly NetPhotoMol
            //funktioniert nicht, wenn Wasserstress zu stark
            //--> im Moment Wasserstress nur über rel.Humidity bei Cint-Berechnung

            if(pFlgPS->NetPhotoMol<0.0)
                  pFlgPS->NetPhotoMol = 0.0;

            //	pFlgPS->StomCond = pFlgPS->StomCondMin+pFlgPS->gfac*1000*(pFlgPS->NetPhotoMol+pFlgPS->fac*pFlgPS->DarkResp)
            //                        *pCl->pWeather->fHumidity/100.0 /dCO2;    //mmol(H2O)/m2/s
	        pFlgPS->StomCond = 1000*(pFlgPS->NetPhotoMol+pFlgPS->fac*pFlgPS->DarkResp)/max(1.0,dCO2-pFlgPS->Cint);    //mmol(H2O)/m2/s

            pFlgPS->StomCond = max(pFlgPS->StomCond,pFlgPS->StomCondMin);
            if(pFlgPS->StomCond<0.0)
                  pFlgPS->StomCond = 0.0;
        }

        if(pPlathoModules->iFlagCi == 3)    //Falge
        {
            extern	double min3(double x1,double x2,double  x3);

            Cint1 = 0.7 * dCO2;
            pFlgPS->Cint = Cint1;

            // newer Eva Falge version:
	        Wc = pFlgPS->Vcmax * (Cint1- pFlgPS->Gamma) / (Cint1 + pFlgPS->Kc*(1.0+CONCO2/pFlgPS->Ko));
	        Wj = pFlgPS->Pm * (Cint1 - pFlgPS->Gamma)/ (Cint1 + 2.0 * pFlgPS->Gamma);

	        for (i = 0; i < 11; i++)
	        {

		        pFlgPS->Cint = Calc_Cint(pFlgPS, Wc,Wj,dCO2, (double)pCl->pWeather->fHumidity); 

		        if (fabs(pFlgPS->Cint-Cint1) > 0.1)
		        {
			        Cint1 = pFlgPS->Cint;
			        Wc    = pFlgPS->Vcmax * (Cint1-pFlgPS->Gamma) / (Cint1 + pFlgPS->Kc*(1.0+CONCO2/pFlgPS->Ko));
			        Wj    = pFlgPS->Pm    * (Cint1-pFlgPS->Gamma) / (Cint1 + 2.0 * pFlgPS->Gamma);

                    //Wc    *= min3(pPStress->dWaterShortage,PhiN,PhiC) * PhiLfPath;
                    //Wj    *= min3(pPStress->dWaterShortage,PhiN,PhiC) * PhiLfPath;
		        }
		        else 
			        break;
	        }
        
            pFlgPS->NetPhotoMol = pFlgPS->NetPhotoMol* min(PhiN,PhiC) * PhiLfPath;

        //  pFlgPS->GrossPhotoMol= pFlgPS->NetPhotoMol+ pFlgPS->fac * pFlgPS->DarkResp;
            pFlgPS->GrossPhotoMol= pFlgPS->NetPhotoMol + pFlgPS->DarkResp;

            pFlgPS->StomCond    = 1.0 / (1.0/pFlgPS->laminarCond + 1.0/pFlgPS->StomCond);

            if(pFlgPS->NetPhotoMol<0.0)
                  pFlgPS->NetPhotoMol = 0.0;

            if(pFlgPS->StomCond<0.0)
                  pFlgPS->StomCond = 0.0;
        } //end iFlagCi = 3 (=Falge)


	    pFlgPS->TranspR     = pFlgPS->vpd / AIRPRESSURE * pFlgPS->StomCond;


        //Umrechnung von µmol(CO2)/m2/s -> g(CO2)/m2/h
        pLLayer->vNettoPsR    = pFlgPS->NetPhotoMol*44.0*3600/1e6;
        pLLayer->vDarkRespR   = pFlgPS->fac*pFlgPS->DarkResp*44.0*3600/1e6; 
        pLLayer->vGrossPsR    = pLLayer->vNettoPsR + pLLayer->vDarkRespR;

  	    // factor 0.0648 is: mmolH2O/m2/s -> mm/h :
	   // pLLayer->vActTranspR  = pFlgPS->TranspR * 0.0648; // mm/h = kg/m2/h

	    // factor 1/400 is : mmolH2O/m2(total)/s -> cm/s
	    //pLLayer->vStomCond = pFlgPS->StomCond/400.0;
	    //weiter in mmolH2O/m2(total)/s
        pLLayer->vStomCond = pFlgPS->StomCond;

        pLLayer->vCint = pFlgPS->Cint;

        // net photosynthesis,transp, cond. is recalc. to microMol/m2(proj)/s:
	    // factor TotalToProjM2Factor is molCO2/m2(total)/s -> microMolCO2/m2(proj)/s :

        // g(CO2)/h
	    pLLayer->vGrossPsR	*= pFlgPS->totaltoprojm2factor * pLLayer->vLeafAreaLay;
	    pLLayer->vNettoPsR	*= pFlgPS->totaltoprojm2factor * pLLayer->vLeafAreaLay;
	    pLLayer->vDarkRespR	*= pFlgPS->totaltoprojm2factor * pLLayer->vLeafAreaLay;
	    //mmolH2O/m2(proj.)/s
        pLLayer->vStomCond    *= pFlgPS->totaltoprojm2factor;
	    //kg/h
        //pLLayer->vActTranspR  *= pFlgPS->totaltoprojm2factor * pLLayer->vLeafAreaLay;

        //Integration over time loop (per leaf layer)
  	    //g(CO2)
        pLLayer->vGrossPsDay   += pLLayer->vGrossPsR*hPs;
	    pLLayer->vNettoPsDay   += pLLayer->vNettoPsR*hPs;
	    pLLayer->vDarkRespDay  += pLLayer->vDarkRespR*hPs;
	    // kg
        //pLLayer->vActTranspDay += pLLayer->vActTranspR*hPs; 
        //mmolH2O/m2(proj.)/s
	    pLLayer->vStomCondDay  += pLLayer->vStomCond*hPs/vDaylengthPs;


        //	Integration of local rates to whole plant values
        vGrossPs   += pLLayer->vGrossPsR;    // g(CO2)/h
        vNettoPs   += pLLayer->vNettoPsR;    // g(CO2)/h
        vDarkResp  += pLLayer->vDarkRespR;   // g(CO2)/h
        //vActTransp += pLLayer->vActTranspR;  // kg/h
        vStomCond  += pLLayer->vStomCond
            * pLLayer->vLeafAreaLay/(pPlant->pCanopy->fPlantLA+EPSILON); // mmolH2O/m2(proj.)/s
    }//end "Farquhar"


	pLLayer = pLLayer->pNext;
	} //Ende der Integration über die Pflanzenhöhe
           
	if(pTi->pTimeStep->fAct == (float) 1.0 )
    {
		//Integration to a daily total
		vAveGrossPs     += vGrossPs*wGauss[it-1];     // g(CO2)/h
		vAveNettoPs     += vNettoPs*wGauss[it-1];     // g(CO2)/h
		vAveDarkResp    += vDarkResp*wGauss[it-1];    // g(CO2)/h
		//vAveActTransp   += vActTransp*wGauss[it-1];   // kg/h
		vAveStomCond    += vStomCond*wGauss[it-1];    // mmolH2O/m2(proj.)/s

 	    //gas exchange during time step kg(CO2)/h
	    pPlant->pPltCarbon->fGrossPhotosynR = (float)(vAveGrossPs * 1E-3 * hPs);    // kg(CO2)
        pPPart->vNettoPhotosynR                 = vAveNettoPs * 1E-3;               // kg(CO2)/h
        pPPart->vDarkRespR                      = vAveDarkResp * 1E-3;              // kg(CO2)/h
        //pPPart->vActTranspR                     = vAveActTransp;                    // kg/h
        pPPart->vAveStomCond                    = vAveStomCond ;                    // mmolH2O/m2(proj.)/s
  	    //gas exchange during day kg(CO2)
	    pPPart->vDailyGrossPhotosynthesis = pPlant->pPltCarbon->fGrossPhotosynR;    // kg(CO2)
        pPPart->vDailyNettoPhotosynthesis = pPPart->vNettoPhotosynR * hPs;   // kg(CO2)
        pPPart->vDailyDarkResp = pPPart->vDarkRespR * hPs;                   // kg(CO2)
        //pPPart->vActTranspDay   = pPPart->vActTranspR * hPs;                        // kg
        pPPart->vAveStomCondDay = vAveStomCond;                                     // mmolH2O/m2(proj.)/s
    }
	else
    {
	    //gas exchangen during time step kg(CO2)/h
	    pPlant->pPltCarbon->fGrossPhotosynR     = (float)(vGrossPs  * 1E-3 * hPs);  // kg(CO2)
        pPPart->vNettoPhotosynR                         = vNettoPs  * 1E-3;         // kg(CO2)/h
        pPPart->vDarkRespR                              = vDarkResp * 1E-3;         // kg(CO2)/h
        //pPPart->vActTranspR                             = vActTransp;               // kg/h
        pPPart->vAveStomCond                            = vStomCond; // mmolH2O/m2(proj.)/s
	    
        //gas exchange during day kg(CO2)

        // wird in DGL_PLATHO aufsummiert
	    pPPart->vDailyGrossPhotosynthesis += pPlant->pPltCarbon->fGrossPhotosynR;   // kg(CO2)   
        
        pPPart->vDailyNettoPhotosynthesis += pPPart->vNettoPhotosynR * hPs;    // kg(CO2)
        pPPart->vDailyDarkResp            += pPPart->vDarkRespR * hPs;         // kg(CO2)
        //pPPart->vActTranspDay += pPPart->vActTranspR * hPs;                           // kg
        pPPart->vAveStomCondDay += pPPart->vAveStomCond*hPs/vDaylengthPs;           // mmolH2O/m2(proj.)/s

    }

    //ozone effect on photosynthesis
    if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3))
        pPPart->vAveStomCond  *= pow(1.0+pPStress->vO3Sensitivity*pPStress->vO3StressIntensity,1);

        return 1;
    }


double CalcLaminarConductivity(double TempAct, double vWind, PFALGEPS pFlgPS, PPLANT pPlant)
{
    //Berechnungen bezueglich der laminaren Grenzschicht
    // new: Mike: 6/8/97: to account for zero wind velocity
    if(vWind < EPSILON)
    {
        pFlgPS->laminarCond=0.001; // chged from 0.000001, this was to small, caused errors!!!
    }
    else
    {
	    pFlgPS->dbl = 0.004 * sqrt(pFlgPS->widthofleaf / (vWind * 100.0)); // 100: m/s -> cm/s
	    pFlgPS->dwv = 2.126e-05 + 1.48e-07 * TempAct;
	    pFlgPS->laminarCond = pFlgPS->dwv/pFlgPS->dbl * 1000.0;
	    pFlgPS->laminarCond *= (AIRPRESSURE * 100.0)/pFlgPS->rtk;

	    // only used for coniferous trees:
	    SPRUCE 
            pFlgPS->laminarCond /= (1.67 * pow(pFlgPS->sigmaleaf,0.43));
    }
    return pFlgPS->laminarCond;
}




double dark_leaf_respiration_temperature(double temperature, double DeltaHaresp)
{           
	return exp(DeltaHaresp*(1/(IDGASKONST*298.0) -1/(IDGASKONST*(temperature+273.16))));
}

double Calc_Cint(PFALGEPS pPs, double Wc,double Wj, double CO2_air, double vhumidity)
{
    extern	double min3(double x1,double x2,double  x3);
    extern	double max3(double x1,double x2,double  x3);
    
	double aaa,bbb,eee;
	double ddd = pPs->Gamma;
	double alpha,beta,gamma,theta; // only intermediate variables
	double rrr,sss,ttt,uuu,vvv;  // same
	double QQQQ,PPPP,DDDD; // same
	double aquad1,bquad1,cquad1; // same
    double Cint;
    double humidity = vhumidity/100.0;

	// this is left curve:
	if (Wc <= Wj)
	{
		aaa = pPs->Vcmax;
		bbb = pPs->Kc * (1.0 + CONCO2/pPs->Ko);
		eee = 1.0;
	}
	else
	{   
		aaa = 4.0*pPs->Pm;
		bbb = 8.0*pPs->Gamma;
		eee = 4.0;
	}            

//	pPs->fac=1.0;
	
	alpha = 1560*1560*(1.0 + pPs->StomCondMin/pPs->laminarCond - pPs->gfac*humidity/1.56);

    if (fabs(alpha) > EPSILON)
	{
	  // this is kubische Loesung:
	  beta  = 1560*CO2_air*(pPs->gfac*humidity/1.56*pPs->laminarCond -
				2.0*pPs->StomCondMin - pPs->laminarCond) -
			1560*1560*(pPs->DarkResp*pPs->fac*pPs->gfac*humidity/1.56); // rd==DarkResp
	  gamma = pPs->laminarCond*(CO2_air*pPs->StomCondMin+1560*pPs->fac*pPs->DarkResp
					*pPs->gfac*humidity/1.56);
	  theta = 1560*(pPs->gfac*humidity/1.56*pPs->laminarCond-pPs->StomCondMin);


	  rrr = (eee*beta+bbb*theta-aaa*alpha+eee*alpha*pPs->fac*pPs->DarkResp)/(eee*alpha);
	  sss = (eee*gamma*CO2_air+bbb*gamma-aaa*beta+aaa*ddd*theta+
			eee*pPs->fac*pPs->DarkResp*beta+bbb*pPs->fac*pPs->DarkResp*theta)/(eee*alpha);
	  ttt = (-aaa*gamma*CO2_air+aaa*ddd*gamma+eee*pPs->fac*pPs->DarkResp*gamma*CO2_air+
			bbb*pPs->fac*pPs->DarkResp*gamma)/(eee*alpha);

	  PPPP=(3*sss-rrr*rrr)/3.0;
	  QQQQ=2*rrr*rrr*rrr/27.0-rrr*sss/3.0+ttt;
	  DDDD=pow(PPPP,3)/27.0+pow(QQQQ,2)/4.0;

	  if (QQQQ<0)       // change 5/11/96; `>` -> `<`
	    vvv=-sqrt(fabs(PPPP)/3.0);
	  else
	    vvv=sqrt(fabs(PPPP)/3.0);

	  uuu=QQQQ/(2.0*vvv*vvv*vvv);

	  if (PPPP>0)
	    // eine reelle solution, zwei conjugierte:
	    pPs->NetPhotoMol = -2*vvv*sinh(log(uuu+sqrt(uuu*uuu+1))/3.0)-rrr/3.0;
	  else
	    if (DDDD>0)
	    {

	      //  eine reelle solution, zwei conjugierte:
	      pPs->NetPhotoMol = -2*vvv*cosh(log(uuu-sqrt(uuu*uuu-1))/3.0)-rrr/3.0;
	    }
	    else
	      // three reelle solutions:
	      {
		double NetPhotoMol1,NetPhotoMol2,NetPhotoMol3;
		NetPhotoMol1=-2*vvv*cos(acos(uuu)/3.0)-rrr/3.0;
		NetPhotoMol2=-2*vvv*cos((acos(uuu)+2*PI)/3.0)-rrr/3.0;
		NetPhotoMol3=-2*vvv*cos((acos(uuu)+4*PI)/3.0)-rrr/3.0;
		if (alpha>0)
		{
		  pPs->NetPhotoMol=NetPhotoMol2;
		  if(NetPhotoMol1>0 && NetPhotoMol2>0 && NetPhotoMol3>0)
		  {
		    // minimum of all three:
		    pPs->NetPhotoMol=min3(NetPhotoMol1,NetPhotoMol2,NetPhotoMol3);
		  }
		  else if (NetPhotoMol1*NetPhotoMol2*NetPhotoMol3<0 &&
			   max3(NetPhotoMol1,NetPhotoMol2,NetPhotoMol3)>0)
			   pPs->NetPhotoMol=min3(NetPhotoMol1,NetPhotoMol2,NetPhotoMol3);
		       else
			   pPs->NetPhotoMol=max3(NetPhotoMol1,NetPhotoMol2,NetPhotoMol3);
		}
		else
		  pPs->NetPhotoMol=NetPhotoMol3;
	      }


	} // end of kubische Rechnung
	else
	{
	  // begin of quadratische Loesung:
	  beta=-1560*CO2_air*pPs->StomCondMin-1560*1560*pPs->DarkResp*pPs->fac*pPs->gfac*humidity/1.56;
	  gamma = pPs->laminarCond*(CO2_air*pPs->StomCondMin+1560*pPs->fac*pPs->DarkResp
					*pPs->gfac*humidity/1.56);
	  theta = 1560* pPs->laminarCond;

	  aquad1 = eee*beta+bbb*theta;
	  bquad1 = eee*gamma*CO2_air-aaa*beta+eee*pPs->fac*pPs->DarkResp*beta+aaa*ddd*theta+
			bbb*pPs->fac*pPs->DarkResp*theta+bbb*gamma;
	  cquad1 = -aaa*gamma*CO2_air+eee*pPs->fac*pPs->DarkResp*gamma*CO2_air+
			bbb*pPs->fac*pPs->DarkResp*gamma+aaa*ddd*gamma;
	  pPs->NetPhotoMol=(-bquad1-sqrt(bquad1*bquad1-4*aquad1*cquad1))/(2.0*aquad1);


	}  // end of quadratische Loesung

	pPs->Cs = CO2_air - pPs->NetPhotoMol*1560/pPs->laminarCond;

	pPs->StomCond = pPs->gfac *1000* (pPs->NetPhotoMol+pPs->fac*pPs->DarkResp) * humidity  / pPs->Cs
			+ pPs->StomCondMin;

	Cint     = pPs->Cs - pPs->NetPhotoMol*1560/pPs->StomCond;

    //   pPs->photosyn_brutto= 1E-6*1560*(pPs->NetPhotoMol+ pPs->fac * pPs->DarkResp);

	//NetPhotoMol*=0.001; // now in mikromol

	return Cint;
}

    int ReadFalgeParameters(int iPlant, PPLANT pPlant)
    {
        PFALGEPS pFlgPS = pPlathoPlant[iPlant]->pPlathoGenotype->pFalgePS;

            pFlgPS->ko298        = 248; //mmol/mol
	        pFlgPS->kc298        = 404; //mmol/mol

            pFlgPS->darkresp298  = 0.921;  //(µmol/m2/s
            pFlgPS->tau298       = 2339.53;

            pFlgPS->DeltaHaPml   = 33485;  //Aktivierungsenergie von Pml
            pFlgPS->DeltaHdPml   = 220000; //Deaktivierungsenergie von Pm
            pFlgPS->DeltaSPml    = 710;    //Entropie von Pml

            pFlgPS->DeltaHaKo    = 35900;
            pFlgPS->DeltaHaKc    = 59500;
            pFlgPS->DeltaHatau   =-28990;
            pFlgPS->DeltaHaresp	= 43460;   //Aktivierungsenergie fuer die Atmung

            pFlgPS->DeltaHaVcMax = 35068;   //Aktivierungsenergie fuer VcMax
            pFlgPS->DeltaHdVcMax = 213000;  //Deaktivierngsenergie fuer VcMax
            pFlgPS->DeltaSVcMax  = 656;     // Entropie fuer VcMax

            pFlgPS->gfacbase	 = 12;      //molCO2/E   Sensitivitaetsfaktor von g_s bezueglich h_s und C_s
                                                        // is base line sensitivity without soil water stress
			pFlgPS->widthofleaf	= 4;        //cm
			pFlgPS->sigmaleaf	= -1;       //irgendwas mit der Dichte der Nadeln am Zweig
		
			pFlgPS->StomCondMax	= 100;  //mmol/m2/s Maximalwert fuer stomatal conductance
			pFlgPS->StomCondMin	= 1;    //mmol/m2/s Minimalwert fuer stomatal conductance
			pFlgPS->totaltoprojm2factor= 2.6;  //m2/m2

       BEECH
        {
            pFlgPS->ko298        = 248; //mmol/mol
	        pFlgPS->kc298        = 404; //mmol/mol

            pFlgPS->darkresp298  = 0.921;  //(µmol/m2/s
            pFlgPS->tau298       = 2339.53;

            pFlgPS->DeltaHaPml   = 33485;  //Aktivierungsenergie von Pml
            pFlgPS->DeltaHdPml   = 220000; //Deaktivierungsenergie von Pm
            pFlgPS->DeltaSPml    = 710;    //Entropie von Pml

            pFlgPS->DeltaHaKo    = 35900;
            pFlgPS->DeltaHaKc    = 59500;
            pFlgPS->DeltaHatau   =-28990;
            pFlgPS->DeltaHaresp	= 43460;   //Aktivierungsenergie fuer die Atmung

            pFlgPS->DeltaHaVcMax = 35068;   //Aktivierungsenergie fuer VcMax
            pFlgPS->DeltaHdVcMax = 213000;  //Deaktivierngsenergie fuer VcMax
            pFlgPS->DeltaSVcMax  = 656;     // Entropie fuer VcMax

 //           pFlgPS->gfacbase	 = 12;      //molCO2/E   Sensitivitaetsfaktor von g_s bezueglich h_s und C_s
 //           pFlgPS->gfacbase	 = 6;      // is base line sensitivity without soil water stress
            pFlgPS->gfacbase	 = 14;      // is base line sensitivity without soil water stress

			pFlgPS->widthofleaf	= 4;        //cm
			pFlgPS->sigmaleaf	= -1;       //irgendwas mit der Dichte der Nadeln am Zweig
		
			pFlgPS->StomCondMax	= 100;  //mmol/m2/s Maximalwert fuer stomatal conductance
			//pFlgPS->StomCondMin	= 1;    //mmol/m2/s Minimalwert fuer stomatal conductance
			pFlgPS->StomCondMin	= 14;    //nach Emberson (2000)
			pFlgPS->totaltoprojm2factor= 1.3;//1.0;  //m2/m2

            pFlgPS->K              = 1.0;
        }
    
        SPRUCE
        {
			pFlgPS->ko298		    = 159.597;
			pFlgPS->kc298		    = 299.469;

			pFlgPS->darkresp298	    = 0.524;
			pFlgPS->tau298			= 2339.53;

			pFlgPS->DeltaHaPml		= 4717;
			pFlgPS->DeltaHdPml		= 200000;
			pFlgPS->DeltaSPml		= 643;

			pFlgPS->DeltaHaKo		= 36000;
			pFlgPS->DeltaHaKc		= 65000;
			pFlgPS->DeltaHatau		= -28990;
			pFlgPS->DeltaHaresp	    = 63500;
		
			pFlgPS->DeltaHaVcMax		= 75750;
			pFlgPS->DeltaHdVcMax		= 200000;
			pFlgPS->DeltaSVcMax		= 656;
	
//			pFlgPS->gfacbase		    = 9.8;
			pFlgPS->gfacbase		    = 5;

			pFlgPS->widthofleaf	    = 0.15;
			pFlgPS->sigmaleaf		= 1.7665;
		
			pFlgPS->StomCondMax		= 60;
			pFlgPS->StomCondMin		= 0.1;
			pFlgPS->totaltoprojm2factor= 1.3;//2.6
            
           //Treedyn: for coniferous trees calculate "seasonal_reduction_photo(double Temperature)"
           //calculate the effect of frost hardiness and not photosynthesis on photosynthesis with model of Ilkka Leinonen
            pFlgPS->K = 1.0;
        }
		
        return 1;
   }

 

