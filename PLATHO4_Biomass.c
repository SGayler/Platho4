//############################################################################################################
//	MODULE - PLATHO.C
//
//	Author:	 Sebastian Gayler
//
//############################################################################################################

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//  INCLUDE FILES, DEFINED VARIABLES AND TYPES 
//------------------------------------------------------------------------------------------------------------
//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"

int BiomassGrowth_PLATHO(EXP_POINTER2,int iPlant);
int Growth(EXP_POINTER2, int);

extern int   PlantNitrogenDemand_PLATHO(EXP_POINTER2, int iPlant);
extern int	 CalculatePotentialNitrogenUptake(EXP_POINTER2, int iPlant);
extern double CalcPotNitrogenTransloc(PPLANT pPlant, int iPlant, double DeltaT);
extern double Daylength(double,int,int);
extern double HourlyTemperature_PLATHO(double,double,double,double,double,int);
extern	int NewDay(PTIME pz);
extern	int EndDay(PTIME pz);
extern double AVGENERATOR(double vInput, VRESPONSE* pvResp);
extern int CalcLeafInternalO3(double TimeDay, double TimeAct, double DeltaT, int iPlant, double PhiCH2O,
				   PPLANT pPlant, PWEATHER pWeather,PLOCATION pLo, PTIME pTi);


/**************************************************************************************
*
*	Biomasse Wachstum:
* 
*	Retranslokation, Erhaltungsatmung, Wachstumsatmung und Substanzaufbau.
*
**************************************************************************************/

int BiomassGrowth_PLATHO(EXP_POINTER2,int iPlant)
{
	//Growth_Old(exp_p2, iPlant);
	Growth(exp_p2, iPlant);

	return 1;
}


int  Growth(EXP_POINTER2,int iPlant)
{
	double	MaintenanceRespiration(EXP_POINTER2,int iPlant, double TempAct);
	double	GrowthRespiration(EXP_POINTER2,int iPlant);
	double	GetCostsForGrowth(EXP_POINTER2, int iPlant);
	int		GetPotentialGrowthRates(EXP_POINTER2, int iPlant, double TempAct);

	double PhiCH2O, PhiN, vCH2OAvailable;//PsiN
	double vGammaC,vGammaN;
	double vTotalCosts;
	double DeltaT = pTi->pTimeStep->fAct;
	double TimeDay = pTi->pSimTime->fTimeDay;		// Tageszeit
	double TempAct, vDaylengthPs;
	double vResGrwNFactor, vAssReserves, vStarchReserves, vMinStarchFactor, vMaxStemReserves;//fStarchGrowR
	double vLimitAssReserves, vLimitStarchReserves;
	double help1;// fRelLeafN;
    double vDemand1, vDemand2, PhiCH2O_1, PhiCH2O_2, vSurplusForDefense;

	double corrfM,  corrfC; //corrfD,corrfN;

	PBIOMASS	pBiom = pPlant->pBiomass;
	PPLTCARBON	pCarb = pPlant->pPltCarbon;
	PGENOTYPE	pGen  = pPlant->pGenotype;
	PDEVELOP	pDev  = pPlant->pDevelop;
	PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;

	PPLATHOPARTITIONING pPPart		= pPlathoPlant[iPlant]->pPlathoPartitioning;
	PPLATHOBIOMASS		pPBiom		= pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLATHOMORPHOLOGY	pPMorph		= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOGENOTYPE		pPGen		= pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHODEVELOP		pPDev		= pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHONITROGEN		pPPltN		= pPlathoPlant[iPlant]->pPlathoNitrogen;
	PPLANTSTRESS		pPStress    = pPlathoPlant[iPlant]->pPlantStress;
	PGROWTHEFFICIENCY	pPGrwEff	= pBiochemistry->pGrowthEfficiency;
	PCO2REQUIRED		pPCO2Req	= pBiochemistry->pCO2Required;


	pBiom->fStemReserveGrowRate = (float)0.0;
	pBiom->fReserveTransRate	= (float)0.0;
	pPBiom->vStarchGrowR		= 0.0;

	pPPart->dCostsForMaintenance	= 0.0;
	pPPart->dCostsForBiomassGrowth	= 0.0;
	pPPart->dCostsForDefense    	= 0.0;

	vTotalCosts = 0.0;

		
	// Temperatur:

	if(pTi->pTimeStep->fAct == (float)1.0)
			TempAct = (pCl->pWeather->fTempAve>(float)-99)?
			pCl->pWeather->fTempAve : (pCl->pWeather->fTempMin+pCl->pWeather->fTempMax)/(float)2.0;
	else //kleine Zeitschritte
	{
		vDaylengthPs = Daylength(pLo->pFarm->fLatitude, pTi->pSimTime->iJulianDay,3);
		
		//Aktuelle Temperatur:
		if(((int)pTi->pSimTime->fTimeAct==0)||(pPlant->pDevelop->iDayAftEmerg==1))
			TempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pCl->pWeather->fTempMax,
							pCl->pWeather->fTempMin, pCl->pWeather->pNext->fTempMin, vDaylengthPs,1);

		else if(pCl->pWeather->pNext==NULL)
			TempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pCl->pWeather->fTempMax,
							pCl->pWeather->fTempMin, pCl->pWeather->fTempMin, vDaylengthPs,0);

		else
			TempAct = HourlyTemperature_PLATHO(TimeDay*24.0, pCl->pWeather->fTempMax,
							pCl->pWeather->fTempMin, pCl->pWeather->pNext->fTempMin, vDaylengthPs,0);
	}



/////////////////////////////////////////////////////////////////////////
// ASSIMILATE PARTITIONING
/////////////////////////////////////////////////////////////////////////

	TREES
	{
		// ab Stadium 3 wird ein Teil des Assimilatpools (Glukose) und der 
		// C-Reserven (Stärke) für den Blattaustrieb im Nächsten Jahr zurückgehalten
		// und steht nicht für Wachstum zur Verfügung

		BEECH
		{
			vLimitAssReserves = 0.03;	    //0.02,0.05	
			vLimitStarchReserves = 0.03;     //0.02,0.05	
		}
		SPRUCE
		{
			vLimitAssReserves = 0.01;		//0.03		
			vLimitStarchReserves = 0.01;   	//0.02	
		}
		APPLE
		{
			vLimitAssReserves = 0.02;      	//0.02	
			vLimitStarchReserves = 0.02;     //0.01	
		}


		switch(pPDev->iStagePlatho)
		{
/*		case 0:
		case 4:
		case 5:
		case 6:
			vAssReserves    = fLimitAssReserves*pBiom->fWoodWeight;
			vStarchReserves = fLimitStarchReserves*pBiom->fWoodWeight;
			break;

		case 1:
		case 2:
			vAssReserves = (float)0.0;
			vStarchReserves = (float)0.0;
			break;

		case 3:
			vAssReserves = (float)max(0.0,fLimitAssReserves * pBiom->fWoodWeight * (pDev->fDevStage-3.0)/2.0); 
			vStarchReserves = (float)max(0.0,fLimitStarchReserves * pBiom->fWoodWeight * (pDev->fDevStage-3.0)/2.0); 
			break;
*/
		case 0:
			vAssReserves = vLimitAssReserves * pBiom->fWoodWeight;
			vStarchReserves = vLimitStarchReserves * pBiom->fWoodWeight;
            break;
		case 1:
		case 2:
			vAssReserves = 0.0;
			vStarchReserves = 0.0;
			break;
		
		case 3:
		case 4:	
			vAssReserves = max(0.0,vLimitAssReserves * pBiom->fWoodWeight * (pDev->fDevStage-3.0)/2.0); 
			vStarchReserves = max(0.0,vLimitStarchReserves * pBiom->fWoodWeight * (pDev->fDevStage-3.0)/2.0); 
			break;

		case 5:
		case 6:
			vAssReserves = vLimitAssReserves * pBiom->fWoodWeight;
			vStarchReserves = vLimitStarchReserves * pBiom->fWoodWeight;
			break;

		
		}
	}
	else
	{
		vAssReserves = 0.0;
		vStarchReserves = 0.0;
	}

	if(vAssReserves > pCarb->fCAssimilatePool)
		vAssReserves = pCarb->fCAssimilatePool;
	 
	if(vStarchReserves > pBiom->fStemReserveWeight)
		vStarchReserves = pBiom->fStemReserveWeight;
	 
		
//	vResGrwNFactor = (pPltN->fStemActConc-pPltN->fStemMinConc)/(pPltN->fStemOptConc-pPltN->fStemMinConc);


	vResGrwNFactor = 1.0;
//	vResGrwNFactor = (float)(pow(vResGrwNFactor,1.0));

	APPLE
	{
		vResGrwNFactor = 1.0;
//		vResGrwNFactor = (float)(pow(vResGrwNFactor,1.0));
	}
	BEECH
	{
		vResGrwNFactor = 1.0;
//		vResGrwNFactor = (float)(pow(vResGrwNFactor,1.0));
 	}
	SPRUCE
	{
		vResGrwNFactor = 1.0;
//		vResGrwNFactor = (float)(pow(vResGrwNFactor,1.0));
 	}

 	//Verfügbare Glucose
	vCH2OAvailable =  (pCarb->fCAssimilatePool - vAssReserves) *min(pPGen->vAssUseRate * DeltaT,1.0)
					+ pCarb->fGrossPhotosynR*30.0/44.0 
					+ (pBiom->fStemReserveWeight-vStarchReserves) * pGen->fRelResvGrwRate * vResGrwNFactor * DeltaT;


