//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"


// W U R Z E L V E R T E I L U N G ///////////////////////////////////////////////
int RootSystem3_PLATHO(EXP_POINTER2,int iPlant);

extern double RelativeTemperatureResponse_PLATHO(double vTemp, double vMinTemp, double vOptTemp, double vMaxTemp);

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//	RootGrowth_PLATHO
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int RootSystem3_PLATHO(EXP_POINTER2,int iPlant)
	{
    int 	L1, NLAYR, i, L;
    double  vCumDepth,DEPMAX;
	double  TRLDF,FRNLF,GRNLF,vRelExtWater,vNewRootGrowFactor,vNewRootLength,vThickness;
	double 	vDepth,vRelWc,vTempFunc,vMoistRed;
	double	vTempMin,vTempMax,vTempOpt;	
	double	vRootDens;
	double	vCalcDepth, alpha, beta;
	double	FRDiameter, FRArea;
	double	GRDiameter = 0.0;
	double  GRArea = 0.0;

	int		iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;

	PPLANT		pPlantLeft, pPlantRight, pPlantUpper, pPlantLower;

	PSLAYER 	pSL		= pSo->pSLayer->pNext;
	PSWATER		pSWL	= pSo->pSWater->pNext;
	PHLAYER		pSLH	= pHe->pHLayer->pNext;
	PWLAYER	   	pSLW	= pWa->pWLayer->pNext; 
	PCLAYER    	pSLN	= pCh->pCLayer;
	PROOT		pRT		= pPlant->pRoot;  
	PLAYERROOT	pLR		= pPlant->pRoot->pLayerRoot;
	PBIOMASS	pBm		= pPlant->pBiomass;    
 
	PPLATHOGENOTYPE		pPGen   = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOMORPHOLOGY	pPMorph = pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLATHOROOTLAYER	pPRL	= pPMorph->pPlathoRootLayer;

	
	PLAYERROOT	pUpperLR;
	PLAYERROOT	pLeftLR;
	PLAYERROOT	pRightLR;
	PLAYERROOT	pLowerLR;

	
	float	DeltaT = pTi->pTimeStep->fAct;

	double	vNewFineRootLength  = 0.0;
	double	vNewGrossRootLength = 0.0;
			
	double	rldUpper, rldLeft, rldRight, rldLower, TotalRLD;

	double	vStressRLD;


	//root length density capacity (abhängig von N und H2O?)
	float	KRLD = (float)3e4; //[m/m3]

	extern int	 GetPlantNeighbours(PPLANT, int, int*, int*, int*, int*);
	extern double Weibull(double, double, double);


    DEPMAX=0.0;
	pSL	  =pSo->pSLayer->pNext;
	for (L=1;L<=pSo->iLayers-2;L++)
		{
		DEPMAX += 0.1*pSL->fThickness;	//cm
		pSL=pSL->pNext;
		}
  
  	pRT->fMaxDepth=(float)(0.01*min(DEPMAX,pPlant->pGenotype->fMaxRootDepth));

	//=========================================================================================
	//Root Depth Growth Rate
	//=========================================================================================
    vDepth 	= 0.0;
	pSL		=pSo->pSLayer->pNext;
	pSLH	=pHe->pHLayer->pNext;
	pSLW	=pWa->pWLayer->pNext;
	pSWL	=pSo->pSWater->pNext;
	for (i=1;i<=pSo->iLayers-2;i++)
	{ 
		vThickness = 0.001*pSL->fThickness; //m
		
		if ((pRT->fDepth > vDepth) && (pRT->fDepth <= vDepth+vThickness))
			break;
		vDepth += vThickness; 
		
		if (i<pSo->iLayers-2)
		{
			pSLW = pSLW->pNext;
			pSWL = pSWL->pNext;
			pSL = pSL ->pNext;
			pSLH= pSLH->pNext;
		}
	}

	if ((pRT->fDepth<pRT->fMaxDepth))
		{    
		//Temperature Effect

		vTempMin = pPGen->vRootExtTempMin;
		vTempOpt = pPGen->vRootExtTempOpt; 
		vTempMax = pPGen->vRootExtTempMax;
		

		if (pPlant->pGenotype->RootExtRateTemp[0].fInput==(float)0.0)
			vTempFunc=RelativeTemperatureResponse_PLATHO(pSLH->fSoilTemp,vTempMin,vTempOpt,vTempMax);
		else
			vTempFunc=AFGENERATOR(pSLH->fSoilTemp, pPlant->pGenotype->RootExtRateTemp);
		
		//-------------------------------------------------------------------------------
		//Soil water deficit factor for root growth (pLR->fWatStressRoot)
        vRelWc =(pSLW->fContAct-pSWL->fContPWP)/(pSWL->fContFK-pSWL->fContPWP);
        
		if (vRelWc<0.25)
			vMoistRed =max(0.0,4.0*vRelWc);
		else
			vMoistRed = 1.0;

		//Tiefenwachstum in [m]
		
		pRT->fDepthGrowR=(float)(0.01*max(0.0,pPlant->pGenotype->fMaxRootExtRate*vTempFunc*vMoistRed
			* pow((1.0-pRT->fDepth/(pPlant->pGenotype->fMaxRootDepth*0.01)),0.5)));
		}
	else
		pRT->fDepthGrowR = (float)0.0;		
		
	//=========================================================================================
	//Root Depth 
	//=========================================================================================
	
	pRT->fDepth += pRT->fDepthGrowR*DeltaT;

	pRT->fDepth = min(pRT->fDepth,pRT->fMaxDepth);	

		
	

    NLAYR=pSo->iLayers-2;

    vNewFineRootLength = pBm->fRootGrowR * pPlant->pGenotype->fRootLengthRatio;
	
	TREES
		vNewGrossRootLength = pBm->fGrossRootGrowR * pPGen->vSpecGrossRootLength;

	vNewRootLength = vNewFineRootLength + vNewGrossRootLength; //[m]

	// parameter for root distribution
	alpha = 1.0 + 2.0*pPGen->vRelDepthMaxRoot;

	if((pPGen->vRelDepthMaxRoot == 0.0)||(alpha == 1.0))
		beta = pPlant->pRoot->fDepth/(3.0*log(2));
	else
		beta = pPGen->vRelDepthMaxRoot*pPlant->pRoot->fDepth
						/(pow((alpha-1)/alpha,1/alpha));


	//=========================================================================================
	//		Factor Calculation for Root Growth
	//=========================================================================================
//	fCumDepth	=(float)0.0;
//	L = 0;   

	// Vier nächste Nachbarn
	GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
											&iPlantUpper, &iPlantLower);

	pPlantLeft	= 	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft;
	pPlantRight	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight;
	pPlantUpper	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper;
	pPlantLower	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower ;

	
	pSL		=pSo->pSLayer->pNext;
	pSWL	=pSo->pSWater->pNext;
	pSLW	=pWa->pWLayer->pNext; 
	pSLN	=pCh->pCLayer->pNext;
	pLR		=pPlant->pRoot->pLayerRoot;
	pUpperLR =pPlantUpper->pRoot->pLayerRoot;
	pLeftLR	 =pPlantLeft->pRoot->pLayerRoot;
	pRightLR =pPlantRight->pRoot->pLayerRoot;
	pLowerLR =pPlantLower->pRoot->pLayerRoot;
	pPRL	= pPMorph->pPlathoRootLayer;

