//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"


int		Canopy_PLATHO	(EXP_POINTER2,int iPlant);
double	LeafAreaCum(PPLANT pPlant, double vLeafArea, double vPlantHeight, double h, int);
double  LeafAreaDens(double vPlantHeight, double h, double vPeak);
int		PlantHeightGrowth(PPLANT pPlant, int iPlant);
double	LeafArea(PPLANT pPlant, int iPlant);
int		PlantArea(PPLANT pPlant, int iPlant);
int		StemDiameter(PPLANT pPlant, int iPlant);
int		CrownAndRootZoneDiameter(PPLANT pPlant, int iPlant);
int		CrownAndRootVolume(PPLANT pPlant, int iLayers, int iPlant);
int		GetGrowthCapacities(PPLANT pPlant, int iPlant);
int		LightCompetitionFactor(PPLANT pPlant, int iPlant);


extern	int	GetPlantNeighbours(PPLANT, int, int*, int*, int*, int*);
extern double CalcKappa(double dSpecLeafWeightMax, double vLAI, int iPlant);
extern double AVGENERATOR(double vInput, VRESPONSE* pvResp);

int Canopy_PLATHO(EXP_POINTER2,int iPlant)
{
	PDEVELOP        pDev = pPlant->pDevelop;
	PGENOTYPE	    pGen = pPlant->pGenotype;
	PBIOMASS	    pBiom= pPlant->pBiomass;
	PCANOPY		    pCan = pPlant->pCanopy;
    PPLTNITROGEN    pPltN = pPlant->pPltNitrogen;

	PPLATHOGENOTYPE		pPGen   = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHODEVELOP		pPDev   = pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
    PLEAFLAYER          pLLayer; //pLLyr;

    PPLATHOMORPHOLOGY   pPMorphRight, pPMorphLower, pPMorphLeft, pPMorphUpper;

	int     N, iSector; //    j
   	int	    iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;
    double   vPlantHeight,vRightPlantHeight,vLowerPlantHeight,vLeftPlantHeight,vUpperPlantHeight;
    double   vHCrownOnset,vRightHCrownOnset,vLowerHCrownOnset,vLeftHCrownOnset,vUpperHCrownOnset;

    double  LAICumRight, LAICumLower, LAICumLeft, LAICumUpper;
    double   TLLDF;
    double   vNewLeafArea, vNewLeafWeight;
    double   avNewLeafAreaFac[LEAFLAYERS];
    double   avLightCompetitionFac[LEAFLAYERS];
    double  MinimalMeanSpecLfWeight;
	double  kappa, h, alphaN;
//    double LeafWeightCum[3];

    double vPlantDensity = pPlathoScenario->vTotalPlantDensity;

    extern double  CalcLAICumNbg(int iLeafLayer, double vPlantHeight, double vNbgHeight, double vHCrownOnset, 
                      double vNbgHCrownOnset, PLEAFLAYER pLLayerNbg);


	//alter Stamm-Durchmessr:
//	StemDiameter(pPlant, iPlant);
	
	//Höhenzuwachs (Stamm)
	PlantHeightGrowth(pPlant, iPlant);

	//alte Blattfläche
	LeafArea(pPlant, iPlant);

/*	TREES
	{
		CrownAndRootZoneDiameter(pPlant, iPlant);
	}
*/

	//Berechnung des Blattflächenindex [m^2(Blatt)/m^2(Boden)]
	if(pPMorph->vCrownArea > (float)0.0)
	{
		if((pCan->fPlantLA==(float)0.0)&&(pBiom->fLeafWeight!=(float)0.0))
			pPMorph->vPlantLAI = (float)0.1;
		else
		{
	/*		if(pPMorph->vCrownArea < (float)1.0/vPlantDensity)
				pPMorph->vPlantLAI = (float)max(0.0,pCan->fPlantLA*vPlantDensity);
//			if(pPMorph->vCrownArea < (float)(PI/(4.0*vPlantDensity)))
//				pPMorph->vPlantLAI = (float)max(0.0,pCan->fPlantLA*vPlantDensity*4.0/PI);
			else
				pPMorph->vPlantLAI = max((float)0.0,pCan->fPlantLA/pPMorph->vCrownArea);
      */           
            pPMorph->vPlantLAI = max((float)0.0,pCan->fPlantLA/pPMorph->vCrownArea);  
		}
	}
	else
		pPMorph->vPlantLAI = (float)0.0;

	HERBS
	{
		if(pPDev->iStagePlatho==1)
			pPMorph->vPlantLAI = (float)min(1.0,pPMorph->vPlantLAI);
		if(pPDev->iStagePlatho>=5)
			pPMorph->vPlantLAI = (float)0.0;
	}

	////////////////////////////////////////////////////////////////////
	//
	//	*****	LEAF LAYERS:	*****
	//
	// Blattflächendichte
	// spezifisches Blattgewicht
	// kumulativer LAI über h
	// kumulative Blattfläche über h
	// kumulatives Blattgewicht über h
	//
	////////////////////////////////////////////////////////////////////
    
    //Verteilung der neugebildeten Blattfläche abhängig von der Lichtverfügbarkeit:
    TLLDF = (float)0.0;

    if(pBiom->fLeafGrowR > (float)0.0)
    {
        if(pPlathoModules->iFlagSpecLeafArea==1)//constant specific leaf weight 
            pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight/pPlant->pGenotype->fSpecLfWeight;
        else if(pPlathoModules->iFlagSpecLeafArea==2)//dynamic specific leaf weight 
		{
			//start value for specific leaf weight:
			MinimalMeanSpecLfWeight = 0.5*pPlant->pGenotype->fSpecLfWeight
				* (pPltN->fLeafOptConc + pPltN->fLeafMinConc)/pPltN->fLeafOptConc;

			if(pPlant->pGenotype->fBeginShadeLAI>(float)0.0)
			{
				alphaN = (2.0*pPltN->fLeafOptConc - pPltN->fLeafActConc - pPltN->fLeafMinConc)
				/(4.0 * pPltN->fLeafOptConc * pPlant->pGenotype->fBeginShadeLAI);
    		
				pPMorph->vMeanSpecLfWeight = 
					max(pPlant->pGenotype->fSpecLfWeight*(1-alphaN*pPMorph->vPlantLAI),
						MinimalMeanSpecLfWeight);
			}
			else
				pPMorph->vMeanSpecLfWeight = MinimalMeanSpecLfWeight;

			pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight 
													/ (float)pPMorph->vMeanSpecLfWeight;
            			    
            // Bestimmung von kappa:
			kappa = CalcKappa((double)pPlant->pGenotype->fSpecLfWeight, pPMorph->vPlantLAI, iPlant);
        }
        else //default
            pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight/pPlant->pGenotype->fSpecLfWeight;
  

        GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
												    &iPlantUpper, &iPlantLower);

        vPlantHeight       = (double)pPlant->pCanopy->fPlantHeight;
        vRightPlantHeight  = (double)pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight->pCanopy->fPlantHeight;
	    vLowerPlantHeight  = (double)pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower->pCanopy->fPlantHeight;
	    vLeftPlantHeight   = (double)pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft->pCanopy->fPlantHeight;
	    vUpperPlantHeight  = (double)pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper->pCanopy->fPlantHeight;

        pPMorphRight =  pPlathoPlant[iPlantRight]->pPlathoMorphology;
        pPMorphLower =  pPlathoPlant[iPlantLower]->pPlathoMorphology;
        pPMorphLeft  =  pPlathoPlant[iPlantLeft]->pPlathoMorphology;
        pPMorphUpper =  pPlathoPlant[iPlantUpper]->pPlathoMorphology;

        vHCrownOnset        = pPMorph->vHeightOfCrownOnset;
        vRightHCrownOnset   = pPMorphRight->vHeightOfCrownOnset;
        vLowerHCrownOnset   = pPMorphLower->vHeightOfCrownOnset;
        vLeftHCrownOnset    = pPMorphLeft->vHeightOfCrownOnset;
        vUpperHCrownOnset   = pPMorphUpper->vHeightOfCrownOnset;

        pLLayer = pPMorph->pLeafLayer;
        for(N=0;N<LEAFLAYERS;N++)
		{
	        //Berechnung des kumulativen LAI über der Blattschicht gleicher Höhe des jeweiligen Nachbarn	
            LAICumRight = CalcLAICumNbg(N, vPlantHeight, vRightPlantHeight, vHCrownOnset, vRightHCrownOnset, pPMorphRight->pLeafLayer);
            LAICumLower = CalcLAICumNbg(N, vPlantHeight, vLowerPlantHeight, vHCrownOnset, vLowerHCrownOnset, pPMorphLower->pLeafLayer);
            LAICumLeft  = CalcLAICumNbg(N, vPlantHeight, vLeftPlantHeight,  vHCrownOnset, vLeftHCrownOnset, pPMorphLeft->pLeafLayer);
            LAICumUpper = CalcLAICumNbg(N, vPlantHeight, vUpperPlantHeight, vHCrownOnset, vUpperHCrownOnset, pPMorphUpper->pLeafLayer);

            avLightCompetitionFac[N] = (float)(pLLayer->vLAICum
                + pPMorph->AbvgCoeffComp[0]*pPMorph->vCrownArea/pPMorphRight->vCrownArea*LAICumRight
                + pPMorph->AbvgCoeffComp[1]*pPMorph->vCrownArea/pPMorphLower->vCrownArea*LAICumLower
                + pPMorph->AbvgCoeffComp[2]*pPMorph->vCrownArea/pPMorphLeft->vCrownArea*LAICumLeft
                + pPMorph->AbvgCoeffComp[3]*pPMorph->vCrownArea/pPMorphUpper->vCrownArea*LAICumUpper);

            pLLayer = pLLayer->pNext;
        }

        for(N=LEAFLAYERS-1;N>=0;N--)
		{
			//	aktuelle Höhe der N-ten Blattschicht:
			h = ((double)N+0.5)/(double)LEAFLAYERS*(double)pCan->fPlantHeight;

            avNewLeafAreaFac[N] = LeafAreaDens((double)pCan->fPlantHeight,h,pPGen->vRelHeightMaxLeaf);

            if(avLightCompetitionFac[N]>(float)1.0)
                avNewLeafAreaFac[N] /= (float)(1.0+pPMorph->vPlasticity*(avLightCompetitionFac[N]-1.0));

            TLLDF += avNewLeafAreaFac[N];
        }
    }
	 
	if (pBiom->fLeafWeight+pBiom->fLeafGrowR == (float)0.0)
	{	
		pPlant->pCanopy->fPlantLA = (float)0.0;
				
        pLLayer = pPMorph->pLeafLayer;       
        for(N = 1;N<=LEAFLAYERS;N++)
        {
            pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight;
			pLLayer->vLeafAreaLay    = (float)0.0;
			pLLayer->vLeafWeightLay      = (float)0.0;
            pLLayer->vLeafAreaCum        = (float)0.0;
            pLLayer->vLeafWeightCum      = (float)0.0;
			pLLayer->vLAICum             = (float)0.0;

            if(pPlathoModules->iFlagSectors==2)
		    {
                for(iSector=0;iSector<4;iSector++)
                {
			        pLLayer->apLeafSector[iSector]->vLeafAreaSec = (float)0.0;
			        pLLayer->apLeafSector[iSector]->vLeafWeightSec = (float)0.0;
			        pLLayer->apLeafSector[iSector]->vSLWeightSec = pPlant->pGenotype->fSpecLfWeight;
			        pLLayer->apLeafSector[iSector]->vLeafWeightCumSec = (float)0.0;
			        pLLayer->apLeafSector[iSector]->vLeafAreaCumSec = (float)0.0;
			        pLLayer->apLeafSector[iSector]->vLAICumSec = (float)0.0;
                }
            }

            pLLayer = pLLayer->pNext;
        }

        
	} //end if fLeafWeight + fLeafGrowR == 0


    if (pBiom->fLeafGrowR>(float)0.0)
    {
	    //Zeiger auf die oberste Blattschicht setzen:
        pLLayer = pPMorph->pLeafLayer;       
        for(N = 1;N<=LEAFLAYERS-1;N++)
        {
            pLLayer = pLLayer->pNext;
        }

   		pLLayer->vLeafWeightCum      = (float)0.0;
            
        //von oben nach unten zählen:
        for(N=LEAFLAYERS-1;N>=0;N--)
		{
    	    //specific leaf weight in layer N
            if(pPlathoModules->iFlagSpecLeafArea==1)//constant specific leaf weight 
		    {
 			    pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight;
            }
		    else if(pPlathoModules->iFlagSpecLeafArea==2)//dynamic specific leaf weight 
		    {
			    // SpecLfW(LAIc) = SpecLfWMax * exp(-kappa*LAIc)
                pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight*(float)exp(-kappa*pLLayer->vLeafAreaCum/max(pPMorph->vCrownArea,1.0/vPlantDensity));
            }
            else //default
		    {
 			    pLLayer->vSpecLeafWeight     = pPlant->pGenotype->fSpecLfWeight;
            }

            vNewLeafArea =  (float)(avNewLeafAreaFac[N]/TLLDF * pBiom->fLeafGrowR/pLLayer->vSpecLeafWeight);

            pLLayer->vLeafAreaLay    += vNewLeafArea;
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

//			pLLayer->vLAICum = pLLayer->vLeafAreaCum/max(pPMorph->vCrownArea,1.0/vPlantDensity);
			pLLayer->vLAICum = pLLayer->vLeafAreaCum/pPMorph->vCrownArea;
            
 		    if(pPlathoModules->iFlagSectors==2)
		    {
                PLEAFSECTOR pLLSec;
                double AbvgDistrFac[4];
                double MeanCompCoeff, Alpha;

                vNewLeafWeight = (float)(vNewLeafArea*pLLayer->vSpecLeafWeight); 

                MeanCompCoeff = (pLLayer->apLeafSector[0]->vSecCompCoeff
                                    +pLLayer->apLeafSector[1]->vSecCompCoeff
                                    +pLLayer->apLeafSector[2]->vSecCompCoeff
                                    +pLLayer->apLeafSector[3]->vSecCompCoeff)/4.0;

                Alpha = MeanCompCoeff < 0.75 ? 0.25/(1.0-MeanCompCoeff) : 0.75/MeanCompCoeff;

                for(iSector=0;iSector<4;iSector++)
                {
                    pLLSec =  pLLayer->apLeafSector[iSector];
                    AbvgDistrFac[iSector] = pPMorph->vPlasticity * Alpha * (MeanCompCoeff - pPMorph->SectorCompCoeff_abvg[iSector]) + 0.25;
			        pLLSec->vLeafWeightSec += vNewLeafWeight * AbvgDistrFac[iSector];

			        pLLSec->vSLWeightSec = pLLayer->vSpecLeafWeight;
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

			        pLLSec->vLAICumSec = (float)4.0*pLLSec->vLeafAreaCumSec/max(pPMorph->vCrownArea,1.0/vPlantDensity);

                    pLLayer->apLeafSector[iSector]->vAbvgDistrFac = pLLSec->vLeafWeightSec/pLLayer->vLeafWeightLay;
                }
            } //end iFlagSectors == 2
                
            pLLayer = pLLayer->pBack;
	    } //end LEAFLAYERS

	} //end fLeafGrowR > 0

	return 1;
}