/*	vCH2OAvailable =  pCarb->fCAssimilatePool - vAssReserves
					+ pCarb->fGrossPhotosynR*(float)(30.0/44.0)
					+ pBiom->fStemReserveWeight * pGen->fRelResvGrwRate * vResGrwNFactor * DeltaT;
*/

	if(vCH2OAvailable<0.0)
		vCH2OAvailable = 0.0;

//	I. MAINTENANCE:

	//	Kosten für die Erhaltung in kg(CH2O):
    pPPart->dCostsForMaintenance = 30.0/44.0*MaintenanceRespiration(exp_p2,iPlant,TempAct);

    // test:
    // pPPart->dCostsForMaintenance =(float)0.0;

	//Überprüfe, ob Assimilatpool für den Bedarf für Erhaltungsatmung ausreicht:
	if((vCH2OAvailable - pPPart->dCostsForMaintenance) < 0.0)
	{
		//Die Erhaltungsatmung wird gedrosselt, für Abwehr/Reparatur und Wachstum 
		//steht keine Glucose mehr zur Verfügung
		
		corrfM = vCH2OAvailable/pPPart->dCostsForMaintenance; 

		pCarb->fMainRespRRoot		*= (float)corrfM;
		pCarb->fMainRespRStem		*= (float)corrfM; 
		pCarb->fMainRespRLeaf		*= (float)corrfM; 
		pCarb->fMainRespRGrain		*= (float)corrfM; 
		pCarb->fMainRespRGrossRoot	*= (float)corrfM; 
		pCarb->fMainRespRBranch		*= (float)corrfM; 
		pCarb->fMainRespRTuber		*= (float)corrfM; 

		pPPart->dCostsForMetabolism *= (float)corrfM;

		//gesamt:
		pCarb->fMaintRespR *= (float)corrfM; //kg(CO2)

		// Umrechnung kg(CH2O):
		pPPart->dCostsForMaintenance = pCarb->fMaintRespR*(float)(30.0/44.0);
	}

//////////////////////////////////////////////////////////////////////////////////////////////////

    if(pPlathoModules->iFlagPriority==2)     //induced allocation to defense has priority over growth processes
    {
	
//  II. INDUCED DEFENCE

	//Verfügbare Glucose für "carbon-based secondary compounds":
	    vCH2OAvailable -= pPPart->dCostsForMaintenance;  //kg

    if(vCH2OAvailable <= 0.0)
	{
	    vCH2OAvailable = 0.0;

        pPStress->vDefCompR = 0.0;

	    // Costs for Defense:
	        pPPart->dCostsForDefense =  0.0;
	    pPPart->dCostsForDefConstitutive = 0.0;
	    pPPart->dCostsForDefInduced = 0.0;
		
            pPPart->dCostsForBiomassGrowth = 0.0;

		pBiom->fRootGrowR = (float)0.0;
	    pBiom->fGrossRootGrowR = (float)0.0; 
		pBiom->fTuberGrowR = (float)0.0; 
		pBiom->fStemGrowR = (float)0.0; 
		pBiom->fBranchGrowR = (float)0.0; 
		pBiom->fLeafGrowR = (float)0.0; 
		pBiom->fFruitGrowR = (float)0.0; 

		pPBiom->vStarchGrowR = 0.0;

		PhiCH2O = 0.0;
		    PhiN = 1.0;

		    goto Label_1002; //kein Wachstum und keine Abwehr möglich
	}

	pPPart->dCostsForDefInduced = min(vCH2OAvailable,pPPart->dDemandForDefInduced);
    }

//  III. GROWTH

    //Verfügbare Glucose für Wachstum
	if(pPlathoModules->iFlagPriority==1)     
    {
        vCH2OAvailable = max(0.0,vCH2OAvailable-pPPart->dCostsForMaintenance);
    }
    else  //iFlagPriority == 2
    {   //induced allocation to defense has priority over growth processes
	vCH2OAvailable -= pPPart->dCostsForDefInduced;
    }

	//potentielle Wachstumsraten
	GetPotentialGrowthRates(exp_p2, iPlant, TempAct);

	//Glukosebedarf für potentielles Wachstum
	pPPart->dCostsForBiomassGrowth = GetCostsForGrowth(exp_p2, iPlant);
	//Total demand for growth and defense [kg(glucose)/plant/day]
	if(pPlathoModules->iFlagPriority==1)     
    {
	pPPart->vTotalDemandForGrowthAndDefense = 
		    pPPart->vDemandForDefense + pPPart->dCostsForBiomassGrowth; 
    }
    else
    {   //iFlagPriority == 2
	    pPPart->vTotalDemandForGrowthAndDefense = 
		    pPPart->dDemandForDefConstitutive + pPPart->dCostsForBiomassGrowth; 
    }

        
    //O3-stress: reduced growth rate due to costs for O3-detoxification
    if(pPlathoModules->iFlagO3 == 4) //O3-stress following van Oijen
    {
        PhiCH2O =  vCH2OAvailable/pPPart->vTotalDemandForGrowthAndDefense;
	    //Ozone Uptake:
	    CalcLeafInternalO3(pTi->pSimTime->fTimeDay, pTi->pSimTime->fTimeAct, pTi->pTimeStep->fAct, iPlant, PhiCH2O, pPlant, pCl->pWeather,pLo,pTi);

        vCH2OAvailable -= (pPStress->dCostForDetox+pPStress->dCostsRepair)* 1e-3;   //kg (CH2O)
        
        if (vCH2OAvailable < 0.0)    vCH2OAvailable = 0.0;

        pPPart->dCostsForDetoxAndRepairCum += (pPStress->dCostForDetox+pPStress->dCostsRepair)* 1e-3;

    }


    if(vCH2OAvailable <= 0.0)
	{
	    vCH2OAvailable = 0.0;
            
	    if(pPlathoModules->iFlagPriority==1)     
        {
            pPPart->dCostsForDefense = 0.0; 
        }
        else
        {   //iFlagPriority == 2
	    pPPart->dCostsForDefConstitutive = 0.0;
        }

 	    
        pPPart->dCostsForBiomassGrowth = 0.0;

		pBiom->fRootGrowR = (float)0.0;
	    pBiom->fGrossRootGrowR = (float)0.0; 
		pBiom->fTuberGrowR = (float)0.0; 
		pBiom->fStemGrowR = (float)0.0; 
		pBiom->fBranchGrowR = (float)0.0; 
		pBiom->fLeafGrowR = (float)0.0; 
		pBiom->fFruitGrowR = (float)0.0; 

		pPBiom->vStarchGrowR = 0.0;

		PhiCH2O = 0.0;
		PhiN = 1.0;

		goto Label_1002; //kein Wachstum und keine Abwehr möglich
	}



	TREES
	{
	//	if(pBiom->fLeafWeight>(float)0.3*pBiom->fWoodWeight)
	//	pBiom->fLeafGrowR = (float)0.0;

		//Ein Teil des Holzwachstums geht in Form von Stärke in den Reserve-Pool.
		//Zusätzliche C-Reserven werden nur bei Assimilat-Überschuss angelegt (siehe unten)

		vMinStarchFactor = 0.015;//0.05 0.02

		APPLE
			vMinStarchFactor = 0.0;
		
		pPBiom->vStarchGrowR = vMinStarchFactor * (pBiom->fGrossRootGrowR + pBiom->fBranchGrowR +
			pBiom->fStemGrowR);

		pBiom->fStemGrowR *= (float)(1.0 - vMinStarchFactor);
		pBiom->fBranchGrowR *= (float)(1.0 - vMinStarchFactor);
		pBiom->fGrossRootGrowR *= (float)(1.0 - vMinStarchFactor);
	}


/*
	//potentielle Wachstumsatmung kg(CO2)/kg(TS):	
	pCarb->fGrowthRespR = GrowthRespiration(exp_p2, iPlant);

	//Glucosebedarf für potentielles Wachstum
	pPPart->dCostsForBiomassGrowth = GetCostsForGrowth(exp_p2, iPlant)+
							pCarb->fGrowthRespR*(float)(30.0/44.0);
*/



