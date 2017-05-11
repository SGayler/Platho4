//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"

int PLATHO_StartValues(PPLANT pPlant, int iPlant, PSPROFILE pSo, PMANAGEMENT pMa);
int DGL_PLATHO(EXP_POINTER2,int iPlant);
int GetPlantNeighbours(PPLANT pPlant, int iPlant, int *iPlantLeft, int *iPlantRight, int *iPlantUpper, int *iPlantLower);

double RelativeTemperatureResponse_PLATHO(double vTemp, double vMinTemp, double vOptTemp, double vMaxTemp);
double Daylength(double vLatitude, int nJulianDay, int nID);
double HourlyTemperature_PLATHO	(double Time, double fTempMax, double fTempMin, double fTempMinNext, 
									 double vDaylength, int iSimDay);
double calc_seasonal_reduction(double Temperature);


double	Weibull(double, double, double);
double	factor1(double DevStage, double StageStart, double rho, double a);
double	GrowthFactor(double DevStage, double StageStart, double StagePeak, double StageEnd, double p);
int		Competition(PPLANT pPlant, PPLANT pPlantUpper, PPLANT pPlantLeft, PPLANT pPlantRight,
					PPLANT pPlantLower, int iPlant, int iPlantUpper, int iPlantLeft, int iPlantRight, 
					int iPlantLower, PSPROFILE pSo);
double	CompetitionFactor(double radius1, double radius2, double MeanDistance);
double	SectorCompFactor(double r_c, double r_nbg, double d);
double f_area(double r, double x1, double x2);
double f2_area(double x1, double x2);
double CalcLAICumNbg(int iLeafLayer, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg);
double CalcLA_Nbg(int iLeafLayer, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg);

double	CalcKappa(double dSpecLfWeightMax, double vLAI, int iPlant);

int Weight_kg_ha(PBIOMASS pBiom, float fSowDensity);
int Weight_kg_plant(PBIOMASS pBiom, float fSowDensity);

extern	int NewDay(PTIME pz);
extern	int	GetNitrogenConcentrationLimits(PPLANT, int);
extern	int		PlantArea(PPLANT pPlant, int iPlant);
extern	int		StemDiameter(PPLANT pPlant, int iPlant);
extern	double	LeafArea(PPLANT pPlant, int iPlant);
extern	int		CrownAndRootZoneDiameter(PPLANT pPlant, int iPlant);
extern int		CrownAndRootVolume(PPLANT pPlant, int iLayers, int iPlant);



	

int DGL_PLATHO(EXP_POINTER2,int iPlant)
{
	PBIOMASS		pBiom = pPlant->pBiomass;
	PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;
	PPLTCARBON		pCarb = pPlant->pPltCarbon;
	PGENOTYPE		pGen  = pPlant->pGenotype;
	PCANOPY			pCan  = pPlant->pCanopy;

	PPLATHODEVELOP		pPDev	  = pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHOGENOTYPE		pPGen	  = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOBIOMASS		pPBiom	  = pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLATHONITROGEN		pPPltN    = pPlathoPlant[iPlant]->pPlathoNitrogen;
	PPLATHOMORPHOLOGY	pPMorph	  = pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOPARTITIONING pPPart	  = pPlathoPlant[iPlant]->pPlathoPartitioning;
	PPLANTMATERIAL		pMaterial = pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;
	PCARBON				pCarbon   = pBiochemistry->pCarbon;
	PPLANTSTRESS		pPStress  = pPlathoPlant[iPlant]->pPlantStress;

    //PLEAFLAYER      pLLayer = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;


	extern	int	LeafNitrogenDistribution(double, double, PPLANT pPlant, PLEAFLAYER);

	double DeltaT = pTi->pTimeStep->fAct;		// Zeitschritt
	int   N,iSector;

	double faktor; //MeanDistance, 

	double dAssimilatePoolGrowthRate, dReservesGrowthRate, dCBalanceTimeStep;
    double dfracLeaf = 0.0;
    double dfracBranch = 0.0;
    double dfracStem= 0.0;
    double dfracGrossRoot= 0.0;
    double dfracFineRoot= 0.0;
    double dfracFruit= 0.0;
    double dfracTuber= 0.0;

	//Umrechnungsfaktor f[kg(CH2O)/kg(CO2)]:
	faktor =  30.0/44.0;

	// BEGINN DER DIFFERENTIALGLEICHUNG:

    //ASSIMILATE POOL:
	dAssimilatePoolGrowthRate = pCarb->fGrossPhotosynR*faktor 
					 +  pPBiom->vAssRelocR
					 -  pBiom->fStemReserveGrowRate
					 +  pBiom->fReserveTransRate
					 -  pPPart->dCostsForBiomassGrowth
					 -  pPPart->dCostsForMaintenance
					 -  pPPart->dCostsForDefense
                     -  pPStress->dCostForDetox * 1e-3
                     -  pPStress->dCostsRepair * 1e-3;        //kg(CH2O)

	pPlant->pPltCarbon->fCAssimilatePool += (float)dAssimilatePoolGrowthRate;

	if(pPlant->pPltCarbon->fCAssimilatePool < (float)0.0)
		pPlant->pPltCarbon->fCAssimilatePool = (float)0.0;

    //RESERVES:
	HERBS
	{
		if(pPDev->iStagePlatho<4)
			pBiom->fStemReserveWeight += pBiom->fStemReserveGrowRate - pBiom->fReserveTransRate;
		else
		{
			pBiom->fStemReserveWeight -= pBiom->fReserveTransRate;

			POTATO
				pBiom->fTuberGrowR += pBiom->fStemReserveGrowRate;
			else				
				pBiom->fFruitGrowR += pBiom->fStemReserveGrowRate;
		}
	}

	TREES
		pBiom->fStemReserveWeight += pBiom->fStemReserveGrowRate - pBiom->fReserveTransRate;

	pBiom->fStemReserveWeight = (float)max(0.0,pBiom->fStemReserveWeight);

    //STRUCTURAL BIOMASS:
	pBiom->fRootWeight		+= pBiom->fRootGrowR	 - pBiom->fRootDeathRate;
	pBiom->fGrossRootWeight += pBiom->fGrossRootGrowR- pBiom->fGrossRootDeathRate;
	pBiom->fStemWeight		+= pBiom->fStemGrowR	 - pBiom->fStemDeathRate;
	pBiom->fBranchWeight	+= pBiom->fBranchGrowR	 - pBiom->fBranchDeathRate;
	pBiom->fLeafWeight		+= pBiom->fLeafGrowR	 - pBiom->fLeafDeathRate;
	pBiom->fFruitWeight		+= pBiom->fFruitGrowR	 - pBiom->fFruitDeathRate;
	pBiom->fTuberWeight		+= pBiom->fTuberGrowR	 - pBiom->fTuberDeathRate;

	pBiom->fWoodWeight = pBiom->fGrossRootWeight+pBiom->fStemWeight+pBiom->fBranchWeight;


    //für Lysimeter-Versuch:
	if((!lstrcmp((LPSTR)pLo->pFieldplot->acWeatherStation,(LPSTR)"GSF-Lysimeter"))&&
		(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BE")))
	{
    //absterben der Blätter bei Frostereignis im Mai 2005
    if (NewDay(pTi))
    {
        if(pTi->pSimTime->lTimeDate==110505)
            pBiom->fLeafWeight = (float)0.0;

     /* if(pTi->pSimTime->lTimeDate==pPlant->pPMeasure->lDate)
        {
            pBiom->fLeafWeight = pPlant->pPMeasure->fLeafWeight;
            if(pPlant->pPMeasure->pNext!=NULL) pPlant->pPMeasure = pPlant->pPMeasure->pNext;
        }  */
    }
	} // Ende für Lysimeter-Versuch
 
	pBiom->fGrainWeight = pBiom->fFruitWeight + pBiom->fTuberWeight;

	if(pBiom->fRootWeight<(float)0.0)		pBiom->fRootWeight=(float)0.0;
	if(pBiom->fGrossRootWeight<(float)0.0)	pBiom->fGrossRootWeight=(float)0.0;
	if(pBiom->fStemWeight<(float)0.0)		pBiom->fStemWeight=(float)0.0;
	if(pBiom->fBranchWeight<(float)0.0)		pBiom->fBranchWeight=(float)0.0;
	if(pBiom->fLeafWeight<(float)0.0)		pBiom->fLeafWeight=(float)0.0;
	if(pBiom->fFruitWeight<(float)0.0)		pBiom->fFruitWeight=(float)0.0;
	if(pBiom->fTuberWeight<(float)0.0)		pBiom->fTuberWeight=(float)0.0;

	pBiom->fTotalBiomass = pBiom->fRootWeight
		+pBiom->fGrossRootWeight
		+pBiom->fStemWeight
		+pBiom->fBranchWeight
		+pBiom->fLeafWeight
		+pBiom->fFruitWeight 
		+pBiom->fTuberWeight
		+pBiom->fSeedReserv;

    pPBiom->vFracRespWood = 1.0/(1.0+pPlant->pDevelop->fAge);
    pPBiom->vLivingBiomass = pBiom->fWoodWeight*pPBiom->vFracRespWood 
                                + pBiom->fRootWeight + pBiom->fLeafWeight;

	pBiom->fBiomassAbvGround = pBiom->fStemWeight+pBiom->fBranchWeight+pBiom->fLeafWeight;
    pPlant->pBiomass->fTotRootWeight    = pPlant->pBiomass->fRootWeight + pPlant->pBiomass->fGrossRootWeight;

    pBiom->fStovWeight                  = pBiom->fStemReserveWeight-pBiom->fFruitWeight-pBiom->fTuberWeight;

	//Atmung::

	pCarb->fTotRespR = pCarb->fMaintRespR  + pCarb->fGrowthRespR;


    //DEFENSIVE COMPOUNDS POOL:

    //Partitioning of the central 'Defensive Compounds Pool' to single organs:
    if(pBiom->fBiomGrowR>(float)1e-15)
    {
  /*      fracLeaf = pBiom->fLeafGrowR/pBiom->fBiomGrowR;
        fracBranch = pBiom->fBranchGrowR/pBiom->fBiomGrowR;
        fracStem = pBiom->fStemGrowR/pBiom->fBiomGrowR;
        fracGrossRoot = pBiom->fGrossRootGrowR/pBiom->fBiomGrowR;
        fracFineRoot = pBiom->fRootGrowR/pBiom->fBiomGrowR;
        fracFruit = pBiom->fFruitGrowR/pBiom->fBiomGrowR;
        fracTuber = pBiom->fTuberGrowR/pBiom->fBiomGrowR;
*/
    }

   if(pBiom->fBiomGrowR>(float)1e-15)
    {
        if((pPDev->iStagePlatho<2)&&(pBiom->fLeafGrowR>(float)0.0))
        {
            double fcorr;
            
//          fracLeaf = (float)1.0-pPlant->pDevelop->fDevStage/(float)2.0*(float)(1.0-pBiom->fLeafGrowR/pBiom->fBiomGrowR);
            dfracLeaf = 2.0-max(1.0,pPlant->pDevelop->fDevStage)
                +pBiom->fLeafGrowR/pBiom->fBiomGrowR*(max(1.0,pPlant->pDevelop->fDevStage)-1.0);

            if(dfracLeaf < 1.0)
                fcorr = (1.0-dfracLeaf)/((1.0-pBiom->fLeafGrowR/pBiom->fBiomGrowR)+EPSILON);
            else
                fcorr = 0.0;

            dfracBranch = fcorr*pBiom->fBranchGrowR/pBiom->fBiomGrowR;
            dfracStem = fcorr*pBiom->fStemGrowR/pBiom->fBiomGrowR;
            dfracGrossRoot = fcorr*pBiom->fGrossRootGrowR/pBiom->fBiomGrowR;
            dfracFineRoot = fcorr*pBiom->fRootGrowR/pBiom->fBiomGrowR;
            dfracFruit = fcorr*pBiom->fFruitGrowR/pBiom->fBiomGrowR;
            dfracTuber = fcorr*pBiom->fTuberGrowR/pBiom->fBiomGrowR;
       }
        else
        {
            dfracLeaf = pBiom->fLeafGrowR/pBiom->fBiomGrowR;
            dfracBranch = pBiom->fBranchGrowR/pBiom->fBiomGrowR;
            dfracStem = pBiom->fStemGrowR/pBiom->fBiomGrowR;
            dfracGrossRoot = pBiom->fGrossRootGrowR/pBiom->fBiomGrowR;
            dfracFineRoot = pBiom->fRootGrowR/pBiom->fBiomGrowR;
            dfracFruit = pBiom->fFruitGrowR/pBiom->fBiomGrowR;
            dfracTuber = pBiom->fTuberGrowR/pBiom->fBiomGrowR;
        }
    }

    if((pPlathoModules->iFlagO3==1)||(pPlathoModules->iFlagO3==4))
        pPStress->vDefCompTurnoverR = pPStress->vDefCompTurnoverR0;
 //       pPStress->vDefCompTurnoverR = pPStress->vDefCompTurnoverR0*((float)1.0+pPStress->vO3StressIntensity);

    if((pPlathoModules->iFlagO3==2)||(pPlathoModules->iFlagO3==3))
        pPStress->vDefCompTurnoverR *= pow(1.0+pPStress->vO3Sensitivity*pPStress->vO3StressIntensity,1);

    //Leaves:
    pPStress->vLeafDefCont += dfracLeaf * pPStress->vDefCompR 
        - pBiom->fLeafDeathRate*pPStress->vLeafDefConc / (1.0-pPStress->vLeafDefConc)
		- pPStress->vLeafDefCont*pPStress->vDefCompTurnoverR*DeltaT;
	//	- pPStress->vLeafDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vLeafDefCont<0.0)
		pPStress->vLeafDefCont = 0.0;

    if(pBiom->fLeafWeight>(float)0.0)
        pPStress->vLeafDefConc = pPStress->vLeafDefCont/(pBiom->fLeafWeight + pPStress->vLeafDefCont);
    else
    {
        pPStress->vLeafDefCont = 0.0;
        pPStress->vLeafDefConc = 0.0;
    }


    //Fine roots:
    pPStress->vFineRootDefCont += dfracFineRoot * pPStress->vDefCompR 
        - pBiom->fRootDeathRate*pPStress->vFineRootDefConc
		- pPStress->vFineRootDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vFineRootDefCont<0.0)
		pPStress->vFineRootDefCont = 0.0;

    if(pBiom->fRootWeight>(float)0.0)
        pPStress->vFineRootDefConc = pPStress->vFineRootDefCont/(pBiom->fRootWeight + pPStress->vFineRootDefCont);
    else
    {
        pPStress->vFineRootDefConc = 0.0;
        pPStress->vFineRootDefCont = 0.0;
    }

    //Gross Roots:
    pPStress->vGrossRootDefCont += dfracGrossRoot * pPStress->vDefCompR 
        - pBiom->fGrossRootDeathRate*pPStress->vGrossRootDefConc
		- pPStress->vGrossRootDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vGrossRootDefCont<0.0)
		pPStress->vGrossRootDefCont = 0.0;

    if(pBiom->fGrossRootWeight>(float)0.0)
        pPStress->vGrossRootDefConc = pPStress->vGrossRootDefCont/(pBiom->fGrossRootWeight + pPStress->vGrossRootDefCont);
    else
    {
        pPStress->vGrossRootDefConc = 0.0;
        pPStress->vGrossRootDefCont = 0.0;
    }


    //Branches:
    pPStress->vBranchDefCont += dfracBranch * pPStress->vDefCompR 
        - pBiom->fBranchDeathRate*pPStress->vBranchDefConc
		- pPStress->vBranchDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vBranchDefCont<0.0)
		pPStress->vBranchDefCont = 0.0;

    if(pBiom->fBranchWeight>(float)0.0)
        pPStress->vBranchDefConc = pPStress->vBranchDefCont/(pBiom->fBranchWeight + pPStress->vBranchDefCont);
    else
    {
        pPStress->vBranchDefConc = 0.0;
        pPStress->vBranchDefCont = 0.0;
    }


    //Stem:
    pPStress->vStemDefCont += dfracStem * pPStress->vDefCompR 
        - pBiom->fStemDeathRate*pPStress->vStemDefConc
		- pPStress->vStemDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vStemDefCont<0.0)
		pPStress->vStemDefCont = 0.0;

    if(pBiom->fStemWeight>(float)0.0)
        pPStress->vStemDefConc = pPStress->vStemDefCont/(pBiom->fStemWeight + pPStress->vStemDefCont);
    else
    {
        pPStress->vStemDefConc = 0.0;
        pPStress->vStemDefCont = 0.0;
    }


    //Fruits:
    pPStress->vFruitDefCont += dfracFruit * pPStress->vDefCompR 
        - pBiom->fFruitDeathRate*pPStress->vFruitDefConc
		- pPStress->vFruitDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vFruitDefCont<0.0)
		pPStress->vFruitDefCont = 0.0;

    if(pBiom->fFruitWeight>(float)0.0)
        pPStress->vFruitDefConc = pPStress->vFruitDefCont/(pBiom->fFruitWeight + pPStress->vFruitDefCont);
    else
    {
        pPStress->vFruitDefConc = 0.0;
        pPStress->vFruitDefCont = 0.0;
    }

    //Tubers:
    pPStress->vTuberDefCont += dfracTuber * pPStress->vDefCompR 
        - pBiom->fTuberDeathRate*pPStress->vTuberDefConc
		- pPStress->vTuberDefCont*pPStress->vDefCompTurnoverR*DeltaT;

    if(pPStress->vTuberDefCont<0.0)
		pPStress->vTuberDefCont = 0.0;

    if(pBiom->fTuberWeight>(float)0.0)
        pPStress->vTuberDefConc = pPStress->vTuberDefCont/(pBiom->fTuberWeight + pPStress->vTuberDefCont);
    else
    {
        pPStress->vTuberDefConc = 0.0;
        pPStress->vTuberDefCont = 0.0;
    }

	//total defensive compounds [kg]
	pPStress->vDefCompCont = pPStress->vLeafDefCont 
                            + pPStress->vFineRootDefCont 
                            + pPStress->vGrossRootDefCont 
                            + pPStress->vStemDefCont 
                            + pPStress->vBranchDefCont 
                            + pPStress->vFruitDefCont 
                            + pPStress->vTuberDefCont;


	    
    //STICKSTOFFKONZENTRATIONEN UND STICKSTOFFVERTEILUNG IN DEN BLÄTTERN
 //   if(pPlathoModules->iFlagNEffectGrw==2)// N effect on growth included
  // {
	    pPltN->fRootCont		+= pPltN->fRootNinc		- pPltN->fRootNtransRate - pBiom->fRootDeathRate*pPltN->fRootActConc;
	    pPltN->fGrossRootCont	+= pPltN->fGrossRootNinc - pPltN->fGrossRootNtransRate - pBiom->fGrossRootDeathRate*pPltN->fGrossRootActConc;
	    pPltN->fLeafCont		+= pPltN->fLeafNinc		- pPltN->fLeafNtransRate - pBiom->fLeafDeathRate*pPltN->fLeafActConc;
	    pPltN->fBranchCont		+= pPltN->fBranchNinc	- pPltN->fBranchNtransRate - pBiom->fBranchDeathRate*pPltN->fBranchActConc;
	    pPltN->fStemCont		+= pPltN->fStemNinc		- pPltN->fStemNtransRate - pBiom->fStemDeathRate*pPltN->fStemActConc;
	    pPltN->fGrainCont		+= pPltN->fGrainNinc	- (float)pPPltN->vFruitNtransRate - pBiom->fFruitDeathRate*pPltN->fFruitActConc;
	    pPltN->fTuberCont		+= pPltN->fTuberNinc	- pPltN->fTuberNtransRate - pBiom->fTuberDeathRate*pPltN->fTuberActConc;

    	
	    HERBS
	    {
		    if(pPDev->iStagePlatho <= 3)
			    pPltN->fStemCont += (float)pPBiom->vNRelocR;
		    else
		    {
			    POTATO
				    pPltN->fTuberCont += (float)pPBiom->vNRelocR;
			    else
				    pPltN->fGrainCont += (float)pPBiom->vNRelocR;
		    }
	    }

	    TREES
	    {
		    pPltN->fGrossRootCont += (float)(pPBiom->vNRelocR * pBiom->fGrossRootWeight/(pBiom->fWoodWeight+EPSILON));
		    pPltN->fStemCont += (float)(pPBiom->vNRelocR * pBiom->fStemWeight/(pBiom->fWoodWeight+EPSILON));
		    pPltN->fBranchCont += (float)(pPBiom->vNRelocR * pBiom->fBranchWeight/(pBiom->fWoodWeight+EPSILON));
	    }
    	

    	
	    // keine negativen Stickstoffgehalte:
        pPltN->fRootCont		= (pBiom->fRootWeight > (float)0.0) ? (float)max(0.0,pPltN->fRootCont) : (float)0.0;
	    pPltN->fGrossRootCont	= (pBiom->fGrossRootWeight > (float)0.0) ? (float)max(0.0,pPltN->fGrossRootCont) : (float)0.0;
	    pPltN->fLeafCont		= (pBiom->fLeafWeight > (float)0.0) ? (float)max(0.0,pPltN->fLeafCont) : (float)0.0;
	    pPltN->fBranchCont		= (pBiom->fBranchWeight > (float)0.0) ? (float)max(0.0,pPltN->fBranchCont) : (float)0.0;
	    pPltN->fStemCont		= (pBiom->fStemWeight > (float)0.0) ? (float)max(0.0,pPltN->fStemCont) : (float)0.0;
	    pPltN->fGrainCont		= (pBiom->fFruitWeight > (float)0.0) ? (float)max(0.0,pPltN->fGrainCont) : (float)0.0;
	    pPltN->fTuberCont		= (pBiom->fTuberWeight > (float)0.0) ? (float)max(0.0,pPltN->fTuberCont) : (float)0.0;


	    // Stickstoffkonzentrationen:
	    if(pBiom->fRootWeight>(float)0.0)
		    pPltN->fRootActConc = pPltN->fRootCont/pBiom->fRootWeight;
	    else
		    pPltN->fRootActConc = (float)0.0;

	    if(pBiom->fGrossRootWeight>(float)0.0)
		    pPltN->fGrossRootActConc = pPltN->fGrossRootCont/pBiom->fGrossRootWeight;
	    else
		    pPltN->fGrossRootActConc = (float)0.0;

	    if(pBiom->fLeafWeight>(float)0.0)
		    pPltN->fLeafActConc = pPltN->fLeafCont/pBiom->fLeafWeight;
	    else
		    pPltN->fLeafActConc = (float)0.0;

	    if(pBiom->fBranchWeight>(float)0.0)
		    pPltN->fBranchActConc = pPltN->fBranchCont/pBiom->fBranchWeight;
	    else
		    pPltN->fBranchActConc = (float)0.0;

	    if(pBiom->fStemWeight>(float)0.0)
		    pPltN->fStemActConc = pPltN->fStemCont/pBiom->fStemWeight;
	    else
		    pPltN->fStemActConc = (float)0.0;

	    if(pBiom->fFruitWeight>(float)0.0)
		    pPltN->fFruitActConc = pPltN->fGrainCont/pBiom->fFruitWeight;
	    else
		    pPltN->fFruitActConc = (float)0.0;

	    if(pBiom->fTuberWeight>(float)0.0)
		    pPltN->fTuberActConc = pPltN->fTuberCont/pBiom->fTuberWeight;
	    else
		    pPltN->fTuberActConc = (float)0.0;


	    pPltN->fRootCont	  = pPltN->fRootActConc	*pBiom->fRootWeight;
	    pPltN->fGrossRootCont = pPltN->fGrossRootActConc *pBiom->fGrossRootWeight;
	    pPltN->fStemCont	  = pPltN->fStemActConc	*pBiom->fStemWeight;
	    pPltN->fBranchCont	  = pPltN->fBranchActConc *pBiom->fBranchWeight;
	    pPltN->fLeafCont	  = pPltN->fLeafActConc	*pBiom->fLeafWeight;
	    pPltN->fGrainCont	  = pPltN->fFruitActConc *pBiom->fFruitWeight;
	    pPltN->fTuberCont	  = pPltN->fTuberActConc *pBiom->fTuberWeight;


	    pPltN->fTotalCont = (float)(pPPltN->vNSeedReserves 
		    + pPltN->fRootCont + pPltN->fGrossRootCont 
		    + pPltN->fStemCont + pPltN->fBranchCont 
		    + pPltN->fLeafCont + pPltN->fGrainCont + pPltN->fTuberCont); 

    //} //end iFlagNEffectGrw included

    //Rubisco:
    if(pPlathoModules->iFlagO3 == 4) //O3-stress following van Oijen
    {
        pPStress->dRubiscoCont += (pBiom->fLeafGrowR*pPGen->dRubiscoConcCrit-pBiom->fLeafDeathRate*pPStress->dRubiscoConc)*1e3;
        if(pPStress->dRubiscoCont<0.0) pPStress->dRubiscoCont = 0.0;

        if(pBiom->fLeafWeight > (float)0.0)
            pPStress->dRubiscoConc = pPStress->dRubiscoCont/(pBiom->fLeafWeight*1e3);
        else
        {
            pPStress->dRubiscoConc = 0.0;
            pPStress->dRubiscoCont = 0.0;
        }
    }


	//ENDE DER DIFFERENTIALGLEICHUNGEN

 //   if(pPlathoModules->iFlagNEffectGrw==2)// N effect on growth included
 //   {
	    //Leaf Nitrogen concentration
	    if(pPlathoModules->iFlagNitrogenDistribution==1)//N homogen verteilt auf Blattschichten und -sektoren:
	    {
             PLEAFLAYER pLLayer = pPMorph->pLeafLayer;       
             for(N = 1;N<=LEAFLAYERS;N++)
             {
			    pLLayer->vLeafNConc = pPltN->fLeafActConc;

                if(pPlathoModules->iFlagSectors==2)
                {
                 for(iSector=0;iSector<4;iSector++)
                    {
			            pLLayer->apLeafSector[iSector]->vLeafNConc = pPltN->fLeafActConc;
                    }
                }

                 pLLayer = pLLayer->pNext;
             }
	    }
	    else if(pPlathoModules->iFlagNitrogenDistribution==2)//N-Verteilung auf Blattschichten:
	    {
            double vLfWeightCrit1, vLfWeightCrit2;
            PLEAFLAYER pLLayer;       

            vLfWeightCrit1 = pBiom->fLeafWeight * (1.0 -
			    2.0*(pPltN->fLeafActConc-pPltN->fLeafMinConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc));

		    vLfWeightCrit2 = 2.0*pBiom->fLeafWeight
			    *(pPltN->fLeafOptConc-pPltN->fLeafActConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc);

             pLLayer = pPMorph->pLeafLayer;
             for(N = 1;N<=LEAFLAYERS;N++)
             {
			    LeafNitrogenDistribution(vLfWeightCrit1, vLfWeightCrit2, pPlant, pLLayer);
                 
                pLLayer = pLLayer->pNext;
             }
	    }
	    else //default
	    {
             PLEAFLAYER pLLayer = pPMorph->pLeafLayer;       
             for(N = 1;N<=LEAFLAYERS;N++)
             {
			    pLLayer->vLeafNConc = pPltN->fLeafActConc;

                if(pPlathoModules->iFlagSectors==2)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
			            pLLayer->apLeafSector[iSector]->vLeafNConc = pPltN->fLeafActConc;
                    }
                }

                pLLayer = pLLayer->pNext;
             }
	    }
    //}      // end N effect on growth included

	//new Leaf Area [m^2]
	LeafArea(pPlant, iPlant);

	//neuer Stamm-Durchmessr:
	StemDiameter(pPlant, iPlant);

    TREES
	{
		CrownAndRootZoneDiameter(pPlant, iPlant);
	}



	//new Plant Area [m^2]
	PlantArea(pPlant, iPlant);

