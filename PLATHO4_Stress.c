//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"

int	CalcLeafInternalO3(double TimeDay, double TimeAct, double DeltaT, int iPlant, double PhiCH2O,
				   PPLANT pPlant, PWEATHER pWeather, PLOCATION pLo,  PTIME pTi);

extern double Daylength(double vLatitude, int nJulianDay, int nID);
double HourlyOzoneConc(double DayTime);
double PhiAssimilatstau(PPLANT pPlant);
double PhiOzone(int);
double PhiLeafPathogenes(int);
double PhiRootPathogenes(int);
extern double HourlyTemperature_PLATHO	(double Time, double fTempMax, double fTempMin, double fTempMinNext, 
									 double vDaylength, int iSimDay);


double HourlyOzoneConc(double DayTime)
	{    
    double	vO3Conc;
    double  fr,r;

	r 	  = (double)DayTime;
	fr	  = 0.5*(1.0+cos((0.7-r)*PI/0.4));
	vO3Conc = pPlathoScenario->vO3ConcMin + fr*
		(pPlathoScenario->vO3ConcMax-pPlathoScenario->vO3ConcMin);
	
	return vO3Conc;
	}



/************************************************************************/
/* Function PhiC: Reduktionsfaktor Assimilatstau
/*
/*	Output:
/*	
/*	float	PhiAss			- Reduktionsfaktor (Assimilatstau) [-]
/*
/************************************************************************/

double PhiAssimilatstau(PPLANT pPlant)
		{
		//float CRatio;
		double FRatio, PhiAss, LimitC, phiN; 

		PBIOMASS pBiom = pPlant->pBiomass;
		PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;

	
        if(pPlathoModules->iFlagNEffectGrw==1)//no N effect on growth
            phiN = 1.0;
        else
		    phiN = max(0.0,min(1.0,(pPltN->fLeafActConc-pPltN->fLeafMinConc)
            /(pPltN->fLeafOptConc-pPltN->fLeafMinConc)));
		
		
		if(pBiom->fLeafWeight>(float)0.0)
		{
			HERBS
			{
				LimitC = 0.3*pow(phiN,1);

				if(pPlant->pDevelop->fDevStage<=2)
					FRatio = 0.0;
				else
					FRatio = pPlant->pPltCarbon->fCAssimilatePool
						/(pBiom->fLeafWeight+pBiom->fStemWeight);

				PhiAss = max(0.5,1.0-pow(FRatio/LimitC,2));
			}
			
			TREES
			{
				LimitC = 0.10;
				//LimitC = 0.15*pow(phiN,1);
				//LimitC = 0.1*pow(phiN,1);

				FRatio = pPlant->pPltCarbon->fCAssimilatePool
							/(pBiom->fLeafWeight+pBiom->fWoodWeight);

				PhiAss = max(0.0,1.0-pow(FRatio/LimitC,2));
				//PhiAss = max(0.1,1.0-pow(FRatio/LimitC,2));
			}
		}
		else
			PhiAss = 1.0;


//		PhiAss = (float)(max(0.3,1.0-pow(FRatio/LimitC,2)));
//		PhiAss = (float)(max(0.1,1.0-pow(FRatio/LimitC,2)));

		return PhiAss;
}

double PhiOzone(int iPlant)
{
	PPLANTSTRESS		pPStress = pPlathoPlant[iPlant]->pPlantStress;
	// float PhiO3;

	//Reduktionsfaktor
	if(pPStress->vO3Sensitivity > 0.0)
	{
		pPStress->PhiO3 = max(0.0,pow(1.0-pPStress->vO3Sensitivity*pPStress->vO3StressIntensity,0.5));
	}
	else
		pPStress->PhiO3 = 1.0;
		

	return pPStress->PhiO3;
}

double PhiLeafPathogenes(int iPlant)
{
	PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
	double PhiLfPath;

	//Reduktionsfaktor
	if(pPlathoScenario->vLeafPathogenes-pPPltStress->vCritLfPath > 0.0)
		PhiLfPath = max(0.0,(1.0-pow(pPlathoScenario->vLeafPathogenes-pPPltStress->vCritLfPath,pPPltStress->vRtPathBeta)));
	else
		PhiLfPath = 1.0;
		
/*
    double p		= 0.5;
	double alpha	= 1;


	//Reduktionsfaktor
	if(pPlathoScenario->vLeafPathogenes-pPStress->vCritLfPath > 0.0)
		PhiLfPath = max(0.0,(1.0-p*pow(pPlathoScenario->vLeafPathogenes-pPStress->vCritLfPath,alpha)));
	else
		PhiLfPath = 1.0;
*/		
	return PhiLfPath;
}