double 	LeafArea(PPLANT pPlant, int iPlant)
{
	PPLTNITROGEN		pPltN	= pPlant->pPltNitrogen;
	PPLATHOMORPHOLOGY	pPMorph = pPlathoPlant[iPlant]->pPlathoMorphology;
	double alphaN,  MinimalMeanSpecLfWeight, vPlantLA;

	if(pPMorph->vPlantLAI==(float)0.0)
		return (float)0.0;

	if(pPlathoModules->iFlagSpecLeafArea==1)//constant specific leaf weight 
	{
		pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight/pPlant->pGenotype->fSpecLfWeight;
	}
	else if(pPlathoModules->iFlagSpecLeafArea==2)//dynamic specific leaf weight 
	{
		MinimalMeanSpecLfWeight = 0.5*pPlant->pGenotype->fSpecLfWeight
		* (pPltN->fLeafOptConc + pPltN->fLeafMinConc)/pPltN->fLeafOptConc;

		if(pPlant->pGenotype->fBeginShadeLAI>(float)0.0)
		{
            if(pPlathoModules->iFlagNEffectGrw==1)//no N effect on growth
            {
                alphaN = (pPltN->fLeafOptConc - pPltN->fLeafMinConc)
			                /(4.0 * pPltN->fLeafOptConc * pPlant->pGenotype->fBeginShadeLAI);
            }
            else // N effect on growth included
            {
			    alphaN = (2.0*pPltN->fLeafOptConc - pPltN->fLeafActConc - pPltN->fLeafMinConc)
			                /(4.0 * pPltN->fLeafOptConc * pPlant->pGenotype->fBeginShadeLAI);
            }
		
			pPMorph->vMeanSpecLfWeight = 
				max(pPlant->pGenotype->fSpecLfWeight*(1-alphaN*pPMorph->vPlantLAI),
					MinimalMeanSpecLfWeight);
		}
		else
			pPMorph->vMeanSpecLfWeight = MinimalMeanSpecLfWeight;
		
		pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight/(float)pPMorph->vMeanSpecLfWeight;
	}
	else //default
		pPlant->pCanopy->fPlantLA = pPlant->pBiomass->fLeafWeight/pPlant->pGenotype->fSpecLfWeight;

	vPlantLA = pPlant->pCanopy->fPlantLA;
	return vPlantLA;
}


