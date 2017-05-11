//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"


int PotentialTranspiration_PLATHO(EXP_POINTER2,int iPlant);
int ActualTranspiration_PLATHO(EXP_POINTER2,double PlantDens,int iPlant);

extern int CalcLeafInternalO3(double TimeDay, double TimeAct, double DeltaT, int iPlant, double PhiCH2O, 
				   PPLANT pPlant, PWEATHER pWeather,PLOCATION pLo, PTIME pTi);

extern int	  GetPlantNeighbours(PPLANT, int, int*, int*, int*, int*);
extern double SectorCompFactor(double r_c, double r_nbg);
extern int NewDay(PTIME);


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	PotentialTranspiration
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int PotentialTranspiration_PLATHO(EXP_POINTER2,int iPlant)
	{
  	 PPLATHOPLANT   pPPlant = pPlathoPlant[iPlant];
     double DeltaT = (double)pTi->pTimeStep->fAct;
     double dRelIntercept;
     double dPotETdt = 0.0;
	 double dPotEVdt = 0.0; 
	 double dTPotDay = 0.0;
     double dTimeSunrise, dTimeSunset, dDaylengthPs, T1, T2;
     double dFracET = 0.0;
     double TimeDay = (double)pTi->pSimTime->fTimeDay;//Tageszeit

     extern double Daylength(double vLatitude, int nJulianDay, int nID);


     if (NewDay(pTi))
     {
         pPlathoPlant[iPlant]->pPlantStress->dWaterShortageDay = 0.0;

     //potential transpiration per ground area:
        //02.07.2009 
        // pPlant->pPltWater->fPotTranspDay = (float)max(0.0,pWa->fPotETDay - pWa->pEvap->fPotDay);
        // pPlathoAllPlants->dPotTranspDay = pPlant->pPltWater->fPotTranspDay/pPlathoAllPlants->dAbvgSpaceOccupation * 1000.0; //cm^3/m^2(plant)

        dTPotDay = (float)max(0.0,pWa->fPotETDay - pWa->pEvap->fPotDay);
     //potential transpiration per plant covered area:
        pPlathoAllPlants->dPotTranspDay = dTPotDay/pPlathoAllPlants->dAbvgSpaceOccupation * 1000.0; //cm^3/m^2(plant)
     }

    /////////////////////////////////////////////////////////////////////////////////
    // Tageslänge, Beginn, Ende und Länge der photosynthetisch aktiven Tageszeit   //
    /////////////////////////////////////////////////////////////////////////////////

    if(pPlathoModules->iFlagLightInterception == 2)  //constant photoperiod
        dDaylengthPs = pPlathoScenario->vPhotoperiod;
    else
	    dDaylengthPs = Daylength(pLo->pFarm->fLatitude, pTi->pSimTime->iJulianDay,3);

	dTimeSunrise = (12.0 - 0.5*dDaylengthPs)/24.0;
	dTimeSunset = 1.0 - dTimeSunrise;

    //Aufteilung von ETpot und EVpot entsprechend der Tageslänge
    if((TimeDay < dTimeSunrise)||(TimeDay-DeltaT > dTimeSunset))
    {
        dFracET = 0.0;
    }
    else
    {
        T1 = max(0.0,(TimeDay - DeltaT - dTimeSunrise)/(dTimeSunset-dTimeSunrise));
        T2 = min(1.0,(TimeDay - dTimeSunrise)/(dTimeSunset-dTimeSunrise));

        dFracET = (cos(PI*T1) - cos(PI*T2))/2.0;
    }

    dPotETdt = pWa->fPotETDay * dFracET;

    dPotEVdt = pWa->pEvap->fPotDay * dFracET;



     //potential transpiration per ground area:
     //02.07.2009: jetzt aus Tageslänge
     //pPlant->pPltWater->fPotTranspdt = (float)max(0.0,pWa->fPotETdt - pWa->pEvap->fPotR  * DeltaT);   //mm
       pPlant->pPltWater->fPotTranspdt = (float)max(0.0,dPotETdt - dPotEVdt);   //mm

     //potential transpiration per plant covered area:
     pPlathoAllPlants->dPotTranspR = pPlant->pPltWater->fPotTranspdt/pPlathoAllPlants->dAbvgSpaceOccupation * 1000.0; //cm^3/m^2(plant)

     //with interception:
     //if(pPlant->pPltWater->fPotTranspDay > (float)1e-6)
     if(dTPotDay > (float)1e-6)
         //dRelIntercept = pPlant->pPltWater->fInterceptDay/pPlant->pPltWater->fPotTranspDay;
         dRelIntercept = pPlant->pPltWater->fInterceptDay/dTPotDay;
     else
         dRelIntercept = 0.0;
	
     if(dRelIntercept > 1.0) dRelIntercept = 1.0;
	
	 //pPlant->pPltWater->vPotTranspdt *= ((float)1.0-dRelIntercept);
     
     //pPlant->pPltWater->vPotTranspdt = (float)max(0.0,pWa->fPotETDay - pWa->pEvap->fPotDay-pPlant->pPltWater->fInterceptDay) * DeltaT;
	
	return 1;
	}