/*	TREES
	{
		//new crown to stem diameter ratio
		CalcCrownToStemRatio(pPlant, iPlant);
	}
*/

    //Crown Volume
    CrownAndRootVolume(pPlant, pSo->iLayers-2, iPlant);

        
    // Pflanzen interne C- Bilanz

	// Input: 
	//		Photosynthese
	// Output:
	//		Erhaltungsatmung
	//		Wachstumsatmung
	//		Absterben
	//		Rhizodeposition
	//
	//	Bilanz = strukturelle Biomass + Assimilat-Pool + Reserven + Abwehrstoffe 
	//           + Atmung + Absterben + Rhizodeposition - Photosynthese

	if(((int)pTi->pSimTime->fTimeAct == pMa->pSowInfo->iDay)&&(pTi->pSimTime->fTimeDay==(float)0.0))
	{
		//Startwerte für C-Bilanz aus vorhandener Biomasse
/*		pPPart->vCBalance = pBiom->fRootWeight*pMaterial->pFineRoot->vFracC
					+pBiom->fGrossRootWeight*pMaterial->pGrossRoot->vFracC
					+pBiom->fStemWeight*pMaterial->pStem->vFracC
					+pBiom->fBranchWeight*pMaterial->pBranch->vFracC
					+pBiom->fLeafWeight*pMaterial->pLeaf->vFracC
					+pBiom->fFruitWeight*pMaterial->pFruit->vFracC
					+pBiom->fTuberWeight*pMaterial->pTuber->vFracC
					+pPlant->pPltCarbon->fCAssimilatePool*(float)(12.0/30.0)
					+pBiom->fStemReserveWeight*(float)(12.0/30.0) 
					+pPPltStress->vO3DefCompCont*pCarbon->fO3DefComp
					+pPPltStress->vLfPathDefCompCont*pCarbon->fLfPathDefComp
					+pPPltStress->vRtPathDefCompCont*pCarbon->fRtPathDefComp;
	*/

		pPPart->vCBalance = 0.0;

		pPPart->vCumPhotosynthesis			= 0.0;
		pPPart->dCostsForBiomassGrowthCum	= 0.0;
		pPPart->dCostsForMaintenanceCum		= 0.0;
		pPPart->dCostsForDefenseCum			= 0.0;

		pPPart->vCumTotalRespiration		= 0.0;
		pPPart->vCumMaintenanceRespiration	= 0.0;
		pPPart->vCumGrowthRespiration		= 0.0;
		pPPart->vCumLitterLossC				= 0.0;

		pPPart->vCumReservesGrow			= 0.0;
		pPPart->vCumReservesTrans			= 0.0;

        pPPart->vPARabsorbedCum              = 0.0;
	}

	if (NewDay(pTi))
    {
        pPPart->vPotTranspDay = 0.0;
        pPPart->vActTranspDay = 0.0;
        pPPltN->dPotNUptDay = 0.0;
        pPPltN->dActNUptDay = 0.0;
//		pPPart->vDailyGrossPhotosynthesis = (float)0.0;
		pPPart->vDailyMaintResp = 0.0;
		pPPart->vDailyGrowthResp = 0.0;

		pPPart->vDailyGrowth = 0.0;
		pPPart->vDailyMaintenance = 0.0;
		pPPart->vDailyDefense = 0.0;
		pPPart->dDailyDefInduced = 0.0;

        pPPart->vReservesGrowDay = 0.0;
    	pPPart->vReservesTransDay = 0.0;

        pPBiom->vFineRootGrowDay = 0.0;
        pPBiom->vLeafGrowDay = 0.0;
        pPBiom->vFineRootLossDay = 0.0;
        pPBiom->vLeafLossDay = 0.0;

        pPStress->vO3UptakeDay = 0.0;
    }
 
	//Aufsummieren der Tageswerte:
    //pPPart->vActTranspDay += pPlant->pPltWater->fActTranspdt;
    pPPart->vPotTranspDay += pPPart->vPotTranspR;  //cm^3
    pPPart->vActTranspDay += pPPart->vActTranspR; //cm^3
    //pPPltN->dNUptakeDay += pPltN->fActNUpt;
    pPPltN->dPotNUptDay += pPltN->fPotNUpt;  //kg
    pPPltN->dActNUptDay += pPltN->fActNUpt;  //kg