//	III. AVAILABILITY OF C (Glucose)

	//Überprüfe, ob der Bedarf an Assimilaten gedeckt ist:

    //O3-stress: reduced growth rate due to costs for O3-detoxification
    //jetzt weiter oben, da O3-Uptake auch dann stattfindet, wenn kein Wachstum möglich ist
	if(vCH2OAvailable - pPPart->dCostsForBiomassGrowth - pPPart->vDemandForDefenseBase
                                                                        -pPBiom->vStarchGrowR > 0.0)
	{
		PhiCH2O = 1.0;
		goto Label_1000;//Genügend Glucose vorhanden, keine Regulation!
	}

    //11/01/2007:
    //Leaves, fine roots and fruits (tubers) have priority over stems
    vDemand1 =  pBiom->fRootGrowR*pGen->fGrwEffRoot
                + pBiom->fLeafGrowR*pGen->fGrwEffLeaf
                + pBiom->fFruitGrowR*pGen->fGrwEffStorage
                + pBiom->fTuberGrowR*pGen->fGrwEffTuber;

    vDemand2 =  pBiom->fGrossRootGrowR*pGen->fGrwEffGrossRoot
                + pBiom->fStemGrowR*pGen->fGrwEffStem
                + pBiom->fBranchGrowR*pGen->fGrwEffBranch;

     PhiCH2O_1 = min(1.0,max(0.0,vCH2OAvailable
                                /(vDemand1 * (1.0+pPStress->vConstDefenseBase))));

     PhiCH2O_2 = min(1.0,max(0.0,(vCH2OAvailable-vDemand1)
                     /(vDemand2 * (1.0+pPStress->vConstDefenseBase)+pPBiom->vStarchGrowR)));
         
    PhiCH2O = max(0.0,vCH2OAvailable/(pPPart->dCostsForBiomassGrowth
        +pPPart->vDemandForDefenseBase+pPBiom->vStarchGrowR));     

	if(vCH2OAvailable <= 0.0)
    {
		PhiCH2O = 0.0;
        PhiCH2O_1 = 0.0;
		PhiCH2O_2 = 0.0;
    }

	pBiom->fRootGrowR		*= (float)PhiCH2O_1;
	pBiom->fGrossRootGrowR	*= (float)PhiCH2O_2; 
	pBiom->fStemGrowR		*= (float)PhiCH2O_2; 
	pBiom->fBranchGrowR		*= (float)PhiCH2O_2; 
	pBiom->fLeafGrowR		*= (float)PhiCH2O_1; 
	pBiom->fFruitGrowR		*= (float)PhiCH2O_1; 
	pBiom->fTuberGrowR		*= (float)PhiCH2O_1; 

	pPBiom->vStarchGrowR			*= PhiCH2O_2;

	//Entsprechend Zurückschrauben des Bedarfs für Abwehr:
//	pPPart->vDemandForDefense *= PhiCH2O;
    pPPart->vDemandForDefenseBase *= PhiCH2O;

	//Glucosebedarf für Wachstum
	pPPart->dCostsForBiomassGrowth = GetCostsForGrowth(exp_p2, iPlant);
	//Überprüfe ob der Glucosebedarf jetzt gedeckt ist:
	if(vCH2OAvailable - pPPart->dCostsForBiomassGrowth - pPPart->vDemandForDefenseBase - pPBiom->vStarchGrowR > (float)0.0)
		goto Label_1000; //jetzt reicht die verfügbare Glucose aus!

	//Veteilung der restlichen verfügbaren Glucose:
    if(pPPart->dCostsForBiomassGrowth+pPPart->vDemandForDefenseBase+pPBiom->vStarchGrowR > 0.0)
    {
	    corrfC = vCH2OAvailable/(pPPart->dCostsForBiomassGrowth+pPPart->vDemandForDefenseBase+pPBiom->vStarchGrowR);
    	
	    pBiom->fRootGrowR		*= (float)corrfC;
	    pBiom->fGrossRootGrowR	*= (float)corrfC; 
	    pBiom->fTuberGrowR		*= (float)corrfC; 
	    pBiom->fStemGrowR		*= (float)corrfC; 
	    pBiom->fBranchGrowR		*= (float)corrfC; 
	    pBiom->fLeafGrowR		*= (float)corrfC; 
	    pBiom->fFruitGrowR		*= (float)corrfC; 

	    pPBiom->vStarchGrowR			*= (float)corrfC;

	    //pPPart->vDemandForDefense *= (float)corrfC;
        pPPart->vDemandForDefenseBase *= (float)corrfC;
    }

	Label_1000:

//	IV. AVAILABILITY OF N

    if(pPlathoModules->iFlagNEffectGrw==1)//no N effect on growth
    {
        PhiN = 1.0;
    }
    else //N effect on growth included
    {
    //	a) Stickstoffbedarf 

	    PlantNitrogenDemand_PLATHO(exp_p2, iPlant);

    //	b) überprüfe, ob genügend N verfügbar ist:

	    // potentielle N-Aufnahme:
	    CalculatePotentialNitrogenUptake(exp_p2,iPlant);

	    // + verfügbares N aus lebenden Organen [kg(N)]:
	    pPPltN->dTotalTransNw = CalcPotNitrogenTransloc(pPlant,iPlant, DeltaT);

	    pPPltN->dNPool = pPPltN->dTotalTransNw + pPPltN->dPotNUpt;


	    PhiN = min(1.0,pPPltN->dNPool / (pPPltN->vNDemandForGrowth+EPS));

	    if(pPPltN->dNPool <= (float)0.0)
		    PhiN = 0.0;


	    // Anpassung an verfügbaren Stickstoff
	    pBiom->fRootGrowR		*= (float)PhiN;
	    pBiom->fGrossRootGrowR  *= (float)PhiN; 
    	
	    pBiom->fLeafGrowR		*= (float)PhiN;
	    pBiom->fStemGrowR		*= (float)PhiN;
	    pBiom->fBranchGrowR		*= (float)PhiN;
	    pBiom->fFruitGrowR		*= (float)PhiN; 
	    pBiom->fTuberGrowR		*= (float)PhiN; 

        //Der Bedarf für Abwehr wird ebenfalls reduziert:
    	//pPPart->vDemandForDefense *= (float)pow(PhiN,0.5);
    	pPPart->vDemandForDefenseBase *= PhiN;
    }	
	
	// REGULATION (C and N)

	/////////////////////////////////////////////
	//Regulation nach Dokumentation, Gl. 51-53 //
	/////////////////////////////////////////////

	//vGammaC = 1 bedeutet keine Regulation bei C-Mangel
	//vGammaN = 1 bedeutet keine Regulation bei N-Mangel

	vGammaC = pPMorph->vCRegulation;
	vGammaN = pPMorph->vNRegulation;
//  vGammaC = (float)1.0 + pPMorph->vPlasticity;
//	vGammaN = (float)1.0 + pPMorph->vPlasticity;

	//Total growth rate:
	pBiom->fBiomGrowR = pBiom->fRootGrowR + pBiom->fGrossRootGrowR + pBiom->fStemGrowR + 
						pBiom->fBranchGrowR + pBiom->fLeafGrowR + pBiom->fFruitGrowR + pBiom->fTuberGrowR;


	if(pBiom->fBiomGrowR>(float)1e-15)
	{
		help1 = (pow(vGammaC,PhiCH2O-1.0)*pow(vGammaN,1.0-PhiN) * pBiom->fRootGrowR
			+	pow(vGammaC,PhiCH2O-1.0)*pow(vGammaN,PhiN-1.0) * pBiom->fGrossRootGrowR
			+	pow(vGammaC,1.0-PhiCH2O)*pow(vGammaN,PhiN-1.0) * pBiom->fStemGrowR
			+	pow(vGammaC,1.0-PhiCH2O)*pow(vGammaN,PhiN-1.0) * pBiom->fBranchGrowR
			+	pow(vGammaC,1.0-PhiCH2O) * pBiom->fLeafGrowR
			+	pBiom->fFruitGrowR
			+	pBiom->fTuberGrowR)/pBiom->fBiomGrowR;
	}
	else
		help1 = (float)1.0;

	pBiom->fRootGrowR		*= (float)(pow(vGammaC,PhiCH2O-1.0)*pow(vGammaN,1.0-PhiN)/help1);
	pBiom->fGrossRootGrowR  *= (float)(pow(vGammaC,PhiCH2O-1.0)*pow(vGammaN,PhiN-1.0)/help1); 
	pBiom->fStemGrowR		*= (float)(pow(vGammaC,1.0-PhiCH2O)*pow(vGammaN,PhiN-1.0)/help1);
	pBiom->fBranchGrowR		*= (float)(pow(vGammaC,1.0-PhiCH2O)*pow(vGammaN,PhiN-1.0)/help1);
	pBiom->fLeafGrowR		*= (float)(pow(vGammaC,1.0-PhiCH2O)/help1); 
	pBiom->fFruitGrowR		*= (float)(1.0/help1); 
	pBiom->fTuberGrowR		*= (float)(1.0/help1); 

	// Anpassen des Bedarfs für Abwehr:
	// pPPart->vDemForDefense *= (float)pow(vGammaC,1.0-PhiCH2O)/help1;
	
	// Überprüfe ob der Glucosebedarf jetzt gedeckt ist:

	//Wachstumsatmung kg(CO2)
	pCarb->fGrowthRespR = (float)GrowthRespiration(exp_p2, iPlant);

	//Neue Biomasse + Atmung
	pPPart->dCostsForBiomassGrowth = GetCostsForGrowth(exp_p2, iPlant);


	if(vCH2OAvailable - pPPart->dCostsForBiomassGrowth - pPPart->vDemandForDefenseBase
                                                                            - pPBiom->vStarchGrowR >= 0.0)
		goto Label_1001; //jetzt reicht die verfügbare Glucose aus!


	//Veteilung der restlichen verfügbaren Glucose:
    if(pPPart->dCostsForBiomassGrowth+pPPart->vDemandForDefenseBase+pPBiom->vStarchGrowR > 0.0)
    {
	    corrfC = vCH2OAvailable/(pPPart->dCostsForBiomassGrowth
            +pPPart->vDemandForDefenseBase+pPBiom->vStarchGrowR);
    
	    pBiom->fRootGrowR		*= (float)corrfC;
	    pBiom->fGrossRootGrowR	*= (float)corrfC; 
	    pBiom->fTuberGrowR		*= (float)corrfC; 
	    pBiom->fStemGrowR		*= (float)corrfC; 
	    pBiom->fBranchGrowR		*= (float)corrfC; 
	    pBiom->fLeafGrowR		*= (float)corrfC; 
	    pBiom->fFruitGrowR		*= (float)corrfC; 

	    pPBiom->vStarchGrowR			*= corrfC;

	    //pPPart->vDemandForDefense *= corrfC;
        pPPart->vDemandForDefenseBase *= corrfC;
    }

	//Wachstumsatmungkg(CO2):
	pCarb->fGrowthRespR = (float)GrowthRespiration(exp_p2, iPlant);


	//Glucosebedarf für Wachstum
	pPPart->dCostsForBiomassGrowth = GetCostsForGrowth(exp_p2, iPlant);

