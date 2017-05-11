//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include  "xinclexp.h"
#include "Platho4.h"


int  NitrogenUptake_PLATHO(EXP_POINTER2,int iPlant);
int  PlantNitrogenDemand_PLATHO(EXP_POINTER2,int iPlant);
int  PlantNitrogenTranslocation_PLATHO(EXP_POINTER2, int iPlant);
int	 LeafNitrogenDistribution(double vLfWeightCrit1, double vLfWeightCrit2, PPLANT pPlant, PLEAFLAYER);

int	CalculatePotentialNitrogenUptake(EXP_POINTER2, int iPlant);
int	GetNitrogenConcentrationLimits(PPLANT, int);
double	CalcPotNitrogenTransloc(PPLANT pPlant, int iPlant, double DeltaT);

extern int GetPlantNeighbours(PPLANT, int, int*, int*, int*, int*);
extern double LeafAreaCum(PPLANT pPlant, double vLeafArea, double vPlantHeight, double h, int iPlant);
extern double SectorCompFactor(double r_c, double r_nbg);
extern double AVGENERATOR(double vInput, VRESPONSE* pvResp);


int PlantNitrogenDemand_PLATHO(EXP_POINTER2, int iPlant)
	{
	PBIOMASS		pBiom = pPlant->pBiomass;
    PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;
	PGENOTYPE		pGen  = pPlant->pGenotype;

	PPLATHOGENOTYPE		pPGen   = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHONITROGEN		pPPltN  = pPlathoPlant[iPlant]->pPlathoNitrogen;
	PPLANTMATERIAL		pPPltM	= pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;

	pPltN->fRootDemand		= (float)0.0;
	pPltN->fGrossRootDemand = (float)0.0;
	pPltN->fLeafDemand		= (float)0.0;
	pPltN->fBranchDemand	= (float)0.0;
	pPltN->fStemDemand		= (float)0.0;
	pPltN->fGrainDemand		= (float)0.0;
	pPltN->fTuberDemand		= (float)0.0;

	pPltN->fShootDemand		= (float)0.0;
	pPltN->fRootDemand		= (float)0.0;

	//concentration Limits

	GetNitrogenConcentrationLimits(pPlant, iPlant);
    
/*	pPltN->fLeafDemand =  (float)(max(0.0,pBiom->fLeafWeight*(pPltN->fLeafOptConc-pPltN->fLeafActConc))
    					+pBiom->fLeafGrowR*(pPPltM->pLeaf->vProteins + pPPltM->pLeaf->vOrganicAcids)/100*0.15);
    pPltN->fStemDemand =  (float)(max(0.0,pBiom->fStemWeight*(pPltN->fStemOptConc-pPltN->fStemActConc))
    					+pBiom->fStemGrowR*(pPPltM->pStem->vProteins + pPPltM->pStem->vOrganicAcids)/100*0.15);
    pPltN->fRootDemand =  (float)(max(0.0,pBiom->fRootWeight*(pPltN->fRootOptConc-pPltN->fRootActConc))
    					+pBiom->fRootGrowR*(pPPltM->pFineRoot->vProteins + pPPltM->pFineRoot->vOrganicAcids)/100*0.15);

	pPltN->fBranchDemand = (float)(max(0.0,pBiom->fBranchWeight*(pPltN->fBranchOptConc-pPltN->fBranchActConc))
						+pBiom->fBranchGrowR*(pPPltM->pBranch->vProteins + pPPltM->pBranch->vOrganicAcids)/100*0.15);
	pPltN->fGrossRootDemand = (float)(max(0.0,pBiom->fGrossRootWeight*(pPltN->fGrossRootOptConc-pPltN->fGrossRootActConc))
						+pBiom->fGrossRootGrowR*(pPPltM->pGrossRoot->vProteins + pPPltM->pGrossRoot->vOrganicAcids)/100*0.15);
	
	pPltN->fShootDemand= pPltN->fLeafDemand+pPltN->fStemDemand+pPltN->fBranchDemand;


	POTATO
		pPltN->fTuberDemand = (float)(max(0.0, pBiom->fTuberWeight*(pPltN->fTuberOptConc-pPltN->fTuberActConc))
							 +pBiom->fTuberGrowR*(pPPltM->pTuber->vProteins + pPPltM->pTuber->vOrganicAcids)/100*0.15);
	else
		pPltN->fGrainDemand = (float)(max(0.0, pBiom->fFruitWeight*(pPltN->fFruitOptConc-pPltN->fFruitActConc))
								 +pBiom->fFruitGrowR*(pPPltM->pFruit->vProteins + pPPltM->pFruit->vOrganicAcids)/100*0.15);
	
    pPltN->fTotalDemand= pPltN->fShootDemand+pPltN->fRootDemand+pPltN->fGrossRootDemand 
		+ pPltN->fGrainDemand + pPltN->fTuberDemand;  


	pPPltN->vNDemandForGrowth =	  
		(float) (pBiom->fRootGrowR * (pPPltM->pFineRoot->vProteins + pPPltM->pFineRoot->vOrganicAcids)/100*0.15
		+ pBiom->fGrossRootGrowR * (pPPltM->pGrossRoot->vProteins + pPPltM->pGrossRoot->vOrganicAcids)/100*0.15
		+ pBiom->fTuberGrowR	* (pPPltM->pTuber->vProteins + pPPltM->pTuber->vOrganicAcids)/100*0.15
		+ pBiom->fStemGrowR		* (pPPltM->pStem->vProteins + pPPltM->pStem->vOrganicAcids)/100*0.15 
		+ pBiom->fBranchGrowR	* (pPPltM->pBranch->vProteins + pPPltM->pBranch->vOrganicAcids)/100*0.15 
		+ pBiom->fLeafGrowR		* (pPPltM->pLeaf->vProteins + pPPltM->pLeaf->vOrganicAcids)/100*0.15 
		+ pBiom->fFruitGrowR	* (pPPltM->pFruit->vProteins + pPPltM->pFruit->vOrganicAcids)/100*0.15); 


*/    
	pPltN->fLeafDemand =  (float)max(0.0,pBiom->fLeafWeight*(pPltN->fLeafOptConc-pPltN->fLeafActConc)
    					+pBiom->fLeafGrowR*pPltN->fLeafOptConc);
    pPltN->fStemDemand =  (float)max(0.0,pBiom->fStemWeight*(pPltN->fStemOptConc-pPltN->fStemActConc)
    					+pBiom->fStemGrowR*pPltN->fStemOptConc);
    pPltN->fRootDemand =  (float)max(0.0,pBiom->fRootWeight*(pPltN->fRootOptConc-pPltN->fRootActConc)
    					+pBiom->fRootGrowR*pPltN->fRootOptConc);

	pPltN->fBranchDemand = (float)max(0.0,pBiom->fBranchWeight*(pPltN->fBranchOptConc-pPltN->fBranchActConc)
						+pBiom->fBranchGrowR*pPltN->fBranchOptConc);
	pPltN->fGrossRootDemand = (float)max(0.0,pBiom->fGrossRootWeight*(pPltN->fGrossRootOptConc-pPltN->fGrossRootActConc)
						+pBiom->fGrossRootGrowR*pPltN->fGrossRootOptConc);
	
	pPltN->fShootDemand= pPltN->fLeafDemand+pPltN->fStemDemand+pPltN->fBranchDemand;


	POTATO
		pPltN->fTuberDemand = pBiom->fTuberWeight*max((float)0.0, pPltN->fTuberOptConc-pPltN->fTuberActConc)
							 +pBiom->fTuberGrowR*pPltN->fTuberOptConc;
	else
		pPltN->fGrainDemand = pBiom->fFruitWeight*max((float)0.0, pPltN->fFruitOptConc-pPltN->fFruitActConc)
								 +pBiom->fFruitGrowR*pPltN->fFruitOptConc;
	
    pPPltN->dTotalDemand= pPltN->fShootDemand+pPltN->fRootDemand+pPltN->fGrossRootDemand 
		+ pPltN->fGrainDemand + pPltN->fTuberDemand;  


	pPPltN->vNDemandForGrowth =	  pBiom->fRootGrowR		* pPltN->fRootOptConc * NFAC_L
					+ pBiom->fGrossRootGrowR * pPltN->fGrossRootOptConc * NFAC_S
					+ pBiom->fTuberGrowR	* pPltN->fTuberOptConc * NFAC_L
					+ pBiom->fStemGrowR		* pPltN->fStemOptConc * NFAC_S 
					+ pBiom->fBranchGrowR	* pPltN->fBranchOptConc * NFAC_S 
					+ pBiom->fLeafGrowR		* pPltN->fLeafOptConc * NFAC_L 
					+ pBiom->fFruitGrowR	* pPltN->fFruitOptConc * NFAC_L; 

   	//pPPltN->vNDemandForGrowth *= NFAC;

	return 1;
	}