// 	while ((fCumDepth<=pRT->fDepth)&&(L<=NLAYR))
    for(vCumDepth=0.0,L=1;(vCumDepth<=pRT->fDepth)&&(L<=NLAYR);L++)
	{
	//	L++;
		
		vThickness = 0.001*pSL->fThickness; //m

		vCumDepth += 0.001*pSL->fThickness; //m
		vCalcDepth = vCumDepth - 0.001*pSL->fThickness/2.0;

        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		//-------------------------------------------------------------------------------
		//Soil water deficit factor for root growth (pLR->fWatStressRoot)
        vRelExtWater=(pSLW->fContAct-pSWL->fContPWP)/(pSWL->fContFK-pSWL->fContPWP);
		if (vRelExtWater < 0.25)
			pLR->fWatStressRoot =(float)max(0.0,4.0*vRelExtWater);
		else
			pLR->fWatStressRoot = (float)1.0;
                                          
		//-------------------------------------------------------------------------------
        //Mineral nitrogen availability effect factor on root growth (pLR->fNStressRoot)
		
		pSLN->fNO3Nmgkg=(float)0.01*pSL->fBulkDens*pSL->fThickness;
		pSLN->fNH4Nmgkg=(float)0.01*pSL->fBulkDens*pSL->fThickness;

		pLR->fNStressRoot =(float)(1.0-(1.17*exp(-0.15*(double)(pSLN->fNO3N/pSLN->fNO3Nmgkg
																   +pSLN->fNH4N/pSLN->fNH4Nmgkg))));
			
		if(pLR->fNStressRoot<(float)0.01)
			   pLR->fNStressRoot=(float)0.01;
		
		//-------------------------------------------------------------------------------
        //root density effect factor on root growth (vStressRLD)
		rldRight = pPMorph->BlwgCoeffComp[0]*pRightLR->fLengthDens*
			pPMorph->vRootZoneArea/pPlathoPlant[iPlantRight]->pPlathoMorphology->vRootZoneArea;

		rldLower = pPMorph->BlwgCoeffComp[1]*pLowerLR->fLengthDens*
			pPMorph->vRootZoneArea/pPlathoPlant[iPlantLower]->pPlathoMorphology->vRootZoneArea;

		rldLeft = pPMorph->BlwgCoeffComp[2]*pLeftLR->fLengthDens*
			pPMorph->vRootZoneArea/pPlathoPlant[iPlantLeft]->pPlathoMorphology->vRootZoneArea;

		rldUpper = pPMorph->BlwgCoeffComp[3]*pUpperLR->fLengthDens*
			pPMorph->vRootZoneArea/pPlathoPlant[iPlantUpper]->pPlathoMorphology->vRootZoneArea;


		TotalRLD = (pLR->fLengthDens + rldUpper + rldLeft + rldRight + rldLower)
					/(pPMorph->vRootZoneArea*vThickness);

		vStressRLD = (float)max(0.05,pow(1.0 - TotalRLD/KRLD,1.0));
//		vStressRLD = (float)min(1.0,pow(max(0.0,1.0 - TotalRLD/KRLD),2.0));
//		vStressRLD = (float)max(fStressRLD,0.1);
//		vfStressRLD = (float)1.0;


        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        //Root length density factor to clculate new root growth distribution 
		// w(z) = alpha/beta * [z/b]^(alpha-1) * Exp[-(z/beta)^alpha]
		vNewRootGrowFactor = Weibull(vCalcDepth, beta, alpha);

		pLR->fLengthDensFac = (float)(vNewRootGrowFactor*min(1.0,vStressRLD));
		pLR->fLengthDensFac *= (float)min(pLR->fWatStressRoot,pLR->fNStressRoot);

		 if(vCalcDepth>pRT->fDepth)
			pLR->fLengthDensFac=(float)0.0;


		if (L<=NLAYR-1)
		{
			pSL  = pSL ->pNext;
			pSWL = pSWL->pNext;
			pSLW = pSLW->pNext;
			pSLN = pSLN->pNext;
			pLR  = pLR->pNext;
			pUpperLR =pUpperLR->pNext;
			pLeftLR  =pLeftLR->pNext;
			pRightLR =pRightLR->pNext;
			pLowerLR =pLowerLR->pNext;
			pPRL =pPRL->pNext;
		}
	}  

	//	Deepest root layer
		//L1=L;
		L1=L-1;

	//=========================================================================================
	//		Total Root Length Density Factor
	//=========================================================================================
	TRLDF =(float)0.0;
	pLR	  =pPlant->pRoot->pLayerRoot;
	for (L=1;L<=L1;L++)
	{
		TRLDF += pLR->fLengthDensFac;
        pLR    = pLR->pNext;
    }

	//=========================================================================================
	//		Root Length Density Calculation
	//=========================================================================================
	if(TRLDF > (float)0.0)
    {
        FRNLF = vNewFineRootLength/TRLDF; 
	    GRNLF = vNewGrossRootLength/TRLDF; 
    }
    else
    {
        FRNLF = 0.0; 
	    GRNLF = 0.0; 
    }

	vRootDens = 0.2; // [g/cm^3]

    FRDiameter = sqrt(4e-3/(PI*vRootDens*pPlant->pGenotype->fRootLengthRatio));  //[m]
	TREES
		GRDiameter = sqrt(4e-3/(PI*vRootDens*pPGen->vSpecGrossRootLength));   //[m]

	pLR		= pPlant->pRoot->pLayerRoot;
	pPRL	= pPMorph->pPlathoRootLayer;
	for (L=1;L<=L1;L++)
	{
        pLR->fLengthDens += pLR->fLengthDensFac*(float)FRNLF;   //[m]
        pLR->fLengthDens  = (float)max(0.0,pLR->fLengthDens);
        
        pPRL->vGRLengthDens += pLR->fLengthDensFac*GRNLF;     //[m]
        pPRL->vGRLengthDens  = max(0.0,pPRL->vGRLengthDens);


        pPRL->vFRLayBiomass = pLR->fLengthDens/pPlant->pGenotype->fRootLengthRatio ;
        
        TREES
            pPRL->vGRLayBiomass = pPRL->vGRLengthDens/pPGen->vSpecGrossRootLength;
        

		//Oberfläche der Wurzeln [m^2]:
	
		FRArea = FRDiameter * PI * pLR->fLengthDens;
		TREES
			GRArea = GRDiameter * PI * pPRL->vGRLengthDens;

		pPRL->vRootArea = FRArea+GRArea;
//		pPRL->vRootArea = FRArea;

        if(pPlathoModules->iFlagSectors==2)
        {
            double vNewLayFRBiomass, vNewLayGRBiomass;
            double MeanCompCoeff, Alpha;
            double BlwgDistrFac[4];

            int iSector;

            vNewLayFRBiomass = pLR->fLengthDensFac*FRNLF/pPlant->pGenotype->fRootLengthRatio;
            TREES
                vNewLayGRBiomass = pLR->fLengthDensFac*GRNLF/pPGen->vSpecGrossRootLength;

            MeanCompCoeff = (pPRL->apRootSector[0]->vSecCompCoeff
                                    +pPRL->apRootSector[1]->vSecCompCoeff
                                    +pPRL->apRootSector[2]->vSecCompCoeff
                                    +pPRL->apRootSector[3]->vSecCompCoeff)/4.0;

            Alpha = MeanCompCoeff < 0.75 ? 0.25/(1.0-MeanCompCoeff) : 0.75/MeanCompCoeff;
                
            for(iSector=0;iSector<4;iSector++)
            {
                BlwgDistrFac[iSector] = pPMorph->vPlasticity * Alpha * (MeanCompCoeff - pPMorph->SectorCompCoeff_blwg[iSector]) + 0.25;

                pPRL->apRootSector[iSector]->vFRSecBiomass += vNewLayFRBiomass*(float)BlwgDistrFac[iSector];
			    TREES
                    pPRL->apRootSector[iSector]->vGRSecBiomass += vNewLayGRBiomass*(float)BlwgDistrFac[iSector];

			    pPRL->apRootSector[iSector]->vFRLengthDens = pPRL->apRootSector[iSector]->vFRSecBiomass * pPlant->pGenotype->fRootLengthRatio;
			    TREES
                    pPRL->apRootSector[iSector]->vGRLengthDens = pPRL->apRootSector[iSector]->vGRSecBiomass * pPGen->vSpecGrossRootLength;

			    TREES
                    pPRL->apRootSector[iSector]->vRootArea = (float)(pPRL->apRootSector[iSector]->vFRLengthDens * FRDiameter * PI 
                    + pPRL->apRootSector[iSector]->vGRLengthDens * GRDiameter * PI);
                else
                    pPRL->apRootSector[iSector]->vRootArea = (float)(pPRL->apRootSector[iSector]->vFRLengthDens * FRDiameter * PI);


                pPRL->apRootSector[iSector]->vBlwgDistrFac = pPRL->apRootSector[iSector]->vRootArea
                                                                /pPRL->vRootArea;

            }//end iSectors

        }//end if iFlagSectors = 2

		pLR  = pLR->pNext;
		pPRL = pPRL->pNext;
	}

	return 1;
	}



// W U R Z E L V E R T E I L U N G    E N D E ///////////////////////////////