Label_1001:
        
    //Total growth rate [kg(dry matter)/d]:
	pBiom->fBiomGrowR = pBiom->fRootGrowR + pBiom->fGrossRootGrowR + pBiom->fTuberGrowR + pBiom->fStemGrowR + 
						pBiom->fBranchGrowR + pBiom->fLeafGrowR + pBiom->fFruitGrowR;


	// V. DEFENSE

	//Verfügbare Glucose für (konstitutive) Abwehr
	vCH2OAvailable -= (pPPart->dCostsForBiomassGrowth + pPBiom->vStarchGrowR);

	if(vCH2OAvailable<0.0)	vCH2OAvailable=0.0;

	vSurplusForDefense =  max(0.0,vCH2OAvailable-pPPart->vDemandForDefenseBase);

    if(pPlathoModules->iFlagNEffectGrw==1)// no N effect on growth
    {
	    if(pPlathoModules->iFlagPriority==1)     
        {
		    pPPart->vDemandForDefense = pPPart->vDemandForDefenseBase 
			    + min(pPPart->vDemandForDefense-pPPart->vDemandForDefenseBase,vSurplusForDefense);
        }
        else
        {   //iFlagPriority == 2
		pPPart->dDemandForDefConstitutive = pPPart->vDemandForDefenseBase 
			+ min(pPPart->dDemandForDefConstitutive-pPPart->vDemandForDefenseBase,vSurplusForDefense);
	}
	}
	else // N effect on growth included
	{
 	    if(pPlathoModules->iFlagPriority==1)     
        {
        // nur die Differenz zwischen Demand und DemandBase ist noch abhängig vom N-Status:
		pPPart->vDemandForDefense = pPPart->vDemandForDefenseBase 
			+ min(pow(PhiN,0.5)*(pPPart->vDemandForDefense-pPPart->vDemandForDefenseBase),vSurplusForDefense);
        }
        else
        {   //iFlagPriority == 2
       // nur die Differenz zwischen DemandConstitutive und DemandBase ist noch abhängig vom N-Status:
		pPPart->dDemandForDefConstitutive = pPPart->vDemandForDefenseBase 
			+ min(pow(PhiN,0.5)*(pPPart->dDemandForDefConstitutive-pPPart->vDemandForDefenseBase),vSurplusForDefense);
   }
   }

	//Überprüfe, ob genügend Glucose für Abwehr und Reparatur zur Verfügung steht:
	if(pPlathoModules->iFlagPriority==1)     
    {
	    if((pPPart->vDemandForDefense > 0.0) &&(vCH2OAvailable - pPPart->vDemandForDefense < 0.0))
            pPPart->vDemandForDefense = vCH2OAvailable;

  	//new defensive compounds [kg]
	    pPStress->vDefCompR = pPPart->vDemandForDefense/pPGrwEff->vDefComp;
    }
    else
    {   //iFlagPriority == 2
	    if((pPPart->dDemandForDefConstitutive > 0.0) && (vCH2OAvailable - pPPart->dDemandForDefConstitutive < 0.0))
            pPPart->dDemandForDefConstitutive = vCH2OAvailable;
        
        //new defensive compounds [kg]
	    pPStress->vDefCompR = (pPPart->dDemandForDefConstitutive+pPPart->dCostsForDefInduced)/pPGrwEff->vDefComp;
    }

 	if(pPlathoModules->iFlagPriority==1)     
    {
	    pPPart->dCostsForDefense =  pPStress->vDefCompR * pPGrwEff->vDefComp;
	    pPPart->dCostsForDefConstitutive = min(pPPart->dCostsForDefense,pPPart->dDemandForDefConstitutive);
	    pPPart->dCostsForDefInduced = max(0.0,pPPart->dCostsForDefense-pPPart->dDemandForDefConstitutive);
    }
    else
    {   //iFlagPriority == 2
	    pPPart->dCostsForDefense =  pPStress->vDefCompR * pPGrwEff->vDefComp;
	    pPPart->dCostsForDefConstitutive = max(0.0,pPPart->dCostsForDefense-pPPart->dCostsForDefInduced);
    }

	//Atmung bei Synthese der Abwehrstoffe:
	pCarb->fGrowthRespR += (float)(pPStress->vDefCompR * pPCO2Req->vDefComp);


Label_1002:

    // VI. MYCORRHIZA

    if(pPlathoModules->iFlagMycorrhiza==1)//no allocation of CH2O to mycorrhizal symbionts
    {
        pPPart->dCostsForMycorrhiza = 0.0;
    }
    else //allocation of CH2O to mycorrhizal symbionts included
    {
	    //Verfügbare Glucose für Abwehr
	    vCH2OAvailable -= pPPart->dCostsForDefense;

	    if(vCH2OAvailable<0.0) vCH2OAvailable=0.0;

       //hier muss das Mycorrhizamodell entstehen
        pPPart->dCostsForMycorrhiza = 0.0;
    }


	
//	VII. RESERVEBILDUNG?

	// Wenn Gesamt-Bedarf > Assimilat-Pool:  
	// Reserven aus dem Stamm/Stängel werden mobilisiert

	// Der C-Reserve-Pool (Stärke) darf 25% der gesamten Stängel/Holz-Biomasse nicht überschreiten:
	HERBS
		vMaxStemReserves = 0.25*pBiom->fStemWeight;
	TREES
		vMaxStemReserves = 0.25*pBiom->fWoodWeight;

	vTotalCosts = pPPart->dCostsForMaintenance + pPPart->dCostsForDefense + (pPStress->dCostForDetox+pPStress->dCostsRepair)* 1e-3 
		+ pPPart->dCostsForBiomassGrowth + pPPart->dCostsForMycorrhiza + pPBiom->vStarchGrowR;
	
	if(vTotalCosts <= (pCarb->fCAssimilatePool - vAssReserves) *min(pPGen->vAssUseRate * DeltaT,1.0)
						+ pCarb->fGrossPhotosynR*30.0/44.0)
	{
		// alles kann aus Assimilatpool gedeckt werden, Überschuß
		// wird zum Aufbau von Reserven verwendet:

		//if(pCarb->fCAssimilatePool>(float)0.1*(pBiom->fStemWeight
		//						+pBiom->fBranchWeight+pBiom->fGrossRootWeight))

			pBiom->fStemReserveGrowRate = (float)(pPBiom->vStarchGrowR + 
					((pCarb->fCAssimilatePool - vAssReserves) *min(pPGen->vAssUseRate * DeltaT,1.0)
					 + pCarb->fGrossPhotosynR*(30.0/44.0)-vTotalCosts)
					 * pGen->fRelResvGrwRate*vResGrwNFactor*DeltaT);
		//else
		//	pBiom->fStemReserveGrowRate = (float)0.0;

		if(pBiom->fStemReserveGrowRate + pBiom->fStemReserveWeight > vMaxStemReserves)
			pBiom->fStemReserveGrowRate = (float)max(0.0,vMaxStemReserves - pBiom->fStemReserveWeight);



		 pBiom->fReserveTransRate = (float)0.0;
	}
	else
	{
//		pBiom->fStemReserveGrowRate = (float)0.0;
		pBiom->fStemReserveGrowRate = (float)pPBiom->vStarchGrowR;

		// Reserven müssen angezapft werden:
		pBiom->fReserveTransRate    = (float)(vTotalCosts - 
			((pCarb->fCAssimilatePool - vAssReserves) * min(pPGen->vAssUseRate * DeltaT,1.0) + pCarb->fGrossPhotosynR*30.0/44.0));
	}

  		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
            if (NewDay(pTi))
            {
                pPStress->vCShortage=0.0;
                pPStress->vNShortage=0.0;
            }
            else
            {
                pPStress->vCShortage += PhiCH2O*DeltaT;
        	    pPStress->vNShortage += PhiN*DeltaT;
            }
            
            if(EndDay(pTi))
            {
                pPStress->vCShortage /= (pTi->pSimTime->fTimeAct-(int)pTi->pSimTime->fTimeAct);
                pPStress->vNShortage /= (pTi->pSimTime->fTimeAct-(int)pTi->pSimTime->fTimeAct);
            }
      
            if(pPStress->vCShortage > 1.0)  pPStress->vCShortage = 1.0;
            if(pPStress->vNShortage > 1.0)  pPStress->vNShortage = 1.0;
            
        }
        else
        {
	        pPStress->vCShortage = PhiCH2O;
	        pPStress->vNShortage = PhiN;
        }

	return 1;
}