//	pPPart->vDailyGrossPhotosynthesis	+= pCarb->fGrossPhotosynR; //kg(CO2)
	pPPart->vDailyMaintResp			+= pCarb->fMaintRespR; //kg(CO2)
	pPPart->vDailyGrowthResp		+= pCarb->fGrowthRespR; //kg(CO2)

	pPPart->vDailyGrowth += pPPart->dCostsForBiomassGrowth;  //kg(CH2O)
	pPPart->vDailyMaintenance += pPPart->dCostsForMaintenance; //kg(CH2O)
	pPPart->vDailyDefense += pPPart->dCostsForDefense; //kg(CH2O)
	pPPart->dDailyDefInduced += pPPart->dCostsForDefInduced; //kg(CH2O)

    pPBiom->vFineRootGrowDay += pBiom->fRootGrowR;
    pPBiom->vLeafGrowDay += pBiom->fLeafGrowR;
    pPBiom->vFineRootLossDay += pBiom->fRootDeathRate + pPBiom->vRootDamage;
    pPBiom->vLeafLossDay += pBiom->fLeafDeathRate + pPBiom->vLeafDamageO3 + pPBiom->vLeafDamagePath;


	//Aufsummieren der kumulativen Werte:
	pPPart->vCumPhotosynthesis			+= pCarb->fGrossPhotosynR; //kg(CO2)
	pPPart->vCumMaintenanceRespiration	+= pCarb->fMaintRespR;//kg(CO2)
	pPPart->vCumGrowthRespiration		+= pCarb->fGrowthRespR;//kg(CO2)

	pPPart->dCostsForBiomassGrowthCum += pPPart->dCostsForBiomassGrowth;  //kg(CH2O)
	pPPart->dCostsForMaintenanceCum += pPPart->dCostsForMaintenance;  //kg(CH2O)
	pPPart->dCostsForDefenseCum += pPPart->dCostsForDefense;  //kg(CH2O)

    pPPart->vCumAssReloc                += pPBiom->vAssRelocR;    //kg(CH2O)
	pPPart->vCumLitterLossC				+=  
		  pBiom->fRootDeathRate*pMaterial->pFineRoot->vFracC  //kg(C)
		+ pBiom->fGrossRootDeathRate*pMaterial->pGrossRoot->vFracC
		+ pBiom->fStemDeathRate*pMaterial->pStem->vFracC
		+ pBiom->fBranchDeathRate*pMaterial->pBranch->vFracC
		+ pBiom->fLeafDeathRate*pMaterial->pLeaf->vFracC
		+ pBiom->fFruitDeathRate*pMaterial->pFruit->vFracC
		+ pBiom->fTuberDeathRate*pMaterial->pTuber->vFracC;
	//	RDCum += pPPart->dCostsForRhizodeposition;

	pPPart->vReservesGrowDay	+= pBiom->fStemReserveGrowRate;
	pPPart->vReservesTransDay	+= pBiom->fReserveTransRate;

	pPPart->vCumReservesGrow	+= pBiom->fStemReserveGrowRate;
	pPPart->vCumReservesTrans	+= pBiom->fReserveTransRate;



	//C-Bilanz aus vorhandener Biomasse
/*	pPPart->vCBalance = pBiom->fRootWeight*pMaterial->pFineRoot->vFracC
					+pBiom->fGrossRootWeight*pMaterial->pGrossRoot->vFracC
					+pBiom->fStemWeight*pMaterial->pStem->vFracC
					+pBiom->fBranchWeight*pMaterial->pBranch->vFracC
					+pBiom->fLeafWeight*pMaterial->pLeaf->vFracC
					+pBiom->fFruitWeight*pMaterial->pFruit->vFracC
					+pBiom->fTuberWeight*pMaterial->pTuber->vFracC
					+pPlant->pPltCarbon->fCAssimilatePool*(float)(12.0/30.0)
					+pBiom->fStemReserveWeight*(float)(12.0/30.0) 
					+pPPltStress->vO3DefCompCont*pCarbon->fO3DefComp
					+pPPltStress->vLfPathDefCompCont*pCarbon->fLfPathDefComp
					+pPPltStress->vRtPathDefCompCont*pCarbon->fRtPathDefComp
					- PSCum + RDCum + MRCum + GRCum + DeathCum;
					*/
	
	// C-balance

	dReservesGrowthRate = pBiom->fStemReserveGrowRate - pBiom->fReserveTransRate;	

	dCBalanceTimeStep =
		pCarb->fGrossPhotosynR*12.0/44.0				// photosynthesis
		-pPPart->dCostsForBiomassGrowth*12.0/30.0		// structural biomass growth
		-(dAssimilatePoolGrowthRate-pPBiom->vAssRelocR)*12.0/30.0// increment of assimilate pool
		-dReservesGrowthRate*12.0/30.0					// increment of reserve pool
		-pPPart->dCostsForMaintenance*12.0/30.0		// maintenance
		-pPPart->dCostsForDefense*12.0/30.0         // defence
        -  pPStress->dCostForDetox*1e-3 * 12.0/30.0
        -  pPStress->dCostsRepair*1e-3 * 12.0/30.0;        //kg(CH2O)

	pPPart->vCBalance += dCBalanceTimeStep;

	return 1;
}