double LeafAreaCum(PPLANT pPlant, double vLeafArea, double vPlantHeight, double h, int iPlant)
{
	//PPLATHOMORPHOLOGY pPMorph = pPlathoPlant[iPlant]->pPlathoMorphology;

	double vLeafAreaCum,vHeightPeak,vDistributionForm;
	double aParam,yParam,vHelp1,vHelp2;

	vHeightPeak = vPlantHeight * pPlathoPlant[iPlant]->pPlathoGenotype->vRelHeightMaxLeaf;

	vDistributionForm = 0.25*vPlantHeight;


	if(h >= vPlantHeight)
		vLeafAreaCum = (float)0.0;
	else if (h == (float)0.0)
		vLeafAreaCum = vLeafArea;
	else
	{
		vHelp1 = (float)exp(-(vPlantHeight-vHeightPeak)/vDistributionForm);
		vHelp2 = (float)exp(vHeightPeak/vDistributionForm);

		aParam = ((float)1.0+vHelp1)*((float)1.0+vHelp2)/(vHelp1-vHelp2);
		yParam = -aParam/((float)1.0+vHelp1);

		vLeafAreaCum   = vLeafArea * yParam + aParam/(1.0+exp(-(h-vHeightPeak)/vDistributionForm));
	}
		
	return vLeafAreaCum;
}