int NitrogenUptake_PLATHO(EXP_POINTER2, int iPlant)
	{
    int		L,L1;
	//float 	fTotRtLenDens;  
	double	vNUF;
    double 	vUNO3,vUNH4;


	PSLAYER 		pSL		=pSo->pSLayer;
	PSWATER			pSWL	=pSo->pSWater;
	PWLAYER	   		pSLW	=pWa->pWLayer; 
	PCLAYER     	pSLN	=pCh->pCLayer;
	PLAYERROOT		pLR		=pPlant->pRoot->pLayerRoot;
	PROOT			pRT		=pPlant->pRoot;  
    PPLTWATER		pPltW	=pPlant->pPltWater;
	PBIOMASS			pBiom=pPlant->pBiomass;
	PPLTNITROGEN		pPltN=pPlant->pPltNitrogen;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHONITROGEN		pPPltN  = pPlathoPlant[iPlant]->pPlathoNitrogen;

    double DeltaT = (double)pTi->pTimeStep->fAct;
    
	//======================================================================================
	//Layer Nitrogen Initiation and Transformation
	//======================================================================================

	for (L=1;L<=pSo->iLayers-2;L++)
		{
		    pPPltN->vRNO3U[L]=0.0;
		    pPPltN->vRNH4U[L]=0.0;

		    //fTotRtLenDens += pLR->fLengthDens;

		    pLR->fActLayNO3NUpt=0.0;
		    pLR->fActLayNH4NUpt=0.0;


		    pLR=pLR->pNext;
		}		
	
		//Check the last layer of roots
		pLR		=pPlant->pRoot->pLayerRoot;
		for (L=1;L<=pSo->iLayers-2;L++)
		{
			//Check the whether there are roots in this layer:
			if (pLR->fLengthDens==(float)0.0)		break;
			//The last layer of root:
			L1=L;
		}


	//Potential Nitrogen Uptake
    if(pPMorph->vPlantLAI > 0.0)
        CalculatePotentialNitrogenUptake(exp_p2,iPlant);
    else
        pPPltN->dPotNUpt = (float)0.0;

	if (pPPltN->dPotNUpt==0.0)
	{
		pPltN->fActNUptR = (float)0.0;
		pPltN->fActNUpt  = (float)0.0;
		return 0;	
	}
				

	// potential nitrogen translocation [kg(N)]
	pPPltN->dTotalTransNw = CalcPotNitrogenTransloc(pPlant, iPlant, DeltaT);


	pPPltN->dNPool = pPPltN->dTotalTransNw + pPPltN->dPotNUpt;

	
	//Nitrogen uptake factor relating supply and plant  demand: fNUF (dimensionless)
	if(pPPltN->dNPool<=pPPltN->dTotalDemand)
		vNUF = 1.0;
	else
	{
		if(pPPltN->dPotNUpt<=pPPltN->dTotalDemand)
			vNUF = 1.0;
		else
			vNUF = pPPltN->dTotalDemand/pPPltN->dPotNUpt;
	}


	//======================================================================================
	//Actual Nitrogen Uptake 
	//======================================================================================
	pPltN->fActNUpt=(float)0.0; 	//the actual total root nitrogen uptake (kg N/ha)

	pPltN->fActNO3NUpt =(float)0.0;
	pPltN->fActNH4NUpt =(float)0.0;
    pPltN->fActNUptR    =(float)0.0;

	pSL		=pSo->pSLayer->pNext;
	pSWL	=pSo->pSWater->pNext;
	pSLW	=pWa->pWLayer->pNext; 
	pSLN	=pCh->pCLayer->pNext;
	pLR		=pPlant->pRoot->pLayerRoot;

	for (L=1;L<=L1;L++)
	{
		//Possible plant uptake from a layer:fUNO3,fUNH4 [Kg (N)]
        vUNO3=pPPltN->vRNO3U[L]*vNUF;
		vUNH4=pPPltN->vRNH4U[L]*vNUF;

		pPltN->fActNO3NUpt += (float)vUNO3;
		pPltN->fActNH4NUpt += (float)vUNH4;

		//Nitrogen uptake from layer L (kg N/ha)
		pLR->fActLayNO3NUpt= (float)vUNO3;
		pLR->fActLayNH4NUpt= (float)vUNH4;

		//Total actual root nitrogen uptake(kg N)
		pPltN->fActNUpt += (float)max(0.0,vUNO3+vUNH4);

		pSL =pSL ->pNext;
		pSWL=pSWL->pNext;
		pSLW=pSLW->pNext;
		pSLN=pSLN->pNext;
		pLR =pLR ->pNext;
	}

	pPltN->fActNUptR = (float)(max(0.0,pPltN->fActNUpt)/DeltaT);


	return 1;
	}