int GetPotentialGrowthRates(EXP_POINTER2, int iPlant, double TempAct)
{

	PBIOMASS		pBiom = pPlant->pBiomass;
	PPLTCARBON		pCarb = pPlant->pPltCarbon;
	PGENOTYPE		pGen  = pPlant->pGenotype;
	PDEVELOP		pDev  = pPlant->pDevelop;
	PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;

	PPLATHOPARTITIONING pPPart	= pPlathoPlant[iPlant]->pPlathoPartitioning;
	PPLATHOBIOMASS		pPBiom	= pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLATHOGENOTYPE		pPGen	= pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLANTSTRESS		pPStress = pPlathoPlant[iPlant]->pPlantStress;
	PPLATHODEVELOP		pPDev	= pPlathoPlant[iPlant]->pPlathoDevelop;

	double	Q10 = 2.0;
	double  fracRoot, fracLeaf, fracStem, f1, f2;
	double	FHeight, vTempEffect, TempOpt;	
	double	vDevStage;
	double	DeltaT = (double)pTi->pTimeStep->fAct;		// Zeitschritt
	double	vAutumn, vFlushRedL,vFlushRedR, vred; //TREES: Reduktion der Wachstumsrate am Ende der Vegetationsperiode
	double	vMaxRootWeight, vMaxLeafWeight;
	double	vBranchFrac, vUGrWoodFrac;

	double   vDefenseFactor;
            //, vDefenseFactorBase, fLfPathDefenseFactor, fO3DefenseFactor, fLvDefenseFactor, fRtDefenseFactor,
            //fLfPathDefenseFactorBase, fO3DefenseFactorBase, fRtDefenseFactorBase,	
    double	vXiStructuralBiomass;
    double  help1, help2, help1Base, help2Base, help2Induced;

	extern	double	factor1(double DevStage, double StageStart, double rho, double a);
    extern int GetGrowthCapacities(pPlant, iPlant);

	vDevStage = pDev->fDevStage;

	TREES
	{
		vUGrWoodFrac = AVGENERATOR(vDevStage,pPMorph->vUGrWoodFrac);
		vBranchFrac = AVGENERATOR(vDevStage,pPMorph->vBranchFrac);
	}

	//Temperaturabhängigkeit der potentiellen Wachstumsraten

	BARLEY
		TempOpt = 20;
	WHEAT
		TempOpt = 20;
	MAIZE
		TempOpt = 20;
	SUNFLOWER
		TempOpt = 20;
	POTATO
		TempOpt = 18;
	ALFALFA
		TempOpt = 20;
	LOLIUM
		TempOpt = 20;
	APPLE
		TempOpt = 25;
	BEECH
		TempOpt = 18;  //18.0
	SPRUCE
		TempOpt = 16;

	vTempEffect = pow(Q10,(TempAct-TempOpt)/10.0);


	////////////////////////////////////////////////////////////////////////
	//Bedarf der einzelnen Organe:
	////////////////////////////////////////////////////////////////////////
	pBiom->fRootGrowR		= (float)0.0;
	pBiom->fGrossRootGrowR	= (float)0.0;
	pBiom->fStemGrowR		= (float)0.0;
	pBiom->fBranchGrowR		= (float)0.0;
	pBiom->fLeafGrowR		= (float)0.0;
	pBiom->fFruitGrowR		= (float)0.0;
	pBiom->fTuberGrowR		= (float)0.0;
	
				
    GetGrowthCapacities(pPlant, iPlant);

	switch(pPDev->iStagePlatho)
	{
		case 0:	
			HERBS// Dormanz (vor Beginn Keimen/Blattaustrieb)
			{
				pBiom->fRootGrowR		= (float)0.0;
				pBiom->fStemGrowR		= (float)0.0;
				pBiom->fLeafGrowR		= (float)0.0;
				pBiom->fFruitGrowR		= (float)0.0;
				pBiom->fTuberGrowR		= (float)0.0;
			}
			TREES // 
			{
				pBiom->fRootGrowR	= (float)0.0;
				pBiom->fLeafGrowR	= (float)0.0;


				BEECH
				{	
					if(pPlant->pDevelop->fDevStage>0.8)	
					{
						pBiom->fRootGrowR = (float)(pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);
					}
				}    
				SPRUCE
				{	
					if(pPlant->pDevelop->fDevStage>0.6)	
					{
						pBiom->fRootGrowR = (float)(pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);
						pBiom->fLeafGrowR	= (float)(pPGen->vLeafFlushR * (float)max(0.0,(pPBiom->vKLeaf-pBiom->fLeafWeight))* DeltaT);
					}
				}
				APPLE
				{	
					if(pPlant->pDevelop->fDevStage>0.9)	
					{
//						pBiom->fRootGrowR = pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT;
						pBiom->fRootGrowR = (float)0.0;
						pBiom->fLeafGrowR = (float)0.0;
					}
				}

				pPBiom->vWoodInc		= 0.0;
				pBiom->fGrossRootGrowR	= (float)0.0;
				pBiom->fStemGrowR		= (float)0.0;
				pBiom->fBranchGrowR		= (float)0.0;
				pBiom->fFruitGrowR		= (float)0.0;
												
/*				pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
					- pBiom->fRootGrowR - pBiom->fLeafGrowR);

				pBiom->fGrossRootGrowR	= pPBiom->vWoodInc*vUGrWoodFrac;
				
				pBiom->fStemGrowR		= 
					pPBiom->vWoodInc*((float)1.0-vUGrWoodFrac)*((float)1.0-vBranchFrac);
				
				pBiom->fBranchGrowR		= 
					pPBiom->vWoodInc*((float)1.0-vUGrWoodFrac)*vBranchFrac;
*/

			}

			break;

		case 1:
			
			HERBS //Germination - Keimblätter
			{
				
				f1 = pPMorph->vFineRootToLeafRatio;
				f2 = pPMorph->vLeafToStemRatio;
				
				
				fracRoot = f1*f2/(f1*f2+f2+1.0);
				fracLeaf = f2/(f1*f2+f2+1.0);
				fracStem = 1.0/(f1*f2+f2+1.0);

				// (Fine) Roots
				pBiom->fRootGrowR = (float)(fracRoot*pPBiom->vSeedWeight/1000 * 
										pPlant->pDevelop->fDevR * DeltaT);
				// Leaves
				pBiom->fLeafGrowR = (float)(fracLeaf*pPBiom->vSeedWeight/1000 * 
										pPlant->pDevelop->fDevR * DeltaT);
				//Stem
				pBiom->fStemGrowR = (float)(fracStem*pPBiom->vSeedWeight/1000 * 
						pPlant->pDevelop->fDevR * DeltaT);

				//pBiom->fStemGrowR		= (float)0.0;

				//Fruit, Tuber			
				pBiom->fFruitGrowR		= (float)0.0;
				pBiom->fTuberGrowR		= (float)0.0;

			}

			TREES	//Blattaustrieb
			{
                // Fine Roots	
				pBiom->fRootGrowR = (float)(pPGen->vFineRootFlushR * max(0.0,pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);			

				// Leaves
				pBiom->fLeafGrowR	= (float)(pPGen->vLeafFlushR * max(0.0,(pPBiom->vKLeaf-pBiom->fLeafWeight))* DeltaT);

				APPLE
                {
                    // Branches
                    pBiom->fBranchGrowR	=  (float)(pBiom->fLeafGrowR / pPMorph->vLeafToBranchRatio);			    
                    
                    // Wood
				    pPBiom->vWoodInc = max(0.0,pPGen->vMaxPlantGrowR * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR - pBiom->fBranchGrowR);
//				    pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
//					    - pBiom->fRootGrowR - pBiom->fLeafGrowR - pBiom->fBranchGrowR);
    							
				    // Coarse Roots
                    pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
    				
				    // Stem
				    pBiom->fStemGrowR		=  (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac));
                }
                else
                {
				    // Wood
				   pPBiom->vWoodInc = max(0.0,pPGen->vMaxPlantGrowR * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
				    // pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
					//    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
    							
				    // Coarse Roots
				    pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
    				
				    // Stem
				    pBiom->fStemGrowR		=  (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*(1.0-vBranchFrac));
    				
                    // Branches
				    pBiom->fBranchGrowR		=   (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*vBranchFrac);
                }
			}

			break;

		case 2:
			HERBS // Blätter, Stängel und Wurzeln
			{
				// Fine Roots	
				vMaxRootWeight = pPlathoPlant[iPlant]->pPlathoMorphology->vFineRootToLeafRatio *
				pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToStemRatio*pBiom->fStemWeight;

				pBiom->fRootGrowR = 
					(float)(min(pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight),
					max(0.0,vMaxRootWeight - pBiom->fRootWeight))* DeltaT);

				// Leaves
				vMaxLeafWeight = 
				pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToStemRatio*pBiom->fStemWeight;

				pBiom->fLeafGrowR = 
					(float)(min(pPGen->vLeafFlushR * (pPBiom->vKLeaf-pBiom->fLeafWeight),
					max(0.0,vMaxLeafWeight - pBiom->fLeafWeight)) * DeltaT);
				
				
				// Stem
		//		FHeight = (float)max(0.0,1.0 - pow(pPlant->pCanopy->fPlantHeight/pPlant->pGenotype->fPlantHeight,2));
				FHeight = 1.0;
				pBiom->fStemGrowR = (float)(max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
					- pBiom->fRootGrowR - pBiom->fLeafGrowR) * FHeight);

				if(pBiom->fStemWeight+pBiom->fStemGrowR>pPBiom->vKStem)
					pBiom->fStemGrowR=(float)max(0.0,pPBiom->vKStem-pBiom->fStemWeight);

				// Fruits, Tuber
				pBiom->fFruitGrowR = (float)0.0;
				pBiom->fTuberGrowR = (float)0.0;

				// Tuber
/*				POTATO				
				{
					pBiom->fFruitGrowR = (float)0.0;
					pBiom->fTuberGrowR = pPBiom->vegAbvEnd1*pPlant->pGenotype->fRelStorageFillRate*DeltaT;
				}*/
			}


			TREES // Wurzeln, Stamm, Blätter wachsen, evtl. Fruchtbildung 
			{
                // Fine Roots	
				vFlushRedL = max(0.0,3.0-1.0*pPlant->pDevelop->fDevStage);
				//vFlushRedR = max(0.0,2.9-0.95*pPlant->pDevelop->fDevStage);
				vFlushRedR = max(0.0,2.0-0.5*pPlant->pDevelop->fDevStage);

				// Fine Roots	
				pBiom->fRootGrowR = (float)(vFlushRedR*pPGen->vFineRootFlushR * max(0.0,pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);

				// Leaves
				pBiom->fLeafGrowR = (float)(vFlushRedL*pPGen->vLeafFlushR * max(0.0,(pPBiom->vKLeaf-pBiom->fLeafWeight)) * DeltaT);

				APPLE
                {
                    //Branches
                    pBiom->fBranchGrowR	=  (float)(pBiom->fLeafGrowR / pPMorph->vLeafToBranchRatio);			    
                    
                    // Wood
				    pPBiom->vWoodInc = (max(0.0,pPGen->vMaxPlantGrowR * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR - pBiom->fBranchGrowR));
//				    pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
//					    - pBiom->fRootGrowR - pBiom->fLeafGrowR - pBiom->fBranchGrowR);
    							
				    pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
    				
				    pBiom->fStemGrowR		= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac));
                }
                else
                {
                    // Wood
			        pPBiom->vWoodInc = max(0.0,pPGen->vMaxPlantGrowR * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
				    //	pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
					//    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
    							
				    pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
    				
				    pBiom->fStemGrowR	=  (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*(1.0-vBranchFrac));
    				
				    pBiom->fBranchGrowR	= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*vBranchFrac);

				    //Fruits
				    pBiom->fFruitGrowR = (float)(pPGen->vMaxPlantGrowR * (pPBiom->vKFruit-pBiom->fFruitWeight)
							    * DeltaT);
                }
			}

			break;
		case 3:
			HERBS // Wurzeln, Stamm, Blätter wachsen, Beginn der Fruchtbildung
			{

				vAutumn = max(0.0,(4.0-pPlant->pDevelop->fDevStage));


				// Fruits 
				pBiom->fFruitGrowR = (float)(min(pPBiom->vVegAbvEnd2*pGen->fRelStorageFillRate,
					 pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass)* DeltaT);
				
				// Tuber
				POTATO				
				{
					pBiom->fFruitGrowR = (float)0.0;
					pBiom->fTuberGrowR = (float)(min(pPBiom->vVegAbvEnd2*pGen->fRelStorageFillRate,
					 pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass)* DeltaT);
				}


				// Fine Roots	
				vMaxRootWeight = pPlathoPlant[iPlant]->pPlathoMorphology->vFineRootToLeafRatio *
				pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToStemRatio*pBiom->fStemWeight;

				pBiom->fRootGrowR = 
					(float)(min(pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight) * vAutumn ,
					max(0.0,vMaxRootWeight - pBiom->fRootWeight))* DeltaT);

				// Leaves
				vMaxLeafWeight = pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToStemRatio*pBiom->fStemWeight;

				pBiom->fLeafGrowR = 
					(float)(min(pPGen->vLeafFlushR * (pPBiom->vKLeaf-pBiom->fLeafWeight) * vAutumn ,
					max(0.0,vMaxLeafWeight - pBiom->fLeafWeight)) * DeltaT);

				
				// Stem
			//	FHeight = (float)max(0.0,1.0 - pow(pPlant->pCanopy->fPlantHeight/pPlant->pGenotype->fPlantHeight,2));
				FHeight = 1.0;

			
				if(pBiom->fRootGrowR + pBiom->fLeafGrowR > 
                    (float)(pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT) - (pBiom->fFruitGrowR+pBiom->fTuberGrowR))
				{
					vred = max(0.0,(pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT - (pBiom->fFruitGrowR+pBiom->fTuberGrowR))
                        /(pBiom->fRootGrowR + pBiom->fLeafGrowR + EPS));

					pBiom->fRootGrowR *= (float)vred;
					pBiom->fLeafGrowR *= (float)vred;
					pBiom->fStemGrowR = (float)0.0;
				}
				else
				{
					pBiom->fStemGrowR = (float)(max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT  
                        - (pBiom->fFruitGrowR+pBiom->fTuberGrowR) - pBiom->fRootGrowR - pBiom->fLeafGrowR) * FHeight);

					if(pBiom->fStemWeight+pBiom->fStemGrowR>pPBiom->vKStem)
						pBiom->fStemGrowR=(float)max(0.0,pPBiom->vKStem-pBiom->fStemWeight);
				}

			}

			TREES	// Wurzeln, Stamm, Blätter wachsen, evtl. Fruchtbildung
			{
				//fFlushRed = (float)max(0.0,2.0-0.5*pPlant->pDevelop->fDevStage);
				//vFlushRedR = 0.05;
				vFlushRedL = 0.0;
				vFlushRedR = 0.5;

				BEECH
				{
					if((pPlant->pDevelop->fDevStage<(float)3.5)&&(pPlant->pPltCarbon->fCAssimilatePool
						+pBiom->fStemReserveWeight>(float)0.005*pBiom->fWoodWeight))

						vFlushRedL=max(0.0,(3.5-pPlant->pDevelop->fDevStage)/3.0);
				}  

				// Fine Roots	
//				pBiom->fRootGrowR = pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT;
				pBiom->fRootGrowR = (float)(vFlushRedR*pPGen->vFineRootFlushR 
                    * max(0.0,pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);

				// Leaves
//				pBiom->fLeafGrowR	= pPGen->vLeafFlushR * (pPBiom->vKLeaf-pBiom->fLeafWeight) * DeltaT;
				pBiom->fLeafGrowR	=  (float)(vFlushRedL*pPGen->vLeafFlushR 
                    *max(0.0,(pPBiom->vKLeaf-pBiom->fLeafWeight)) * DeltaT);

                if(pPlant->pDevelop->fDevStage>(float)3.5)
                    vAutumn = 4.5 - pPlant->pDevelop->fDevStage;
                else
                    vAutumn = 1.0;

				// Wood
				    pPBiom->vWoodInc = max(0.0,pPGen->vMaxPlantGrowR * vAutumn * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
				//    pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * pBiom->fTotalBiomass * DeltaT 
				//	    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
							
				pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
				
				pBiom->fStemGrowR	=  (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*(1.0-vBranchFrac));
				
				pBiom->fBranchGrowR	= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*vBranchFrac);

				//Fruits
				pBiom->fFruitGrowR = (float)(pPGen->vMaxPlantGrowR * (pPBiom->vKFruit-pBiom->fFruitWeight)
							* DeltaT);

			}
			break;
		case 4:
			HERBS	// nur noch Früchte
			{
				// Roots
				pBiom->fRootGrowR = (float)0.0;

				// Stem
				pBiom->fStemGrowR = (float)0.0;

				// Leaves
				pBiom->fLeafGrowR = (float)0.0;

				// Fruits 
				pBiom->fFruitGrowR = (float)(pPBiom->vVegAbvEnd2*pGen->fRelStorageFillRate * DeltaT);

				// Tuber
				POTATO				
				{
					pBiom->fFruitGrowR = (float)0.0;
					pBiom->fTuberGrowR = (float)(pPBiom->vVegAbvEnd2*pGen->fRelStorageFillRate	* DeltaT);
				}

			}

			TREES // Beginn des Blattfalls
			{
			//	fFlushRed = (float)max(0.0,0.75-0.125*pPlant->pDevelop->fDevStage);

				//vFlushRedR = max(0.0,-0.75+0.2*pPlant->pDevelop->fDevStage);
				vFlushRedL = 0.0;
				vFlushRedR = 0.5;

				vAutumn = max(0.0,(5.0-pPlant->pDevelop->fDevStage)/2.0);
				//vAutumn = (float)max(0.0,(6.0-pPlant->pDevelop->fDevStage)/2.0);
				//vAutumn = (float)1.0;

				// Fine Roots	
	//			pBiom->fRootGrowR = (float)0.25*(pPlant->pDevelop->fDevStage-(float)3.0)*pPGen->vFineRootFlushR * (pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT;
				pBiom->fRootGrowR = (float)(vFlushRedR*pPGen->vFineRootFlushR * max(0.0,pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);

				// Leaves
				//SPRUCE
					//pBiom->fLeafGrowR	= fFlushRed*pPGen->vLeafFlushR * (pPBiom->vKLeaf-pBiom->fLeafWeight) * DeltaT;
				//else
					pBiom->fLeafGrowR	= (float)0.0;

				// Wood
				    pPBiom->vWoodInc = max(0.0,pPGen->vMaxPlantGrowR * vAutumn * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
				//    pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * vAutumn * pBiom->fTotalBiomass * DeltaT 
				//	    - pBiom->fRootGrowR - pBiom->fLeafGrowR);

				pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
				
				pBiom->fStemGrowR		= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*(1.0-vBranchFrac));
				
				pBiom->fBranchGrowR		= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*vBranchFrac);

				//Fruits
				pBiom->fFruitGrowR = (float)(pPGen->vMaxPlantGrowR * (pPBiom->vKFruit-pBiom->fFruitWeight)
							* DeltaT);
			}
			break;

		case 5:
			HERBS //Reife
			{
				pBiom->fRootGrowR		= (float)0.0;
				pBiom->fStemGrowR		= (float)0.0;
				pBiom->fLeafGrowR		= (float)0.0;
				pBiom->fFruitGrowR		= (float)0.0;
				pBiom->fTuberGrowR		= (float)0.0;
			}

			TREES // 
			{
				//vFlushRedR = max(0.0,1.5-0.25*pPlant->pDevelop->fDevStage);
				vFlushRedR = max(0.0,3.0-0.5*pPlant->pDevelop->fDevStage);

				//vAutumn = (float)max(0.0,(6.0-pPlant->pDevelop->fDevStage)/2.0);
				//vAutumn = (float)max(0.0,1.75-pPlant->pDevelop->fDevStage/4.0);
				vAutumn = 0.0;

				//Fine Roots
				pBiom->fRootGrowR = (float)(vFlushRedR*pPGen->vFineRootFlushR 
                    * max(0.0,pPBiom->vKFRoot-pBiom->fRootWeight) * DeltaT);
				
				//Leaves
				SPRUCE
				//	pBiom->fLeafGrowR	= fFlushRed*pPGen->vLeafFlushR * (pPBiom->vKLeaf-pBiom->fLeafWeight) * DeltaT;
					pBiom->fLeafGrowR = (float)0.0;
				else
					pBiom->fLeafGrowR = (float)0.0;

				// Wood
				    pPBiom->vWoodInc = max(0.0,pPGen->vMaxPlantGrowR * vAutumn * pPBiom->vLivingBiomass * DeltaT 
					    - pBiom->fRootGrowR - pBiom->fLeafGrowR);
				//    pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * vAutumn * pBiom->fTotalBiomass * DeltaT 
				//	    - pBiom->fRootGrowR - pBiom->fLeafGrowR);

				pBiom->fGrossRootGrowR	= (float)(pPBiom->vWoodInc*vUGrWoodFrac);
				
				pBiom->fStemGrowR		= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*(1.0-vBranchFrac));
				
				pBiom->fBranchGrowR		= (float)(pPBiom->vWoodInc*(1.0-vUGrWoodFrac)*vBranchFrac);

				//Fruits
				pBiom->fFruitGrowR		= (float)0.0;

			}

			break;

		case 6:
			HERBS //Dormanz
			{
				pBiom->fRootGrowR		= (float)0.0;
				pBiom->fStemGrowR		= (float)0.0;
				pBiom->fLeafGrowR		= (float)0.0;
				pBiom->fFruitGrowR		= (float)0.0;
				pBiom->fTuberGrowR		= (float)0.0;
			}

			TREES // 
			{
				vAutumn = 0.0;
				pBiom->fRootGrowR = (float)0.0;
				pBiom->fLeafGrowR	= (float)0.0;

				pPBiom->vWoodInc		= 0.0;
				pBiom->fGrossRootGrowR	= (float)0.0;
				pBiom->fStemGrowR		= (float)0.0;
				pBiom->fBranchGrowR		= (float)0.0;
								
/*				pPBiom->vWoodInc = (float)max(0.0,pPGen->vMaxPlantGrowR * vAutumn * pBiom->fTotalBiomass * DeltaT 
					- pBiom->fRootGrowR - pBiom->fLeafGrowR);

				pBiom->fGrossRootGrowR	= pPBiom->vWoodInc*vUGrWoodFrac;
				
				pBiom->fStemGrowR		= 
					pPBiom->vWoodInc*((float)1.0-vUGrWoodFrac)*((float)1.0-vBranchFrac);
				
				pBiom->fBranchGrowR		= 
					pPBiom->vWoodInc*((float)1.0-vUGrWoodFrac)*vBranchFrac;
*/		
				
				//Fruits
				pBiom->fFruitGrowR		= (float)0.0;

			}

			break;

	}

	//potential total growth rate (structural biomass + defense pool) [kg(dry matter)]
	pBiom->fRootGrowR		= (float)max(0.0,pBiom->fRootGrowR*vTempEffect);
	pBiom->fGrossRootGrowR	= (float)max(0.0,pBiom->fGrossRootGrowR*vTempEffect);
	pBiom->fStemGrowR		= (float)max(0.0,pBiom->fStemGrowR*vTempEffect);
	pBiom->fBranchGrowR		= (float)max(0.0,pBiom->fBranchGrowR*vTempEffect);
	pBiom->fLeafGrowR		= (float)max(0.0,pBiom->fLeafGrowR*vTempEffect);
	pBiom->fFruitGrowR		= (float)max(0.0,pBiom->fFruitGrowR*vTempEffect);
	pBiom->fTuberGrowR		= (float)max(0.0,pBiom->fTuberGrowR*vTempEffect);


	//Total potential growth rate [kg(dry matter)]:
	pBiom->fBiomGrowR = pBiom->fRootGrowR + pBiom->fGrossRootGrowR + pBiom->fTuberGrowR + pBiom->fStemGrowR + 
						pBiom->fBranchGrowR + pBiom->fLeafGrowR + pBiom->fFruitGrowR;

	if(pBiom->fBiomGrowR == (float)0.0)
	{
		pPPart->vDemandForDefense = 0.0;
		return 0;
	}

	//growth efficiency of structural biomass [g(glucose)/g(dry matter)]
	vXiStructuralBiomass = GetCostsForGrowth(exp_p2, iPlant)/pBiom->fBiomGrowR;


	//Abwehr:
	//vDefenseFactor = sigma_C + sigma_I

    pPStress->dInducedDefense = min(1.0,pPStress->vO3IndDefense*pow(pPStress->vO3StressIntensity,1)
        + pPStress->vLfPathIndDefense*pow(pPlathoScenario->vLeafPathogenes,1)
        + pPStress->vRtPathIndDefense*pow(pPlathoScenario->vRootPathogenes,1));

    vDefenseFactor =  min(1.0,max(0.0,pPStress->vConstDefense
        + pPStress->dInducedDefense)); //%/100


	help1 = pBiochemistry->pGrowthEfficiency->vDefComp * (1.0-vDefenseFactor);
	help2 = vXiStructuralBiomass * vDefenseFactor;
	help1Base = pBiochemistry->pGrowthEfficiency->vDefComp * (1.0-pPStress->vConstDefenseBase);
	help2Base = vXiStructuralBiomass * pPStress->vConstDefenseBase;
	help2Induced = vXiStructuralBiomass * pPStress->dInducedDefense;

	//reduced potential growth rates (kg(dry matter)/plant/day)
	pBiom->fRootGrowR		*= (float)(help1/(help1+help2));
	pBiom->fGrossRootGrowR	*= (float)(help1/(help1+help2));
	pBiom->fStemGrowR		*= (float)(help1/(help1+help2));
	pBiom->fBranchGrowR		*= (float)(help1/(help1+help2));
	pBiom->fLeafGrowR		*= (float)(help1/(help1+help2));
	pBiom->fFruitGrowR		*= (float)(help1/(help1+help2));
	pBiom->fTuberGrowR		*= (float)(help1/(help1+help2));

	//Demand for defense-related compounds (kg(glucose)/plant/day):
	pPPart->vDemandForDefense = pBiom->fBiomGrowR*help2/(help1+help2)*pBiochemistry->pGrowthEfficiency->vDefComp;
	pPPart->vDemandForDefenseBase = pBiom->fBiomGrowR*help2Base/(help1+help2)*pBiochemistry->pGrowthEfficiency->vDefComp;
    pPPart->dDemandForDefInduced = pBiom->fBiomGrowR*help2Induced/(help1+help2)*pBiochemistry->pGrowthEfficiency->vDefComp;
    pPPart->dDemandForDefConstitutive = pPPart->vDemandForDefense - pPPart->dDemandForDefInduced;

	//Total potential growth [kg(dry matter)]:
	pBiom->fBiomGrowR = pBiom->fRootGrowR + pBiom->fGrossRootGrowR + pBiom->fTuberGrowR + pBiom->fStemGrowR + 
						pBiom->fBranchGrowR + pBiom->fLeafGrowR + pBiom->fFruitGrowR;
	return 1;
}