double LeafAreaDens(double vPlantHeight, double h, double vPeak)
{

	double vLeafAreaDens, vHeightPeak, vDistributionForm;
	double beta, vHelp1, vHelp2;

	vHeightPeak = vPlantHeight * vPeak;

	vDistributionForm = 0.25*vPlantHeight;

	if(h >= vPlantHeight)
		vLeafAreaDens = 0.0;
	else
	{
		vHelp1 = exp(-(vPlantHeight-vHeightPeak)/vDistributionForm);
		vHelp2 = exp(vHeightPeak/vDistributionForm);

		beta = (1.0+vHelp1)*(1.0+vHelp2)/(vHelp1-vHelp2);

		vLeafAreaDens   = -4.0 * beta * exp(-(h-vHeightPeak)/vDistributionForm)/pow((1.0+exp(-(h-vHeightPeak)/vDistributionForm)),2);
	}
		
	return vLeafAreaDens;
}

int		GetGrowthCapacities(PPLANT pPlant, int iPlant)
{
    PBIOMASS    		pBiom	= pPlant->pBiomass;
	PPLTNITROGEN		pPltN	= pPlant->pPltNitrogen;
	PPLATHOGENOTYPE		pPGen	= pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOBIOMASS		pPBiom	= pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
    PPLANTSTRESS        pPPltStress = pPlathoPlant[iPlant]->pPlantStress;

	double vDend;
//	float fMaxLAI;
	double alphaN, MinimalMeanSpecLfWeight;

    double vPlantDensity = pPlathoScenario->vTotalPlantDensity;

	// maximal LAI from minimal light fraction required for netto carbon assimilation	
	//fMaxLAI = (float)(-log(pPGen->vMinLightFraction)/pPGen->vKDiffuse);

	TREES
	{
//		StemDiameter(pPlant,iPlant);
//		PlantArea(pPlant,iPlant);
	}

	HERBS
	{
		// maximal plant area:
        POTATO
		    pPMorph->vCrownArea = 1.5/vPlantDensity;
        else
		    pPMorph->vCrownArea = 2.0/vPlantDensity;

		vDend = pPlant->pGenotype->fPlantHeight/pPMorph->vHtoDmin; //m
		pPMorph->vCrownArea = min(pPMorph->vCrownArea,(PI/4.0 * 
					pow(pPMorph->vCrownStemRatio*vDend,2))); //m^2
	}



	//potential maximal leaf weight
	if(pPlathoModules->iFlagSpecLeafArea==1)//constant specific leaf weight 
	{
		//pPBiom->vKLeaf = pPGen->vLAImax*pPMorph->vCrownArea*pPlant->pGenotype->fSpecLfWeight;

		//pPBiom->vKLeaf = pPGen->vLAImax/vPlantDensity*pPlant->pGenotype->fSpecLfWeight;

/*        if(pPMorph->vCrownArea*vPlantDensity <= (float)1.0)
            pPBiom->vKLeaf = pPGen->vLAImax*pPMorph->vCrownArea*pPlant->pGenotype->fSpecLfWeight;
        else
		    pPBiom->vKLeaf = pPGen->vLAImax/vPlantDensity*pPlant->pGenotype->fSpecLfWeight;
*/

		pPBiom->vKLeaf = pPGen->vLAImax*pPMorph->vCrownArea/((float)1.0+pPPltStress->vCompCoeff)
                        *pPlant->pGenotype->fSpecLfWeight;

	}
	else if(pPlathoModules->iFlagSpecLeafArea==2)//dynamic specific leaf weight 
	{
		//mean specific leaf weight:

		MinimalMeanSpecLfWeight = 0.5*pPlant->pGenotype->fSpecLfWeight
				* (pPltN->fLeafOptConc + pPltN->fLeafMinConc)/pPltN->fLeafOptConc;

		if(pPlant->pGenotype->fBeginShadeLAI>(float)0.0)
		{
            if(pPlathoModules->iFlagNEffectGrw==1)//no N effect on growth
            {
                alphaN = (pPltN->fLeafOptConc - pPltN->fLeafMinConc)
			                /(4.0 * pPltN->fLeafOptConc * pPlant->pGenotype->fBeginShadeLAI);
            }
            else // N effect on growth included
            {
			    alphaN = (2.0*pPltN->fLeafOptConc - pPltN->fLeafActConc - pPltN->fLeafMinConc)
			    /(4.0 * pPltN->fLeafOptConc * pPlant->pGenotype->fBeginShadeLAI);
            }
		
			pPMorph->vMeanSpecLfWeight = 
				max(pPlant->pGenotype->fSpecLfWeight*(1-alphaN*pPGen->vLAImax),
					MinimalMeanSpecLfWeight);
		}
		else
		{
			pPMorph->vMeanSpecLfWeight = MinimalMeanSpecLfWeight;
		}

		//pPBiom->vKLeaf = pPGen->vLAImax/vPlantDensity*(float)pPMorph->vMeanSpecLfWeight;

        //pPBiom->vKLeaf = pPGen->vLAImax*pPMorph->vCrownArea*(float)pPMorph->vMeanSpecLfWeight;
                
        if(pPMorph->vCrownArea*vPlantDensity <= (float)1.0)
            pPBiom->vKLeaf = pPGen->vLAImax*pPMorph->vCrownArea*(float)pPMorph->vMeanSpecLfWeight;
        else
		    pPBiom->vKLeaf = pPGen->vLAImax/vPlantDensity*(float)pPMorph->vMeanSpecLfWeight;


	}
	else //default
    {
        //pPBiom->vKLeaf = pPGen->vLAImax/vPlantDensity*pPlant->pGenotype->fSpecLfWeight;

        if(pPMorph->vCrownArea*vPlantDensity <= (float)1.0)
            pPBiom->vKLeaf = pPGen->vLAImax*pPMorph->vCrownArea*pPlant->pGenotype->fSpecLfWeight;
        else
		    pPBiom->vKLeaf = pPGen->vLAImax/vPlantDensity*pPlant->pGenotype->fSpecLfWeight;
    }


	TREES
	{
        //28.11.2007
        pPBiom->vMaxLeafWeight = (float)pPGen->vLa * (float)pow(pBiom->fStemWeight+pBiom->fBranchWeight,pPGen->vLb);
//        pPBiom->vMaxLeafWeight = (float)pPGen->vLa * (float)pow(pBiom->fWoodWeight,pPGen->vLb);
//        pPBiom->vMaxFRootWeight = (float)pPGen->vFRa * (float)pow(pPBiom->vMaxLeafWeight,pPGen->vFRb);

		pPBiom->vKLeaf=(float)min(pPBiom->vKLeaf,pPBiom->vMaxLeafWeight);
//		pPBiom->vKLeaf=(float)min(pPBiom->vKLeaf,pPMorph->vMaxLeafToWoodRatio*pPlant->pBiomass->fWoodWeight);
	}

	HERBS
        pPBiom->vKFRoot = pPBiom->vKLeaf*pPMorph->vFineRootToLeafRatio;
    TREES
		pPBiom->vKFRoot = pPGen->vFRa * pow(pPBiom->vKLeaf,pPGen->vFRb);


	HERBS
	{
		vDend = pPlant->pGenotype->fPlantHeight/pPMorph->vHtoDmin; //m
		pPBiom->vKStem = min(PI/4.0*pPlant->pGenotype->fPlantHeight*pow(vDend,2)
			*pPMorph->vStemDensity,pPBiom->vKLeaf/pPMorph->vLeafToStemRatio); //kg
	}


	TREES
	{
		pPBiom->vKFruit = pPBiom->vKLeaf*pPMorph->vFruitToLeafRatio;
	}

	return 1;
}