double PhiRootPathogenes(int iPlant)
{
	PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
	double PhiRtPath;


	//Reduktionsfaktor
	if(pPlathoScenario->vRootPathogenes-pPPltStress->vCritRtPath > 0.0)
		PhiRtPath = max(0.0,(1.0-pow(pPlathoScenario->vRootPathogenes-pPPltStress->vCritRtPath,pPPltStress->vRtPathBeta)));
	else
		PhiRtPath = 1.0;
		
/*
    double p		= 0.5;
	double alpha	= 0.5;


	//Reduktionsfaktor
	if(pPlathoScenario->vRootPathogenes-pPStress->vCritRtPath > 0.0)
		PhiRtPath = max(0.0,(1.0-p*pow(pPlathoScenario->vRootPathogenes-pPStress->vCritRtPath,alpha)));
	else
		PhiRtPath = 1.0;
*/		

	return PhiRtPath;
}

int	CalcLeafInternalO3(double TimeDay, double TimeAct, double DeltaT, int iPlant, double PhiCH2O, 
				   PPLANT pPlant, PWEATHER pWeather, PLOCATION pLo, PTIME pTi)
{
	PGENOTYPE			pGen        = pPlant->pGenotype;
	PBIOMASS			pBiom       = pPlant->pBiomass;
    PPLTCARBON          pPltC       = pPlant->pPltCarbon;
	PPLATHOGENOTYPE		pPGen       = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLANTSTRESS		pPStress    = pPlathoPlant[iPlant]->pPlantStress;
	PPLATHOMORPHOLOGY	pPMorph	    = pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOPARTITIONING	pPPart	    = pPlathoPlant[iPlant]->pPlathoPartitioning;
	PPLATHODEVELOP  	pPDev	    = pPlathoPlant[iPlant]->pPlathoDevelop;

	double HourlyOzoneConc(double DayTime);

	double vDaylengthPs;
	double vO3Conc, fTempAct;
	double KAtmLeaf;
	double E, PA;

	
	if(pBiom->fLeafWeight<=(float)0.0)
	{
		pPStress->vO3ConcLeaf           = 0.0;
        pPStress->vO3StressIntensity    = 0.0;

        pPStress->vO3LeafConcCumCrit    = 0.0;
        pPStress->vO3LeafConcCum        = 0.0;
        pPStress->vO3DetoxCum           = 0.0;
        pPStress->vPotO3DetoxCum        = 0.0;
       
        pPStress->vO3UptakeCum          = 0.0;

        pPStress->dRubiscoCont = 0.0;
        pPStress->dRubiscoConc = 0.0;

        return 1;
	}

	//Länge des Tages
	vDaylengthPs = Daylength(pLo->pFarm->fLatitude, pTi->pSimTime->iJulianDay,2);

	//externe O3-Konzebtration
	vO3Conc = HourlyOzoneConc(TimeDay);    //µg/m^3

	//Berechnung der stomatären Leitfähigkeit (nach Trapp, 1992)
	//Die Aufnahme von Ozon über die Kutkula spielt nach Ch. Langebartels keine Role

	//Berechnung der aktuellen Temperatur:
	if(pTi->pTimeStep->fAct == (float)1.0)
		fTempAct = (pWeather->fTempAve>(float)-99)?
		pWeather->fTempAve : (pWeather->fTempMin+pWeather->fTempMax)/2.0;
	else
	{
		if(((int)TimeAct==0)||(pPlant->pDevelop->iDayAftSow==1))
			fTempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pWeather->fTempMax,
							pWeather->fTempMin, pWeather->pNext->fTempMin, vDaylengthPs,1);

		else if(pWeather->pNext==NULL)
			fTempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pWeather->fTempMax,
							pWeather->fTempMin, pWeather->fTempMin, vDaylengthPs,0);

		else
			fTempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pWeather->fTempMax,
							pWeather->fTempMin, pWeather->pNext->fTempMin, vDaylengthPs,0);
	}

    //Stomatäre Leitfähigkeit für Ozon:
	//Sättigungsdampfdruck von Wasserdampf in der Atmosphäre [Pa]:
	E = 610.7 * pow(10.0,7.5*fTempAct/(fTempAct+237));

	//Sättigungskonzentration von Wasserdampf [kg/m3]:
	//(Gaskonstante für Wasserdampf = 461 J/kg/°C)
	PA = E/(461*(fTempAct+273));    // bei T=20°C->PA=0.0173 kg/m^3

    if(pPlathoModules->iFlagPhotosynthesis==1) //Goudriaan
    {
	    ///////////////////////////////////////////////
	    //Leitfähigkeit g_H2O für Wasserdampf [m/d]:
	    //
	    //g_H2O = qw/(A_L*(PA-h*PA))
	    //
	    //qw [kg/d]
	    //A_L [m^2]
	    //PA [kg/m^3]
	    ///////////////////////////////////////////////
	    if(pPlant->pCanopy->fPlantLA==(float)0.0)
		    pPPart->vH2OCond = 0.0;
	    else
		    pPPart->vH2OCond = (double)pPlant->pPltWater->fActTranspdt*pPMorph->vRootZoneArea/DeltaT/
			    (pPlant->pCanopy->fPlantLA*(PA-pWeather->fHumidity/100.0*PA)+ EPS);

	    //Leitfähigkeit g_CO2 für CO2 [m/d]:
	    pPPart->vCO2Cond = pPPart->vH2OCond * sqrt(18.0/44.0);
	    //Leitfähigkeit g_O3 für Ozon [m/d]:
	    pPPart->vO3Cond = pPPart->vH2OCond * sqrt(18.0/48.0);
    }
    else  //Farquhar
    {
       	if(pPlant->pCanopy->fPlantLA==(float)0.0)
		    pPPart->vO3Cond = 0.0;
        else
        {
            //   (mmol/m^2/s * g/mol)/(µg/m^3) * s/d
            //  1mol = 22.4 l
            //   2.1428 µg/m^3 = 1 ppb = 1e-3 mmol/mol
            pPPart->vO3Cond  = pPPart->vAveStomCond * sqrt(18.0/48.0) * 0.0224 * 86400 * 1e-3; // m/d
        }         
    }

    if(pPlathoModules->iFlagO3 == 4) //O3-effect following van Oijen et al. (2004)
    {
        double dPotRepair;

	    //effective O3-uptake:
        pPStress->vO3Uptake = pPlant->pCanopy->fPlantLA *pPPart->vO3Cond 
                                * vO3Conc * (1.0-pPGen->fdetox) * 1e-6 * DeltaT;//g(O3)
        
        //costs for detoxification;
        pPStress->dCostForDetox = pPlant->pCanopy->fPlantLA *pPPart->vO3Cond 
                                    * vO3Conc * pPGen->fdetox * pPGen->cdetox * 1e-6 * DeltaT; //g(CH2O)

	    pPStress->vO3Detox = pPlant->pCanopy->fPlantLA *pPPart->vO3Cond 
                                    * vO3Conc * pPGen->fdetox * 1e-6 * DeltaT;      // g(O3)
    	
        pPStress->vO3DetoxCum += pPStress->vO3Detox; // g(O3)

        pPStress->dRubiscoCont -= pPGen->dO3damage*pPStress->vO3Uptake;   //g(Rubisco)
        if(pPStress->dRubiscoCont <= 0.0)  pPStress->dRubiscoCont = 0.0;

        //repair rate of Rubisco
        if(pPGen->crepair>0.0)
        {
            dPotRepair =  pBiom->fLeafGrowR * pGen->fGrwEffLeaf * pPGen->frepair/pPGen->crepair * 1e3;  //g(Rubisco)
            //dPotRepair =  pPltC->fCAssimilatePool * pPGen->frepair/pPGen->crepair * 1e3;  //g(Rubisco)
        }
       else
           dPotRepair =  max(0.0,pPStress->dRubiscoContCrit-pPStress->dRubiscoCont);  //g(Rubisco)
        
        pPStress->dRubiscoContCrit = pBiom->fLeafWeight * pPGen->dRubiscoConcCrit * 1e3;  //g(Rubisco)

        pPStress->dRepairR =  min(dPotRepair,max(0,pPStress->dRubiscoContCrit-pPStress->dRubiscoCont));  //g(Rubisco)

        //costs for repair
        pPStress->dCostsRepair = pPStress->dRepairR*pPGen->crepair; //g(CH2O)

        pPStress->dRubiscoCont += pPStress->dRepairR;   //g(Rubisco)

        if(pBiom->fLeafWeight>(float)0.0)
            pPStress->dRubiscoConc = pPStress->dRubiscoCont/(pBiom->fLeafWeight*1e3); //g(Rubisco)/g
        else
            pPStress->dRubiscoConc = 0.0;

        pPStress->vO3UptakeDay += pPStress->vO3Uptake;   // g(O3)
        //pPStress->vO3UptakeCum += pPStress->vO3Uptake;   // g(O3)
        if(pPlant->pCanopy->fPlantLA>0.0)
            pPStress->vO3UptakeCum += pPStress->vO3Uptake/48.0/pPlant->pCanopy->fPlantLA*1e3;   // mmol/m^2

        pPStress->dCostForDetoxCum += pPStress->dCostForDetox;   // g(CH2O)
        pPStress->dCostsRepairCum  += pPStress->dCostsRepair;    // g(CH2O)

    }
    else
    {

	
	//Gleichgewichts-Verteilungskoeffizient Blatt-Atmosphäre [(µg(O3)/m3(Luft))/(µg(O3)/kg(TS))]:
	//KAtmLeaf(T=20) = 15.4 //Wert aus Plöchel et al. (2000) abgeleitet!
	//KAtmLeaf = (float)(15.4 * exp(-6.7*(fTempAct-20)/(fTempAct+273)));
	
	//Gleichgewichts-Verteilungskoeffizient Blatt-Atmosphäre [(µg(O3)/kg(TS)))/(µg(O3)/m3(Luft)]:
	//KAtmLeaf(T=20) = 0.43 
    //Wert aus Plöchel et al. (2000) abgeleitet: Annahme 50% TS
	//KAtmLeaf = (float)(0.43 * exp(-6.7*(fTempAct-20)/(fTempAct+273)));
	
    KAtmLeaf = 0.43;
	
		
	//pPStress->vO3Uptake = pPlant->pCanopy->fPlantLA *(float)pPPart->vO3Cond * 
	//				(vO3Conc - pPStress->vO3ConcLeaf/KAtmLeaf) * DeltaT;

	// O3-Konzentration im Blatt = 0! Was durch die Abwehrstoffe nicht abgebaut (detoxifiziert) 
    // werden kann, verursacht Schäden am Photosynthetischen Apparat und wird dabei verbraucht
	pPStress->vO3Uptake = pPlant->pCanopy->fPlantLA *pPPart->vO3Cond * vO3Conc * DeltaT;    //µg

	
	pPStress->vO3ConcLeaf += pPStress->vO3Uptake/(pBiom->fLeafWeight + pPStress->vLeafDefCont + EPS);   //µg/kg

	//if(pPStress->vO3ConcLeaf>vO3Conc*KAtmLeaf)
	//	pPStress->vO3ConcLeaf = vO3Conc*KAtmLeaf;

	if(pPStress->vO3ConcLeaf<0.0)
		pPStress->vO3ConcLeaf=0.0;
 
	//Abbaurate im Blatt (durch Ascorbat?)
    //Solange Abwehrstoffe vorhanden sind, wird alles Ozon sofort abgebaut
    //potentieller Ozon-Abbau im gegebenen Zeitschritt
    pPStress->vPotO3Detox = 1e9*pPStress->vO3DefenseEff*pPStress->vLeafDefConc*DeltaT; // µg(O3)/kg(TS)
         
//  if(pPDev->iStagePlatho<2)
    if(pPlant->pDevelop->fDevStage < (float)1.0)
        pPStress->vPotO3DetoxCum = 0.0;
    else
        pPStress->vPotO3DetoxCum +=  pPStress->vPotO3Detox; // µg(O3)/kg(TS)   *(pBiom->fLeafWeight + pPStress->vLeafDefCont + EPS); // µg(O3)

	pPStress->vO3Detox = min(pPStress->vO3ConcLeaf,pPStress->vPotO3Detox);      // µg(O3)/kg(TS)
	
    pPStress->vO3DetoxCum += pPStress->vO3Detox; // µg(O3)/kg(TS)   *(pBiom->fLeafWeight + pPStress->vLeafDefCont + EPS); // µg(O3)

	pPStress->vO3ConcLeaf -= pPStress->vO3Detox; // µg/kg(TS)

    pPStress->vO3ConcLeafMes = pPStress->vO3ConcLeaf;

	if(pPStress->vO3ConcLeaf<0.0)
		pPStress->vO3ConcLeaf =0.0; // µg/kg(TS)

	//Verbrauch der Abwehrstoffe 
    //(Ascorbat 176g/mol, 2mol(ASC)/mol(O3),Plöchel et al.)

	//Umrechnung von µg in kg
/*	if(pPStress->vO3DefenseEff > (float)0.0)
		pPStress->vLeafDefCont -= 
		(float)(1e-9)*pPStress->vO3Detox*(pBiom->fLeafWeight + pPStress->vLeafDefCont + EPS)/pPStress->vO3DefenseEff;

	if(pPStress->vLeafDefCont<(float)0.0)
		pPStress->vLeafDefCont = (float)0.0;

	//concentration
	if((pBiom->fLeafWeight+pPStress->vLeafDefCont+EPS)>(float)0.0)
		pPStress->vLeafDefConc = pPStress->vLeafDefCont/(pBiom->fLeafWeight + pPStress->vLeafDefCont + EPS);
	else
		pPStress->vLeafDefConc = (float)0.0;

	if(pPStress->vLeafDefConc<(float)0.0)
		pPStress->vLeafDefConc =(float)0.0;
  */

	//Cumulative leaf internal ozone concentration
    // jetzt pPStress->vCritO3Conc=0.0 19/10/2006
	//pPStress->vO3LeafConcCum += (float)max(pPStress->vO3ConcLeaf,pPStress->vCritO3Conc)*DeltaT;
	//Cumulative leaf internal ozone concentration exceeding fCritO3Conc 
	//if(pPStress->vO3ConcLeaf-pPStress->vCritO3Conc > (float)0.0)
	//	pPStress->vO3LeafConcCumCrit += (pPStress->vO3ConcLeaf-pPStress->vCritO3Conc)*DeltaT;

     //pPStress->vO3LeafConcCumCrit += pPStress->vO3ConcLeaf*DeltaT;

    pPStress->vO3LeafConcCrit     = pPStress->vO3ConcLeaf;  //alles was nach Detoxifikation durch Antioxidantien noch übrig ist
       
    if(pPDev->iStagePlatho<2)
        pPStress->vO3LeafConcCumCrit = 0.0;
    else
        pPStress->vO3LeafConcCumCrit += pPStress->vO3LeafConcCrit;   // µg/kg(TS)

    pPStress->vO3LeafConcCum += (pPStress->vO3ConcLeaf+pPStress->vO3Detox);     // µg/kg(TS)

/*    if(pPStress->vO3LeafConcCum>(float)0.0)
	{
     // pPStress->vO3StressIntensity = (float)min(1.0,pPStress->vO3LeafConcCumCrit/(pPStress->vO3LeafConcCum+EPS));
        if(pPStress->vPotO3DetoxCum>(float)0.0)
            pPStress->vO3StressIntensity = (float)min(1.0,pPStress->vO3LeafConcCumCrit/(pPStress->vPotO3DetoxCum+EPS));
        else
           pPStress->vO3StressIntensity = (float)1.0;
    //  pPStress->vO3StressIntensity = pPStress->vO3ConcLeaf/(pPStress->vO3ConcLeaf+pPStress->vO3Detox+EPS);
	}
    else
        pPStress->vO3StressIntensity = (float)0.0;
 */

    //nach Stressreaktion (Photosynthese) kein blattinternes O3 mehr vorhanden!
    pPStress->vO3ConcLeaf = 0.0;

    pPStress->vO3UptakeDay += pPStress->vO3Uptake;   //µg
    pPStress->vO3UptakeCum += pPStress->vO3Uptake;   //µg 

/*    if(pPStress->vO3UptakeCum>pPStress->vPotO3DetoxCum)
        pPStress->vO3StressIntensity = (float)min(1.0,max(0.0,(pPStress->vO3UptakeCum-pPStress->vPotO3DetoxCum))/(pPStress->vO3UptakeCum+EPS));
    else
        pPStress->vO3StressIntensity = (float)0.0;
 */ 

    if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3))
    {
        //pPStress->vO3StressIntensity = min(1.0,pPStress->vO3LeafConcCumCrit/(pPStress->vO3LeafConcCum+EPS));
          pPStress->vO3StressIntensity = min(1.0,pPStress->vO3LeafConcCumCrit/(pPStress->vPotO3DetoxCum+EPS));

        /*  if(pPDev->iStagePlatho<2)
                pPStress->vO3StressIntensity = 0.0;
            else
                pPStress->vO3StressIntensity = min(1.0,pPStress->vO3LeafConcCumCrit/(pPStress->vPotO3DetoxCum+EPS));
    */
    }
    }
 
    return 1;
}