double	MaintenanceRespiration(EXP_POINTER2,int iPlant, double TempAct)
	{
		// Berechnung der aktuellen Erhaltungsatmung in kg(CO2)
		//
		// Berücksichtigung der Temperatur

		double Q10 = 2.0;
		double vTempEffect;
		double DeltaT = (double)pTi->pTimeStep->fAct;
        double vMaintRespR;

		extern int	GetMaintenanceRespirationCoefficients(PPLANT pPlant, int iPlant);

		PBIOMASS	pBiom = pPlant->pBiomass;
		PPLTCARBON	pCarb = pPlant->pPltCarbon;
		PGENOTYPE	pGen  = pPlant->pGenotype;

		PPLATHOPARTITIONING pPPart	= pPlathoPlant[iPlant]->pPlathoPartitioning;
		PPLATHOGENOTYPE		pPGen	= pPlathoPlant[iPlant]->pPlathoGenotype;
		PPLATHOBIOMASS		pPBiom	= pPlathoPlant[iPlant]->pPlathoBiomass;
		PPLATHODEVELOP		pPDev	= pPlathoPlant[iPlant]->pPlathoDevelop;
		PPLANTSTRESS		pPStress= pPlathoPlant[iPlant]->pPlantStress;

		//Temperatureffekt:
		vTempEffect = pow(Q10,(TempAct-25.0)/10.0);      //für A6 V2, wegen hoher Temperaturen im GSF-Gewächshaus
//		vTempEffect = (float)pow(Q10,(TempAct-20.0)/10.0);


		//	Koeffizienten der Erhaltungsatmung in Abhängigkeit
		//	vom N-Status der Organe
		GetMaintenanceRespirationCoefficients(pPlant, iPlant); // kg(CO2)/kg/d


	// Erhaltungsatmung während eines Zeitschritts
	pCarb->fMainRespRRoot		= (float)(pBiom->fRootWeight  * pGen->fMaintRoot * vTempEffect * DeltaT);
	pCarb->fMainRespRStem		= (float)(pBiom->fStemWeight  * pGen->fMaintStem * vTempEffect * DeltaT);
	pCarb->fMainRespRLeaf		= (float)(pBiom->fLeafWeight  * pGen->fMaintLeaf * vTempEffect * DeltaT);
	if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3))
		pCarb->fMainRespRLeaf	*= (float)pow(1.0+pPStress->vO3Sensitivity*pPStress->vO3StressIntensity,1);

	pCarb->fMainRespRGrain		= (float)(pBiom->fFruitWeight * pGen->fMaintStorage * vTempEffect * DeltaT);
	pCarb->fMainRespRGrossRoot	= (float)(pBiom->fGrossRootWeight  * pGen->fMaintGrossRoot * vTempEffect * DeltaT);
	pCarb->fMainRespRBranch		= (float)(pBiom->fBranchWeight * pGen->fMaintBranch * vTempEffect * DeltaT);
	pCarb->fMainRespRTuber		= (float)(pBiom->fTuberWeight  * pGen->fMaintTuber  * vTempEffect * DeltaT);

	TREES
	{

        pCarb->fMainRespRStem		*= (float)pPBiom->vFracRespWood;
		pCarb->fMainRespRGrossRoot  *= (float)pPBiom->vFracRespWood;
		pCarb->fMainRespRBranch	    *= (float)pPBiom->vFracRespWood;
	}
	//Zusätzlich Kosten für Metabolismus und Regulation:

	// eigentlich müsste hier noch eine Zeitverzögerung um 
	// fMaintMetabolTimeDelay berücksichtigt werden
	pPPart->dCostsForMetabolism = pCarb->fGrossPhotosynR * pPGen->vMaintMetabol*0.01;