int	PlantHeightGrowth(PPLANT pPlant, int iPlant)
{
	PCANOPY				pCan = pPlant->pCanopy;
	PDEVELOP			pDev = pPlant->pDevelop;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLANTSTRESS		pPPltStress	= pPlathoPlant[iPlant]->pPlantStress;


	double	vHDakt,  PhiHD, vLightHDFactor;
//	float	fLightCompStressFactor;
//	int		iCase;

	//stem growth rate
	float	g = pPlant->pBiomass->fStemGrowR;

	//critical light competition stress
	double	vCompCoeffCrit = (float)0.15;

	if(pPMorph->vStemDiameter==(float)0.0)
	{
		pCan->fPlantHeight = (float)pow(4.0*g*pow(pPMorph->vHtoDmin,2)/
						(PI*pPMorph->vStemDensity),1.0/3.0);

		return 1;
	}
	else
		vHDakt = pCan->fPlantHeight/pPMorph->vStemDiameter;

	HERBS
	{	 	
			if (!pPMorph->vHDmax[0].vInput==(float)0.0)
				pPMorph->vHtoDmax = AVGENERATOR(100*pCan->fPlantHeight,pPMorph->vHDmax);
	}




	/*

	// Berechnung der Zunahme von Stammhöhe und -durchmesser in Abhängigkeit 
	// von H/D und Konkurrenz
	// Fallunterscheidung wie bei Bossel (1996), Ecol. Modelling 90, S. 211


	if((pPPltStress->vCompCoeff <= (float)0.0)&&(fHDakt == pPMorph->vHtoDmin))
		iCase = 3;

	if((pPPltStress->vCompCoeff <= (float)0.0)&&(fHDakt > pPMorph->vHtoDmin))
		iCase = 1;

	if((pPPltStress->vCompCoeff > (float)0.0)&&(fHDakt < pPMorph->vHtoDmax))
		iCase = 2;

	if((pPPltStress->vCompCoeff> (float)0.0)&&(fHDakt == pPMorph->vHtoDmax))
		iCase = 3;


	if(fHDakt > pPMorph->vHtoDmax)
		iCase = 1;
									// H/D ausserhalb des erlaubten Bereiches!
	if(fHDakt < pPMorph->vHtoDmin)
		iCase = 2;
		


	switch(iCase)
	{
	case 1: // nur Dickenwachstum (dH/dt = 0)
		pPMorph->vStemHeightR = (float)0.0;
		break;

	case 2: // nur Längenwachstum (dD/dt = 0)
		pPMorph->vStemHeightR = (float)(4.0*g/
						(PI*pPMorph->vStemDensity*pow(pPMorph->vStemDiameter,2)));		
		break;

	case 3: // (Height/Diameter) = const.
		pPMorph->vStemHeightR = (float)(4.0*g/
						(3.0*PI*pPMorph->vStemDensity*pow(pPMorph->vStemDiameter,2)));
		break;
	}
	*/


	//critical light competition stress
	WHEAT		vCompCoeffCrit = 0.15;
	BARLEY		vCompCoeffCrit = 0.15;
	POTATO		vCompCoeffCrit = 0.15;
	SUNFLOWER	vCompCoeffCrit = 0.15;
	MAIZE		vCompCoeffCrit = 0.15;
	ALFALFA		vCompCoeffCrit = 0.15;
	LOLIUM		vCompCoeffCrit = 0.15;
	BEECH		vCompCoeffCrit = 0.15;//0.6;//0.2;
	SPRUCE		vCompCoeffCrit = 0.6;//0.6;//0.025;
	APPLE		vCompCoeffCrit = 0.15;


/*
	if((pPPltStress->vCompCoeff <= (float)0.0))
	{
		if(fHDakt > pPMorph->vHtoDmin)
			iCase = 3; //iCase = 1
		else
			iCase = 3;
	}

	if((pPPltStress->vCompCoeff > (float)0.0)&&(fHDakt <= pPMorph->vHtoDmax))
	{
		if(fHDakt < pPMorph->vHtoDmax)
			iCase = 4;
		else
			iCase = 3;
	}


	if(fHDakt > pPMorph->vHtoDmax)
		iCase = 1;
									// H/D ausserhalb des erlaubten Bereiches!
	if(fHDakt < pPMorph->vHtoDmin)
		iCase = 2;
		

	switch(iCase)
	{
	case 1: // nur Dickenwachstum (dH/dt = 0)
		pPMorph->vStemHeightR = (float)0.0;
		break;

	case 2: // nur Längenwachstum (dD/dt = 0)
		pPMorph->vStemHeightR = (float)(4.0*g/
						(PI*pPMorph->vStemDensity*pow(pPMorph->vStemDiameter,2)));		
		break;

	case 3: // (Height/Diameter) = const.
		pPMorph->vStemHeightR = (float)(4.0*g/
						(3.0*PI*pPMorph->vStemDensity*pow(pPMorph->vStemDiameter,2)));
		break;

	case 4: // (Height/Diameter) = var.

		if(vCompCoeffCrit>(float)0.0)
			fLightCompStressFactor = (float)(1+0+2.0*min(pPPltStress->vCompCoeff,vCompCoeffCrit)/
				(vCompCoeffCrit));
		else
			fLightCompStressFactor=(float)3.0;

			pPMorph->vStemHeightR = (float)(4.0*fLightCompStressFactor/(3.0*PI) * g /
							(pPMorph->vStemDensity*pow(pPMorph->vStemDiameter,2)));
		
		break;
	}
	*/

	PhiHD = (vHDakt - pPMorph->vHtoDmin)/(pPMorph->vHtoDmax - pPMorph->vHtoDmin); 
	if(PhiHD>1.0) PhiHD=1.0;
	if(PhiHD<0.0) PhiHD=0.0;

	vLightHDFactor = (1.0-PhiHD)/3.0 + (2.0-PhiHD)/3.0 * min(1.0,pPPltStress->vCompCoeff/vCompCoeffCrit);

	pPMorph->vStemHeightR = 4.0*g /(PI*pPMorph->vStemDensity*pow(pPMorph->vStemDiameter,2))*vLightHDFactor;
							


	pCan->fPlantHeight		+= (float)pPMorph->vStemHeightR;
	pCan->fPlantHeight		= (float)min(pCan->fPlantHeight,pPlant->pGenotype->fPlantHeight);

	pPMorph->vHeightToDiameter = pCan->fPlantHeight/pPMorph->vStemDiameter;

    pPMorph->vHeightOfCrownOnset = 0.1*pCan->fPlantHeight;


	return 1;
}