int ActualTranspiration_PLATHO(EXP_POINTER2, double PlantDens, int iPlant)
{
	int		NLayer, L, ll, iSector;
	double	alpha, vContAct ;    //vCover
	double	TRWU, WUF;

    double   vActLayUptakedt, vActLayWatUpt, vFracActLayWatUpt_Plant;  //  vPotTranspdt
    double   vActSecWatUpt_Plant[4];


	double	rtlEff[4];
	double	rtlEfvRight, rtlEfvLower, rtlEfvLeft, rtlEfvUpper;
    double  radius_c,r_nbg_right,r_nbg_lower,r_nbg_left,r_nbg_upper;
	double	vMaxWuptR, vUpperMaxWuptR, vLeftMaxWuptR, vRightMaxWuptR, vLowerMaxWuptR;
	double	DeltaT = (double)pTi->pTimeStep->fAct;
    double  dWaterAv;


	int		iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;

	extern double PhiRootPathogenes(int iPlant);


	PPLANT				pPlantLeft, pPlantRight, pPlantUpper, pPlantLower;
	PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
	PPLATHOPARTITIONING	pPPart = pPlathoPlant[iPlant]->pPlathoPartitioning;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOROOTLAYER	pPRL	= pPMorph->pPlathoRootLayer;
    PPLANTSTRESS		pPStress    = pPlathoPlant[iPlant]->pPlantStress;

    PGENOTYPE			pGen	= pPlant->pGenotype;
	PBIOMASS			pBiom	= pPlant->pBiomass;
	PPLTNITROGEN		pPltN	= pPlant->pPltNitrogen;
	PROOT				pRT		=pPlant->pRoot;  
    PPLTWATER			pPltW	=pPlant->pPltWater;

	PLAYERROOT 	pLR = pPlant->pRoot->pLayerRoot;
	PWLAYER		pWL = pWa->pWLayer->pNext;
	PSLAYER		pSL = pSo->pSLayer->pNext;
	PSWATER		pSWL= pSo->pSWater->pNext;


	pLR	 = pPlant->pRoot->pLayerRoot;
        for (L=1;L<=pSo->iLayers-2;L++)
        {
            pLR->fActLayWatUpt = (float)0.0;
      	    pLR =pLR ->pNext;
        }
            
	if(pPMorph->vRootZoneArea == 0.0)
    {
        pPlant->pPltWater->fActUptakedt = (float)0.0;
        pPlant->pPltWater->fActTranspdt = (float)0.0;
	    pPlant->pRoot->fUptakeR = (float)0.0;
        pPStress->dWaterShortage = 1.0;

        return 1;
    }

    if(pPlant->pPltWater->fPotTranspdt == (float)0.0)
    {
        pPlant->pPltWater->fActUptakedt = (float)0.0;
        pPlant->pPltWater->fActTranspdt = (float)0.0;
	    pPlant->pRoot->fUptakeR = (float)0.0;
        pPPart->vPotTranspR = 0.0;
        pPPart->vActTranspR = 0.0;
        pPStress->dWaterShortage = 1.0;

        goto Label_500;
    }

	pLR	 = pPlant->pRoot->pLayerRoot;

	pPlant->pPltWater->fActTranspdt = (float)0.0;
	NLayer=pSo->iLayers-2;

	TRWU = 0.0;

	// Vier nächste Nachbarn
	GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
											&iPlantUpper, &iPlantLower);

	pPlantLeft	= 	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft;
	pPlantRight	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight;
	pPlantUpper	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper;
	pPlantLower	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower ;




	//Reduktion des Wasseraufnahmevermögens der Wurzeln
	//bei Schädigung durch Wurzelpathogene:

    vMaxWuptR	    = pPlant->pGenotype->fMaxWuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlant) : 1.0);
	vUpperMaxWuptR	= pPlantUpper->pGenotype->fMaxWuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantUpper) : 1.0);
	vLeftMaxWuptR	= pPlantLeft->pGenotype->fMaxWuptRate  * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantLeft) : 1.0);
	vRightMaxWuptR	= pPlantRight->pGenotype->fMaxWuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantRight) : 1.0);
	vLowerMaxWuptR	= pPlantLower->pGenotype->fMaxWuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantLower) : 1.0);


    radius_c    = 0.5*pPMorph->vRootZoneDiameter;
    r_nbg_right = 0.5*pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneDiameter;
    r_nbg_lower = 0.5*pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneDiameter;
    r_nbg_left  = 0.5*pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneDiameter;
    r_nbg_upper = 0.5*pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneDiameter;


	for (L=1;L<=NLayer;L++)
		{
			//nach Feddes: alpha(Psi) (Wasserspannung statt Wassergehalt):

			//geht nur mit Richardsgleichungs-Modell
/*
			if(((pWL->fMatPotAct>0))||(pWL->fMatPotAct<Psi3))
				alpha = (float)0.0;
			else if((pWL->fMatPotAct<0)&&(pWL->fMatPotAct>Psi1))
				alpha = pWL->fMatPotAct/Psi1;
			else if((pWL->fMatPotAct<Psi1)&&(pWL->fMatPotAct>Psi2))
				alpha = 1.0;
			else
				alpha =  (pWL->fMatPotAct - Psi3)/(Psi2-Psi3);
		
*/

		// alpha(theta)			
		if((pWL->fContAct>=pSWL->fContSat)||(pWL->fContAct<(float)0.5*pSWL->fContPWP))
				alpha = 0.0;
			else if((pWL->fContAct<pSWL->fContSat)&&(pWL->fContAct>0.9*pSWL->fContSat))
				alpha = 1.0-(pWL->fContAct - 0.9*pSWL->fContSat)/
												(pSWL->fContSat-0.9*pSWL->fContSat);
			else if((pWL->fContAct<=0.9*pSWL->fContSat)&&(pWL->fContAct>pSWL->fContFK))
				alpha = 1.0;
			else
				alpha =  (pWL->fContAct - (float)0.5*pSWL->fContPWP)/(pSWL->fContFK-(float)0.5*pSWL->fContPWP);


    if(pPlathoModules->iFlagSectors==1)
    {
		// COMPETITION FOR WATER: /////////////////////////////////////////////

		rtlEfvUpper = 0.0;
		rtlEfvLeft =  0.0;
		rtlEfvRight = 0.0;
		rtlEfvLower = 0.0;


		if(pPMorph->BlwgCoeffComp[3]>0.0)
		{
			//PLAYERROOT		 pUpperLR  = pPlantUpper->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pUpperPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pUpperPRL = pUpperPRL->pNext;
				ll++;
			}

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
			rtlEfvUpper=pPMorph->BlwgCoeffComp[3]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneArea
				*pUpperPRL->vRootArea*vUpperMaxWuptR*1e4;
		}

		if(pPMorph->BlwgCoeffComp[2]>0.0)
		{
			//PLAYERROOT pLeftLR = pPlantLeft->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pLeftPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pLeftPRL = pLeftPRL->pNext;
				ll++;
			}

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
			rtlEfvLeft=pPMorph->BlwgCoeffComp[2]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneArea
				*pLeftPRL->vRootArea*vLeftMaxWuptR*1e4;
		}

		if(pPMorph->BlwgCoeffComp[0]>0.0)
		{
			//PLAYERROOT pRightLR = pPlantRight->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pRightPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pRightPRL = pRightPRL->pNext;
				ll++;
			}

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
			rtlEfvRight=pPMorph->BlwgCoeffComp[0]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneArea
				*pRightPRL->vRootArea*vRightMaxWuptR*1e4;
		}

		if(pPMorph->BlwgCoeffComp[1]>0.0)
		{
			//PLAYERROOT pLowerLR = pPlantLower->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pLowerPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pLowerPRL = pLowerPRL->pNext;
				ll++;
			}

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
			rtlEfvLower=pPMorph->BlwgCoeffComp[1]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneArea
				*pLowerPRL->vRootArea*vLowerMaxWuptR*1e4;
		}

		if(pPRL->vRootArea==0.0)
			pPRL->vWCompCoeff = 0.0;
		else
			pPRL->vWCompCoeff = (pPRL->vRootArea*vMaxWuptR*1e4
				/(pPRL->vRootArea*vMaxWuptR*1e4 +rtlEfvUpper+rtlEfvLeft+rtlEfvRight+rtlEfvLower));
    } //end iFlagSectors = 1
 

        
     if(pPlathoModules->iFlagSectors==2)
     {
     
        // COMPETITION FOR WATER: /////////////////////////////////////////////

		for(iSector=0;iSector<4;iSector++)
            rtlEff[iSector] =0.0;


		if(pPMorph->BlwgCoeffComp[3]>0.0)
		{
			//PLAYERROOT		 pUpperLR  = pPlantUpper->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pUpperPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pUpperPRL = pUpperPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[3] = SectorCompFactor(radius_c, r_nbg_upper);

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
            rtlEff[3]=pPMorph->SectorCompCoeff_blwg[3]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneArea
				    *pUpperPRL->vRootArea*vUpperMaxWuptR*1e4;
		}

		if(pPMorph->BlwgCoeffComp[2]>0.0)
		{
			//PLAYERROOT pLeftLR = pPlantLeft->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pLeftPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pLeftPRL = pLeftPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[2] = SectorCompFactor(radius_c, r_nbg_left);

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
            rtlEff[2]=pPMorph->SectorCompCoeff_blwg[2]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneArea
				    *pLeftPRL->vRootArea*vLeftMaxWuptR*1e4;
		}

		if(pPMorph->BlwgCoeffComp[0]>0.0)
		{
			//PLAYERROOT pRightLR = pPlantRight->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pRightPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pRightPRL = pRightPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[0] = SectorCompFactor(radius_c, r_nbg_right);

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
            rtlEff[0]=pPMorph->SectorCompCoeff_blwg[0]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneArea
				    *pRightPRL->vRootArea*vRightMaxWuptR*1e4;
		}

		if(pPMorph->BlwgCoeffComp[1]>0.0)
		{
			//PLAYERROOT pLowerLR = pPlantLower->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pLowerPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pLowerPRL = pLowerPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[1] = SectorCompFactor(radius_c, r_nbg_lower);

			//effektive Wasseraufnahmekapazität [cm^3/d] = Wurzeloberfläche [m^2] * MaxWUpt [cm^3/cm^2/d] * 10^4 [cm^2/m^2]
            rtlEff[1]=pPMorph->SectorCompCoeff_blwg[1]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneArea
				    *pLowerPRL->vRootArea*vLowerMaxWuptR*1e4;
		}

        for(iSector=0;iSector<4;iSector++)
        {
            if(pPRL->apRootSector[iSector]->vRootArea==0.0)
			    pPRL->apRootSector[iSector]->vWCompCoeff = 0.0;
		    else
			    pPRL->apRootSector[iSector]->vWCompCoeff = pPRL->apRootSector[iSector]->vRootArea*vMaxWuptR*1e4
				    /(pPRL->apRootSector[iSector]->vRootArea*vMaxWuptR*1e4 + rtlEff[iSector]);
        }
     } //end iFlagSectors=2

     if(pPlathoModules->iFlagSectors==1)
     {
    	// Potential water uptake from plant layer L (all plants)
		// (=Limitierung durch Wurzelwiderstand)
		// PotWupt[mm] = MaxWuptR[cm^3/cm^2/d]*RootArea[m^2]*dt[d]/PlantArea[m^2]*alpha[-]
			
		if(pPMorph->vRootZoneArea==0.0)
			pLR->fPotLayWatUpt = (float)0.0;
		else
        {
			 //in cm^3
            pLR->fPotLayWatUpt = (float)((pPRL->vRootArea*vMaxWuptR*1e4
				  +rtlEfvUpper+rtlEfvLeft+rtlEfvRight+rtlEfvLower)* alpha * DeltaT);

            //in mm
/*			pLR->fPotLayWatUpt = (pPRL->vRootArea*vMaxWuptR
				  +rtlEfvUpper+rtlEfvLeft+rtlEfvRight+rtlEfvLower) * DeltaT/
							pPMorph->vRootZoneArea * alpha * (float)10.0; */ 
        }
     }//end iFlagSectors = 1

     if(pPlathoModules->iFlagSectors==2)
     {
		// Potential water uptake from plant layer L, sector iSector (all plants)
		// (=Limitierung durch Wurzelwiderstand)
		// PotWupt[mm] = MaxWuptR[cm^3/cm^2/d]*RootArea[m^2]*dt[d]/PlantArea[m^2]*alpha[-]
		pLR->fPotLayWatUpt = (float)0.0;

		for(iSector=0;iSector<4;iSector++)
        {
            if(pPMorph->vRootZoneArea==0.0)
			    pPRL->apRootSector[iSector]->vPotSecWatUpt = 0.0;
		    else
            {
			    //in cm^3
                pPRL->apRootSector[iSector]->vPotSecWatUpt = 
                (pPRL->apRootSector[iSector]->vRootArea*vMaxWuptR*1e4 + rtlEff[iSector]) * alpha * DeltaT;

			    //in mm
                /*pPRL->apRootSector[iSector]->vPotSecWatUpt = 
                (pPRL->apRootSector[iSector]->vRootArea*vMaxWuptR*(float)1e4 + (float)rtlEff[iSector]) * DeltaT/
							    pPMorph->vRootZoneArea * alpha * (float)10.0; */
            }

            pLR->fPotLayWatUpt += (float)pPRL->apRootSector[iSector]->vPotSecWatUpt; //[cm^3]
        }//end Sectors
     }//end iFlagSectors = 2
        
     //potential water uptake per single plant area (all plants) [cm^3]
 		TRWU += pLR->fPotLayWatUpt;  //[cm^3], all plants

		pLR  = pLR->pNext;
		pWL  = pWL->pNext;
	    pSWL = pSWL->pNext;
		pSL  = pSL->pNext;
		pPRL = pPRL->pNext;
	}

    //Potential transpiration from plant area [cm^3]:
    // "->fPotTranspdt" [mm] --> pot. transpiration of the whole ground area
    // "vPotTranspdt" [cm^3] --> pot. transpiration of the plant covered soil area

    //Uptake per single plant area (all plants): [cm^3]
    pPPart->vPotTranspR   = pPlathoAllPlants->dPotTranspR   * pPMorph->vCrownArea; //cm^3
    pPPart->vPotTranspDay = pPlathoAllPlants->dPotTranspDay * pPMorph->vCrownArea; //cm^3
  //  pPPart->vPotTranspR = pPlathoAllPlants->dPotTranspR * pPMorph->vRootZoneArea/pPlathoAllPlants->dPlantCoveredSoil; //cm^3
  //  vPotTranspdt = pPlant->pPltWater->fPotTranspdt * pPMorph->vRootZoneArea * 1000.0; //cm^3