int PlantNitrogenTranslocation_PLATHO(EXP_POINTER2, int iPlant)
	{   
	PBIOMASS			pBiom=pPlant->pBiomass;
	PDEVELOP			pDev =pPlant->pDevelop;
	PCANOPY				pCan =pPlant->pCanopy;
	PPLTNITROGEN	 	pPltN=pPlant->pPltNitrogen;  
	PPLATHONITROGEN		pPPltN=pPlathoPlant[iPlant]->pPlathoNitrogen;  
	PPLANTMATERIAL		pPPltM	= pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;
	PWEATHER			pWth =pCl->pWeather;
	PMSOWINFO			pSI  =pMa->pSowInfo;
	
	double	vTransNconst=pPlathoPlant[iPlant]->pPlathoGenotype->vNTransRate; 
	double	PhiTrans, vNSeedReservesOld, vNDeficit;
	double	vNUptakeFactor;
	double  vNUF1, vNUF2;
	double	vNGainTot;
	float	fN;
	double	DeltaT = (double)pTi->pTimeStep->fAct;
	
	//===========================================================================================
	//Amount of translocatable nitrogen
	//===========================================================================================
	
	// potential nitrogen translocation [kg(N)]
	pPPltN->dTotalTransNw = CalcPotNitrogenTransloc(pPlant,iPlant, DeltaT);

	pPPltN->dNPool = pPPltN->dTotalTransNw + pPltN->fActNUpt;


	if(pPPltN->dTotalDemand >= pPPltN->dNPool) //N-Bedarf übersteigt aktuelle N-Aufnahme + pflanzenintern verfügbares N
	{
		if(pPPltN->vNSeedReserves>0.0)
		{//Verlagerung aus N-Vorrat im Saatgut
			HERBS  
			{
				POTATO
				{
					vNSeedReservesOld = pPPltN->vNSeedReserves;

					pPPltN->vNSeedReserves -= min(DeltaT*2.0*vTransNconst
						*0.02*pBiom->fSeedReserv,vNSeedReservesOld);

					if(pPPltN->vNSeedReserves<0.0)
						pPPltN->vNSeedReserves = 0.0;
				}
				else //others
				{
					vNSeedReservesOld = pPPltN->vNSeedReserves;

					pPPltN->vNSeedReserves -= min(DeltaT*2.0*vTransNconst
						*0.02*pBiom->fSeedReserv,vNSeedReservesOld);

					if(pPPltN->vNSeedReserves<0.0)
						pPPltN->vNSeedReserves = 0.0;
				} // end others
			} // end HERBS
		} // end fNSeedReserves > 0

		pPltN->fRootNtransRate		= (float)(pPltN->fRootTransNw*vTransNconst*DeltaT);
		pPltN->fGrossRootNtransRate	= (float)(pPPltN->vGrossRootTransNw*vTransNconst*DeltaT);
		pPltN->fLeafNtransRate		= (float)(pPltN->fLeafTransNw*vTransNconst*DeltaT);
		pPltN->fStemNtransRate		= (float)(pPltN->fStemTransNw*vTransNconst*DeltaT);
		pPltN->fBranchNtransRate	= (float)(pPPltN->vBranchTransNw*vTransNconst*DeltaT);
		pPPltN->vFruitNtransRate	= pPPltN->vFruitTransNw*vTransNconst*DeltaT;
		pPltN->fTuberNtransRate		= (float)(pPPltN->vTuberTransNw*vTransNconst*DeltaT);
	}
	else //N-Bedarf geringer als aktuelle N-Aufnahme + pflanzenintern verfügbares N
	{
		if(pPltN->fActNUpt <= pPPltN->dTotalDemand) //N-Bedarf übersteigt aktuelle N-Aufnahme
		{
			vNDeficit = pPPltN->dTotalDemand-pPltN->fActNUpt; // Differenz aus Bedarf und aktueller Aufnahme
															 //muss pflanzenintern verlagert werden
			if(pPPltN->vNSeedReserves>0.0)
			{//Verlagerung aus N-Vorrat im Saatgut
				HERBS //Verlagerung aus N-Vorrat im Saatgut 
				{
					POTATO
					{
						vNSeedReservesOld = pPPltN->vNSeedReserves;

						pPPltN->vNSeedReserves -= min(vNDeficit,min(DeltaT*2.0*vTransNconst
							                            *0.02*pBiom->fSeedReserv,vNSeedReservesOld));
		
				
						if(pPPltN->vNSeedReserves<0.0)
							pPPltN->vNSeedReserves = 0.0;
						
						vNDeficit -= (vNSeedReservesOld-pPPltN->vNSeedReserves);
					}
					else // others
					{
						vNSeedReservesOld = pPPltN->vNSeedReserves;

						pPPltN->vNSeedReserves -= 
							min(vNDeficit,min(DeltaT*2.0*vTransNconst
							*0.02*pBiom->fSeedReserv,vNSeedReservesOld));
				
						if(pPPltN->vNSeedReserves<0.0)
							pPPltN->vNSeedReserves = 0.0;
						
						vNDeficit -= (vNSeedReservesOld-pPPltN->vNSeedReserves);
					} //end others
				} //end HERBS
			} // end fNSeedReserves > 0

			//Bruchteil des labilen N, der zur Deckung des verbliebenen Defizits verlagert werden muss
			PhiTrans = vNDeficit/(pPPltN->dTotalTransNw+1e-15);

			pPltN->fRootNtransRate		= (float)(PhiTrans*pPltN->fRootTransNw*vTransNconst*DeltaT);
			pPltN->fGrossRootNtransRate	= (float)(PhiTrans*pPPltN->vGrossRootTransNw*vTransNconst*DeltaT);
			pPltN->fLeafNtransRate		= (float)(PhiTrans*pPltN->fLeafTransNw*vTransNconst*DeltaT);
			pPltN->fStemNtransRate		= (float)(PhiTrans*pPltN->fStemTransNw*vTransNconst*DeltaT);
			pPltN->fBranchNtransRate	= (float)(PhiTrans*pPPltN->vBranchTransNw*vTransNconst*DeltaT);
			pPPltN->vFruitNtransRate	= PhiTrans*pPPltN->vFruitTransNw*vTransNconst*DeltaT;
			pPltN->fTuberNtransRate		= (float)(PhiTrans*pPPltN->vTuberTransNw*vTransNconst*DeltaT);
		}
		else //N-Bedarf geringer als aktuelle N-Aufnahme
			// --> keine pflanzeninterne Verlagerung
		{	
			pPltN->fRootNtransRate		= (float)0.0;
			pPltN->fGrossRootNtransRate	= (float)0.0;
			pPltN->fLeafNtransRate		= (float)0.0;
			pPltN->fStemNtransRate		= (float)0.0;
			pPltN->fBranchNtransRate	= (float)0.0;
			pPPltN->vFruitNtransRate	= 0.0;
			pPltN->fTuberNtransRate		= (float)0.0;
		}
	}


	if(pPPltN->dTotalDemand>=pPPltN->dNPool)
	{	// Bedarf ist größer als der verfügbare Stickstoff

		// Fallunterscheidung:
		if(pPltN->fActNUpt + pPPltN->dTotalTransNw > pPPltN->vNDemandForGrowth)
		{	// Mehr Stickstoff verfügbar als zum Wachsen benötigt wird;
			// Mobiler Stickstoff wird auf die Organe verteilt:

			// Bruchteil des den Bedarf für Wachstum übersteigenden verfügbaren N
			vNUptakeFactor = max(0.0,min(1.0,(pPltN->fActNUpt + pPPltN->dTotalTransNw - pPPltN->vNDemandForGrowth)
							/(pPPltN->dTotalDemand - pPPltN->vNDemandForGrowth + 1e-15)));
			
			// Faktoren zur Berücksichtigung der Bevorzugung von Feinwurzeln, Blättern und Früchten (Knollen)
			vNUF1	= pow(vNUptakeFactor,1.0);
			vNUF2	= pow(vNUptakeFactor,1.0);


			pPltN->fRootNinc =  (float)(vNUF1*max(0.0,pBiom->fRootWeight*(pPltN->fRootOptConc-pPltN->fRootActConc))
    							+pBiom->fRootGrowR*pPltN->fRootOptConc*NFAC_L);

			pPltN->fGrossRootNinc = (float)(vNUF2*max(0.0,pBiom->fGrossRootWeight*(pPltN->fGrossRootOptConc-pPltN->fGrossRootActConc))
									+pBiom->fGrossRootGrowR*pPltN->fGrossRootOptConc*NFAC_S);
			
			pPltN->fLeafNinc =  (float)(vNUF1*max(0.0,pBiom->fLeafWeight*(pPltN->fLeafOptConc-pPltN->fLeafActConc))
    							+pBiom->fLeafGrowR*pPltN->fLeafOptConc*NFAC_L);
			
			pPltN->fStemNinc =  (float)(vNUF2*max(0.0,pBiom->fStemWeight*(pPltN->fStemOptConc-pPltN->fStemActConc))
    							+pBiom->fStemGrowR*pPltN->fStemOptConc*NFAC_S);

			pPltN->fBranchNinc = (float)(vNUF2*max(0.0,pBiom->fBranchWeight*(pPltN->fBranchOptConc-pPltN->fBranchActConc))
								+pBiom->fBranchGrowR*pPltN->fBranchOptConc*NFAC_S);

			pPltN->fGrainNinc = (float)(vNUF1*max(0.0, pBiom->fFruitWeight*(pPltN->fFruitOptConc-pPltN->fFruitActConc))
								+pBiom->fFruitGrowR*pPltN->fFruitOptConc*NFAC_L);
			
			pPltN->fTuberNinc = (float)(vNUF1*max(0.0, pBiom->fTuberWeight*(pPltN->fTuberOptConc-pPltN->fTuberActConc))
								 +pBiom->fTuberGrowR*pPltN->fTuberOptConc*NFAC_L);


			vNGainTot = pPltN->fLeafNinc + pPltN->fBranchNinc + pPltN->fStemNinc 
						+ pPltN->fRootNinc	+ pPltN->fGrossRootNinc + pPltN->fGrainNinc
						+ pPltN->fTuberNinc;

			if(vNGainTot > pPPltN->dNPool)
			{	//Reduktion, falls zuviel verteilt wurde:
				fN = (float)(pPPltN->dNPool/vNGainTot);

				pPltN->fRootNinc *= fN;
				pPltN->fGrossRootNinc *= fN;
				pPltN->fLeafNinc *= fN;
				pPltN->fStemNinc *= fN;
				pPltN->fBranchNinc *= fN;
				pPltN->fGrainNinc *= fN;
				pPltN->fTuberNinc *= fN;
			} 

			// ohne Bevorzugung von Feinwurzeln, Blättern und Früchten
	/*		pPltN->fRootNinc =  vNUptakeFactor*(float)max(0.0,pBiom->fRootWeight*(pPltN->fRootOptConc-pPltN->fRootActConc))
    							+pBiom->fRootGrowR*pPltN->fRootOptConc;

			pPltN->fGrossRootNinc = vNUptakeFactor*(float)max(0.0,pBiom->fGrossRootWeight*(pPltN->fGrossRootOptConc-pPltN->fGrossRootActConc))
								+pBiom->fGrossRootGrowR*pPltN->fGrossRootOptConc;
			
			pPltN->fLeafNinc =  vNUptakeFactor*(float)max(0.0,pBiom->fLeafWeight*(pPltN->fLeafOptConc-pPltN->fLeafActConc))
    							+pBiom->fLeafGrowR*pPltN->fLeafOptConc;
			
			pPltN->fStemNinc =  vNUptakeFactor*(float)max(0.0,pBiom->fStemWeight*(pPltN->fStemOptConc-pPltN->fStemActConc))
    							+pBiom->fStemGrowR*pPltN->fStemOptConc;

			pPltN->fBranchNinc = vNUptakeFactor*(float)max(0.0,pBiom->fBranchWeight*(pPltN->fBranchOptConc-pPltN->fBranchActConc))
								+pBiom->fBranchGrowR*pPltN->fBranchOptConc;

			pPltN->fGrainNinc = vNUptakeFactor*(float)max(0.0, pBiom->fFruitWeight*(pPltN->fFruitOptConc-pPltN->fFruitActConc))
								+pBiom->fFruitGrowR*pPltN->fFruitOptConc;
			
			pPltN->fTuberNinc = vNUptakeFactor*(float)max(0.0, pBiom->fTuberWeight*(pPltN->fTuberOptConc-pPltN->fTuberActConc))
								 +pBiom->fTuberGrowR*pPltN->fTuberOptConc;
    */
		}
		else
		{	// verfügbarer Stickstoff reicht nur für Bedarf für Wachstum:
			pPltN->fRootNinc	= pBiom->fRootGrowR	* pPltN->fRootOptConc*NFAC_L;
			pPltN->fGrossRootNinc= pBiom->fGrossRootGrowR * pPltN->fGrossRootOptConc*NFAC_S;
			pPltN->fLeafNinc	= pBiom->fLeafGrowR	* pPltN->fLeafOptConc*NFAC_L;
			pPltN->fStemNinc	= pBiom->fStemGrowR	*pPltN->fStemOptConc*NFAC_S;
			pPltN->fBranchNinc	= pBiom->fBranchGrowR* pPltN->fBranchOptConc*NFAC_S;
			pPltN->fGrainNinc	= pBiom->fFruitGrowR* pPltN->fFruitOptConc*NFAC_L;
			pPltN->fTuberNinc	= pBiom->fTuberGrowR* pPltN->fTuberOptConc*NFAC_L;
		}
	}
	else
	{	// genügend Stickstoff vorhanden um gesamten Bedarf abzudecken		
		pPltN->fRootNinc		= pPltN->fRootDemand;
		pPltN->fGrossRootNinc	= pPltN->fGrossRootDemand;
		pPltN->fLeafNinc		= pPltN->fLeafDemand;
		pPltN->fStemNinc		= pPltN->fStemDemand;
		pPltN->fBranchNinc		= pPltN->fBranchDemand;
		pPltN->fGrainNinc		= pPltN->fGrainDemand;
		pPltN->fTuberNinc		= pPltN->fTuberDemand;

	}


	vNGainTot = pPltN->fRootNinc
				+ pPltN->fGrossRootNinc
				+ pPltN->fBranchNinc 
				+ pPltN->fStemNinc 
				+ pPltN->fLeafNinc 	
				+ pPltN->fGrainNinc
				+ pPltN->fTuberNinc;

			
			if(vNGainTot > pPPltN->dNPool)
			{	//Reduktion, falls zuviel verteilt wurde:
				fN = (float)(pPPltN->dNPool/vNGainTot);

				pPltN->fRootNinc *= fN;
				pPltN->fGrossRootNinc *= fN;
				pPltN->fLeafNinc *= fN;
				pPltN->fStemNinc *= fN;
				pPltN->fBranchNinc *= fN;
				pPltN->fGrainNinc *= fN;
				pPltN->fTuberNinc *= fN;
			} 


	return 1;
	}