if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3))
        pPPart->dCostsForMetabolism *= pow(1.0+pPStress->vO3Sensitivity*pPStress->vO3StressIntensity,1);

	//gesamt:
	vMaintRespR =	pCarb->fMainRespRRoot +
							pCarb->fMainRespRStem +
							pCarb->fMainRespRLeaf +
							pCarb->fMainRespRGrain + 
							pCarb->fMainRespRGrossRoot + 
							pCarb->fMainRespRBranch	+ 
							pCarb->fMainRespRTuber +
							(float)pPPart->dCostsForMetabolism;

/*    if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3))
       pCarb->fMaintRespR *= (float)pow(1.0+pPStress->vO3Sensitivity*pPStress->vO3StressIntensity,1);
*/
	// Runterfahren der Stoffwechselaktivität bei Bäumen während der Wintermonate
	TREES
	{
		double vFacDorm;

		if((pPDev->iStagePlatho == 0)||(pPDev->iStagePlatho >= 5))
		{
			BEECH
				vFacDorm = 0.1; //fFacDorm = (float)0.5;
			APPLE
				vFacDorm = 0.1; //fFacDorm = (float)0.5;
			SPRUCE
				vFacDorm = 1.0; //fFacDorm = (float)0.5;

			vMaintRespR *= vFacDorm; 
		}
	}
	
    pCarb->fMaintRespR = (float)vMaintRespR;

	return vMaintRespR; /* kg(CO2) */
}