int PLATHO_StartValues(PPLANT pPlant, int iPlant, PSPROFILE pSo, PMANAGEMENT pMa)
{
    extern int ReadFalgeParameters(int iPlant, PPLANT pPlant);

    double RootFraction[MAXSOILLAYERS];
    double vLeafAreaFac[LEAFLAYERS];
    double TLLDF;
	double vCumRootFrac, Depth, vCalcDepth, h;
	double alpha,beta,gamma;
	double TT1,TT2;

    double  DEPMAX;

    int	  L,iStage,NLayer,N,iSector;
	int	  iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;

	double FRDiameter, FRArea;
	double GRDiameter =0.0;
	double GRArea	  =0.0;

	//float fMaxLAI;
	double alphaN, vMinimalMeanSpecLfWeight, kappa;

	double vRootDens;

    double vPlantDensity = pPlathoScenario->vTotalPlantDensity;


	PGENOTYPE		pGen	= pPlant->pGenotype;
	PDEVELOP		pDev	= pPlant->pDevelop;
	PCANOPY			pCan	= pPlant->pCanopy;
	PPLTNITROGEN	pPltN	= pPlant->pPltNitrogen;
	PROOT			pRT		= pPlant->pRoot;
	PLAYERROOT		pLR		= pPlant->pRoot->pLayerRoot;
	PSLAYER 		pSL		= pSo->pSLayer;
	PBIOMASS		pBiom	= pPlant->pBiomass;

	PPLATHODEVELOP		pPDev   = pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHOBIOMASS		pPBiom	= pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLATHOGENOTYPE		pPGen	= pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHONITROGEN		pPPltN	= pPlathoPlant[iPlant]->pPlathoNitrogen;
	PPLANTSTRESS		pPStress = pPlathoPlant[iPlant]->pPlantStress;
	PPLATHOMORPHOLOGY	pPMorph = pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOROOTLAYER	pPRL	= pPMorph->pPlathoRootLayer;
    PLEAFLAYER          pLLayer = pPMorph->pLeafLayer;
	PPLANT				pPlantLeft, pPlantRight, pPlantUpper, pPlantLower;

   	extern double	LeafAreaCum(PPLANT pPlant, double vLeafArea, double vPlantHeight, double h, int);
    extern double   LeafAreaDens(double vPlantHeight, double h, double vPeak);
	extern int      GetGrowthCapacities(PPLANT pPlant, int iPlant);

	int		GetBiochemicalData(int);
	int		GetGrowthEfficiencyAndRespirationCoefficients(PPLANT pPlant, int);
	int		GetMaintenanceRespirationCoefficients(PPLANT pPlant, int);



	//======================================================================================
	//Development Stage
	//======================================================================================
	
	iStage = (int)pPlant->pDevelop->fDevStage;

	if(iStage>=1) pPlant->pDevelop->iDayAftEmerg = 1;

	if(iStage>5) iStage =0; //Dormanz
	
	if(iStage==0)
		TT1 = 0;
	else
		TT1 = pPlant->pGenotype->pStageParam->afThermalTime[iStage-1];

	TT2 = pPlant->pGenotype->pStageParam->afThermalTime[iStage];
	
	pPlant->pDevelop->fCumDTT = (float)(TT1	+ (pPlant->pDevelop->fDevStage-(float)iStage)
																		* (TT2-TT1));

	pPlathoPlant[iPlant]->pPlathoDevelop->iStagePlatho = iStage;

   	//======================================================================================
	//Plant density
	//======================================================================================

			
	if (pCan->fPlantDensity==(float)0.0) 
	{
		TREES
			pCan->fPlantDensity=(float)1.0;
		BARLEY
			pCan->fPlantDensity=(float)300.0;
		WHEAT
			pCan->fPlantDensity=(float)400.0;	
		MAIZE
			pCan->fPlantDensity=(float)11.0;
		SUNFLOWER
			pCan->fPlantDensity=(float)10.0;
		POTATO
			pCan->fPlantDensity=(float)5.0;
		ALFALFA
			pCan->fPlantDensity=(float)100.0;
		LOLIUM
			pCan->fPlantDensity=(float)1000.0;
	}
		
	//----------------------------------------------------------------------------
	// initial nitrogen contents 
	//----------------------------------------------------------------------------

	GetNitrogenConcentrationLimits(pPlant,iPlant);


	pPltN->fRootActConc	= pPltN->fNStressFac 
					* (pPltN->fRootOptConc - pPltN->fRootMinConc) 
					+ pPltN->fRootMinConc; 

	pPltN->fGrossRootActConc= pPltN->fNStressFac 
					* (pPltN->fGrossRootOptConc - pPltN->fGrossRootMinConc) 
					+ pPltN->fGrossRootMinConc; 

	pPltN->fStemActConc	= pPltN->fNStressFac 
					* (pPltN->fStemOptConc - pPltN->fStemMinConc) 
					+ pPltN->fStemMinConc; 

	pPltN->fBranchActConc= pPltN->fNStressFac 
					* (pPltN->fBranchOptConc - pPltN->fBranchMinConc) 
					+ pPltN->fBranchMinConc; 

	pPltN->fLeafActConc	= pPltN->fNStressFac 
					* (pPltN->fLeafOptConc - pPltN->fLeafMinConc) 
					+ pPltN->fLeafMinConc; 
	
	
  	pPltN->fFruitActConc = (float)0.0;
	pPltN->fTuberActConc = (float)0.0;

		
	pPltN->fLeafCont  = pBiom->fLeafWeight * pPltN->fLeafActConc; 
	pPltN->fStemCont  = pBiom->fStemWeight * pPltN->fStemActConc;	//[kg(N)]
	pPltN->fBranchCont= pBiom->fBranchWeight * pPltN->fBranchActConc; //[kg(N)]
	pPltN->fGrossRootCont= pBiom->fGrossRootWeight * pPltN->fGrossRootActConc; //[kg(N)]
	pPltN->fRootCont  = pBiom->fRootWeight * pPltN->fRootActConc; //[kg(N)]
	pPltN->fGrainCont = (float)0.0; 
	pPltN->fTuberCont = (float)0.0; 

	pPltN->fTotalCont = pPltN->fRootCont + pPltN->fGrossRootCont 
		+ pPltN->fStemCont + pPltN->fBranchCont 
		+ pPltN->fLeafCont + pPltN->fGrainCont + pPltN->fTuberCont; 


	HERBS
	{
		POTATO
			pPPltN->vNSeedReserves = 0.02 * pBiom->fSeedReserv;
		else
			pPPltN->vNSeedReserves = 0.02 * pBiom->fSeedReserv;
	}

    // OZONE //
    
		pPStress->vO3ConcLeaf           = 0.0;
        pPStress->vO3StressIntensity    = 0.0;

        pPStress->vO3LeafConcCumCrit    = 0.0;
        pPStress->vO3LeafConcCum        = 0.0;
        pPStress->vO3DetoxCum           = 0.0;
        pPStress->vPotO3DetoxCum        = 0.0;
       
        pPStress->vO3UptakeCum          = 0.0;
    
    if(pPlathoModules->iFlagO3 == 4) //following van Oijen
    {
        pPStress->dRubiscoCont = pBiom->fLeafWeight * pPGen->dRubiscoConcCrit * 1e3;  //g(Rubisco)

        if(pBiom->fLeafWeight>(float)0.0)
            pPStress->dRubiscoConc = pPStress->dRubiscoCont/(pBiom->fLeafWeight * 1e3);
        else
            pPStress->dRubiscoConc = 0.0;
    }


	//Capacities of plant organs
	TREES //vPotFru aus FAGUS. Passt nicht für PLATHO (Einheiten!)
	{
		if(pPlant->pBiomass->fStemReserveWeight>(float)0.1*pPlant->pBiomass->fStemWeight)
		{
			pPlathoPlant[iPlant]->pPlathoPartitioning->vPotFru = 90.0*(1.0-exp(-0.001*((double)pPlant->pPltCarbon->fCAssimilatePool-4500.0)));//[kg(C)/ha]
		}
		
		if (pPlathoPlant[iPlant]->pPlathoPartitioning->vPotFru < 50.0)
			pPlathoPlant[iPlant]->pPlathoPartitioning->vPotFru = 0.0;
		else
			pPlathoPlant[iPlant]->pPlathoPartitioning->vPotFru=2.0 * pPlathoPlant[iPlant]->pPlathoPartitioning->vPotFru;

		pPlathoPlant[iPlant]->pPlathoPartitioning->vDetJohannis = 0.05 * pPlant->pBiomass->fWoodWeight; //sind zu Beginn von Stadium 4 genügend Assimilate verfügbar, kommt es zum Johannistrieb
	}

	//--------------------------------------------------------
//	TREES
//		StemDiameter(pPlant, iPlant);
	
//	HERBS
//	{
		// maximal LAI from minimal light fraction required for netto carbon assimilation	
		//fMaxLAI = (float)(-log(pPGen->vMinLightFraction)/pPGen->vKDiffuse);

//		pPMorph->vCrownStemRatio = (float)sqrt(pPMorph->vLeafToStemRatio * pPlant->pGenotype->fPlantHeight
//			*pPMorph->vStemDensity/(pPGen->vLAImax*pGen->fSpecLfWeight));
	
		StemDiameter(pPlant, iPlant);

		if(pPMorph->vStemDiameter>0.0)
			pPMorph->vHeightToDiameter = pCan->fPlantHeight/pPMorph->vStemDiameter;
		else
			pPMorph->vHeightToDiameter = pPMorph->vHtoDmin;


//		CalcCrownToStemRatio(pPlant, iPlant);

	HERBS
	{
		pPMorph->vCrownStemRatio = sqrt(pPMorph->vLeafToStemRatio * pPlant->pGenotype->fPlantHeight
			*pPMorph->vStemDensity/(pPGen->vLAImax*pPlant->pGenotype->fSpecLfWeight));
	}

	TREES
	{
		pPMorph->vCrownStemRatio = pPMorph->vCrownDiameter/pPMorph->vStemDiameter;

        pPMorph->vCrownRootRatio = pPMorph->vCrownDiameter/pPMorph->vRootZoneDiameter;
	}

    pPMorph->vHeightOfCrownOnset = 0.1*pCan->fPlantHeight;


//	}

	
	// Growth capacities for leaf and (fine) root and fruits (trees)
	GetGrowthCapacities(pPlant, iPlant);


	////////////////////////////////////////////////
	//  RootSystem
	///////////////////////////////////////////////

	// maximale Wurzeltiefe

	pPlant->pRoot->fTotalLength = (float)0.0; //Gesamtwurzellänge
	pPlant->pRoot->fMaxDepth = pPlant->pGenotype->fMaxRootDepth;
	NLayer=pSo->iLayers-2;

    DEPMAX=0.0;
	pSL	  =pSo->pSLayer->pNext;
	for (L=1;L<=NLayer;L++)
	{
		DEPMAX += 0.1*pSL->fThickness;	//cm
		pSL=pSL->pNext;
	}

	
	if(pPlant->pRoot->fMaxDepth>(float)DEPMAX)
		pPlant->pRoot->fMaxDepth = (float)DEPMAX;
	
	//Tiefe der Wurzelfront [m]

	if(pRT->fDepth == (float)0.0)
	{
		HERBS
		{
			if(pBiom->fRootWeight!=(float)0.0)			
				pRT->fDepth = 
				(float)max(0.5*pPlant->pRoot->fMaxDepth * pPlant->pBiomass->fRootWeight 
				/ pPBiom->vKFRoot, pMa->pSowInfo->fSowDepth+ (float)0.5);
			else //Beginn der Vegetationsperiode
				pRT->fDepth	= pMa->pSowInfo->fSowDepth+ (float)0.5;
		}

		TREES
		{
			if(pBiom->fRootWeight+pBiom->fGrossRootWeight!=(float)0.0)
						pRT->fDepth = (float)max(0.5*pPlant->pRoot->fMaxDepth * (pPlant->pBiomass->fRootWeight + 
				pPlant->pBiomass->fGrossRootWeight)/ (pPBiom->vKFRoot+pPBiom->vKGRoot),(float)0.2);

			else
				pRT->fDepth	= pPlant->pRoot->fMaxDepth
					*(float)(1.0-exp(-(pBiom->fRootWeight+pBiom->fGrossRootWeight)));
		}
				
		pRT->fDepth *= (float)0.01; // mittlere Eindringtiefe in m

	}


	alpha = 1.0 + 2.0*pPGen->vRelDepthMaxRoot;
	//gamma = pMa->pSowInfo->fSowDepth * (float)0.01; //in m
	gamma = 0.0; 

	if(pPGen->vRelDepthMaxRoot == 0.0)
		beta = pPlant->pRoot->fDepth/(3.0*log(2));
	else
		beta = pPGen->vRelDepthMaxRoot*pPlant->pRoot->fDepth
						/pow((alpha-1)/alpha,1/alpha);

    Depth = 0.0;
	vCumRootFrac = 0.0;

	pSL	= pSo->pSLayer->pNext;
	pLR	= pPlant->pRoot->pLayerRoot;

	for (L=0;L<=NLayer-1;L++)
	{
		Depth += 0.001*pSL->fThickness;	//Depth in m

		if(Depth<=gamma)
			RootFraction[L] = 0.0;
		else
		{
			if((Depth-0.001*pSL->fThickness<=gamma)&&(Depth>=gamma))
				vCalcDepth = gamma + (Depth-gamma)/2.0;
			else
				vCalcDepth = Depth - 0.001*pSL->fThickness/2.0;


			RootFraction[L] = Weibull(vCalcDepth-gamma, beta, alpha);

			if(vCalcDepth>pRT->fDepth)
				RootFraction[L] = 0.0;

			vCumRootFrac += RootFraction[L];
		}
			
		pSL = pSL->pNext;
		pLR = pLR->pNext;
	}

        if(vCumRootFrac>0)
        {
			vRootDens = 0.2; // [g/cm^3]

            FRDiameter = sqrt(4e-3/(PI*vRootDens*pPlant->pGenotype->fRootLengthRatio));
			TREES
				GRDiameter = sqrt(4e-3/(PI*vRootDens*pPGen->vSpecGrossRootLength));

		    pLR->fLengthDens = (float)0.0;

		    //Wurzellänge in der betrachteten Bodenschicht [m]
		    pSL	= pSo->pSLayer->pNext;
		    pLR	= pPlant->pRoot->pLayerRoot;
		    pPRL	= pPMorph->pPlathoRootLayer;
		    for (L=0;L<=NLayer-1;L++)
		    {
                pPRL->vFRLayBiomass = RootFraction[L]/vCumRootFrac * pBiom->fRootWeight;
			    pPRL->vGRLayBiomass = RootFraction[L]/vCumRootFrac * pBiom->fGrossRootWeight;

                pLR->fLengthDens = (float)(RootFraction[L]/(vCumRootFrac + EPSILON)
				    * pBiom->fRootWeight * pGen->fRootLengthRatio);

			    pPRL->vGRLengthDens = RootFraction[L]/vCumRootFrac
				    * pBiom->fGrossRootWeight * pPGen->vSpecGrossRootLength;

			    pPlant->pRoot->fTotalLength += pLR->fLengthDens * (float)0.1*pSL->fThickness;

			    //Oberfläche der Wurzeln:
			    FRArea = FRDiameter * PI * pLR->fLengthDens;

			    TREES
				    GRArea = GRDiameter * PI * pPRL->vGRLengthDens;

			    pPRL->vRootArea = FRArea+GRArea;

			    pSL = pSL->pNext;
			    pLR = pLR->pNext;
			    pPRL =pPRL->pNext;
		    }

            if(pPlathoModules->iFlagSectors==2)
            {
                double BlwgDistrFac[4];

               // Initialisierung
               for(iSector=0;iSector<4;iSector++)  // right = 0, lower = 1, left = 2, upper = 3 
                    BlwgDistrFac[iSector] = pPMorph->pPlathoRootLayer->apRootSector[iSector]->vBlwgDistrFac;

		        pSL	    = pSo->pSLayer->pNext;
		        pLR 	= pPlant->pRoot->pLayerRoot;
		        pPRL	= pPMorph->pPlathoRootLayer;

		        for (L=0;L<=NLayer-1;L++)
		        {

                    for(iSector=0;iSector<4;iSector++)
                    {
			            pPRL->apRootSector[iSector]->vFRSecBiomass = pPRL->vFRLayBiomass*BlwgDistrFac[iSector];
			            pPRL->apRootSector[iSector]->vGRSecBiomass = pPRL->vGRLayBiomass*BlwgDistrFac[iSector];

			            pPRL->apRootSector[iSector]->vFRLengthDens = pLR->fLengthDens*BlwgDistrFac[iSector];
			            pPRL->apRootSector[iSector]->vGRLengthDens = pPRL->vGRLengthDens*BlwgDistrFac[iSector];
			            pPRL->apRootSector[iSector]->vRootArea = pPRL->vRootArea*BlwgDistrFac[iSector];
                    }// end Sectors

			        pSL = pSL->pNext;
			        pLR = pLR->pNext;
			        pPRL =pPRL->pNext;
                } // end soil layers
            } // end if iFlagSectors
        }//end if vCumRootFrac > 0

	//
	
	if(pPlant->pCanopy->fPlantHeight==(float)0.0)
	{
		pPlant->pCanopy->fPlantHeight	= (float) 0.0; // m
		pPMorph->vStemDiameter			= 0.0;
		pPMorph->vStemDiameterOld		= 0.0;
	//	pPlant->pCanopy->fPlantHeight	   = (float) 0.001; // m
	//	pPMorph->vStemDiameter = pPlant->pCanopy->fPlantHeight/pPMorph->vHtoDmin;
	}
	else
	{
		pPMorph->vStemDiameter = max(pPMorph->vStemDiameter,sqrt(4.0*pPlant->pBiomass->fStemWeight
			/(PI*pPlant->pCanopy->fPlantHeight*pPMorph->vStemDensity)));
		
		pPMorph->vStemDiameterOld	= pPMorph->vStemDiameter;
	}

	//Plant Area [m^2]
//	HERBS
//	{
		PlantArea(pPlant, iPlant);
//	}



		/////////////////////////////////////////////////////////////////////
        //  *****	LEAF LAYERS:	*****
		//
		// Blattflächendichte
		// spezifisches Blattgewicht
		// kumulativer LAI über h
		// kumulative Blattfläche über h
		// kumulatives Blattgewicht über h
		//
		////////////////////////////////////////////////////////////////////    
    	
    //Berechnung des Blattflächenindex [m^2(Blatt)/m^2(Boden)]

	if(pPMorph->vCrownDiameter==0.0)
    {
		pPlant->pCanopy->fPlantLA = (float)0.0;
        pPMorph->vPlantLAI = 0.0;
    }

	if (pBiom->fLeafWeight > (float)0.0)
	{
		// maximal LAI from minimal light fraction required for netto carbon assimilation	
		//fMaxLAI = (float)(-log(pPGen->vMinLightFraction)/pPGen->vKDiffuse);
		
		if(pPlathoModules->iFlagSpecLeafArea==1)//constant specific leaf weight 
        {
            pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight/pPlant->pGenotype->fSpecLfWeight;
            pPMorph->vPlantLAI = (pPlant->pCanopy->fPlantLA/max(pPMorph->vCrownArea,1.0/vPlantDensity));
        }
        else if(pPlathoModules->iFlagSpecLeafArea==2)//dynamic specific leaf weight 
		{
			//start value for specific leaf weight:
			vMinimalMeanSpecLfWeight = 0.5*pPlant->pGenotype->fSpecLfWeight
				* (pPltN->fLeafOptConc + pPltN->fLeafMinConc)/pPltN->fLeafOptConc;

			if(pPlant->pGenotype->fBeginShadeLAI>(float)0.0)
			{
                //start value for LAI:
		        //pPMorph->vPlantLAI = pPGen->vLAImax * pPlant->pBiomass->fLeafWeight/pPBiom->vKLeaf;
		        pPMorph->vPlantLAI = (float)(pPGen->vLAImax * pBiom->fLeafWeight/vMinimalMeanSpecLfWeight/max(pPMorph->vCrownArea,1.0/vPlantDensity));

                alphaN = (2.0*pPltN->fLeafOptConc - pPltN->fLeafActConc - pPltN->fLeafMinConc)
				        /(4.0 * pPltN->fLeafOptConc * pPlant->pGenotype->fBeginShadeLAI);
    		
				pPMorph->vMeanSpecLfWeight = 
					max(pPlant->pGenotype->fSpecLfWeight*(1-alphaN*pPMorph->vPlantLAI),
						vMinimalMeanSpecLfWeight);
			}
			else
				pPMorph->vMeanSpecLfWeight = vMinimalMeanSpecLfWeight;

			pPlant->pCanopy->fPlantLA = pBiom->fLeafWeight 
													/ (float)pPMorph->vMeanSpecLfWeight;

            pPMorph->vPlantLAI = pPlant->pCanopy->fPlantLA/max(pPMorph->vCrownArea,1.0/vPlantDensity);
            			    
            // Bestimmung von kappa:
			kappa = CalcKappa((double)pPlant->pGenotype->fSpecLfWeight, pPMorph->vPlantLAI, iPlant);
        }
        else //default
        {
            pPlant->pCanopy->fPlantLA = pBiom->fLeafWeight/pPlant->pGenotype->fSpecLfWeight;
            pPMorph->vPlantLAI = pPlant->pCanopy->fPlantLA/max(pPMorph->vCrownArea,1.0/vPlantDensity);
        }


        //Verteilung der Blattflaeche über die Höhe
        TLLDF = 0.0;

        for(N=LEAFLAYERS-1;N>=0;N--)
		{
			//	aktuelle Höhe der N-ten Blattschicht:
			h = ((double)N+0.5)/LEAFLAYERS*pCan->fPlantHeight;

            vLeafAreaFac[N] = LeafAreaDens(pCan->fPlantHeight,h,pPGen->vRelHeightMaxLeaf);
            TLLDF += vLeafAreaFac[N];
        }

	    pLLayer = pPMorph->pLeafLayer;       
        for(N = 1;N<=LEAFLAYERS-1;N++)
        {
            pLLayer = pLLayer->pNext;
        }

   		pLLayer->vLeafWeightCum      = 0.0;
            
        for(N=LEAFLAYERS-1;N>=0;N--)
		{
    	    if(pPlathoModules->iFlagSpecLeafArea==1)//constant specific leaf weight 
		    {
 			    pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight;
            }
		    else if(pPlathoModules->iFlagSpecLeafArea==2)//dynamic specific leaf weight 
		    {
			    // SpecLfW(LAIc) = SpecLfWMax * exp(-kappa*LAIc)
                pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight*exp(-kappa*pLLayer->vLeafAreaCum/max(pPMorph->vCrownArea,1.0/vPlantDensity));
            }
            else //default
		    {
 			    pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight;
            }

			pLLayer->vLeafAreaLay    = pCan->fPlantLA*vLeafAreaFac[N]/TLLDF;
			pLLayer->vLeafWeightLay = pLLayer->vLeafAreaLay * pLLayer->vSpecLeafWeight;

            if(pLLayer->pNext!=NULL)
            {
                pLLayer->vLeafAreaCum   = pLLayer->pNext->vLeafAreaCum + pLLayer->vLeafAreaLay;
                pLLayer->vLeafWeightCum = pLLayer->pNext->vLeafWeightCum + pLLayer->vLeafWeightLay;
            }
            else
            {
                pLLayer->vLeafAreaCum   = pLLayer->vLeafAreaLay;
                pLLayer->vLeafWeightCum = pLLayer->vLeafWeightLay;
            }

			pLLayer->vLAICum             = pLLayer->vLeafAreaCum/max(pPMorph->vCrownArea,1.0/vPlantDensity);

 		    if(pPlathoModules->iFlagSectors==2)
		    {
                double AbvgDistrFac[4];

               // Initialisierung
               for(iSector=0;iSector<4;iSector++)  // right = 0, lower = 1, left = 2, upper = 3 
                    AbvgDistrFac[iSector] = pPMorph->pLeafLayer->apLeafSector[iSector]->vAbvgDistrFac;

                for(iSector=0;iSector<4;iSector++)
                {
                    pLLayer->apLeafSector[iSector]->vLeafAreaCumSec = pLLayer->vLeafAreaCum * AbvgDistrFac[iSector];
			        pLLayer->apLeafSector[iSector]->vLAICumSec = pLLayer->vLAICum * AbvgDistrFac[iSector];
			        pLLayer->apLeafSector[iSector]->vLeafAreaSec = pLLayer->vLeafAreaLay * AbvgDistrFac[iSector];

			        pLLayer->apLeafSector[iSector]->vSLWeightSec = pLLayer->vSpecLeafWeight;
			        pLLayer->apLeafSector[iSector]->vLeafWeightSec = pLLayer->vLeafWeightLay * AbvgDistrFac[iSector];

			        pLLayer->apLeafSector[iSector]->vLeafWeightCumSec = pLLayer->vLeafWeightCum * AbvgDistrFac[iSector];
                }
            } //end iFlagSectors == 2
            
            pLLayer = pLLayer->pBack;
		} //end LEAFLAYERS


    
        if(pPlathoModules->iFlagNitrogenDistribution==1)//N homogen verteilt auf Blattschichten und -sektoren:
	    {
             PLEAFLAYER pLLayer = pPMorph->pLeafLayer;       
             for(N = 1;N<=LEAFLAYERS;N++)
             {
			    pLLayer->vLeafNConc = pPltN->fLeafActConc;

                if(pPlathoModules->iFlagSectors==2)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
			            pLLayer->apLeafSector[iSector]->vLeafNConc = pPltN->fLeafActConc;
                    }
                }

                pLLayer = pLLayer->pNext;
             }
	    }
	    else if(pPlathoModules->iFlagNitrogenDistribution==2)//N-Verteilung auf Blattschichten:
	    {
            double vLfWeightCrit1, vLfWeightCrit2;
            PLEAFLAYER pLLayer;       

            vLfWeightCrit1 = pBiom->fLeafWeight * (1.0 -
			    2.0*(pPltN->fLeafActConc-pPltN->fLeafMinConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc));

		    vLfWeightCrit2 = 2.0*pBiom->fLeafWeight
			    *(pPltN->fLeafOptConc-pPltN->fLeafActConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc);

             pLLayer = pPMorph->pLeafLayer;       
             for(N = 1;N<=LEAFLAYERS;N++)
             {
			    LeafNitrogenDistribution(vLfWeightCrit1, vLfWeightCrit2, pPlant, pLLayer);
                 
                pLLayer = pLLayer->pNext;
             }
	    }
	    else //default
	    {
             PLEAFLAYER pLLayer = pPMorph->pLeafLayer;       
             for(N = 1;N<=LEAFLAYERS;N++)
             {
			    pLLayer->vLeafNConc = pPltN->fLeafActConc;

                if(pPlathoModules->iFlagSectors==2)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
			            pLLayer->apLeafSector[iSector]->vLeafNConc = pPltN->fLeafActConc;
                    }
                }

                 pLLayer = pLLayer->pNext;
             }
	    }
	}// end if fLeafWeight > 0
	else  //  fLeafWeight = 0
	{
		pPlant->pCanopy->fPlantLA = (float)0.0;
		pPMorph->vPlantLAI = 0.0;

        pLLayer = pPMorph->pLeafLayer;       
        for(N = 1;N<=LEAFLAYERS;N++)
        {
			pLLayer->vLeafWeightLay  = 0.0;
			pLLayer->vLeafAreaLay    = 0.0;
 			pLLayer->vSpecLeafWeight = pPlant->pGenotype->fSpecLfWeight;

            pLLayer->vLeafAreaCum        = 0.0;
			pLLayer->vLAICum             = 0.0;
			pLLayer->vLeafWeightCum      = 0.0;

    		pLLayer->vLeafNConc          = pPltN->fLeafOptConc;

            for(iSector=0;iSector<4;iSector++)
            {
			    pLLayer->apLeafSector[iSector]->vLeafWeightSec = 0.0;
			    pLLayer->apLeafSector[iSector]->vLeafAreaSec = 0.0;
			    pLLayer->apLeafSector[iSector]->vSLWeightSec = pPlant->pGenotype->fSpecLfWeight;

                pLLayer->apLeafSector[iSector]->vLeafAreaCumSec = 0.0;
			    pLLayer->apLeafSector[iSector]->vLAICumSec = 0.0;
			    pLLayer->apLeafSector[iSector]->vLeafWeightCumSec = 0.0;

    			pLLayer->apLeafSector[iSector]->vLeafNConc = pPltN->fLeafActConc;
            }
        } //end LeafLayers

        pLLayer = pLLayer->pNext;
     } //end if fLeafWeight == 0
 
	HERBS
	{
		if(pPDev->iStagePlatho<=1)
			pPMorph->vPlantLAI = 0.0;
		if(pPDev->iStagePlatho>=5)
			pPMorph->vPlantLAI = 0.0;
	}

    	
	//////////////////////////////////////////////////////////////////////////////////
    //   Neighbours and Competition coefficients
	//////////////////////////////////////////////////////////////////////////////////
    GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
												&iPlantUpper, &iPlantLower);

	pPlantLeft	= 	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft;
	pPlantRight	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight;
	pPlantUpper	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper;
	pPlantLower	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower ;




	Competition(pPlant, pPlantUpper, pPlantLeft, pPlantRight, pPlantLower, 
				iPlant, iPlantUpper, iPlantLeft, iPlantRight, iPlantLower, pSo);
	//////////////////////////////////////////////////////////////////////////////////


	

	//pPlant->pPltWater->fStressFacPhoto = (float)1.0;
	pPStress->dWaterShortage = (float)1.0;

	pPlant->pPltNitrogen->fNStressPhoto = (float)1.0; //entspricht cnsdf
	pPlant->pPltNitrogen->fNStressLeaf = (float)1.0;  //entspricht cnsdf2

	pPlant->pBiomass->fTotalBiomass = pBiom->fSeedReserv
										+pBiom->fRootWeight
										+pBiom->fGrossRootWeight
										+pBiom->fLeafWeight
										+pBiom->fStemWeight
										+pBiom->fBranchWeight
										+pBiom->fFruitWeight
										+pBiom->fTuberWeight;

    pPBiom->vFracRespWood = 1.0/(1.0+pPlant->pDevelop->fAge);
    pPBiom->vLivingBiomass = pBiom->fWoodWeight*pPBiom->vFracRespWood 
                                + pBiom->fRootWeight + pBiom->fLeafWeight;


	
	// Einlesen der biochemischen Daten und Berechnung 
	// der Wachstums- und Erhaltungsparameter
	GetBiochemicalData(iPlant);
	GetGrowthEfficiencyAndRespirationCoefficients(pPlant, iPlant);
	GetMaintenanceRespirationCoefficients(pPlant, iPlant);

    if(pPlathoModules->iFlagPhotosynthesis == 2) // "Farquhar"
    {
        // Einlesen der Falge-Parameter:
        ReadFalgeParameters(iPlant, pPlant);
    }

	//Stress
	pPStress->vCritLfPath = 0.0;
	pPStress->vCritRtPath = 0.0;