int PlantArea(PPLANT pPlant, int iPlant)
{
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHODEVELOP	    pPDev	= pPlathoPlant[iPlant]->pPlathoDevelop;

    double vPlantDensity = pPlathoScenario->vTotalPlantDensity;

	//von der Pflanze bedeckte Bodenfläche [m^2]
	if(pPMorph->vStemDiameter==(float)0.0)
		pPMorph->vCrownArea = pPlant->pCanopy->fPlantLA;
	else
		pPMorph->vCrownArea =(float)(PI/4.0 * 
				pow((double)pPMorph->vCrownStemRatio*(double)pPMorph->vStemDiameter,2.0));

	//Begrenzung des maximalen Kronendurchmessers
    TREES
    {
       if(pPMorph->vCrownArea > (float)PI/vPlantDensity)    //B5
		    pPMorph->vCrownArea = (float)PI/vPlantDensity;
    //    if(pPMorph->vCrownArea > (float)0.25*pPlant->pDevelop->fAge/vPlantDensity)      //Lysimeter
	//	    pPMorph->vCrownArea = (float)0.25*pPlant->pDevelop->fAge/vPlantDensity;
     //   if(pPMorph->vCrownArea > (float)(0.03*pow(pPlant->pDevelop->fAge,2)+0.5)/vPlantDensity)      //Lysimeter
	//	    pPMorph->vCrownArea = (float)(0.03*pow(pPlant->pDevelop->fAge,2)+0.5)/vPlantDensity;
    //    if(pPlant->pDevelop->fAge < (float)7.)
    //    {
     //       if(pPMorph->vCrownArea > (float)(0.03*pow(pPlant->pDevelop->fAge,2)+0.5)/vPlantDensity)      //Lysimeter
	//	    pPMorph->vCrownArea = (float)(0.03*pow(pPlant->pDevelop->fAge,2)+0.5)/vPlantDensity;
     //   }
    //    if(pPMorph->vCrownArea > (float)(2.0*PI)/vPlantDensity)      //Lysimeter
	//	    pPMorph->vCrownArea = (float)(2.0*PI)/vPlantDensity;
    }

    HERBS
    {
        POTATO
        {
            if(pPMorph->vCrownArea > (float)1.5/vPlantDensity)
		    pPMorph->vCrownArea = (float)1.5/vPlantDensity;
        }
        else
        {
            if(pPMorph->vCrownArea > (float)2.0/vPlantDensity)
		    pPMorph->vCrownArea = (float)2.0/vPlantDensity;
        }
    }

    if((pPDev->iStagePlatho>0)&&(pPMorph->vCrownArea < (float)0.001))
	    pPMorph->vCrownArea = (float)0.001;

    pPMorph->vCrownDiameter = (float)(2.0*sqrt(pPMorph->vCrownArea/PI));

    HERBS
        pPMorph->vRootZoneDiameter = pPMorph->vCrownDiameter;
    TREES
        pPMorph->vRootZoneDiameter = pPMorph->vCrownDiameter/pPMorph->vCrownRootRatio;

    pPMorph->vRootZoneArea = (float)(pow(pPMorph->vRootZoneDiameter/2.0,2)*PI);

	return 1;
}