//	if((TRWU != (float)0.0)&&(pPlant->pPltWater->vPotTranspdt<TRWU))
//    	WUF = pPlant->pPltWater->vPotTranspdt/TRWU; //Transpiration!
	if((TRWU > 0.0)&&(pPPart->vPotTranspR<TRWU))
    	WUF = pPPart->vPotTranspR/TRWU; //Transpiration!
    else											
        WUF = 1.0;

 	pPlant->pPltWater->fActUptakedt=(float)0.0;

    dWaterAv = 0.0; //verfügbares Wasser

	pSL	 = pSo->pSLayer->pNext;
	pSWL = pSo->pSWater->pNext;
	pWL  = pWa->pWLayer->pNext; 
	pLR	 = pPlant->pRoot->pLayerRoot;
	pPRL = pPMorph->pPlathoRootLayer;
    
    for (L=1;L<=pSo->iLayers-2;L++)
    {
        if(pLR->fPotLayWatUpt>(float)0.0)
        {
            if(pPlathoModules->iFlagSectors==1)
            {
   		        // alle Pflanzen [cm^3]
                vActLayWatUpt 	=  pLR->fPotLayWatUpt*WUF;
       		    
                //Anteil, der auf die betrachtete Pflanze fällt [cm^3]
                vFracActLayWatUpt_Plant =  pLR->fPotLayWatUpt * WUF * pPRL->vWCompCoeff;
            }

            if(pPlathoModules->iFlagSectors==2)
            {
                vActLayWatUpt = 0.0;	
		        for(iSector=0;iSector<4;iSector++)
                {
                    // alle Pflanzen [cm^3]
                    pPRL->apRootSector[iSector]->vActSecWatUpt 	=  pPRL->apRootSector[iSector]->vPotSecWatUpt*WUF;

                    vActLayWatUpt 	+= pPRL->apRootSector[iSector]->vActSecWatUpt;

                    //Anteil, der auf die betrachtete Pflanze fällt:
                    vActSecWatUpt_Plant[iSector] =  pPRL->apRootSector[iSector]->vActSecWatUpt * pPRL->apRootSector[iSector]->vWCompCoeff;
                }//end Sectors
            }

            dWaterAv +=  (pWL->fContAct - pSWL->fContPWP)*pPMorph->vRootZoneArea * pSL->fThickness * 1000.0;// in mm/mm

		    vContAct  = pWL->fContAct; // in mm/mm = cm^3/cm^3
            vContAct -= vActLayWatUpt/(pPMorph->vRootZoneArea * pSL->fThickness * 1000.0);

    //       vContAct  = pWL->vContAct; // in mm/mm
    //		vContAct -= vActLayWatUpt*vCover/pSL->fThickness;
    							
		    if (vContAct<pSWL->fContPWP) //verfügbares Bodenwasser limitiert Transpiration
			    pLR->fActLayWatUpt = (float)(max(0.0,pWL->fContAct-pSWL->fContPWP)*(pPMorph->vRootZoneArea * pSL->fThickness * 1000.0)); //[cm^3]
    //			pLR->fActLayWatUpt = (float)max(0.0,pWL->vContAct-pSWL->fContPWP)*pSL->fThickness/(vCover+EPSILON);
            else
                pLR->fActLayWatUpt = (float)vActLayWatUpt;   // [cm^3]

            if(pPlathoModules->iFlagSectors==1)
            {
                //Anteil, der auf die betrachtete Pflanze fällt [cm^3]
		        if(pLR->fActLayWatUpt > (float)0.0)
                {
                    pLR->fActLayWatUpt = (float)(pLR->fActLayWatUpt
                                    /vActLayWatUpt*vFracActLayWatUpt_Plant);//*vPlantCoveredSoil);
                }
            }
            
            
            if(pPlathoModules->iFlagSectors==2)
            {
		        vActLayUptakedt = 0.0;
                if(pLR->fActLayWatUpt > (float)0.0)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
                    //Anteil, der auf die betrachtete Pflanze fällt:
		                vActLayUptakedt += pLR->fActLayWatUpt/vActLayWatUpt*vActSecWatUpt_Plant[iSector];//*vPlantCoveredSoil;
                    }//end Sectors
                }
                pLR->fActLayWatUpt = (float)(vActLayUptakedt);
            }
        }
        else
        {
            pLR->fActLayWatUpt = (float)0.0;
        }
        
        //single plant:
        pPlant->pPltWater->fActUptakedt += pLR->fActLayWatUpt; //[cm^3]

      	pSL =pSL ->pNext;
      	pSWL=pSWL->pNext;
      	pWL=pWL->pNext;
      	pLR =pLR ->pNext;
		pPRL = pPRL->pNext;
	}

    //all identical plants
    //in mm = l/m^2 (soil)
    pPlant->pPltWater->fActUptakedt *= (float)(1e-3*PlantDens);// /pPMorph->vRootZoneArea);                          
     
    //in mm = l/m^2(soil)
   if (pPPart->vPotTranspR*1e-3*PlantDens >= pPlant->pPltWater->fActUptakedt)
        pPlant->pPltWater->fActTranspdt=pPlant->pPltWater->fActUptakedt;
    else
        pPlant->pPltWater->fActTranspdt=(float)(pPPart->vPotTranspR*1e-3*PlantDens);


   if(pPlant->pPltWater->fActTranspdt<=(float)0.0) pPlant->pPltWater->fActTranspdt = (float)0.0;
        
	pPPart->vActTranspR = pPlant->pPltWater->fActTranspdt*1000.0/PlantDens; //cm^3
    
    pPlant->pRoot->fUptakeR = pPlant->pPltWater->fActTranspdt/pTi->pTimeStep->fAct;

    //water shortage:
    if(pPlathoModules->iFlagH2OEffectPhot==1)//no water deficit effect on photosynthesis
    {
	    //pPlant->pPltWater->fStressFacPhoto = (float)1.0; 
	    pPStress->dWaterShortage = 1.0; 
    }
    else //water deficit effect on photosynthesis included
    {
	    // Am Tag des Feldaufgangs kein Wasserstress                           

	    //02.07.2009 if((pPlant->pDevelop->iDayAftEmerg <= 1)||(pPlant->pPltWater->fPotTranspDay<=(float)0.0))
	    if((pPlant->pDevelop->iDayAftEmerg <= 1)||(pPlant->pPltWater->fPotTranspdt<=(float)0.0)||(pPPart->vPotTranspR<=0.0))
		    pPStress->dWaterShortage = 1.0;
	    else
        {
            /*    if((pTi->pSimTime->fTimeDay < (float)0.33)||(pTi->pSimTime->fTimeDay > (float)0.83))
		        pPStress->dWaterShortage = min(1.0,min(TRWU,dWaterAv)/(pPPart->vPotTranspDay*DeltaT));
            else
		        pPStress->dWaterShortage = min(1.0,pPPart->vActTranspR/pPPart->vPotTranspR);
            */

            //02.07.2009:
            // Transpiration folgt jetzt Tagesgang, deshalb keinen Trick mehr für fTimeDay <= (float)0.3 
            pPStress->dWaterShortage = min(1.0,pPPart->vActTranspR/pPPart->vPotTranspR);
        }
    }     

Label_500:

    pPStress->dWaterShortageDay += DeltaT*pPStress->dWaterShortage;


	//Ozone Uptake:
    // O3-stress following van Ojien --> jetzt Aufruf in Platho_Biomass.c -> Growth
	if((pPlathoModules->iFlagO3 == 1)||(pPlathoModules->iFlagO3 == 2)) 
    {
        CalcLeafInternalO3(pTi->pSimTime->fTimeDay, pTi->pSimTime->fTimeAct, pTi->pTimeStep->fAct, iPlant, 0, pPlant, pCl->pWeather,pLo,pTi);

    }

	return 1;
}