int	GetNitrogenConcentrationLimits(PPLANT pPlant, int iPlant)
{
	float	fDevStage	= pPlant->pDevelop->fDevStage;

	PGENOTYPE		pGen	= pPlant->pGenotype;
	PPLTNITROGEN	pPltN	= pPlant->pPltNitrogen;
	PPLATHOGENOTYPE	pPGen	= pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLANTMATERIAL	pPPltM	= pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial;

	PPLATHONITROGEN		pPPltN  = pPlathoPlant[iPlant]->pPlathoNitrogen;


	pPltN->fLeafOptConc =		(float)AFGENERATOR(fDevStage,pGen->OptLvNc);
	pPltN->fBranchOptConc =		(float)AVGENERATOR(fDevStage,pPGen->OptBrNc);
	pPltN->fStemOptConc =		(float)AFGENERATOR(fDevStage,pGen->OptStNc);
	pPltN->fRootOptConc =		(float)AFGENERATOR(fDevStage,pGen->OptRtNc);
	pPltN->fGrossRootOptConc =	(float)AVGENERATOR(fDevStage,pPGen->OptGRtNc);
	pPltN->fFruitOptConc =		(float)AVGENERATOR(fDevStage,pPGen->OptFruitNc);
	pPltN->fTuberOptConc =		(float)AVGENERATOR(fDevStage,pPGen->OptTuberNc);

	pPltN->fLeafMinConc =		(float)AFGENERATOR(fDevStage,pGen->MinLvNc);
	pPltN->fBranchMinConc =		(float)AVGENERATOR(fDevStage,pPGen->MinBrNc);
	pPltN->fStemMinConc =		(float)AFGENERATOR(fDevStage,pGen->MinStNc);
	pPltN->fRootMinConc =		(float)AFGENERATOR(fDevStage,pGen->MinRtNc);
	pPltN->fGrossRootMinConc =	(float)AVGENERATOR(fDevStage,pPGen->MinGRtNc);
	pPPltN->vFruitMinConc	=	AVGENERATOR(fDevStage,pPGen->MinFruitNc);
	pPltN->fTuberMinConc	=	(float)AVGENERATOR(fDevStage,pPGen->MinTuberNc);

	return 1;
}