int		StemDiameter(PPLANT pPlant, int iPlant)
{
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;

    pPMorph->vStemDiameterOld	= pPMorph->vStemDiameter;

	if(pPlant->pCanopy->fPlantHeight>(float)0.0)
		pPMorph->vStemDiameter = (float)max(pPMorph->vStemDiameter,sqrt(4.0*pPlant->pBiomass->fStemWeight
		/(PI*pPlant->pCanopy->fPlantHeight*pPMorph->vStemDensity)));
	else
		pPMorph->vStemDiameter = (float)0.0;

	return 1;
}

int	CrownAndRootZoneDiameter(PPLANT pPlant, int iPlant)
{
	double vElongationStress;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOGENOTYPE 	pPGen	= pPlathoPlant[iPlant]->pPlathoGenotype;


	TREES
	{
        if(pPlathoModules->iFlagElasticity==1) //constant ratio of plant diameter vs. stem diameter
        {
            		pPMorph->vCrownDiameter += pPMorph->vCrownDiameter*(pPMorph->vStemDiameter-pPMorph->vStemDiameterOld)/pPMorph->vStemDiameterOld;
        }
        else  //variable ratio of plant diameter vs. stem diameter
        {
		    vElongationStress =									// = 1, wenn Pflanze mit HDmin wachsen konnte,
			    (pPMorph->vHtoDmax-pPMorph->vHeightToDiameter)/ // = 0, Pflanze HDmax erreicht hat
			    (pPMorph->vHtoDmax-pPMorph->vHtoDmin);			// (r_C/S wird kleiner, wenn sich die Pflanze strecken muss)

		    if(vElongationStress<(float)0.0) vElongationStress=(float)0.0;
		    if(vElongationStress>(float)1.0) vElongationStress=(float)1.0;

            // nur strecken:
            //		pPMorph->vCrownDiameter += pPMorph->vCrownDiameter*(pPMorph->vStemDiameter-pPMorph->vStemDiameterOld)/pPMorph->vStemDiameterOld*
            //			(float)1.0/(pPMorph->vElastCSR+((float)1.0-pPMorph->vElastCSR)*vElongationStress);

            // strecken und stauchen (linear):
            //		pPMorph->vCrownDiameter += pPMorph->vCrownDiameter*(pPMorph->vStemDiameter-pPMorph->vStemDiameterOld)/pPMorph->vStemDiameterOld*
            //			(float)(2.0*vElongationStress*(1.0-1.0/pPMorph->vElastCSR)+1.0/pPMorph->vElastCSR);

            //		pPMorph->vCrownDiameter += pPMorph->vCrownDiameter*(pPMorph->vStemDiameter-pPMorph->vStemDiameterOld)/pPMorph->vStemDiameterOld*
            //			(float)(2.0*vElongationStress*(1.0-1.0/(pPMorph->vPlasticity+1.0))+1.0/(pPMorph->vPlasticity+1.0));

            // strecken und stauchen (nicht linear):
		    pPMorph->vCrownDiameter += pPMorph->vCrownDiameter*(pPMorph->vStemDiameter-pPMorph->vStemDiameterOld)/pPMorph->vStemDiameterOld*
			    (float)((1.0+vElongationStress*(1.0-1.0/(pPMorph->vPlasticity+1.0)))/((pPMorph->vPlasticity+1.0)-pPMorph->vPlasticity*vElongationStress));

            // hier muss noch ein Modell für die Wurzelzone eingefügt werden
            pPMorph->vRootZoneDiameter = pPMorph->vCrownDiameter/pPMorph->vCrownRootRatio;
    		////////////////////////////////////////////////////////////////////////////////

		    pPMorph->vCrownStemRatio = pPMorph->vCrownDiameter/pPMorph->vStemDiameter;
        } // end iFlagElasticity = 2 
	
	    //pPMorph->vStemDiameterOld	= pPMorph->vStemDiameter;

	}

	return 1;
}