//	pPStress->vCritLfPath = min(1.0,pPStress->vConstDefenseBase*1.0);
//	pPStress->vCritRtPath = min(1.0,pPStress->vConstDefenseBase*1.0);

//	pPStress->vCritO3Conc		= (float)(1.0+10.0*pPStress->vO3ConstDefense);//[µg/kg]
	pPStress->vO3LeafConcCum		= 0.0;
	pPStress->vO3LeafConcCumCrit	= 0.0;

	//Startwerte für Konzentration der Abwehrstoffe (konstitutive Abwehr)
    if(pPStress->vConstDefense == 0.0)
    {
        pPStress->vFineRootDefCont   = 0.0;
        pPStress->vGrossRootDefCont  = 0.0;
        pPStress->vStemDefCont       = 0.0;
        pPStress->vBranchDefCont     = 0.0;
        pPStress->vLeafDefCont       = 0.0;
        pPStress->vFruitDefCont      = 0.0;
        pPStress->vTuberDefCont      = 0.0;

        pPStress->vFineRootDefConc   = 0.0;
        pPStress->vGrossRootDefConc  = 0.0;
        pPStress->vStemDefConc       = 0.0;
        pPStress->vBranchDefConc     = 0.0;
        pPStress->vLeafDefConc       = 0.0;
        pPStress->vFruitDefConc      = 0.0;
        pPStress->vTuberDefConc      = 0.0;
    }
    else
    {
        pPStress->vFineRootDefCont   = pBiom->fRootWeight * pPStress->vFineRootDefConc;
        pPStress->vGrossRootDefCont  = pBiom->fGrossRootWeight * pPStress->vStemDefConc;
        pPStress->vStemDefCont       = pBiom->fStemWeight * pPStress->vStemDefConc;
        pPStress->vBranchDefCont     = pBiom->fBranchWeight * pPStress->vStemDefConc;
        pPStress->vLeafDefCont       = pBiom->fLeafWeight * pPStress->vLeafDefConc;
        pPStress->vFruitDefCont      = pBiom->fFruitWeight * pPStress->vFruitDefConc;
        pPStress->vTuberDefCont      = pBiom->fTuberWeight * pPStress->vTuberDefConc;
    }

/*	if((pPlant->pBiomass->fLeafWeight == (float)0.0)||(pPPltStress->vConstDefense == (float)0.0))
		pPPltStress->vLeafDefConc = (float)0.0;
	else
	{
		if(pPPltStress->vDefCompConc==(float)0.0)
		{
			double d0  = pPPltStress->vConstDefenseBase;
			double xiS = pBiochemistry->pGrowthEfficiency->fDefComp;
			double xiL = pPlant->pGenotype->fGrwEffLeaf;

			pPPltStress->vLeafDefConc = (float)(d0/(d0+(1-d0)*xiS/xiL));
		}
		else
			pPPltStress->vLeafDefConc = pPPltStress->vLeafDefConc;
	}


	if((pPlant->pBiomass->fLeafWeight == (float)0.0)||(pPPltStress->vO3ConstDefense == (float)0.0))
		pPPltStress->vO3DefCompConc = (float)0.0;
	else
	{
		if(pPPltStress->vO3DefCompConc==(float)0.0)
		{
			double d0  = pPPltStress->vO3ConstDefenseBase;
			double xiS = pBiochemistry->pGrowthEfficiency->fO3DefComp;
			double xiL = pPlant->pGenotype->fGrwEffLeaf;

			pPPltStress->vO3DefCompConc = (float)(d0/(d0+(1-d0)*xiS/xiL));
		}
		else
			pPPltStress->vO3DefCompConc = pPPltStress->vO3DefCompConc;
	}


	if((pPlant->pBiomass->fRootWeight == (float)0.0)||(pPPltStress->vRtPathConstDefense == (float)0.0))
		pPPltStress->vRtPathDefCompConc = (float)0.0;
	else
	{
		if(pPPltStress->vRtPathDefCompConc==(float)0.0)
		{
			double d0  = pPPltStress->vRtPathConstDefenseBase;
			double xiS = pBiochemistry->pGrowthEfficiency->fRtPathDefComp;
			double xiR = pPlant->pGenotype->fGrwEffRoot;

			pPPltStress->vRtPathDefCompConc = (float)(d0/(d0+(1-d0)*xiS/xiR));
		}
		else
			pPPltStress->vRtPathDefCompConc = pPPltStress->vRtPathDefCompConc;
	}



	pPPltStress->vLeafDefCont = pPPltStress->vLfPathDefCompConc * pPlant->pBiomass->fLeafWeight
										/((float)1.0 - pPPltStress->vLfPathDefCompConc);

	pPPltStress->vWoodDefCont = pPPltStress->vO3DefCompConc * pPlant->pBiomass->fLeafWeight
										/((float)1.0 - pPPltStress->vO3DefCompConc);

	pPPltStress->vFineRootDefCont = pPPltStress->vRtPathDefCompConc * pPlant->pBiomass->fRootWeight
										/((float)1.0 - pPPltStress->vRtPathDefCompConc);

*/
    
    pPlathoPlant[iPlant]->pPlathoPartitioning->vCLitterCum =  0.0;
    pPlathoPlant[iPlant]->pPlathoPartitioning->vNLitterCum =  0.0;

    pPStress->vO3UptakeDay = 0.0;
    pPStress->vO3UptakeCum = 0.0;
    pPStress->dCostForDetoxCum = 0.0;
    pPStress->dCostsRepairCum = 0.0;

	return 1;

}


int	GetBiochemicalData(int iPlant)
{

	PGROWTHEFFICIENCY		pGrwEff	= pBiochemistry->pGrowthEfficiency;
	PCO2REQUIRED			pCO2Req = pBiochemistry->pCO2Required;
	PCARBON					pPCarb = pBiochemistry->pCarbon;

	//	a) Growth efficiency [g(Glucose)/g]

	pGrwEff->vCarbohydrates =	1.275;
	pGrwEff->vProteins =		1.887;
	pGrwEff->vLipids =			3.189;
	pGrwEff->vLignins =			2.231;
	pGrwEff->vOrganicAcids =	0.954;
	pGrwEff->vMinerals =		0.12;

	// including nitrate reduction
	pGrwEff->vProteinsNitRed =	2.984;

	//	b) CO2 released during Biosynthesis [g(CO2)/g]

	pCO2Req->vCarbohydrates =	0.216;
	pCO2Req->vProteins =		0.817;
	pCO2Req->vLipids =			1.840;
	pCO2Req->vLignins =			0.740;
	pCO2Req->vOrganicAcids =	0.025;
	pCO2Req->vMinerals =		0.176;

	// including nitrate reduction
	pCO2Req->vProteinsNitRed =	2.133;

	
	//	c) Carbon fraction [g(C)/g]

	pPCarb->vCarbohydrates =	0.451;
	pPCarb->vProteins =			0.532;
	pPCarb->vLipids =			0.774;
	pPCarb->vLignins =			0.690;
	pPCarb->vOrganicAcids =		0.375;
	pPCarb->vMinerals =			0.0;
	pPCarb->vO3DefComp =		0.409;
	pPCarb->vLfPathDefComp =	0.62;
	pPCarb->vRtPathDefComp =	0.5;


	return 1;
}


int	GetGrowthEfficiencyAndRespirationCoefficients(PPLANT pPlant, int iPlant)
{
	PPLANTMATERIAL	pMaterial = pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;
	PGROWTHEFFICIENCY		pGrwEff	= pBiochemistry->pGrowthEfficiency;
	PCO2REQUIRED			pCO2Req = pBiochemistry->pCO2Required;
	PCARBON					pPCarb   = pBiochemistry->pCarbon;

	
	//	a) Growth Efficiency
		pPlant->pGenotype->fGrwEffRoot	 	
			= (float)(pMaterial->pFineRoot->vCarbohydrates/100.0 * pGrwEff->vCarbohydrates
			+ pMaterial->pFineRoot->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pFineRoot->vLipids/100.0 * pGrwEff->vLipids
			+ pMaterial->pFineRoot->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pFineRoot->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pFineRoot->vMinerals/100.0 * pGrwEff->vMinerals);

		pPlant->pGenotype->fGrwEffStem	 	
			= (float)(pMaterial->pStem->vCarbohydrates/100.0 * pGrwEff->vCarbohydrates
			+ pMaterial->pStem->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pStem->vLipids/100.0 * pGrwEff->vLipids
			+ pMaterial->pStem->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pStem->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pStem->vMinerals/100.0 * pGrwEff->vMinerals);

		pPlant->pGenotype->fGrwEffLeaf	 	
			= (float)(pMaterial->pLeaf->vCarbohydrates/100.00 * pGrwEff->vCarbohydrates
			+ pMaterial->pLeaf->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pLeaf->vLipids/100.00 * pGrwEff->vLipids
			+ pMaterial->pLeaf->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pLeaf->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pLeaf->vMinerals/100.0 * pGrwEff->vMinerals);

		pPlant->pGenotype->fGrwEffStorage	 	
			= (float)(pMaterial->pFruit->vCarbohydrates/100.0 * pGrwEff->vCarbohydrates
			+ pMaterial->pFruit->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pFruit->vLipids/100.0 * pGrwEff->vLipids
			+ pMaterial->pFruit->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pFruit->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pFruit->vMinerals/100.0 * pGrwEff->vMinerals);


		POTATO
		{
		pPlant->pGenotype->fGrwEffTuber	 	
			= (float)(pMaterial->pTuber->vCarbohydrates/100.0 * pGrwEff->vCarbohydrates
			+ pMaterial->pTuber->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pTuber->vLipids/100.0 * pGrwEff->vLipids
			+ pMaterial->pTuber->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pTuber->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pTuber->vMinerals/100.0 * pGrwEff->vMinerals);
		}
		
		TREES
		{
		pPlant->pGenotype->fGrwEffGrossRoot	 	
			= (float)(pMaterial->pGrossRoot->vCarbohydrates/100.0 * pGrwEff->vCarbohydrates
			+ pMaterial->pGrossRoot->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pGrossRoot->vLipids/100.0 * pGrwEff->vLipids
			+ pMaterial->pGrossRoot->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pGrossRoot->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pGrossRoot->vMinerals/100.0 * pGrwEff->vMinerals);
		
		pPlant->pGenotype->fGrwEffBranch	 	
			= (float)(pMaterial->pBranch->vCarbohydrates/100.0 * pGrwEff->vCarbohydrates
			+ pMaterial->pBranch->vProteins/100.0 * pGrwEff->vProteins
			+ pMaterial->pBranch->vLipids/100.0 * pGrwEff->vLipids
			+ pMaterial->pBranch->vLignins/100.0 * pGrwEff->vLignins
			+ pMaterial->pBranch->vOrganicAcids/100.0 * pGrwEff->vOrganicAcids
			+ pMaterial->pBranch->vMinerals/100.0 * pGrwEff->vMinerals);
		}


	//	b) Growth Respiration
		pPlant->pGenotype->fCO2EffRoot	 	
			= (float)(pMaterial->pFineRoot->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pFineRoot->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pFineRoot->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pFineRoot->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pFineRoot->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pFineRoot->vMinerals/100.0 * pCO2Req->vMinerals);

		pPlant->pGenotype->fCO2EffStem	 	
			= (float)(pMaterial->pStem->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pStem->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pStem->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pStem->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pStem->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pStem->vMinerals/100.0 * pCO2Req->vMinerals);

		pPlant->pGenotype->fCO2EffLeaf	 	
			= (float)(pMaterial->pLeaf->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pLeaf->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pLeaf->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pLeaf->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pLeaf->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pLeaf->vMinerals/100.0 * pCO2Req->vMinerals);

		pPlant->pGenotype->fCO2EffStorage	 	
			= (float)(pMaterial->pFruit->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pFruit->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pFruit->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pFruit->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pFruit->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pFruit->vMinerals/100.0 * pCO2Req->vMinerals);


		POTATO
		{
		pPlant->pGenotype->fCO2EffTuber	 	
			= (float)(pMaterial->pTuber->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pTuber->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pTuber->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pTuber->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pTuber->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pTuber->vMinerals/100.0 * pCO2Req->vMinerals);
		}
		
		TREES
		{
		pPlant->pGenotype->fCO2EffGrossRoot	 	
			= (float)(pMaterial->pGrossRoot->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pGrossRoot->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pGrossRoot->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pGrossRoot->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pGrossRoot->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pGrossRoot->vMinerals/100.0 * pCO2Req->vMinerals);
		
		pPlant->pGenotype->fCO2EffBranch	 	
			= (float)(pMaterial->pBranch->vCarbohydrates/100.0 * pCO2Req->vCarbohydrates
			+ pMaterial->pBranch->vProteins/100.0 * pCO2Req->vProteins
			+ pMaterial->pBranch->vLipids/100.0 * pCO2Req->vLipids
			+ pMaterial->pBranch->vLignins/100.0 * pCO2Req->vLignins
			+ pMaterial->pBranch->vOrganicAcids/100.0 * pCO2Req->vOrganicAcids
			+ pMaterial->pBranch->vMinerals/100.0 * pCO2Req->vMinerals);
		}

	//	c) C-Fraction
		pMaterial->pFineRoot->vFracC	 	
			= pMaterial->pFineRoot->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pFineRoot->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pFineRoot->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pFineRoot->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pFineRoot->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pFineRoot->vMinerals/100.0 * pPCarb->vMinerals;

		pMaterial->pStem->vFracC	 	
			= pMaterial->pStem->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pStem->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pStem->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pStem->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pStem->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pStem->vMinerals/100.0 * pPCarb->vMinerals;

		pMaterial->pLeaf->vFracC	 	
			= pMaterial->pLeaf->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pLeaf->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pLeaf->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pLeaf->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pLeaf->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pLeaf->vMinerals/100.0 * pPCarb->vMinerals;

		pMaterial->pFruit->vFracC	 	
			= pMaterial->pFruit->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pFruit->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pFruit->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pFruit->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pFruit->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pFruit->vMinerals/100.0 * pPCarb->vMinerals;


		POTATO
		{
		pMaterial->pTuber->vFracC	 	
			= pMaterial->pTuber->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pTuber->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pTuber->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pTuber->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pTuber->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pTuber->vMinerals/100.0 * pPCarb->vMinerals;
		}
		
		TREES
		{
		pMaterial->pGrossRoot->vFracC	 	
			= pMaterial->pGrossRoot->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pGrossRoot->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pGrossRoot->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pGrossRoot->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pGrossRoot->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pGrossRoot->vMinerals/100.0 * pPCarb->vMinerals;
		
		pMaterial->pBranch->vFracC	 	
			= pMaterial->pBranch->vCarbohydrates/100.0 * pPCarb->vCarbohydrates
			+ pMaterial->pBranch->vProteins/100.0 * pPCarb->vProteins
			+ pMaterial->pBranch->vLipids/100.0 * pPCarb->vLipids
			+ pMaterial->pBranch->vLignins/100.0 * pPCarb->vLignins
			+ pMaterial->pBranch->vOrganicAcids/100.0 * pPCarb->vOrganicAcids
			+ pMaterial->pBranch->vMinerals/100.0 * pPCarb->vMinerals;
		}



	return 1;
}