int	CalculatePotentialNitrogenUptake(EXP_POINTER2, int iPlant)
{
	double	vFNO3, vFNH4, vSMDFR,SNO3min,SNH4min;
	int		L,ll,iRL,iSector;
	int		L1 = 0;
	double	DeltaT = (double)pTi->pTimeStep->fAct;

	double	rtlEff[4];
	double	rtlEfvRight, rtlEfvLower, rtlEfvLeft, rtlEfvUpper;
    double  radius_c,r_nbg_right,r_nbg_lower,r_nbg_left,r_nbg_upper;
    double  vPotSecNO3Upt_Plant[4], vPotSecNH4Upt_Plant[4];
	int		iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;
	double	vMaxNuptR, vUpperMaxNuptR, vLeftMaxNuptR, vRightMaxNuptR, vLowerMaxNuptR;

	extern double PhiRootPathogenes(int iPlant);

	PPLANT				pPlantLeft, pPlantRight, pPlantUpper, pPlantLower;

	PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
	PPLATHOMORPHOLOGY	pPMorph		= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOROOTLAYER	pPRL		= pPMorph->pPlathoRootLayer;
	PPLATHONITROGEN		pPPltN  = pPlathoPlant[iPlant]->pPlathoNitrogen;

	PSLAYER 		pSL;
	PSWATER			pSWL;
	PWLAYER	   		pSLW; 
	PCLAYER     	pSLN;
	PLAYERROOT		pLR = pPlant->pRoot->pLayerRoot;

	PGENOTYPE			pGen	= pPlant->pGenotype;
	PBIOMASS			pBiom	= pPlant->pBiomass;
	PPLTNITROGEN		pPltN	= pPlant->pPltNitrogen;
	PROOT				pRT		= pPlant->pRoot;  
    PPLTWATER			pPltW	= pPlant->pPltWater;
	
	//======================================================================================
	//Layer Nitrogen Initiation and Transformation
	//======================================================================================

	for (L=1;L<=pSo->iLayers-2;L++)
	{
		pPPltN->vRNO3U[L]=0.0;
		pPPltN->vRNH4U[L]=0.0;

		pLR=pLR->pNext;
	}

	pPPltN->dPotNUpt		= 0.0;
	pPPltStress->vNCC	= 0.0;
	iRL = 0;


	pSL		= pSo->pSLayer->pNext;
	pSWL	= pSo->pSWater->pNext;
	pSLW	= pWa->pWLayer->pNext; 
	pSLN	= pCh->pCLayer->pNext;
	pLR		= pPlant->pRoot->pLayerRoot;

	for (L=1;L<=pSo->iLayers-2;L++)
	{
		//Check the whether there are roots in this layer:
		if (pLR->fLengthDens==(float)0.0)		break;
		//The last layer of root:
		L1=L;

		pLR = pLR->pNext;
	}


	// Vier nächste Nachbarn:
	GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
											&iPlantUpper, &iPlantLower);

	pPlantLeft	= 	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft;
	pPlantRight	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight;
	pPlantUpper	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper;
	pPlantLower	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower ;


	//Reduktion des N-Aufnahmevermögens der Wurzeln
	//bei Schädigung durch Wurzelpathogene:

	vMaxNuptR	    = pPlant->pGenotype->fMaxNuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlant) : 1.0);
	vUpperMaxNuptR	= pPlantUpper->pGenotype->fMaxNuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantUpper) : 1.0);
	vLeftMaxNuptR	= pPlantLeft->pGenotype->fMaxNuptRate  * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantLeft) : 1.0);
	vRightMaxNuptR	= pPlantRight->pGenotype->fMaxNuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantRight) : 1.0);
	vLowerMaxNuptR	= pPlantLower->pGenotype->fMaxNuptRate * (pPlathoModules->iFlagRootPathEffUpt ==2 ? PhiRootPathogenes(iPlantLower) : 1.0);

    radius_c    = 0.5*pPMorph->vRootZoneDiameter;
    r_nbg_right = 0.5*pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneDiameter;
    r_nbg_lower = 0.5*pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneDiameter;
    r_nbg_left  = 0.5*pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneDiameter;
    r_nbg_upper = 0.5*pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneDiameter;



	pLR	 = pPlant->pRoot->pLayerRoot;
	pPRL = pPMorph->pPlathoRootLayer;
	for (L=1;L<=L1;L++)
	{

		pSLN->fNO3Nmgkg=(float)0.01*pSL->fBulkDens*pSL->fThickness;
		pSLN->fNH4Nmgkg=(float)0.01*pSL->fBulkDens*pSL->fThickness;
// 0.01 rechnet [kg(N)/ha]/[kg/dm^3]/[mm] in [mg(N)]/[kg(S)]

		//Potential nitrogen availability fFACTOR for NO3 (vFNO3) and NH4 (vFNH4) (0-1):
		
		BARLEY
		{
			vFNO3=(1.0-exp(-0.0275*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.025*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

		//	if (vFNO3<(float)0.03) vFNO3=(float)0.0;
		//	if (vFNH4<(float)0.03) vFNH4=(float)0.0;

			if (vFNO3<0.03) vFNO3 *= (vFNO3/0.03)*(vFNO3/0.03)*(vFNO3/0.03);//(float)0.0;
		  	if (vFNH4<0.03) vFNH4 *= (vFNH4/0.03)*(vFNH4/0.03)*(vFNH4/0.03);//(float)0.0;

		}

		WHEAT
		{
			vFNO3=(1.0-exp(-0.0275*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.025*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

			//if (vFNO3<(float)0.04) vFNO3=(float)0.0;
			//if (vFNH4<(float)0.04) vFNH4=(float)0.0;
					  	
			if (vFNO3<0.03) vFNO3 *= (vFNO3/0.03)*(vFNO3/0.03)*(vFNO3/0.03);//(float)0.0;
		  	if (vFNH4<0.03) vFNH4 *= (vFNH4/0.03)*(vFNH4/0.03)*(vFNH4/0.03);//(float)0.0;

		}

		POTATO
		{
			vFNO3=(1.0-exp(-0.030*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.030*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

		//	if (vFNO3<(float)0.04) vFNO3=(float)0.0;
		//	if (vFNH4<(float)0.04) vFNH4=(float)0.0;

			if (vFNO3<0.03) vFNO3 *= (vFNO3/0.03)*(vFNO3/0.03)*(vFNO3/0.03);//(float)0.0;
		  	if (vFNH4<0.03) vFNH4 *= (vFNH4/0.03)*(vFNH4/0.03)*(vFNH4/0.03);//(float)0.0;
		}
			
		MAIZE 		
		{
			vFNO3=(1.0-exp(-0.030*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.030*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

//			if (vFNO3<(float)0.03) vFNO3=(float)0.0;
//			if (vFNH4<(float)0.03) vFNH4=(float)0.0;

			if (vFNO3<0.04) vFNO3=0.0;
			if (vFNH4<0.04) vFNH4=0.0;
		}

		SUNFLOWER			
		{
			vFNO3=(1.0-exp(-0.030*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.030*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

		//	if (vFNO3<(float)0.04) vFNO3=(float)0.0;
		//	if (vFNH4<(float)0.04) vFNH4=(float)0.0;

			if (vFNO3<0.03) vFNO3 *= (vFNO3/0.03)*(vFNO3/0.03)*(vFNO3/0.03);//(float)0.0;
		  	if (vFNH4<0.03) vFNH4 *= (vFNH4/0.03)*(vFNH4/0.03)*(vFNH4/0.03);//(float)0.0;
		}
		  
		LOLIUM			
		{
			vFNO3=(1.0-exp(-0.030*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.030*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

			if (vFNO3<0.04) vFNO3=0.0;
			if (vFNH4<0.04) vFNH4=0.0;
		}

		BEECH			
		{
			vFNO3=(1.0-exp(-0.03*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.03*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

			//if (vFNO3<0.02) vFNO3=0.0;
			//if (vFNH4<0.02) vFNH4=0.0;
			if (vFNO3<0.00) vFNO3=0.0;
			if (vFNH4<0.00) vFNH4=0.0;
		}

		SPRUCE			
		{
			vFNO3=(1.0-exp(-0.030*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.030*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

			//if (vFNO3<0.02) vFNO3=0.0;
			//if (vFNH4<0.02) vFNH4=0.0;
			if (vFNO3<0.00) vFNO3=0.0;
			if (vFNH4<0.00) vFNH4=0.0;
		}

		APPLE	 		
		{
			vFNO3=(1.0-exp(-0.030*((double)(pSLN->fNO3N/pSLN->fNO3Nmgkg))));
			vFNH4=(1.0-exp(-0.030*(((double)(pSLN->fNH4N/pSLN->fNH4Nmgkg))-0.5)));

			if (vFNO3<0.02) vFNO3=0.0;
			if (vFNH4<0.02) vFNH4=0.0;
		}

		if (vFNO3>1.0) vFNO3=1.0;
		if (vFNH4>1.0) vFNH4=1.0;


		//Soil moisture deficit factor (vSMDFR) affecting nitrogen uptake at low soil water:
        if(pPlathoModules->iFlagH2OEffectPhot==1)//no water deficit effect on photosynthesis
        {
            vSMDFR = 1.0;
        }
        else
        {
            float fracPWP = (float)0.5;
		  
        if (pSLW->fContAct<pSWL->fContFK)
			    vSMDFR=(pSLW->fContAct-fracPWP*pSWL->fContPWP)/(pSWL->fContFK-fracPWP*pSWL->fContPWP);
		   else
			    vSMDFR=(pSWL->fContSat-pSLW->fContAct)/(pSWL->fContSat-pSWL->fContFK);
/*
					
		if(((pSLW->fMatPotAct>0))||(pSLW->fMatPotAct<Psi3))
				vSMDFR = (float)0.0;
			else if((pSLW->fMatPotAct<0)&&(pSLW->fMatPotAct>Psi1))
				vSMDFR = pSLW->fMatPotAct/Psi1;
			else if((pSLW->fMatPotAct<Psi1)&&(pSLW->fMatPotAct>Psi2))
				vSMDFR = 1.0;
			else
				vSMDFR =  (pSLW->fMatPotAct - Psi3)/(Psi2-Psi3);
		
*/
        
     /*     
            if((pSLW->fContAct>=pSWL->fContSat)||(pSLW->fContAct<fracPWP*pSWL->fContPWP))
				vSMDFR = 0.0;
			else if(pSLW->fContAct>0.9*pSWL->fContSat)
				vSMDFR = 1.0-(pSLW->fContAct - 0.9*pSWL->fContSat)/
												(pSWL->fContSat-0.9*pSWL->fContSat);
			else if((pSLW->fContAct<=0.9*pSWL->fContSat)&&(pSLW->fContAct>pSWL->fContFK))
				vSMDFR = 1.0;
			else
				vSMDFR =  (pSLW->fContAct - fracPWP*pSWL->fContPWP)/(pSWL->fContFK-fracPWP*pSWL->fContPWP);
      */       
        }

		vSMDFR = max(min(1.0,pow(vSMDFR,2.0)),0.0);

        if(pPMorph->vPlantLAI == 0.0)
			vSMDFR = 0.0;

		
		// COMPETITION FOR NITROGEN: /////////////////////////////////////////////

    if(pPlathoModules->iFlagSectors==1)
    {
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

			rtlEfvUpper=pPMorph->BlwgCoeffComp[3]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneArea
				*pUpperPRL->vRootArea*vUpperMaxNuptR;
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

			rtlEfvLeft=pPMorph->BlwgCoeffComp[2]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneArea
				*pLeftPRL->vRootArea*vLeftMaxNuptR;
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

			rtlEfvRight=pPMorph->BlwgCoeffComp[0]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneArea
				*pRightPRL->vRootArea*vRightMaxNuptR;
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

			rtlEfvLower=pPMorph->BlwgCoeffComp[1]*pPMorph->vRootZoneArea/pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneArea
				*pLowerPRL->vRootArea*vLowerMaxNuptR;
		}

		if(pPRL->vRootArea > 0.0)
            pPRL->vNCompCoeff = pPRL->vRootArea*vMaxNuptR
				/(pPRL->vRootArea*vMaxNuptR + rtlEfvUpper+rtlEfvLeft+rtlEfvRight+rtlEfvLower);
        else
            pPRL->vNCompCoeff = 0.0;
    }

    if(pPlathoModules->iFlagSectors==2)
    {

		for(iSector=0;iSector<4;iSector++)
            rtlEff[iSector] = 0.0;


		if(pPMorph->BlwgCoeffComp[0] > 0.0)
		{
			//PLAYERROOT		 pRightLR  = pPlantRight->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pRightPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pRightPRL = pRightPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[0] = SectorCompFactor(radius_c, r_nbg_right);

			//potential nitrogen uptake of the next neighbour [kg/d] = root surface [m^2] * MaxNUpt [kg(N)/m^2/d]
            rtlEff[0]=pPMorph->SectorCompCoeff_blwg[0]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneArea
				    *pRightPRL->vRootArea*vRightMaxNuptR;
		}


		if(pPMorph->BlwgCoeffComp[1] > 0.0)
		{
			//PLAYERROOT		 pLowerLR  = pPlantLower->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pLowerPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pLowerPRL = pLowerPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[1] = SectorCompFactor(radius_c, r_nbg_lower);

			//potential nitrogen uptake of the next neighbour [kg/d] = root surface [m^2] * MaxNUpt [kg(N)/m^2/d]
            rtlEff[1]=pPMorph->SectorCompCoeff_blwg[1]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneArea
				    *pLowerPRL->vRootArea*vLowerMaxNuptR;
		}

		if(pPMorph->BlwgCoeffComp[2] > 0.0)
		{
			//PLAYERROOT		 pLeftLR  = pPlantLeft->pRoot->pLayerRoot;
			PPLATHOROOTLAYER pLeftPRL = pPlathoPlant[iPlant]->pPlathoMorphology->pPlathoRootLayer;
			ll=1;
			while(ll<L)
			{
				pLeftPRL = pLeftPRL->pNext;
				ll++;
			}

            pPMorph->SectorCompCoeff_blwg[2] = SectorCompFactor(radius_c, r_nbg_left);

			//potential nitrogen uptake of the next neighbour [kg/d] = root surface [m^2] * MaxNUpt [kg(N)/m^2/d]
            rtlEff[2]=pPMorph->SectorCompCoeff_blwg[2]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneArea
				    *pLeftPRL->vRootArea*vLeftMaxNuptR;
		}

		if(pPMorph->BlwgCoeffComp[3]> 0.0)
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

			//potential nitrogen uptake of the next neighbour [kg/d] = root surface [m^2] * MaxNUpt [kg(N)/m^2/d]
            rtlEff[3]=pPMorph->SectorCompCoeff_blwg[3]*pPMorph->vRootZoneArea/4.0/pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneArea
				    *pUpperPRL->vRootArea*vUpperMaxNuptR;
		}

        for(iSector=0;iSector<4;iSector++)
        {
            if(pPRL->apRootSector[iSector]->vRootArea==0.0)
			    pPRL->apRootSector[iSector]->vNCompCoeff = 0.0;
		    else
			    pPRL->apRootSector[iSector]->vNCompCoeff = pPRL->apRootSector[iSector]->vRootArea*vMaxNuptR
				    /(pPRL->apRootSector[iSector]->vRootArea*vMaxNuptR + rtlEff[iSector]);
        }
    }  // end iFlagSector = 2


     if(pPlathoModules->iFlagSectors==1)
     {
		//potenzielle Aufnahme aller Pflanzen aus der betrachteten Bodenscheibe [kg(N)]
			
		if(pPMorph->vCrownDiameter==0.0)
        {
        	pPPltN->vRNO3U[L]=0.0; 	
		    pPPltN->vRNH4U[L]=0.0;  

        }
        else
        {
		    pPPltN->vRNO3U[L]=(pPRL->vRootArea*vMaxNuptR+rtlEfvUpper+rtlEfvLeft+rtlEfvRight+rtlEfvLower)*vFNO3*vSMDFR*DeltaT; 	
		    pPPltN->vRNH4U[L]=(pPRL->vRootArea*vMaxNuptR+rtlEfvUpper+rtlEfvLeft+rtlEfvRight+rtlEfvLower)*vFNH4*vSMDFR*DeltaT;  
        }
     }//end iFlagSectors = 1

     if(pPlathoModules->iFlagSectors==2)
     {
		//potenzielle Aufnahme aller Pflanzen aus der betrachteten Bodenscheibe [kg(N)]
		pPPltN->vRNO3U[L]=0.0; 	
		pPPltN->vRNH4U[L]=0.0;  

		for(iSector=0;iSector<4;iSector++)
        {
		    if(pPMorph->vCrownDiameter==0.0)
            {
                pPRL->apRootSector[iSector]->vPotSecNO3Upt = 0.0;
                pPRL->apRootSector[iSector]->vPotSecNH4Upt = 0.0;
            }
            else
            {
		        pPRL->apRootSector[iSector]->vPotSecNO3Upt=(pPRL->apRootSector[iSector]->vRootArea*vMaxNuptR+rtlEff[iSector])*vFNO3*vSMDFR*DeltaT; 	
		        pPRL->apRootSector[iSector]->vPotSecNH4Upt=(pPRL->apRootSector[iSector]->vRootArea*vMaxNuptR+rtlEff[iSector])*vFNH4*vSMDFR*DeltaT;  
            }
            		
            pPPltN->vRNO3U[L] += pPRL->apRootSector[iSector]->vPotSecNO3Upt; 	
		    pPPltN->vRNH4U[L] += pPRL->apRootSector[iSector]->vPotSecNH4Upt;  
        }//end Sectors
     }//end iFlagSectors = 2

	//Low limit of extractable nitrogen in layer L: SNO3min,SNH4min[Kg(N)]
	SNO3min=(float)(0.25*pSLN->fNO3Nmgkg*pPMorph->vRootZoneArea*1E-4);
	SNH4min=(float)(0.50*pSLN->fNH4Nmgkg*pPMorph->vRootZoneArea*1E-4);


     if(pPlathoModules->iFlagSectors==1)
     {
        //Possible plant uptake from a layer:(->fUNO3,->fUNH4 [Kg(N)])
		if(pPPltN->vRNO3U[L]>pSLN->fNO3N*pPMorph->vRootZoneArea*1E-4-SNO3min)
			pPPltN->vRNO3U[L] = pPRL->vNCompCoeff*max(0.0, pSLN->fNO3N*pPMorph->vRootZoneArea*1E-4-SNO3min);
		else
			pPPltN->vRNO3U[L] *= pPRL->vNCompCoeff;

		if(pPPltN->vRNH4U[L]>(pSLN->fNH4N*pPMorph->vRootZoneArea*1E-4-SNH4min))
			pPPltN->vRNH4U[L]=pPRL->vNCompCoeff*max(0.0, pSLN->fNH4N*pPMorph->vRootZoneArea*1E-4-SNH4min);
		else
			pPPltN->vRNH4U[L] *= pPRL->vNCompCoeff;
     }

     if(pPlathoModules->iFlagSectors==2)
     {
		//Possible plant uptake from a layer:(->fUNO3,->fUNH4 [Kg(N)])

        pPPltN->vRNO3U[L] = 0.0;
        pPPltN->vRNH4U[L] = 0.0;

        for(iSector=0;iSector<4;iSector++)
        {
            //NO3
		    if(pPRL->apRootSector[iSector]->vPotSecNO3Upt > (pSLN->fNO3N*pPMorph->vRootZoneArea*1E-4-SNO3min)/4.0)
			    vPotSecNO3Upt_Plant[iSector] = pPRL->apRootSector[iSector]->vNCompCoeff*max(0.0,(pSLN->fNO3N*pPMorph->vRootZoneArea*1E-4-SNO3min)/4.0);
		    else
                vPotSecNO3Upt_Plant[iSector] = pPRL->apRootSector[iSector]->vPotSecNO3Upt*pPRL->apRootSector[iSector]->vNCompCoeff;

		    //NH4
            if(pPRL->apRootSector[iSector]->vPotSecNH4Upt > (pSLN->fNH4N*pPMorph->vRootZoneArea*1E-4-SNH4min)/4.0)
			    vPotSecNH4Upt_Plant[iSector] = pPRL->apRootSector[iSector]->vNCompCoeff*max(0.0,(pSLN->fNH4N*pPMorph->vRootZoneArea*1E-4-SNH4min)/4.0);
		    else
                vPotSecNH4Upt_Plant[iSector] = pPRL->apRootSector[iSector]->vPotSecNH4Upt*pPRL->apRootSector[iSector]->vNCompCoeff;

            pPPltN->vRNO3U[L] += vPotSecNO3Upt_Plant[iSector];
            pPPltN->vRNH4U[L] += vPotSecNH4Upt_Plant[iSector];
        }// end iSectors
     }// end iFlagSectors==2
															
		//Total Potential Root Nitrogen Uptake [kg(N)]
		pPPltN->dPotNUpt += (float)max(0.0,pPPltN->vRNO3U[L]+pPPltN->vRNH4U[L]);
                                    
		pSL =pSL ->pNext;
		pSWL=pSWL->pNext;
		pSLW=pSLW->pNext;
		pSLN=pSLN->pNext;
		pLR =pLR ->pNext;
		pPRL=pPRL->pNext;
	}

	return 1;
}


double	CalcPotNitrogenTransloc(PPLANT pPlant, int iPlant, double DeltaT)
{
	double vPotNitrogenTransloc;
    double vTransNconst = pPlathoPlant[iPlant]->pPlathoGenotype->vNTransRate;

	PBIOMASS		pBiom	= pPlant->pBiomass;
	PPLTNITROGEN	pPltN	= pPlant->pPltNitrogen;
	PPLATHONITROGEN	pPPltN	= pPlathoPlant[iPlant]->pPlathoNitrogen;


	pPltN->fRootTransNw		  = (float)(max(0.0,pBiom->fRootWeight * (pPltN->fRootActConc-pPltN->fRootMinConc)));
	pPPltN->vGrossRootTransNw = max(0.0,pBiom->fGrossRootWeight * (pPltN->fGrossRootActConc-pPltN->fGrossRootMinConc));
	pPltN->fStemTransNw		  = (float)(max(0.0,pBiom->fStemWeight * (pPltN->fStemActConc-pPltN->fStemMinConc)));
	pPPltN->vBranchTransNw	  = max(0.0,pBiom->fBranchWeight * (pPltN->fBranchActConc-pPltN->fBranchMinConc));
	pPltN->fLeafTransNw		  = (float)(max(0.0,pBiom->fLeafWeight * (pPltN->fLeafActConc-pPltN->fLeafMinConc)));
//	pPltN->fLeafTransNw		  = (float)0.0; //keine Retranslokation aus Blättern
	pPPltN->vFruitTransNw	  = max(0.0,pBiom->fBranchWeight * (pPltN->fGrainConc-pPPltN->vFruitMinConc));
	pPPltN->vTuberTransNw	  = max(0.0,pBiom->fTuberWeight * (pPltN->fTuberActConc-pPltN->fTuberMinConc));


	pPltN->fTotalNLabile = pPltN->fRootTransNw 
						 + (float)pPPltN->vGrossRootTransNw 
					     + pPltN->fStemTransNw 
						 + (float)pPPltN->vBranchTransNw  
						 + pPltN->fLeafTransNw
						 + (float)pPPltN->vFruitTransNw
						 + (float)pPPltN->vTuberTransNw;

    vPotNitrogenTransloc =  (pPltN->fTotalNLabile*vTransNconst + pPPltN->vNSeedReserves*2.0*vTransNconst)*DeltaT;

	return vPotNitrogenTransloc;
}



int	LeafNitrogenDistribution(double vLfWeightCrit1, double vLfWeightCrit2, PPLANT pPlant, PLEAFLAYER pLLayer)
{
	PPLTNITROGEN	pPltN = pPlant->pPltNitrogen;
	PBIOMASS		pBiom = pPlant->pBiomass;

	int iSector;

    if(pPltN->fLeafActConc <= (pPltN->fLeafOptConc+pPltN->fLeafMinConc)/(float)2.0)
	{
		if(pLLayer->vLeafWeightCum <= vLfWeightCrit1)
			pLLayer->vLeafNConc = pPltN->fLeafMinConc;
		else
			pLLayer->vLeafNConc = pPltN->fLeafMinConc+(pPltN->fLeafOptConc-pPltN->fLeafMinConc)
				/(pBiom->fLeafWeight-vLfWeightCrit1) * (pLLayer->vLeafWeightCum-vLfWeightCrit1);
	}
	else
	{
		if(pLLayer->vLeafWeightCum <= vLfWeightCrit2)
			pLLayer->vLeafNConc = 
			pPltN->fLeafMinConc+(pPltN->fLeafOptConc-pPltN->fLeafMinConc)
			/vLfWeightCrit2* pLLayer->vLeafWeightCum;
		else
			pLLayer->vLeafNConc = pPltN->fLeafOptConc;
	}

    //Im moment noch gleiche N-Konzentration in allen Sektoren
    if(pPlathoModules->iFlagSectors==2)
    {
        for(iSector=0;iSector<4;iSector++)
        {
            pLLayer->apLeafSector[iSector]->vLeafNConc = pLLayer->vLeafNConc;
        }
    } // end iFlagSectors == 2

	return 1;
}