int CrownAndRootVolume(PPLANT pPlant, int iLayers, int iPlant)
{
    PPLATHOMORPHOLOGY pPMorph = pPlathoPlant[iPlant]->pPlathoMorphology;
    PPLATHOROOTLAYER pPRL;
    PLAYERROOT pLR;

    int L;
    double dDepletionZone = 0.02; //[m]

                             //   _   _   //
    BEECH                    //  |  |   | //
    {                        //  |  |   | //
        pPMorph->dCrownVolume = pow(pPMorph->vCrownDiameter,2)/4*PI*(double)pPlant->pCanopy->fPlantHeight;
        //pPMorph->dRootVolume = pow(pPMorph->vRootZoneDiameter,2)/4*PI*(double)pPlant->pRoot->fDepth;
		
        pLR		=pPlant->pRoot->pLayerRoot;
		pPRL = pPMorph->pPlathoRootLayer;
        pPMorph->dRootVolume = 0.0;
        for (L=1;L<=iLayers-2;L++)
		{
			//Check the whether there are roots in this layer:
			if (pLR->fLengthDens==(float)0.0)		break;

            pPRL->vVolumeOfLayer = pLR->fLengthDens * pow(dDepletionZone,2) * PI;  //[m^3]
            pPMorph->dRootVolume += pPRL->vVolumeOfLayer;
		
            pLR =pLR ->pNext;
		    pPRL=pPRL->pNext;
		}
    }
    SPRUCE
    {                        //   /\  /\  //
        double dTop = 0.6;   //  |  |   | //
        pPMorph->dCrownVolume = pow(pPMorph->vCrownDiameter,2)/4*PI*(double)pPlant->pCanopy->fPlantHeight
            * (dTop +(1.0-dTop)/3.0);
        //pPMorph->dRootVolume = pow(pPMorph->vRootZoneDiameter,2)/4*PI*(double)pPlant->pRoot->fDepth;
		
        pLR		=pPlant->pRoot->pLayerRoot;
		pPRL = pPMorph->pPlathoRootLayer;
        pPMorph->dRootVolume = 0.0;
        for (L=1;L<=iLayers-2;L++)
		{
			//Check the whether there are roots in this layer:
			if (pLR->fLengthDens==(float)0.0)		break;

            pPRL->vVolumeOfLayer = pLR->fLengthDens * pow(dDepletionZone,2) * PI;  //[m^3]
            pPMorph->dRootVolume += pPRL->vVolumeOfLayer;
		
            pLR =pLR ->pNext;
		    pPRL=pPRL->pNext;
		}
    }                        //   _   _   //
    else                     //  |  |   | //
    {                        //  |  |   | //
        pPMorph->dCrownVolume = pow(pPMorph->vCrownDiameter,2)/4*PI*(double)pPlant->pCanopy->fPlantHeight;
        //pPMorph->dRootVolume = pow(pPMorph->vRootZoneDiameter,2)/4*PI*(double)pPlant->pRoot->fDepth;
		
        pLR		=pPlant->pRoot->pLayerRoot;
		pPRL = pPMorph->pPlathoRootLayer;
        pPMorph->dRootVolume = 0.0;
        for (L=1;L<=iLayers-2;L++)
		{
			//Check the whether there are roots in this layer:
			if (pLR->fLengthDens==(float)0.0)		break;

            pPRL->vVolumeOfLayer = pLR->fLengthDens * pow(dDepletionZone,2) * PI;  //[m^3]
            pPMorph->dRootVolume += pPRL->vVolumeOfLayer;
		
            pLR =pLR ->pNext;
		    pPRL=pPRL->pNext;
		}
    }

    //m^3 --> cm^3
    pPMorph->dCrownVolume *= 1e6;   // cm^3
    pPMorph->dRootVolume  *= 1e6;   // cm^3


    return 1;
}


int	LightCompetitionFactor(PPLANT pPlant, int iPlant)
{
	PPLANTSTRESS		pPPltStress	= pPlathoPlant[iPlant]->pPlantStress;
	PPLATHOMORPHOLOGY	pPMorph		= pPlathoPlant[iPlant]->pPlathoMorphology;

	PPLATHOMORPHOLOGY	pPMorphRight, pPMorphLower, pPMorphLeft, pPMorphUpper;

	int	iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;
    float fPlantHeight,vRightPlantHeight,vLowerPlantHeight,vLeftPlantHeight,vUpperPlantHeight;
	
	///////////////////////////////////////////////
	// Bestimmung der Nachbarn der i-ten Pflanze. 

	GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
												&iPlantUpper, &iPlantLower);

    fPlantHeight       = pPlant->pCanopy->fPlantHeight;
    vRightPlantHeight  = pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight->pCanopy->fPlantHeight;
	vLowerPlantHeight  = pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower->pCanopy->fPlantHeight;
	vLeftPlantHeight   = pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft->pCanopy->fPlantHeight;
	vUpperPlantHeight  = pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper->pCanopy->fPlantHeight;

    pPMorphRight = pPlathoPlant[iPlantRight]->pPlathoMorphology;
	pPMorphLower = pPlathoPlant[iPlantLower]->pPlathoMorphology;
	pPMorphLeft  = pPlathoPlant[iPlantLeft]->pPlathoMorphology;
	pPMorphUpper = pPlathoPlant[iPlantUpper]->pPlathoMorphology;

 /*   if(fPlantHeight>(float)0.0)
    {
        pPPltStress->vCompCoeff = 
            (float)(pPMorph->AbvgCoeffComp[3]*pPMorphUpper->fPlantLAI*min(1.0,vUpperPlantHeight/fPlantHeight)
		        + pPMorph->AbvgCoeffComp[1]*pPMorphLower->fPlantLAI*min(1.0,vLowerPlantHeight/fPlantHeight)
		        + pPMorph->AbvgCoeffComp[0]*pPMorphRight->fPlantLAI*min(1.0,vRightPlantHeight/fPlantHeight)
		        + pPMorph->AbvgCoeffComp[2]*pPMorphLeft->fPlantLAI *min(1.0,vLeftPlantHeight/fPlantHeight));

        pPPltStress->vCompCoeff /= (pPMorphUpper->fPlantLAI
								+pPMorphLower->fPlantLAI
								+pPMorphRight->fPlantLAI
								+pPMorphLeft->fPlantLAI
								+EPSILON);
       }
    else
        pPPltStress->vCompCoeff = (float)1.0;
        */

    if(fPlantHeight>(float)0.0)
    {
        pPPltStress->vCompCoeff = 
            (float)(pPMorph->AbvgCoeffComp[3]*min(1.0,vUpperPlantHeight/fPlantHeight)
		        + pPMorph->AbvgCoeffComp[1]*min(1.0,vLowerPlantHeight/fPlantHeight)
		        + pPMorph->AbvgCoeffComp[0]*min(1.0,vRightPlantHeight/fPlantHeight)
		        + pPMorph->AbvgCoeffComp[2]*min(1.0,vLeftPlantHeight/fPlantHeight));
    }
    else
        pPPltStress->vCompCoeff = (float)4.0;

	return 1;
}