int	GetMaintenanceRespirationCoefficients(PPLANT pPlant, int iPlant)
{
	// Berechnung der Erhaltungsatmungskoeffizienten [kg(CO2)/kg(Trockenmasse)/d]

	PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;
	PPLATHODEVELOP	pPDev = pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLANTMATERIAL	pMaterial = pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;


	double dFineRootMaintProteins, dGrossRootMaintProteins, dLeafMaintProteins, 
		   dStemMaintProteins,dBranchMaintProteins,dFruitMaintProteins, dTuberMaintProteins;
	
	double dFineRootMaintLipids, dGrossRootMaintLipids, dLeafMaintLipids, 
		   dStemMaintLipids,dBranchMaintLipids,dFruitMaintLipids, dTuberMaintLipids;
	
	double dFineRootMaintMinerals, dGrossRootMaintMinerals, dLeafMaintMinerals, 
		   dStemMaintMinerals,dBranchMaintMinerals,dFruitMaintMinerals, dTuberMaintMinerals;
	
	float fGrossRootRelNc,fBranchRelNc;
	//float fCarbonFraction = (float)0.45;

    if(pPlathoModules->iFlagNEffectGrw==1)// no N effect on growth
    {
 		pPltN->fLeafRelNc = (float)1.0;
		pPltN->fStemRelNc = (float)1.0;
		pPltN->fRootRelNc = (float)1.0;
		fBranchRelNc	  = (float)1.0;
		fGrossRootRelNc   = (float)1.0;
    }
    else   // N effect on growth included
    {
	    if((pPDev->iStagePlatho==1)||(pPDev->iStagePlatho==2))
	    {
		    pPltN->fLeafRelNc = (float)1.0;
		    pPltN->fStemRelNc = (float)1.0;
		    pPltN->fRootRelNc = (float)1.0;
		    fBranchRelNc	  = (float)1.0;
		    fGrossRootRelNc   = (float)1.0;
	    }
	    else
	    {
		    pPltN->fLeafRelNc = (float)max(0.0,(pPltN->fLeafActConc-pPltN->fLeafMinConc)/(pPltN->fLeafOptConc-pPltN->fLeafMinConc+EPSILON));
		    pPltN->fLeafRelNc = (float)min(1.0,pPltN->fLeafRelNc);

		    pPltN->fStemRelNc = (float)max(0.0,(pPltN->fStemActConc-pPltN->fStemMinConc)/(pPltN->fStemOptConc-pPltN->fStemMinConc+EPSILON));
		    pPltN->fStemRelNc = (float)min(1.0,pPltN->fLeafRelNc);
    		
		    pPltN->fRootRelNc = (float)max(0.0,(pPltN->fRootActConc-pPltN->fRootMinConc)/(pPltN->fRootOptConc-pPltN->fRootMinConc+EPSILON));
		    pPltN->fRootRelNc = (float)min(1.0,pPltN->fLeafRelNc);
    		
		    fBranchRelNc	  = (float)max(0.0,(pPltN->fBranchActConc-pPltN->fBranchMinConc)/(pPltN->fBranchOptConc-pPltN->fBranchMinConc+EPSILON));
		    fBranchRelNc = (float)min(1.0,pPltN->fLeafRelNc);
    		
		    fGrossRootRelNc   = (float)max(0.0,(pPltN->fGrossRootActConc-pPltN->fGrossRootMinConc)/(pPltN->fGrossRootOptConc-pPltN->fGrossRootMinConc+EPSILON));
		    fGrossRootRelNc = (float)min(1.0,pPltN->fLeafRelNc);
	    }
    } // end 'N effect on growth included'

	///////////////////////////////////////////////////////
	////////////////////Proteins////Lipids//////Minerals //
	//	cATP		//	4			7			1		 //
	//	RMM (g/mol)	//	120			900			40		 //
	///////////////////////////////////////////////////////


/*	Beispiel:

                     f_pt[g(pt)/g(TS)]*k_pt[1/d]*cATP[mol(ATP)/mol(pt)]     72[g(C)/mol(Glucose)]       44[g(CO2)]
	m_pt[g(CO2)/g] = -------------------------------------------------- * ------------------------- * ----------
                                   RMM[g(pt)/mol(pt)]                     36[mol(ATP)/mol(Glucose)]    12[g(C)]

*/

	dFineRootMaintProteins = 2.0*pMaterial->pFineRoot->vProteins/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR
	    * 4.0/120.0 * (44.0/12.0);

	dFineRootMaintLipids = 2.0*pMaterial->pFineRoot->vLipids/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
		* 7.0/900.0 * (44.0/12.0);

	dFineRootMaintMinerals = 2.0*pMaterial->pFineRoot->vMinerals/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR
		* 1.0/40.0 * (44.0/12.0);

	pPlant->pGenotype->fMaintRoot = (float)(dFineRootMaintProteins*pow(pPltN->fRootRelNc,0.5)
									+ dFineRootMaintLipids
									+ dFineRootMaintMinerals);


	dStemMaintProteins = 2.0*pMaterial->pStem->vProteins/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR
		* 4.0/120.0 * 44.0/12.0;

	TREES
		dStemMaintProteins *= 0.1;

	dStemMaintLipids = 2.0*pMaterial->pStem->vLipids/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
		* 7.0/900.0 * (44.0/12.0);

	dStemMaintMinerals = 2.0*pMaterial->pStem->vMinerals/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR 
		* 1.0/40.0 * (44.0/12.0);

	pPlant->pGenotype->fMaintStem = (float)(dStemMaintProteins*pow(pPltN->fStemRelNc,0.5)
									+ dStemMaintLipids
									+ dStemMaintMinerals);



	dLeafMaintProteins = 2.0*pMaterial->pLeaf->vProteins/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR * 4.0
		/120.0 * (44.0/12.0);

	dLeafMaintLipids = 2.0*pMaterial->pLeaf->vLipids/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
		* 7.0/900.0 * (44.0/12.0);

	dLeafMaintMinerals = 2.0*pMaterial->pLeaf->vMinerals/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR 
		* 1.0/40.0 * (44.0/12.0);

	pPlant->pGenotype->fMaintLeaf = (float)(dLeafMaintProteins*pow(pPltN->fLeafRelNc,0.5)
									+ dLeafMaintLipids
									+ dLeafMaintMinerals);


	dFruitMaintProteins = 2.0*pMaterial->pFruit->vProteins/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR 
		* 4.0/120.0 * (44.0/12.0);

	dFruitMaintLipids = 2.0*pMaterial->pFruit->vLipids/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
		* 7.0/900.0 * (44.0/12.0);

	dFruitMaintMinerals = 2.0*pMaterial->pFruit->vMinerals/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR 
		* 1.0/40.0 * (44.0/12.0);

	pPlant->pGenotype->fMaintStorage = (float)(dFruitMaintProteins
									+ dFruitMaintLipids
									+ dFruitMaintMinerals);



	TREES
	{
		dGrossRootMaintProteins = 2.0*pMaterial->pGrossRoot->vProteins/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR 
			* 4.0/120.0 * (44.0/12.0);

		dGrossRootMaintLipids = 2.0*pMaterial->pGrossRoot->vLipids/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
			* 7.0/900.0 * (44.0/12.0);

		dGrossRootMaintMinerals = 2.0*pMaterial->pGrossRoot->vMinerals/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR 
			* 1.0/40.0 * (44.0/12.0);

		pPlant->pGenotype->fMaintGrossRoot = (float)(dGrossRootMaintProteins*0.1*pow(fGrossRootRelNc,0.5)
									+ dGrossRootMaintLipids
									+ dGrossRootMaintMinerals);


		dBranchMaintProteins = 2.0*pMaterial->pBranch->vProteins/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR 
			* 4.0/120.0 * (44.0/12.0);

		dBranchMaintLipids = 2.0*pMaterial->pBranch->vLipids/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
			* 7.0/900.0 * (44.0/12.0);

		dBranchMaintMinerals = 2.0*pMaterial->pBranch->vMinerals/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR 
			* 1.0/40.0 * (44.0/12.0);

		pPlant->pGenotype->fMaintBranch = (float)(dBranchMaintProteins*0.1*pow(fBranchRelNc,0.5)
									+ dBranchMaintLipids
									+ dBranchMaintMinerals);

	}
	
	
	POTATO
	{
		dTuberMaintProteins = 2.0*pMaterial->pTuber->vProteins/100.0
		* pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR 
		* 4.0/120.0 * (44.0/12.0);

		dTuberMaintLipids = 2.0*pMaterial->pTuber->vLipids/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR 
			* 7.0/900.0 * (44.0/12.0);

		dTuberMaintMinerals = 2.0*pMaterial->pTuber->vMinerals/100.0
			* pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR 
			* 1.0/40.0 * (44.0/12.0);

		pPlant->pGenotype->fMaintTuber = (float)((dTuberMaintProteins
									+ dTuberMaintLipids
									+ dTuberMaintMinerals)*0.5);
	}


	return 1;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Function: float DLL RelativeTemperatureResponse_PLATHO(float fTemp, 
//														float fTempMin, float fTempOpt, float fTempMax)
//Author:	Enli Wang
//Date:		07.11.1996
//Purpose:	This function calculates the effect of temperature on the rate of certain plant process with
//			a temperature optimum (fOptTemp)
//Inputs:	1. fTemp	- Current temperature (C)
//			2. fMinTemp	- Minimum temperature for the process (C) 	
//			2. fOptTemp	- Optimum temperature for the process (C) 	
//			2. fMaxTemp	- Maximum temperature for the process (C) 	
//Outputs:	The calculated temperature effect (0-1)  0-fMinTemp,fMaxTemp; 1-fOptTemp
//Functions Called:
//			None 
//Reference:1. Wang,Enli. xxxx.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
double RelativeTemperatureResponse_PLATHO(double vTemp, double vMinTemp, double vOptTemp, double vMaxTemp)
	{
	double p, vRelEff;

	if ((vTemp<=vMinTemp)||(vTemp>=vMaxTemp))
		vRelEff=0.0;
	else
		{
	    p =log(2)/log((vMaxTemp-vMinTemp)/(vOptTemp-vMinTemp));
		vRelEff = (2*pow(vTemp-vMinTemp,p)*pow(vOptTemp-vMinTemp,p)-pow(vTemp-vMinTemp,2*p))
                    /pow(vOptTemp-vMinTemp,2*p);   
		}
			
	return vRelEff;
	}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Function: float DLL HourlyTemperature_PLATHO(float Time, float fTempMax, float fTempMin, double daylength)
//Purpose:	This function uses daily maximum & minimum temperature to calculate the hourly temperature
//			(temperature daily course)  
//Inputs:	1. Time	- Current time in hours
//			2. fTempMax	- Daily maxmimum temperature of the air (C) 	
//			3. fTempMin	- Daily minimum temperature of the air (C)
//			4. daylength 
//Outputs:	The calculated temperature at time Time (C)
//Functions Called: 
//Reference:1. Wang,Enli. xxxx.
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
double HourlyTemperature_PLATHO(double DayTime, double vTempMax, double vTempMin, 
								   double vTempMinNext, double vDaylength, int iSimDay)
	{    
    double vTemp, vSunRise, vSunSet;
	double NightTime,NightLength;
	double vTempMaxDelay = 2.0;
	static double vTempSunSet;

	vSunRise = 12.0 - 0.5*vDaylength; //sunrise
	vSunSet	 = 12.0 + 0.5*vDaylength; //sunset

	NightLength = 24.0-vDaylength;

	vTempMax	 +=	273.16;
	vTempMin	 +=	273.16;
	vTempMinNext += 273.16;



	if(DayTime>=vSunRise && DayTime<=vSunSet)
	{
		vTemp = vTempMin + (vTempMax - vTempMin) * 
		  sin(PI*(DayTime-vSunRise)/(vDaylength+2.0*vTempMaxDelay));

		vTempSunSet = vTemp;
	}


     if(DayTime>vSunSet)
	 {
		  NightTime  =  DayTime - vSunSet;

		  /*fTemp = fTempSunSet * (float)exp(- NightTime *  
				log(fTempSunSet/fTempMinNext) / (24-fDaylength));*/

		  vTemp = (vTempSunSet-vTempMinNext)/pow(NightLength,2) * pow(NightTime,2) -
			  2*(vTempSunSet-vTempMinNext)/NightLength * NightTime + vTempSunSet;
	 }

	 if(DayTime<vSunRise) 
	 {
	 	if(iSimDay==1) 
			vTempSunSet =  vTempMin;  //für die erste Nacht ist die Sonnenuntergangstemperatur nicht bekannt!

		NightTime =  24.0 - vSunSet + DayTime;

		/*fTemp = fTempSunSet * (float)exp(- NightTime *  
				log(fTempSunSet/fTempMin)/(24-fDaylength));*/

		vTemp = (vTempSunSet-vTempMin)/pow(NightLength,2) * pow(NightTime,2) -
			  2*(vTempSunSet-vTempMin)/NightLength * NightTime + vTempSunSet;
	  }


	return vTemp-273.16;
}


int GetPlantNeighbours(PPLANT pPlant, int iPlant, int *iPlantLeft, int *iPlantRight, int *iPlantUpper, int *iPlantLower)
	{
    ////////////////////////////////////////////////////////////////////////////////////
	// Bestimmung der Nachbarn der i-ten Pflanze. Die Anordnung der Pflanzen 
	// ist durch die Anzahl der Spalten und Reihen, die im Input-File
	// Platho-Scenario.pss eingegeben werden, festgelegt.
	// (Anzahl der Spalten) x (Anzahl der Reihen) muss mit der Anzahl der in  
	// der Datenbank eingegebenen Pflanzen übereinstimmen!
	// 
	// Da im Modell keine Randeffekte berücksichtigt werden, werden die
	// Gitterzellen am Rand des Gitters gespiegelt, wobei Randzellen nicht
	// verdoppelt werden.
	//
	//	Die Pflanzen werden in Reihen nummeriert:
	//
	//			z.B.	1 2 3 4 
	//					5 6 7 8 
	//				...


	PPLANT pPlantLeft;
	PPLANT pPlantRight;
	PPLANT pPlantUpper;
	PPLANT pPlantLower;

		int i_Mod_iCol, iEnd, iskip, j, m;

		i_Mod_iCol = 0;

		for(m=iPlant;m>=0;m-=pPlathoScenario->iColumns)
		{
			if(m>=0)
				i_Mod_iCol = m;
		}


	// In dieser Variante der Bestimmung der Nachbarn entstehen bei einreihigen bzw.
	// einspaltigen Pflanzmustern "karierte" Muster:
                             
	//                        1
	//		 1 0 1 		   	  -	
	//		 - - -			1|0|1            1 0
	//	   1|0|1|2|1	      -				 - - 	
	//		 - - -			2|1|0		   1|0|1|0
	//		 1 2 1           -               - - 
    //                      1|2|1            1 0
	//                        -
	//                        1


	//left
	if((pPlathoScenario->iColumns==1)&&(pPlathoScenario->iRows==1))
	{	// nur eine Pflanze = Bestandesmodell
		pPlantLeft = pPlant;
		*iPlantLeft = iPlant;
	}
	else if (i_Mod_iCol == 0)
	{	// linker Rand (Spiegelung, Randzellen werden nicht verdoppelt)
		if(pPlant->pNext!=NULL)
		{	// entweder mehrere Spalten oder nur eine Spalte, 
			// dann aber nicht die letzte Pflanze
			pPlantLeft = pPlant->pNext;
			*iPlantLeft = iPlant+1;
		}
		else
		{	// nur eine Spalte, letzte Pflanze
			pPlantLeft = pPlant->pBack;
			*iPlantLeft = iPlant-1;
		}
	}
	else
	{	// mehrere Spalten, nicht linker Rand
		pPlantLeft = pPlant->pBack;
		*iPlantLeft = iPlant-1;
	}

	//right
	if((pPlathoScenario->iColumns==1)&&(pPlathoScenario->iRows==1))
	{	//nur eine Pflanze = Bestandesmodell
		pPlantRight = pPlant;
		*iPlantRight = iPlant;
	}
	else if (i_Mod_iCol == pPlathoScenario->iColumns-1)
	{	//rechter Rand
		if(pPlathoScenario->iColumns>1)
		{	// mehrere Spalten
			pPlantRight = pPlant->pBack;
			*iPlantRight = iPlant - 1;
		}
		else
		{	// nur eine Spalte
			if(pPlant->pBack!=NULL)
			{	// nicht erste Pflanze
				pPlantRight = pPlant->pBack;
				*iPlantRight = iPlant - 1;
			}
			else
			{	// erste Pflanze
				pPlantRight = pPlant->pNext;
				*iPlantRight = iPlant + 1;
			}
		}
	} // Ende rechter Rand
	else
	{	// nicht rechter Rand
		pPlantRight = pPlant->pNext;
		*iPlantRight = iPlant + 1;
	}

		
	
	//upper

	if((pPlathoScenario->iColumns==(int)1)&&(pPlathoScenario->iRows==(int)1))
	{	//nur eine Pflanze = Bestandesmodell
		pPlantUpper = pPlant;
		*iPlantUpper = iPlant;
	}
	else
	{	//mehrere Pflanzen
		iEnd = iPlant-pPlathoScenario->iColumns;
		iskip = -1;

		if(iEnd<0)
		{	// oberer Rand
			iEnd = iPlant+pPlathoScenario->iColumns;
			iskip = +1;
		}
		
		if(iEnd>=pPlathoScenario->iColumns*pPlathoScenario->iRows)
		{	// nur eine Reihe, 
			if(pPlant->pNext!=NULL) // nicht rechteste Rand
			{
				pPlantUpper = pPlant->pNext;
				*iPlantUpper = iPlant + 1;
			}
			else					// rechter Rand
			{
				pPlantUpper = pPlant->pBack;
				*iPlantUpper = iPlant -1;
			}
		}
		else
		{	// mehrere Reihen

			pPlantUpper = pPlant;
			for(j=1; j<=pPlathoScenario->iColumns; j++)
			{
				if(iskip == -1)	// nicht oberer Rand 
					pPlantUpper = pPlantUpper->pBack;

				if(iskip == 1)	// oberer Rand
					pPlantUpper = pPlantUpper->pNext;
			}
			
			*iPlantUpper = iPlant + iskip * pPlathoScenario->iColumns;
		}	//Ende mehrer Reihen
	}

	
	//lower

	if((pPlathoScenario->iColumns==(int)1)&&(pPlathoScenario->iRows==(int)1))
	{	//nur eine Pflanze = Bestandesmodell
		pPlantLower = pPlant;
		*iPlantLower = iPlant;
	}
	else
	{
		iEnd = iPlant+pPlathoScenario->iColumns;
		iskip = +1;

		if(iEnd>=pPlathoScenario->iColumns*pPlathoScenario->iRows)
		{	// unterer Rand
			iEnd = iPlant-pPlathoScenario->iColumns;
			iskip = -1;
		}
		
		if(iEnd<0)
		{	// nur eine Reihe
			if(pPlant->pBack!=NULL) // nicht linker Rand
			{
				pPlantLower = pPlant->pBack;
				*iPlantLower = iPlant - 1;
			}
			else
			{
				pPlantLower = pPlant->pNext; //linker Rand
				*iPlantLower = iPlant + 1;
			}
		}
		else
		{
			pPlantLower = pPlant;
			for(j=1; j<=pPlathoScenario->iColumns; j++)
			{
				if(iskip == -1)
					pPlantLower = pPlantLower->pBack;
				if(iskip == 1)
					pPlantLower = pPlantLower->pNext;
			}

			*iPlantLower = iPlant + iskip * pPlathoScenario->iColumns;

		}
	}

	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft  = pPlantLeft;
	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight = pPlantRight;
	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper = pPlantUpper;
	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower = pPlantLower;


		return 1;
	}



	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//Daylength and Photoperiod
// Aus SPASS.C (Enli Wang, 1998)
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
double Daylength(double vLatitude, int nJulianDay, int nID)
	{
	double SINLD     = 0.0;
	double COSLD     = 0.0;
	double vDay      = 0.0;
	double vDec      = 0.0;
	double RAD       = 0.0;
	double vReturn   = 0.0;
	double vDaylengthAst = 0.0;
	double vPhotoperiod = 0.0;
	double vDaylengthPs = 0.0;
	
	RAD 	  = PI/180.0;
	vDay 	  = (double)nJulianDay;

	//Declination of the sun as function of iJulianDaynumber (iJulianDay)
	vDec = -asin( sin(23.45*RAD)*cos(2.0*PI*(vDay+10.0)/365.0));

	//Intermediate variables
	SINLD = sin(RAD*vLatitude)*sin(vDec);
	COSLD = cos(RAD*vLatitude)*cos(vDec);
	
	//Astronomical daylength (hr) 
	vDaylengthAst  =12.0*(1.0+2.0*asin(SINLD/COSLD)/PI);         
   
   	//Photoperiodically active daylength (hr)
	vPhotoperiod =12.0*(1.0+2.0*asin((-sin(-4.0*RAD)+SINLD)/COSLD)/PI);         

   	//Photosythesis active daylength (hr)
	vDaylengthPs =12.0*(1.0+2.0*asin((-sin(8.0*RAD)+SINLD)/COSLD)/PI);         
   

	vReturn=vDaylengthAst;

	if (nID==1)
		vReturn=vDaylengthAst;
	if (nID==2)
		vReturn=vPhotoperiod;
	if (nID==3)
		vReturn=vDaylengthPs;
	
	return vReturn;
	}


double Weibull(double Argument, double Beta, double alpha)
{
	double arg, beta;
	double vValue;

	arg   = Argument;
	beta  = Beta;

	if(arg==0.0)
		vValue = 0.0;
	else
		vValue = alpha/beta*pow((arg/beta),alpha-1.0)*exp(-pow(arg/beta,alpha));
	
	return vValue;
}


double	GrowthFactor(double DevStage, double StageStart, double StagePeak, double StageEnd, double p)
{
	//////////////////////////////////////////////////////////////////////////////
	// Reduktion der Wachstumsrate in Abhängigkeit vom Entwicklungsstadium
	//
	// f2(t) = d/dt F2(t), mit F2(t) = y0+m*exp(-b*(tend-t)^p)
	//
	// Die Parameter beta, y0 und m sind so gewählt, dass f2(t) seinen maximalen
	// Wert bei t = tpeak erreicht, F2(tstart) = 0, und F2(tend) = 1.
	// p ist ein Form-Parameter
	//////////////////////////////////////////////////////////////////////////////

	double y0,m,beta;
	double  f2;

	if((DevStage>=StageStart)&&(DevStage<=StageEnd))
	{
		beta	= (p-1)/(p*pow(StageEnd-StagePeak,p));

		y0		=exp(-beta*pow(StageEnd-StageStart,p))
					/(exp(-beta*pow(StageEnd-StageStart,p))-1);

		m	= 1.0 - y0;


			f2	= m*beta*p*pow(StageEnd-DevStage,p-1.0)
							*exp(-beta*pow(StageEnd-DevStage,p));
	}
	else
		f2 = 0.0;

	return f2*(StageEnd-StageStart);
}


int		Competition(PPLANT pPlant, PPLANT pPlantUpper, PPLANT pPlantLeft, PPLANT pPlantRight,
					PPLANT pPlantLower, int iPlant, int iPlantUpper, int iPlantLeft, int iPlantRight, 
					int iPlantLower, PSPROFILE pSo)
{
	double crown_radius[5], rootzone_radius[5];
    int    i;

	PPLATHOMORPHOLOGY	pPMorph	     =  pPlathoPlant[iPlant]->pPlathoMorphology;
    PPLATHOMORPHOLOGY	pPMorphRight =  pPlathoPlant[iPlantRight]->pPlathoMorphology;
    PPLATHOMORPHOLOGY	pPMorphLower =  pPlathoPlant[iPlantLower]->pPlathoMorphology;
    PPLATHOMORPHOLOGY	pPMorphLeft  =  pPlathoPlant[iPlantLeft]->pPlathoMorphology;
    PPLATHOMORPHOLOGY	pPMorphUpper =  pPlathoPlant[iPlantUpper]->pPlathoMorphology;

    PPLATHOROOTLAYER	pPRL	    = pPMorph->pPlathoRootLayer;
    PPLATHOROOTLAYER	pPRLRight	= pPMorphRight->pPlathoRootLayer;
    PPLATHOROOTLAYER	pPRLLower	= pPMorphLower->pPlathoRootLayer;
    PPLATHOROOTLAYER	pPRLLeft	= pPMorphLeft->pPlathoRootLayer;
    PPLATHOROOTLAYER	pPRLUpper	= pPMorphUpper->pPlathoRootLayer;

	PPLATHOGENOTYPE		pPGen       = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOGENOTYPE		pPGenRight  = pPlathoPlant[iPlantRight]->pPlathoGenotype;
	PPLATHOGENOTYPE		pPGenLower  = pPlathoPlant[iPlantLower]->pPlathoGenotype;
	PPLATHOGENOTYPE		pPGenLeft   = pPlathoPlant[iPlantLeft]->pPlathoGenotype;
	PPLATHOGENOTYPE		pPGenUpper  = pPlathoPlant[iPlantUpper]->pPlathoGenotype;

    PLEAFLAYER          pLLayer = pPMorph->pLeafLayer;     

    double MeanDistance = pPlathoScenario->vLatticeSpacing;


    for(i=0;i<=3;i++) //right = 0, lower = 1, left = 2, upper = 3 
    {
	    pPMorph->AbvgCoeffComp[i] = 0.0;
	    pPMorph->BlwgCoeffComp[i] = 0.0;
    }

	//calculation of the overlap with the four next neighbours:
    //central = 0, right = 1, lower = 2, left = 3, upper = 4 

	crown_radius[0]    = 0.5*pPMorph->vCrownDiameter;
	rootzone_radius[0] = 0.5*pPMorph->vRootZoneDiameter;

	// Right -----------------------------------------------------------
	if(pPlantRight->pCanopy->fPlantHeight>(float)0.0)
    {
		crown_radius[1]    = 0.5*pPlathoPlant[iPlantRight]->pPlathoMorphology->vCrownDiameter;
		rootzone_radius[1] = 0.5*pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneDiameter;
    }
	else
    {
		crown_radius[1]    = 0.0;
		rootzone_radius[1] = 0.0;
    }

	// Lower -----------------------------------------------------------
	if(pPlantLower->pCanopy->fPlantHeight>(float)0.0)
    {
		crown_radius[2]    = 0.5*pPlathoPlant[iPlantLower]->pPlathoMorphology->vCrownDiameter;
		rootzone_radius[2] = 0.5*pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneDiameter;
    }
	else
    {
		crown_radius[2]    = 0.0;
		rootzone_radius[2] = 0.0;
    }

	// Left -----------------------------------------------------------
	if(pPlantLeft->pCanopy->fPlantHeight>(float)0.0)
    {
		crown_radius[3]    = 0.5*pPlathoPlant[iPlantLeft]->pPlathoMorphology->vCrownDiameter;
		rootzone_radius[3] = 0.5*pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneDiameter;
    }
	else
    {
		crown_radius[3]    = 0.0;
		rootzone_radius[3] = 0.0;
    }

	// Upper -----------------------------------------------------------
	if(pPlantUpper->pCanopy->fPlantHeight>(float)0.0)
    {
		crown_radius[4]    = 0.5*pPlathoPlant[iPlantUpper]->pPlathoMorphology->vCrownDiameter;
		rootzone_radius[4] = 0.5*pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneDiameter;
    }
	else
    {
		crown_radius[4]    = 0.0;
		rootzone_radius[4] = 0.0;
    }

	for(i=1;i<=4;i++)
    {   
        // AbvgCoeffComp, BlwgCoeffComp:  right = 0, lower = 1, left = 2, upper = 3 
        // crown_radius, rootzone_radius: central = 0, right = 1, lower = 2, left = 3, upper = 4 

        //above ground:
        if(!((crown_radius[0]<=0.0)||(crown_radius[i]<=0.0)
			||(crown_radius[i] + crown_radius[0] <= MeanDistance)))
        {
		    pPMorph->AbvgCoeffComp[i-1] = CompetitionFactor(crown_radius[0], crown_radius[i],MeanDistance);
	        pPMorph->AbvgCoeffComp[i-1] = max(0.0,min(1.0,pPMorph->AbvgCoeffComp[i-1]));
        }
        else
            pPMorph->AbvgCoeffComp[i-1] = 0.0;

        //below ground:
        if(!((rootzone_radius[0]<=0.0)||(rootzone_radius[i]<=0.0)
			||(rootzone_radius[i] + rootzone_radius[0] <= MeanDistance)))
        {
		    pPMorph->BlwgCoeffComp[i-1] = CompetitionFactor(rootzone_radius[0], rootzone_radius[i],MeanDistance);
	        pPMorph->BlwgCoeffComp[i-1] = max(0.0,min(1.0,pPMorph->BlwgCoeffComp[i-1]));
        }
        else
            pPMorph->BlwgCoeffComp[i-1] = 0.0;
    }
	//-------------------------------------------------------------------


    //Consideration of sectors
    if(pPlathoModules->iFlagSectors==2)
    {
        double AbvgSC_area[4], BlwgSC_area[4];
        double LeafAreaSecCP, RootAreaSecCP;
        double SecLeafArea[4], SecRootArea[4];
        double SumSecLeafArea, SumSecRootArea;
        double SumSecCC, SumLaySecCC;

        int N, L, LMAX;
        double PlantHeight = (double)pPlant->pCanopy->fPlantHeight;

        //Schnittflächen der Sektoren 1-4: [%/100]
        for(i=1;i<=4;i++) //radius: central = 0, right = 1, lower = 2, left = 3, upper = 4
        {                 //sector: right = 0, lower = 1, left = 2, upper = 3
            //above ground:
            if(!((crown_radius[0] <= 0.0)||(crown_radius[i] <= 0.0)
			    ||(crown_radius[i] + crown_radius[0] <= MeanDistance)))
            {
		        pPMorph->SectorCompCoeff_abvg[i-1] = SectorCompFactor(crown_radius[0], crown_radius[i],MeanDistance);
	            pPMorph->SectorCompCoeff_abvg[i-1] = max(0.0,min(1.0,pPMorph->SectorCompCoeff_abvg[i-1]));
            }
            else
                pPMorph->SectorCompCoeff_abvg[i-1] = 0.0;

            //below ground:
            if(!((rootzone_radius[0]<=0.0)||(rootzone_radius[i]<=0.0)
			    ||(rootzone_radius[i] + rootzone_radius[0] <= MeanDistance)))
            {
		        pPMorph->SectorCompCoeff_blwg[i-1] = SectorCompFactor(rootzone_radius[0], rootzone_radius[i],MeanDistance);
	            pPMorph->SectorCompCoeff_blwg[i-1] = max(0.0,min(1.0,pPMorph->SectorCompCoeff_blwg[i-1]));
            }
            else
                pPMorph->SectorCompCoeff_blwg[i-1] = 0.0;

            //Fläche der Schnittfläche                
            AbvgSC_area[i-1] = pPMorph->SectorCompCoeff_abvg[i-1] * pow(crown_radius[0],2)*PI/4.0;
            BlwgSC_area[i-1] = pPMorph->SectorCompCoeff_blwg[i-1] * pow(rootzone_radius[0],2)*PI/4.0;
        }

        
        //Gewichtete Konkurrenzindizes der Sektoren in jeder Schicht

        //above ground
        pLLayer = pPMorph->pLeafLayer;       
        for(N = 0; N < LEAFLAYERS; N++)
        {    			
            //	leaf area of neighbours in height h:
            pLLayer->vLeafAreaNbg[0] = CalcLA_Nbg(N, PlantHeight, pPlantRight->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphRight->vHeightOfCrownOnset, pPMorphRight->pLeafLayer);
            pLLayer->vLeafAreaNbg[1] = CalcLA_Nbg(N, PlantHeight, pPlantLower->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphLower->vHeightOfCrownOnset, pPMorphLower->pLeafLayer);
            pLLayer->vLeafAreaNbg[2] = CalcLA_Nbg(N, PlantHeight, pPlantLeft->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphLeft->vHeightOfCrownOnset, pPMorphLeft->pLeafLayer);
            pLLayer->vLeafAreaNbg[3] = CalcLA_Nbg(N, PlantHeight, pPlantUpper->pCanopy->fPlantHeight, pPMorph->vHeightOfCrownOnset, pPMorphUpper->vHeightOfCrownOnset, pPMorphUpper->pLeafLayer);
        
            //Leaf area in sectors 1-4: [m^2]
            SumSecLeafArea = 0.0;
            for(i=1;i<=4;i++) //central = 0, right = 1, lower = 2, left = 3, upper = 4
            {
                // leaf area of central plant
                LeafAreaSecCP =  pLLayer->apLeafSector[i-1]->vLeafAreaSec;

                //total leaf area in sector
                if(crown_radius[i] > 0.0)
                {
                    SecLeafArea[i-1] = LeafAreaSecCP + min(pLLayer->vLeafAreaNbg[i-1],
                        AbvgSC_area[i-1]/(pow(crown_radius[i],2)*PI)*pLLayer->vLeafAreaNbg[i-1]);
                }
                else
                    SecLeafArea[i-1] = LeafAreaSecCP;
                
                //Total leaf area in leaf layer
                SumSecLeafArea += SecLeafArea[i-1];
            }

            if(SumSecLeafArea > 0.0)
            {
                //Gewichtung:
                SumSecCC = 0.0;
                SumLaySecCC = 0.0;
                for(i=0;i<4;i++) //right = 0, lower = 1, left = 2, upper = 3
                {
                    pLLayer->apLeafSector[i]->vSecCompCoeff = pPMorph->SectorCompCoeff_abvg[i] 
                            * SecLeafArea[i]/SumSecLeafArea;

                    SumSecCC += pPMorph->SectorCompCoeff_abvg[i];
                    SumLaySecCC += pLLayer->apLeafSector[i]->vSecCompCoeff;
                }

                //Normierung:
                for(i=0;i<4;i++) //right = 0, lower = 1, left = 2, upper = 3
                {
                    if(SumLaySecCC>0.0)
                        pLLayer->apLeafSector[i]->vSecCompCoeff *= SumSecCC/SumLaySecCC;
                    else
                        pLLayer->apLeafSector[i]->vSecCompCoeff = 0.0;
                }
            }
            else
            {
                for(i=0;i<4;i++) //right = 0, lower = 1, left = 2, upper = 3
                    pLLayer->apLeafSector[i]->vSecCompCoeff = 0.0;
            }

                pLLayer = pLLayer->pNext;
        }


        //below ground

        //deepest layer
        LMAX = (int)((pSo->iLayers-2)*pPlant->pRoot->fDepth/pPlant->pRoot->fMaxDepth)+1;
        LMAX = (int)min((pSo->iLayers-2),LMAX);

        pPRL	    = pPMorph->pPlathoRootLayer;
        pPRLRight	= pPMorphRight->pPlathoRootLayer;
        pPRLLower	= pPMorphLower->pPlathoRootLayer;
        pPRLLeft	= pPMorphLeft->pPlathoRootLayer;
        pPRLUpper	= pPMorphUpper->pPlathoRootLayer;
        for(L = 1;L<=LMAX;L++)
        {    			
            // root area of neighbours in height h
            pPRL->RootAreaNbg[0] =  pPRLRight->vRootArea;
            pPRL->RootAreaNbg[1] =  pPRLLower->vRootArea;
            pPRL->RootAreaNbg[2] =  pPRLLeft->vRootArea;
            pPRL->RootAreaNbg[3] =  pPRLUpper->vRootArea;
        
            //root area in sectors 1-4: [m^2]
            SumSecRootArea = 0.0;
            for(i=1;i<=4;i++) //central = 0, right = 1, lower = 2, left = 3, upper = 4
            {
                // root area of central plant
                RootAreaSecCP =  pPRL->apRootSector[i-1]->vRootArea;

                //total root area in sector
                if(rootzone_radius[i] > 0.0)
                {
                    SecRootArea[i-1] = RootAreaSecCP + min(pPRL->RootAreaNbg[i-1], 
                        BlwgSC_area[i-1]/(pow(rootzone_radius[i],2)*PI)*pPRL->RootAreaNbg[i-1]);
                }
               else
                    SecRootArea[i-1] = RootAreaSecCP;

                
                //Total root area in soil layer
                SumSecRootArea += SecRootArea[i-1];
            }

           if(SumSecRootArea>0.0)
           {
               //Gewichtung:
                SumSecCC = 0.0;
                SumLaySecCC = 0.0;
                for(i=0;i<4;i++) //right = 0, lower = 1, left = 2, upper = 3
                {
                    pPRL->apRootSector[i]->vSecCompCoeff =  pPMorph->SectorCompCoeff_blwg[i] 
                            * SecRootArea[i]/SumSecRootArea;

                    SumSecCC += pPMorph->SectorCompCoeff_blwg[i];
                    SumLaySecCC += pPRL->apRootSector[i]->vSecCompCoeff;
                }

                //Normierung:
                for(i=0;i<4;i++) //right = 0, lower = 1, left = 2, upper = 3
                {
                    if(SumLaySecCC>0.0)
                        pPRL->apRootSector[i]->vSecCompCoeff *= SumSecCC/SumLaySecCC;
                    else
                        pPRL->apRootSector[i]->vSecCompCoeff = 0.0;
                }
           }
           else
           {
                for(i=0;i<4;i++) //right = 0, lower = 1, left = 2, upper = 3
                    pPRL->apRootSector[i]->vSecCompCoeff = 0.0;
           }


            pPRL	    = pPRL->pNext;
            pPRLRight	= pPRLRight->pNext;
            pPRLLower	= pPRLLower->pNext;
            pPRLLeft	= pPRLLeft->pNext;
            pPRLUpper	= pPRLUpper->pNext;
        } //end rooted soil layers
    } //end sectors
	
	return 1;
}


double	CompetitionFactor(double radius1, double radius2, double MeanDistance)
{
	double fx, fA1, fB1, fAx, fBx;

	//Berechnung der Schnittlinie
	if(radius1 + radius2 <= MeanDistance)
		return 0.0;
    else if (radius1 <= radius2 - MeanDistance)
        return 1.0;
    else if (radius1 - MeanDistance >= radius2)
        return  pow(radius2,2)/pow(radius1,2);
	else
		fx=(pow(MeanDistance,2)+pow(radius1,2)-pow(radius2,2))
				/(2*MeanDistance);

	if((fx<radius1)&&(-fx<radius2))
		fAx = fx*sqrt(pow(radius1,2)-pow(fx,2))+
			pow(radius1,2)*asin(fx/radius1)+pow(radius1,2)*PI/2.0;
	else
		fAx = 0.0;

	fA1 = pow(radius1,2)*PI - fAx;


	if((fx<MeanDistance+radius2)&&(fx>MeanDistance-radius2))
		fBx = (MeanDistance-fx)*sqrt(pow(radius2,2)-
			pow(MeanDistance-fx,2))+pow(radius2,2)*asin((MeanDistance-fx)/radius2)+pow(radius2,2)*PI/2.0;
	else
		fBx = 0.0;

	fB1 = pow(radius2,2)*PI - fBx;

	return (fA1+fB1)/(pow(radius1,2)*PI);
}



double	CalcKappa(double  dSpecLfWeightMax, double vLAI, int iPlant)
{
	double kappa, kappa0, kappa1, f_kappa, df_kappa, help1;
	int k;

	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;

	// SpecLfW(LAIc) = SpecLfWMax * exp(-kappa*LAIc)
	
	//Bestimmung von kappa:

	// Startwert:
	if(vLAI==0.0)
	{
		kappa=EPSILON;
		return 1;
	}
	else
	{
		kappa0 = 2.0*(dSpecLfWeightMax-pPMorph->vMeanSpecLfWeight)/(vLAI*dSpecLfWeightMax);
		kappa = max(kappa0,EPSILON);

		//Newton-Verfahren
		for(k=1;k<=1000;k++)
		{
			help1 = dSpecLfWeightMax/(vLAI*kappa)*(1.0-exp(-kappa*vLAI));
			f_kappa = pPMorph->vMeanSpecLfWeight - help1;
			df_kappa = dSpecLfWeightMax/(vLAI*kappa*kappa)*(1.0-exp(-2.0*kappa*vLAI));
			kappa1 = kappa - f_kappa/(df_kappa);

			//Abbruchkriterium:
			if(kappa1-kappa<1e-6)
				break;

			kappa = kappa1;
		}

		if(k==1000)
		{
			//Iterationsverfahren hat nicht konvergiert!
			kappa = kappa0;
		}
	}

	return kappa;
}


int Weight_kg_ha(PBIOMASS pBiom, float fSowDensity)
{
	pBiom->fRootWeight *= fSowDensity * (float)1e4;
	pBiom->fGrossRootWeight *= fSowDensity * (float)1e4;
	pBiom->fStemWeight *= fSowDensity * (float)1e4;
	pBiom->fBranchWeight *= fSowDensity * (float)1e4;
	pBiom->fLeafWeight *= fSowDensity * (float)1e4;
	pBiom->fFruitWeight *= fSowDensity * (float)1e4;
	pBiom->fTuberWeight *= fSowDensity * (float)1e4;
	pBiom->fGrainWeight *= fSowDensity * (float)1e4;

	pBiom->fTotalBiomass  *= fSowDensity * (float)1e4;
	pBiom->fBiomassAbvGround  *= fSowDensity * (float)1e4; 
	pBiom->fStovWeight  *= fSowDensity * (float)1e4;

	return 1;
}

int Weight_kg_plant(PBIOMASS pBiom, float fSowDensity)
{
	pBiom->fRootWeight /= fSowDensity * (float)1e4;
	pBiom->fGrossRootWeight /= fSowDensity * (float)1e4;
	pBiom->fStemWeight /= fSowDensity * (float)1e4;
	pBiom->fBranchWeight /= fSowDensity * (float)1e4;
	pBiom->fLeafWeight /= fSowDensity * (float)1e4;
	pBiom->fFruitWeight /= fSowDensity * (float)1e4;
	pBiom->fTuberWeight /= fSowDensity * (float)1e4;
	pBiom->fGrainWeight /= fSowDensity * (float)1e4;

	pBiom->fTotalBiomass  /= fSowDensity * (float)1e4;
	pBiom->fBiomassAbvGround  /= fSowDensity * (float)1e4; 
	pBiom->fStovWeight  /= fSowDensity * (float)1e4;

	return 1;
}

double calc_seasonal_reduction(double Temperature)
{
    double Rp; // change of photosynthetic capacity
	double K;  // relative photosynthetic capacity
    double  A_Seasonal, C_Seasonal, SP;

    A_Seasonal  = 2.0;		// value of Ilkka Leinonen
	C_Seasonal  = 600.0;	// value of Ilkka Leinonen
	SP          = 5000.0;	// seed


	Rp=(100/(1+100*pow(A_Seasonal,-(Temperature-SP/C_Seasonal))))-
		(100/(1+100*pow(A_Seasonal,+(Temperature-SP/C_Seasonal))));

	Rp+=SP; // make sum
	if(Rp<EPSILON)
		K = EPSILON;
	else
	if (Rp<6500)
		K = Rp / 6500.0;
	else
		K = 1;

	return K ;
}




double	SectorCompFactor(double r_c, double r_nbg, double d)
{
	double c_factor, xsi, help1;
    double f1, f1_1, f1_2, f2, f3;

    #define SQRT2 sqrt(2)
	
    //d = MeanDistance;

    // Case differentiation: 
        // r_c = radius of the (central) individuum
        // r_nbg = radius of the neighbour individuum
        // d = distance between two neighbours

    if(r_c + r_nbg <= d) return 0.0;
    else xsi = (pow(d,2) + pow(r_c,2) - pow(r_nbg,2))/(2.0*d);


    if(r_nbg <= d/SQRT2)
    {
        if(r_c <= d+r_nbg) 
        {
            f1 = -f_area(r_nbg,r_nbg,d-xsi);         
            f3 = f_area(r_c,xsi,r_c);  

            c_factor = (f1+f3)/(pow(r_c,2)*PI/4.0);
        }
        else
            c_factor = pow(r_nbg,2)/(pow(r_c,2));

    }
    else if (r_nbg <= d)  // d/sqrt(2) < r_nbg <= d
    {
        help1 = sqrt(2*pow(r_nbg,2)-pow(d,2));

        if(r_c <= (d - help1)/SQRT2) 
        {
            f1 = -f_area(r_nbg,r_nbg,d-xsi);         
            f3 = f_area(r_c,xsi,r_c);  

            c_factor = (f1+f3)/(pow(r_c,2)*PI/4.0);
        }
        else if(r_c <= (d + help1)/SQRT2) 
        {
            f1 = -f_area(r_nbg,r_nbg,(d+help1)/2.0);    
            f2 = f2_area((d-help1)/2,r_c/SQRT2);
            f3 = f_area(r_c,r_c/SQRT2,r_c);  

            c_factor = (f1+f2+f3)/(pow(r_c,2)*PI/4.0);
        }
        else if(r_c <= d + r_nbg) 
        {
            f1_1 = -f_area(r_nbg,r_nbg,(d+help1)/2.0);    
            f2 = f2_area((d-help1)/2,(d+help1)/2);
            f1_2 = -f_area(r_nbg,(d-help1)/2.0,d-xsi);    
            f3   = f_area(r_c,xsi,r_c);  

            c_factor = (f1_1 + f1_2 + f2 + f3)/(pow(r_c,2)*PI/4.0);
        }
        else // r_c > d + r_nbg
        {
            f1_1 = -f_area(r_nbg,r_nbg,(d+help1)/2.0);    
            f2 = f2_area((d-help1)/2,(d+help1)/2);
            f1_2 = -f_area(r_nbg,(d-help1)/2.0,-r_nbg);    

            c_factor = (f1_1 + f1_2 + f2)/(pow(r_c,2)*PI/4.0);
        }
    }
    else //r_nbg > d
    {
        help1 = sqrt(2*pow(r_nbg,2)-pow(d,2));

        if(r_c <= (d + help1)/SQRT2) 
        {
            c_factor = 1.0;
        }
        else if(r_c <= d + r_nbg)
        {
            f2 = f2_area(0,(d+help1)/2);
            f1 = -f_area(r_nbg,(d-help1)/2,d-xsi);
            f3 = f_area(r_c,xsi,r_c); 

            c_factor = (f1 + f2 + f3)/(pow(r_c,2)*PI/4.0);
        }
        else //r_c > d + r_nbg
        {
            f2 = f2_area(0,(d+help1)/2);
            f1 = -f_area(r_nbg,(d-help1)/2,-r_nbg);
 
            c_factor = (f1 + f2)/(pow(r_c,2)*PI/4.0);
       }
    }


	return c_factor;
}



double f_area(double r, double x1, double x2)
{
    /*   f_area = int_x1^x2 sqrt(r^2-x^2) dx   */
    
    double ff1,ff2,ff3,ff4;
	
    ff1 = x2*sqrt(pow(r,2)-pow(x2,2));
    ff2 = pow(r,2)*asin(x2/r);

    ff3 = x1*sqrt(pow(r,2)-pow(x1,2));
    ff4 = pow(r,2)*asin(x1/r);

	return ff1+ff2-ff3-ff4;
}

double f2_area(double x1, double x2)
{
	return pow(x2,2) - pow(x1,2);
}


double  CalcLAICumNbg(int iLeafLayer, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg)
        { 		
            int iNbg,j;
            double LAICum;
            		
            if(vNbgHeight>vNbgHCrownOnset)
		    {
                //Berechnung der Blattschicht gleicher Höhe des jeweiligen Nachbarn	
                iNbg = (int)(iLeafLayer*(vPlantHeight-vHCrownOnset)/(vNbgHeight-vNbgHCrownOnset));

                //Zeiger auf die Blattschicht setzen:
		        if(iNbg<LEAFLAYERS)
                {
                    for(j=1;j<=iNbg;j++)
                        pLLayerNbg=pLLayerNbg->pNext;
                        
			        LAICum = pLLayerNbg->vLAICum;
                }
		        else
			        LAICum = 0.0; 
		    }
		    else //kein Nachbar
			    LAICum = 0.0; 

            return LAICum;
        }

double  CalcLA_Nbg(int iLeafLayer, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg)
        { 		
            int iNbg,j;
            double LA_Nbg;
            		
            if(vNbgHeight>vNbgHCrownOnset)
		    {
                //Berechnung der Blattschicht gleicher Höhe des jeweiligen Nachbarn	
                iNbg = (int)(iLeafLayer*(vPlantHeight-vHCrownOnset)/(vNbgHeight-vNbgHCrownOnset));

                //Zeiger auf die Blattschicht setzen:
		        if(iNbg<LEAFLAYERS)
                {
                    for(j=1;j<=iNbg;j++)
                        pLLayerNbg=pLLayerNbg->pNext;
                        
                    LA_Nbg = pLLayerNbg->vLeafAreaLay;
                }
		        else
			        LA_Nbg = 0.0; 
		    }
		    else //kein Nachbar
			    LA_Nbg = 0.0; 

            return LA_Nbg;
        }