double	GrowthRespiration(EXP_POINTER2,int iPlant)
{
	PGENOTYPE			pGen		= pPlant->pGenotype;
	PBIOMASS			pBiom		= pPlant->pBiomass;
	PPLTCARBON			pCarb       = pPlant->pPltCarbon;

	//////////////////////////////////////////////////////////////////////////
	//Wachstumsatmung:	
	// GR[kg(CO2)] = dX(i)[kg(TS)] * GRCoeff[kg(CO2)/kg(TS)]
	//////////////////////////////////////////////////////////////////////////

	pCarb->fGrowRespRRoot		= pBiom->fRootGrowR * pGen->fCO2EffRoot;
	pCarb->fGrowRespRGrossRoot	= pBiom->fGrossRootGrowR * pGen->fCO2EffGrossRoot;
	pCarb->fGrowRespRTuber		= pBiom->fTuberGrowR * pGen->fCO2EffTuber;
	pCarb->fGrowRespRStem		= pBiom->fStemGrowR * pGen->fCO2EffStem;
	pCarb->fGrowRespRBranch		= pBiom->fBranchGrowR * pGen->fCO2EffBranch;
	pCarb->fGrowRespRLeaf		= pBiom->fLeafGrowR * pGen->fCO2EffLeaf;
	pCarb->fGrowRespRGrain		= pBiom->fFruitGrowR * pGen->fCO2EffStorage;

	return	  pCarb->fGrowRespRRoot 
							+ pCarb->fGrowRespRStem 
							+ pCarb->fGrowRespRLeaf 
							+ pCarb->fGrowRespRGrain 
							+ pCarb->fGrowRespRGrossRoot 
							+ pCarb->fGrowRespRBranch 
							+ pCarb->fGrowRespRTuber; /* kg(CO2)*/
}



double	GetCostsForGrowth(EXP_POINTER2, int iPlant)
{
	PGENOTYPE			pGen		= pPlant->pGenotype;
	PBIOMASS			pBiom		= pPlant->pBiomass;

	// Bedarf für den Aufbau neuer Biomasse///////////////////////////////////
	//																		//
	// Costs = Sum X(i)[kg(TS)] * CostCoeff(i)[kg(CH2O)/kg(TS)]				//
	//																		//
	//////////////////////////////////////////////////////////////////////////

	return	pBiom->fRootGrowR * pGen->fGrwEffRoot + 
			pBiom->fGrossRootGrowR * pGen->fGrwEffGrossRoot +
			pBiom->fStemGrowR * pGen->fGrwEffStem +
			pBiom->fBranchGrowR * pGen->fGrwEffBranch +
			pBiom->fLeafGrowR * pGen->fGrwEffLeaf +
			pBiom->fFruitGrowR * pGen->fGrwEffStorage +
			pBiom->fTuberGrowR * pGen->fGrwEffTuber;
}




