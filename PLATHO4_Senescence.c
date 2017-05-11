//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"

int Senescence_PLATHO(EXP_POINTER2, float fPlantDens, int iPlant);

extern double HourlyTemperature_PLATHO(double,double,double,double,double,int);
extern double Daylength(double,int,int);
extern double AVGENERATOR(double vInput, VRESPONSE* pvResp);

int Senescence_PLATHO(EXP_POINTER2, float fPlantDens, int iPlant)
{
	double vCH2OFRootRelocR, vCH2OGRootRelocR, vCH2OStemRelocR, vCH2OLeafRelocR, vCH2OBranchRelocR, vCH2OFruitRelocR, vCH2OTuberRelocR;
	double vNFRootRelocR, vNGRootRelocR, vNStemRelocR, vNLeafRelocR, vNBranchRelocR, vNFruitRelocR, vNTuberRelocR;
	//double vLeafDamageO3, vLeafDamagePath, vRootDamage;
    double vFRootLengthLoss, vGRootLengthLoss, TFRLLR, TGRLLR;

    double LAreaLossR, FRBiomLossLay, FRLengthLossLay, GRBiomLossLay, GRLengthLossLay;

	double DeltaT = (double)pTi->pTimeStep->fAct;
	double TimeDay = pTi->pSimTime->fTimeDay;		// Tageszeit
	int L,L1, N, iSector;

	double TempAct;
    double vDaylengthPs, vLeafSenescAge, vStemSenescAge, vRootSenescAge, vSenescLAI;
	//float fTempEffect;
	double vTempEffectLeaf, vTempEffectRoot;

    double vLeafCLitter,vLeafNLitter,vStemCLitter,vStemNLitter,vBranchCLitter,vBranchNLitter;
    double vFRCLitter, vFRNLitter, vGRCLitter, vGRNLitter;

	PDEVELOP		pDev	= pPlant->pDevelop;
	PBIOMASS		pBiom	= pPlant->pBiomass;
	PGENOTYPE		pGen	= pPlant->pGenotype;
	PPLTNITROGEN	pPltN	= pPlant->pPltNitrogen;
	PLAYERROOT		pLR		= pPlant->pRoot->pLayerRoot;

    PCLAYER         pCL     = pCh->pCLayer;

	PPLATHODEVELOP		pPDev		= pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHOGENOTYPE		pPGen		= pPlathoPlant[iPlant]->pPlathoGenotype;
    PPLANTMATERIAL      pMaterial   = pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;
	PPLATHOBIOMASS		pPBiom		= pPlathoPlant[iPlant]->pPlathoBiomass;
    PPLATHOMORPHOLOGY	pPMorph		= pPlathoPlant[iPlant]->pPlathoMorphology;
    PPLATHOPARTITIONING pPPart      = pPlathoPlant[iPlant]->pPlathoPartitioning;
	PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
    PLEAFLAYER          pLLayer     = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;    
    PPLATHOROOTLAYER    pPRL        = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
    PLEAFSECTOR         pLLSec      = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer->apLeafSector[0];

	vCH2OFRootRelocR   = 0.0;
	vCH2OGRootRelocR   = 0.0;
	vCH2OStemRelocR    = 0.0;
	vCH2OBranchRelocR  = 0.0;
	vCH2OLeafRelocR    = 0.0;
	vCH2OFruitRelocR   = 0.0;
	vCH2OTuberRelocR   = 0.0;

	vNFRootRelocR   = 0.0;
	vNGRootRelocR   = 0.0;
	vNStemRelocR    = 0.0;
	vNBranchRelocR  = 0.0;
	vNLeafRelocR    = 0.0;
	vNFruitRelocR   = 0.0;
	vNTuberRelocR   = 0.0;

	pBiom->fRootDeathRate		= (float)0.0;
	pBiom->fGrossRootDeathRate  = (float)0.0;
	pBiom->fStemDeathRate		= (float)0.0;
	pBiom->fBranchDeathRate		= (float)0.0;
	pBiom->fLeafDeathRate		= (float)0.0;
	pBiom->fFruitDeathRate		= (float)0.0;
	pBiom->fTuberDeathRate		= (float)0.0;

//	vLeafDamageO3   = 0.0;
//	vLeafDamagePath = 0.0;
//	vRootDamage     = 0.0;

    pPBiom->vLeafDamageO3   = 0.0;
	pPBiom->vLeafDamagePath = 0.0;
	pPBiom->vRootDamage     = 0.0;

    vLeafCLitter= 0.0;
    vLeafNLitter= 0.0;
    vStemCLitter= 0.0;
    vStemNLitter= 0.0;
    vBranchCLitter= 0.0;
    vBranchNLitter= 0.0;
    vFRCLitter= 0.0;
    vFRNLitter= 0.0;
    vGRCLitter= 0.0;
    vGRNLitter= 0.0;

	L1 = 0;
	for (L=1;L<=pSo->iLayers-2;L++)
	{
		//Check the whether there are roots in this layer:
		if (pLR->fLengthDens==(float)0.0)		break;
		//The last layer of root:
		L1=L;

		pLR = pLR->pNext;
	}

				
	//Temperatureffekt:
	if(pTi->pTimeStep->fAct == (float)1.0)
		TempAct = (pCl->pWeather->fTempAve>(float)-99)?
		pCl->pWeather->fTempAve : (pCl->pWeather->fTempMin+pCl->pWeather->fTempMax)/(float)2.0;
	else //kleine Zeitschritte
	{
		if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))
			TempAct = pPlathoClimate->vTemp;
		else
		{
			vDaylengthPs = Daylength(pLo->pFarm->fLatitude, pTi->pSimTime->iJulianDay,3);
			//Aktuelle Temperatue:
			if(((int)pTi->pSimTime->fTimeAct==0)||(pPlant->pDevelop->iDayAftSow==1))
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

	// fTempEffect = (float)pow(1.5,(TempAct-20.0)/10.0);
	vTempEffectLeaf = (double)AFGENERATOR((float)TempAct,pPlant->pGenotype->LvDeathRateTemp);
	vTempEffectRoot = (double)AFGENERATOR((float)TempAct,pPlant->pGenotype->RtDeathRateTemp);

	
	//Damage by ozone
	//if(pPPltStress->vO3StressIntensity>(float)0.0)

    if(pPlathoModules->iFlagO3==3)
	{
		pPBiom->vLeafDamageO3 =max(0,pPPltStress->vO3Sensitivity*pPPltStress->vO3DamageRate 
			* pow(pPPltStress->vO3StressIntensity,1)*pBiom->fLeafWeight)* DeltaT;

		pPlant->pBiomass->fLeafWeight -= (float)pPBiom->vLeafDamageO3;
		
		if(pPlant->pBiomass->fLeafWeight<=(float)0.0)
		{
			pPBiom->vLeafDamageO3 = pPlant->pBiomass->fLeafWeight;
			pPlant->pBiomass->fLeafWeight = (float)0.0;
			pPltN->fLeafCont = (float)0.0;
            pPPltStress->vLeafDefCont = 0.0;
		}
		else
		{
			pPltN->fLeafCont -= (float)pPBiom->vLeafDamageO3 * pPltN->fLeafActConc;
            pPltN->fLeafCont = (float)max(0.0,pPltN->fLeafCont);

            pPPltStress->vLeafDefCont -= pPBiom->vLeafDamageO3 * pPPltStress->vLeafDefConc;
            pPPltStress->vLeafDefCont = max(0.0,pPPltStress->vLeafDefConc);
		}


	}

	//Damage by leaf destroying pathogenes
	if(pPlathoScenario->vLeafPathogenes>(float)0.0)
	{
		pPBiom->vLeafDamagePath =max(0,(pPPltStress->vLfPathDamageRate 
			* pow(pPlathoScenario->vLeafPathogenes,1) - pPPltStress->vLfPathDefenseEff*
			pow(pPPltStress->vLeafDefConc,pPPltStress->vLfPathBeta)))
			*pBiom->fLeafWeight* DeltaT;

		pPlant->pBiomass->fLeafWeight -= (float)pPBiom->vLeafDamagePath;
		
		if(pPlant->pBiomass->fLeafWeight<=(float)0.0)
        {
            pPBiom->vLeafDamagePath = pPlant->pBiomass->fLeafWeight;
			pPlant->pBiomass->fLeafWeight = (float)0.0;
        }

		pPltN->fLeafCont -= (float)pPBiom->vLeafDamagePath * pPltN->fLeafActConc;
		pPltN->fLeafCont = (float)max(0.0,pPltN->fLeafCont);

        pPPltStress->vLeafDefCont -= pPBiom->vLeafDamagePath * pPPltStress->vLeafDefConc;
        pPPltStress->vLeafDefCont = max(0.0,pPPltStress->vLeafDefConc);
	}


	//Damage by root destroying pathogenes
	if(pPlathoScenario->vRootPathogenes>(float)0.0)
	{
		pPBiom->vRootDamage = max(0,
			(pPPltStress->vRtPathDamageRate * 
			pow(pPlathoScenario->vRootPathogenes,1) - pPPltStress->vRtPathDefenseEff
			*pow(pPPltStress->vFineRootDefConc,pPPltStress->vRtPathBeta)))
			* pBiom->fRootWeight * DeltaT;

		pPlant->pBiomass->fRootWeight -= (float)pPBiom->vRootDamage;

		if(pPlant->pBiomass->fRootWeight<=(float)0.0)
        {
		    pPBiom->vRootDamage =  pPlant->pBiomass->fRootWeight;
            pPlant->pBiomass->fRootWeight = (float)0.0;
        }

		pPltN->fRootCont -= (float)pPBiom->vRootDamage * pPltN->fRootActConc;
		pPltN->fRootCont = (float)max(0.0,pPltN->fRootCont);

        pPPltStress->vFineRootDefCont -= pPBiom->vRootDamage * pPPltStress->vFineRootDefConc;
        pPPltStress->vFineRootDefCont = max(0.0,pPPltStress->vFineRootDefCont);
	}

    ////////////////////////////////////////////////////////////
    // senescence due to shadowing, aging or continous turnover:

    //shadowing effect
	vSenescLAI = (pPMorph->vPlantLAI>pPlant->pGenotype->fBeginShadeLAI)?
				pow(pPMorph->vPlantLAI/pPlant->pGenotype->fBeginShadeLAI,2):1.0;
	HERBS
	{
		if(pPDev->iStagePlatho <= 1)
		{
			pBiom->fRootDeathRate	= (float)0.0;		
			pBiom->fStemDeathRate   = (float)0.0;
			pBiom->fLeafDeathRate   = (float)0.0;
		}
		else if (pPDev->iStagePlatho <= 3)
		{

			pBiom->fRootDeathRate = (float)(pBiom->fRootWeight * pPGen->vFineRootLossR*vTempEffectRoot*DeltaT);
			pBiom->fStemDeathRate = (float)(pBiom->fStemWeight * pPGen->vStemLossR*DeltaT);
			pBiom->fLeafDeathRate = (float)(pBiom->fLeafWeight * pPGen->vLeafLossR*vSenescLAI*vTempEffectLeaf*DeltaT);
		}
		else //else if (pPDev->iStagePlatho <= 5)
		{

			//senescence due to aging:
			vLeafSenescAge =pDev->fDTT
				/(pGen->pStageParam->afThermalTime[5]-pGen->pStageParam->afThermalTime[3]);
			
			vStemSenescAge =0.2*pDev->fDTT
				/(pGen->pStageParam->afThermalTime[5]-pGen->pStageParam->afThermalTime[3]);

			vRootSenescAge =(pDev->fDevStage>4.3)? pDev->fDTT
				/(pGen->pStageParam->afThermalTime[5]-pGen->pStageParam->afThermalTime[3]):0.0;

			pBiom->fRootDeathRate = (float)min(pBiom->fRootWeight,pBiom->fRootWeight * vRootSenescAge*vTempEffectRoot*DeltaT);
			pBiom->fStemDeathRate = (float)min(pBiom->fStemWeight,pBiom->fStemWeight * vStemSenescAge*DeltaT);
			pBiom->fLeafDeathRate = (float)min(pBiom->fLeafWeight,pBiom->fLeafWeight * vSenescLAI*vLeafSenescAge*vTempEffectLeaf*DeltaT);


/*			vLeafSenescAge =pDev->fDTT
				/(float)max(1e-6,pGen->pStageParam->avThermalTime[5]-pDev->fCumDTT);
			
			vStemSenescAge =(float)0.5*pDev->fDTT
				/(float)max(1e-6,pGen->pStageParam->avThermalTime[5]-pDev->fCumDTT);

			vRootSenescAge =(pDev->fDevStage>(float)4.3)? pDev->fDTT
				/(float)max(1e-6,pGen->pStageParam->avThermalTime[5]-pDev->fCumDTT):(float)0.0;

			pBiom->fRootDeathRate = pBiom->fRootWeight * vRootSenescAge*vTempEffectRoot*DeltaT;
			pBiom->fStemDeathRate = pBiom->fStemWeight * vStemSenescAge*DeltaT;
			pBiom->fLeafDeathRate = pBiom->fLeafWeight * vSenescLAI*vLeafSenescAge*vTempEffectLeaf*DeltaT;
			
		}
		else
		{
			pBiom->fRootDeathRate = pBiom->fRootWeight;
			pBiom->fStemDeathRate = pBiom->fStemWeight;
			pBiom->fLeafDeathRate = pBiom->fLeafWeight;
			
		*/
		}
	}

	TREES
	{
		pBiom->fRootDeathRate		= (float)(pBiom->fRootWeight *pPGen->vFineRootLossR*vTempEffectRoot*DeltaT);
		pBiom->fGrossRootDeathRate  = (float)(pBiom->fGrossRootWeight *pPGen->vGrossRootLossR*DeltaT);
		pBiom->fStemDeathRate		= (float)(pBiom->fStemWeight *pPGen->vStemLossR*DeltaT);
		pBiom->fBranchDeathRate		= (float)(pBiom->fBranchWeight *pPGen->vBranchLossR*DeltaT);

		SPRUCE
			pBiom->fLeafDeathRate = (float)(pBiom->fLeafWeight *pPGen->vLeafLossR*vSenescLAI*vTempEffectLeaf*DeltaT);
		else //APPLE, BEECH
		{
			if((pPDev->iStagePlatho == 4)||(pPDev->iStagePlatho == 5))
			{
				pBiom->fLeafDeathRate  = (float)min(pPBiom->vLeafEnd3 * pDev->fDTT * DeltaT
				/(pGen->pStageParam->afThermalTime[5]-pGen->pStageParam->afThermalTime[3]),pBiom->fLeafWeight);
			
				if(pTi->pSimTime->iJulianDay>=300)
				{
					pBiom->fLeafDeathRate = (float)max(pBiom->fLeafDeathRate,max(0.0,pBiom->fLeafWeight*DeltaT*(float)(pTi->pSimTime->iJulianDay-300)/35.0));
				}
			}
			else
				pBiom->fLeafDeathRate = (pPDev->iStagePlatho <= 1)?(float)0.0
                    :(float)(pBiom->fLeafWeight *pPGen->vLeafLossR*vSenescLAI*vTempEffectLeaf*DeltaT);
		}

		if(pPDev->iStagePlatho == 5)
		{
			pBiom->fFruitDeathRate  = (float)min(pPBiom->vFruitEnd4 * pDev->fDevR * DeltaT / 
				(pGen->pStageParam->afThermalTime[5]-pGen->pStageParam->afThermalTime[4]),
				pBiom->fFruitWeight);
		}
	} // end TREES

    //C- und N-Litter kommt in die oberste Bodenschicht:
    vLeafCLitter = (pPBiom->vLeafDamageO3 + pPBiom->vLeafDamagePath + pPlant->pBiomass->fLeafDeathRate*(1.0-pPGen->vAssRlcLeaf)) 
                                        * pMaterial->pLeaf->vFracC;
    vLeafNLitter = pPlant->pBiomass->fLeafDeathRate*pPlant->pPltNitrogen->fLeafMinConc
                                        + (pPBiom->vLeafDamageO3 + pPBiom->vLeafDamagePath)*pPlant->pPltNitrogen->fLeafActConc;
    vStemCLitter = pPlant->pBiomass->fStemDeathRate*(1.0-pPGen->vAssRlcStem) * pMaterial->pStem->vFracC; //kg
    vStemNLitter = pPlant->pBiomass->fStemDeathRate*pPlant->pPltNitrogen->fStemMinConc; //kg
    vBranchCLitter = pPlant->pBiomass->fBranchDeathRate*(1.0-pPGen->vAssRlcBranch) * pMaterial->pBranch->vFracC;  //kg
    vBranchNLitter = pPlant->pBiomass->fBranchDeathRate*pPlant->pPltNitrogen->fBranchMinConc;  //kg

    pCh->pCLayer->pNext->fCLitter += (float)((vLeafCLitter+vStemCLitter+vBranchCLitter)*fPlantDens*1e4); //kg/ha
	pCh->pCLayer->pNext->fNLitter += (float)((vLeafNLitter+vStemNLitter+vBranchNLitter)*fPlantDens*1e4); //kg(N)/ha
 
    pPPart->vCLitterCum +=  (vLeafCLitter+vStemCLitter+vBranchCLitter)*fPlantDens*1e4;
    pPPart->vNLitterCum +=  (vLeafNLitter+vStemNLitter+vBranchNLitter)*fPlantDens*1e4;



    //Verlust an Blattflaeche
    if(pBiom->fLeafWeight>(float)0.0)
    {
        LAreaLossR = (pPBiom->vLeafDamageO3 + pPBiom->vLeafDamagePath + pBiom->fLeafDeathRate)/pBiom->fLeafWeight;
        pLLayer    = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;

        //Zeiger auf die oberste Blattschicht setzen:
        pLLayer    = pPlathoPlant[iPlant]->pPlathoMorphology->pLeafLayer;
        for(N = 1;N<=LEAFLAYERS-1;N++)
        {
            pLLayer = pLLayer->pNext;
        }

   		pLLayer->vLeafWeightCum      = (float)0.0;
            
        //von oben nach unten zählen:
        for(N=LEAFLAYERS-1;N>=0;N--)
//	    for (N=1;N<=LEAFLAYERS;N++)
	    {
    	    pLLayer->vLeafAreaLay    *= (1.0-LAreaLossR);
            if(pLLayer->vLeafAreaLay<0.0) pLLayer->vLeafAreaLay = 0.0;

                        
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

			pLLayer->vLAICum = pLLayer->vLeafAreaCum/max(pPMorph->vCrownArea,1.0/pPlathoScenario->vTotalPlantDensity);


            if(pPlathoModules->iFlagSectors==2)
            {
                for(iSector=0;iSector<4;iSector++)
                {
                    pLLSec =  pLLayer->apLeafSector[iSector];

			        pLLSec->vLeafWeightSec *= (1.0-LAreaLossR);
                    if(pLLSec->vLeafWeightSec<0.0) pLLSec->vLeafWeightSec = 0.0;

                    pLLSec->vLeafAreaSec = pLLSec->vLeafWeightSec / pLLSec->vSLWeightSec;

                    if(pLLayer->pNext!=NULL)
                    {
                        pLLSec->vLeafAreaCumSec   = pLLayer->pNext->apLeafSector[iSector]->vLeafAreaCumSec + pLLSec->vLeafAreaSec;
                        pLLSec->vLeafWeightCumSec = pLLayer->pNext->apLeafSector[iSector]->vLeafWeightCumSec + pLLSec->vLeafWeightSec;
                    }
                    else
                    {
                        pLLSec->vLeafAreaCumSec   = pLLSec->vLeafAreaSec;
                        pLLSec->vLeafWeightCumSec = pLLSec->vLeafWeightSec;
                    }

			        pLLSec->vLAICumSec = 4.0*pLLSec->vLeafAreaCumSec/max(pPMorph->vCrownArea,1.0/pPlathoScenario->vTotalPlantDensity);
   
                    pLLayer->apLeafSector[iSector]->vAbvgDistrFac = pLLSec->vLeafWeightSec/pLLayer->vLeafWeightLay;

                }
            }

            //if(pLLayer->pNext != NULL) pLLayer = pLLayer->pNext;
            if(pLLayer->pBack != NULL) pLLayer = pLLayer->pBack;

        }
    }
    else
    {
	    for (N=1;N<=LEAFLAYERS;N++)
	    {
    	    pLLayer->vLeafAreaLay    = 0.0;

            if(pPlathoModules->iFlagSectors==2)
            {
                for(iSector=0;iSector<4;iSector++)
                {
			        pLLayer->apLeafSector[iSector]->vLeafWeightSec   = 0.0;
                    pLLayer->apLeafSector[iSector]->vLeafAreaSec = 0.0;
                }
            }

            if(pLLayer->pNext != NULL) pLLayer = pLLayer->pNext;
        }
    }
 
	//Verlust an Wurzellänge:
    if(pBiom->fRootWeight>(float)0.0)
    {
        // Verlust durch Wurzelpathogene (nur Feinwurzeln)

        //bei Pathogenen kein Effekt der Ressourcenverfügbarkeit auf Verlust an Wurzellänge
	    pLR		= pPlant->pRoot->pLayerRoot;
        pPRL    = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
        pCL     = pCh->pCLayer->pNext;
	    for (L=1;L<=L1;L++)
	    {
            FRLengthLossLay = max(0.0,pLR->fLengthDens*pPBiom->vRootDamage/(pPlant->pBiomass->fRootWeight+pPBiom->vRootDamage));
            pLR->fLengthDens -= (float)FRLengthLossLay;  //[m]

            FRBiomLossLay = FRLengthLossLay/pGen->fRootLengthRatio; //[kg]

            //Litter auf LEACHN-Pools schreiben:
            vFRCLitter = FRBiomLossLay * pMaterial->pFineRoot->vFracC * (1.0-pPGen->vAssRlcFineRoot);
            vFRNLitter = FRBiomLossLay * pPltN->fRootMinConc;
            pCL->fCLitter +=  (float)(vFRCLitter*fPlantDens*1e4); //kg/ha
            pCL->fNLitter +=  (float)(vFRNLitter*fPlantDens*1e4); //kg/ha

            pPPart->vCLitterCum +=  vFRCLitter*fPlantDens*1e4;
            pPPart->vNLitterCum +=  vFRNLitter*fPlantDens*1e4;


            if(pPlathoModules->iFlagSectors==2)
            {
                for(iSector=0;iSector<4;iSector++)
                {
                    pPRL->apRootSector[iSector]->vFRSecBiomass *= (1.0-FRBiomLossLay/pPRL->vFRLayBiomass);
                    if(pPRL->apRootSector[iSector]->vFRSecBiomass<0.0) 
                        pPRL->apRootSector[iSector]->vFRSecBiomass = 0.0;
                }
            }

		    pCL = pCL->pNext;
            if(pLR->pNext != NULL)  pLR  = pLR->pNext;
            if(pPRL->pNext != NULL) pPRL = pPRL->pNext;
	    }

        //Verlust durch Turnover:

        // In ungünstigen Schichten sterben mehr Wurzeln ab
        TFRLLR = 0.0; //total fine root length loss
        TGRLLR = 0.0; //total gross root length loss
	    vFRootLengthLoss = pBiom->fRootDeathRate*pGen->fRootLengthRatio;
	    vGRootLengthLoss = pBiom->fGrossRootDeathRate*pPGen->vSpecGrossRootLength;
            
        pLR		= pPlant->pRoot->pLayerRoot;
        pPRL    = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
	    pCL     = pCh->pCLayer->pNext;
        for (L=1;L<=L1;L++)
	    {
            TFRLLR += pLR->fLengthDens*max(1.0-pLR->fWatStressRoot,1.0-pLR->fNStressRoot);
            TGRLLR += pPRL->vGRLengthDens*max(1.0-pLR->fWatStressRoot,1.0-pLR->fNStressRoot);

		    if(pLR->pNext != NULL) pLR  = pLR->pNext;
            if(pPRL->pNext != NULL) pPRL = pPRL->pNext;
        }

        if(TFRLLR > 0.0)
        {
            pLR		= pPlant->pRoot->pLayerRoot;
            pPRL    = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
	        pCL     = pCh->pCLayer->pNext;
	        for (L=1;L<=L1;L++)
	        {
                FRLengthLossLay = vFRootLengthLoss*pLR->fLengthDens*max(1.0-pLR->fWatStressRoot,1.0-pLR->fNStressRoot)/TFRLLR;
                pLR->fLengthDens -= (float)FRLengthLossLay;  //[m]

                FRBiomLossLay = FRLengthLossLay/pGen->fRootLengthRatio; //[kg]

                //Litter auf LEACHN-Pools schreiben:
                vFRCLitter = FRBiomLossLay * pMaterial->pFineRoot->vFracC * (1.0-pPGen->vAssRlcFineRoot);
                vFRNLitter = FRBiomLossLay * pPltN->fRootMinConc;
                pCL->fCLitter +=  (float)(vFRCLitter*fPlantDens*1e4); //kg/ha
                pCL->fNLitter +=  (float)(vFRNLitter*fPlantDens*1e4); //kg/ha
  
                pPPart->vCLitterCum +=  vFRCLitter*fPlantDens*1e4;
                pPPart->vNLitterCum +=  vFRNLitter*fPlantDens*1e4;

    
    if(pPlathoModules->iFlagSectors==2)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
                        if(pPRL->vFRLayBiomass>(float)0.0)
                            pPRL->apRootSector[iSector]->vFRSecBiomass *= (1.0-FRBiomLossLay/pPRL->vFRLayBiomass);
                        else
                            pPRL->apRootSector[iSector]->vFRSecBiomass = 0.0;

                        if(pPRL->apRootSector[iSector]->vFRSecBiomass<0.0) pPRL->apRootSector[iSector]->vFRSecBiomass = 0.0;
                    }
                }

		        pCL = pCL->pNext;
                if(pLR->pNext != NULL)  pLR  = pLR->pNext;
                if(pPRL->pNext != NULL) pPRL = pPRL->pNext;
            }
        }

        if(TGRLLR > 0.0)
        {
            pLR		= pPlant->pRoot->pLayerRoot;
            pPRL    = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
    	    pCL     = pCh->pCLayer->pNext;
	        for (L=1;L<=L1;L++)
	        {
                GRLengthLossLay = vGRootLengthLoss*pPRL->vGRLengthDens*max(1.0-pLR->fWatStressRoot,1.0-pLR->fNStressRoot)/TGRLLR;
                pPRL->vGRLengthDens -= GRLengthLossLay;

                GRBiomLossLay = GRLengthLossLay/pPGen->vSpecGrossRootLength; //[kg]

                //Litter auf LEACHN-Pools schreiben:
                vGRCLitter = GRBiomLossLay * pMaterial->pGrossRoot->vFracC * (1.0-pPGen->vAssRlcGrossRoot);
                vGRNLitter = GRBiomLossLay * pPltN->fGrossRootMinConc;
                pCL->fCLitter +=  (float)(vGRCLitter*fPlantDens*1e4); //kg/ha
                pCL->fNLitter +=  (float)(vGRNLitter*fPlantDens*1e4); //kg/ha

                pPPart->vCLitterCum +=  vGRCLitter*fPlantDens*1e4;
                pPPart->vNLitterCum +=  vGRNLitter*fPlantDens*1e4;


                if(pPlathoModules->iFlagSectors==2)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
                        if(pPRL->vGRLayBiomass>0.0)
                            pPRL->apRootSector[iSector]->vGRSecBiomass *= (1.0-GRBiomLossLay/pPRL->vGRLayBiomass);
                        else
                            pPRL->apRootSector[iSector]->vGRSecBiomass = 0.0;

                        if(pPRL->apRootSector[iSector]->vGRSecBiomass<0.0) 
                            pPRL->apRootSector[iSector]->vGRSecBiomass = 0.0;
                    }
                }

                pCL = pCL->pNext;
		        if(pLR->pNext != NULL)  pLR  = pLR->pNext;
                if(pPRL->pNext != NULL) pPRL = pPRL->pNext;
            }
        }
    } //end if fRootWeight > = 0.0
    else
    {
            pLR		= pPlant->pRoot->pLayerRoot;
            pPRL    = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
	        for (L=1;L<=L1;L++)
	        {
                pLR->fLengthDens    = (float)0.0;
                pPRL->vGRLengthDens = 0.0;
                               
                if(pPlathoModules->iFlagSectors==2)
                {
                    for(iSector=0;iSector<4;iSector++)
                    {
                        pPRL->apRootSector[iSector]->vFRSecBiomass = 0.0;
                        pPRL->apRootSector[iSector]->vGRSecBiomass = 0.0;
                    }
                }
            }
            	
		        if(pLR->pNext != NULL)  pLR  = pLR->pNext;
                if(pPRL->pNext != NULL) pPRL = pPRL->pNext;
    }

 /*
//alt: (kein Effekt der Ressourcenverfügbarkeit auf Verlust an Wurzellänge)
	pLR		= pPlant->pRoot->pLayerRoot;
	for (L=1;L<=L1;L++)
	{
		pLR->fLengthDens  -= (float)max(0.0,pLR->fLengthDens
							*pBiom->fRootDeathRate/(pBiom->fRootWeight+EPSILON));

		pLR  = pLR->pNext;
	}

   */
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Retranslokation aus absterbenden Organen:
	////////////////////////////////////////////////////////////////////////////////////////////////

	vCH2OFRootRelocR   = pBiom->fRootDeathRate		* pPGen->vAssRlcFineRoot;
	vCH2OGRootRelocR   = pBiom->fGrossRootDeathRate * pPGen->vAssRlcGrossRoot;
	vCH2OStemRelocR    = pBiom->fStemDeathRate		* pPGen->vAssRlcStem;
	vCH2OBranchRelocR  = pBiom->fBranchDeathRate	* pPGen->vAssRlcBranch;
	vCH2OLeafRelocR    = pBiom->fLeafDeathRate		* pPGen->vAssRlcLeaf;
	vCH2OFruitRelocR   = 0.0;
	vCH2OTuberRelocR   = 0.0;

    if(pPlathoModules->iFlagNEffectGrw==1)//no N effect on growth
    {
	    vNFRootRelocR   = 0.0;
	    vNGRootRelocR   = 0.0;
		vNStemRelocR    = 0.0;
	    vNBranchRelocR  = 0.0;
	    vNLeafRelocR    = 0.0;
	    vNFruitRelocR   = 0.0;
	    vNTuberRelocR   = 0.0;
    }
    else     //N effect on growth included
    {
	    vNFRootRelocR   = pBiom->fRootDeathRate * max(0.0,(pPltN->fRootActConc - pPltN->fRootMinConc));
	    vNGRootRelocR   = 0.0;
	    HERBS
		    vNStemRelocR    = pBiom->fStemDeathRate * max(0.0,(pPltN->fStemActConc - pPltN->fStemMinConc));
	    TREES
		    vNStemRelocR    = 0.0;
	    vNBranchRelocR  = 0.0;
	    vNLeafRelocR    = pBiom->fLeafDeathRate * max(0.0,(pPltN->fLeafActConc - pPltN->fLeafMinConc));
	    vNFruitRelocR   = 0.0;
	    vNTuberRelocR   = 0.0;
    }

	
	//gesamte Retranslokation:
	pPBiom->vAssRelocR = vCH2OFRootRelocR + vCH2OGRootRelocR + vCH2OStemRelocR + vCH2OLeafRelocR +
							vCH2OBranchRelocR + vCH2OFruitRelocR + vCH2OTuberRelocR;
	
//	pPBiom->vNRelocR = vNFRootRelocR*(float)0.5+ vNGRootRelocR*(float)0.1 + vNStemRelocR*(float)0.0 + 
//        vNLeafRelocR*(float)0.5 + vNBranchRelocR*(float)0.0 + vNFruitRelocR*(float)0.0 + vNTuberRelocR*(float)0.1;

	pPBiom->vNRelocR = vNFRootRelocR + vNGRootRelocR + vNStemRelocR + 
        vNLeafRelocR + vNBranchRelocR + vNFruitRelocR + vNTuberRelocR;


return 1;
}



