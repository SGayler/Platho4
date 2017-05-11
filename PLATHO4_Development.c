//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"
#include "Platho4.h"

int Development_PLATHO(EXP_POINTER2,int iPlant);

extern double Daylength(double vLatitude, int nJulianDay, int nID);
double Photoperiodism(double vDaylength, double vOptDaylength, double vSensF, LPSTR acDaylenType);
double Vernalization(EXP_POINTER2); //aus CERES



extern	double RelativeTemperatureResponse_PLATHO(double vTemp, double vMinTemp, double vOptTemp, double vMaxTemp);
extern	int PlantArea(PPLANT pPlant, int iPlant);


int Development_PLATHO(EXP_POINTER2,int iPlant)
{
	PPLATHODEVELOP		pPDev	= pPlathoPlant[iPlant]->pPlathoDevelop;
	PPLATHOGENOTYPE		pPGen   = pPlathoPlant[iPlant]->pPlathoGenotype;
	PPLATHOBIOMASS		pPBiom	= pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;
	PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;

	PGENOTYPE			pGen	= pPlant->pGenotype;
	PDEVELOP			pDev	= pPlant->pDevelop;
	PBIOMASS			pBiom	= pPlant->pBiomass;

	double vT, vPh, vVern, vDaylength;
    double TempAkt;
	double TempMax, TempOpt, TempBase;
	double vSeedDepth;
	//float rOpt;
	//float qPar, TempFac, wFac, xExp;
	double TDT[6];
	double TT1, TT2;
	int   i, iStage, iStageOld;
	double vO3; //alphaO3;

	BOOL	bDaylengthSensitive,bVernalization;


	extern int PLATHO_StartValues(PPLANT pPlant, int iPlant, PSPROFILE pSo, PMANAGEMENT pMa );
	extern int GetGrowthCapacities(PPLANT pPlant, int iPlant);

	pDev->iDayAftSow++;

	iStage		= pPDev->iStagePlatho;
	iStageOld	= pPDev->iStagePlatho;


	//aktuelle Temperatur:
	TempAkt = pCl->pWeather->fTempAve;
	if(TempAkt <= -99.0)
		TempAkt = (pCl->pWeather->fTempMax + pCl->pWeather->fTempMin)/2.0;

	//Kardinale Temperaturen für Entwicklung:
	TempMax = pGen->pStageParam->afTempMax[iStage];
	TempOpt = pGen->pStageParam->afTempOpt[iStage];
	TempBase= pGen->pStageParam->afTempBase[iStage];
	
	//qPar = (float)2.7;
	//maximale Entwicklungsrate:
	//rOpt = (float)0.644; 
		//pGen->pStageParam->afMaxDevR[iStage];
	

	//Entwicklungsstadien 

	//Stadium	Getreide/Kartoffel/Sonnenblume:	|	Buche:
	//											|
	//	0:		Juvenil-Stadium					|bis Aufbrechen der Knospen
	//	1:		Bestockung						|bis komplette Entfaltung der Blätter
	//	2:		Schossen						|bis Ende Blattwachstum
	//	3:		Ährenschieben					|bis Beginn Blattabwurf
	//	4:		Fruchtbildung					|bis Ende des Holz-/Früchtewachstums
	//	5:		Reife							|wie Stadium 1

	for(i=0;i<6;i++)
		TDT[i] = pGen->pStageParam->afThermalTime[i];

	HERBS
	{
		if(pMa->pSowInfo->fSowDepth>(float)0.0)
			vSeedDepth = pMa->pSowInfo->fSowDepth;
		else
			vSeedDepth = 6.0;

		if(pDev->iDayAftEmerg ==0)
		{
			WHEAT
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+0.5*vSeedDepth;
			}
			
			BARLEY
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+0.5*vSeedDepth;
			}
			
			SUNFLOWER
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+0.6*vSeedDepth;
			}
			
			MAIZE
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+0.3*vSeedDepth;
			}
			
			POTATO
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+2.0*vSeedDepth;
			}
			
			ALFALFA
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+0.5*vSeedDepth;
			}
			
			LOLIUM
			{
				TDT[0] = pGen->pStageParam->afThermalTime[0]+0.5*vSeedDepth;
			}

			//TDT[0] must be lower than TDT[1]
			TDT[0] = min(TDT[0],TDT[1]-1.0);

		} // end if iDayAftEmerg == 0
	} // end annuals
	


	
	/*
	// "Biologische Zeit" nach Ute Schröder
	//Hilfsgrößen:
	TempFac = (TempMax - TempAkt)/(TempMax - TempOpt);
	wFac = (qPar-(float)1.0) * (TempMax-TempOpt);
	xExp = wFac*wFac * (float)pow(1.0+pow(1.0+40.0/(double)wFac,0.5),2)/(float)400.0;
	
	
	//Entwicklungsrate:
	pDev->fDevR = rOpt * (float)pow(TempFac ,xExp)*(float)exp(xExp*(TempAkt-TempOpt)/(TempMax-TempOpt));

	pDev->fDevStage = pDev->fDevStage + pDev->fDevR;

  */

	TREES
	{
		//Beginn eines neuen Jahres:
		if(pTi->pSimTime->iJulianDay==1)
		{
			pPDev->iStagePlatho = 0;

			pDev->fTimeOfStage1 = (float)0.0;
			pDev->fTimeOfStage2 = (float)0.0;
			pDev->fTimeOfStage3 = (float)0.0;
			pDev->fTimeOfStage4 = (float)0.0;
			pDev->fTimeOfStage5 = (float)0.0;
			pDev->fTimeOfStage6 = (float)0.0;

			pDev->fCumDTT	= (float)0.0;
			pDev->fDevStage = (float)0.0;

			SPRUCE{}
			else
			{
				pBiom->fLeafWeight                   = (float)0.0;
				pPlant->pPltNitrogen->fLeafCont      = (float)0.0; 
				pPlant->pPltNitrogen->fLeafActConc   = (float)0.0;
				pPlant->pCanopy->fPlantLA            = (float)0.0; 
				pPMorph->vPlantLAI                   = 0.0; 
				pPPltStress->vO3LeafConcCum          = 0.0; 
				pPPltStress->vO3LeafConcCumCrit      = 0.0; 
			}

			GetGrowthCapacities(pPlant, iPlant);
		}
	}




	//=========================================================================================
	//		Check whether the crop in this stage is Daylength sensitive or vernalizes
	//=========================================================================================
	bDaylengthSensitive	=FALSE;
	bVernalization		=FALSE;
	
	//Maize, SunvLower, Potato
	if ((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"MZ"))||
		(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"PT"))||
		(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SF")))
	{
		if (iStage==2)
			bDaylengthSensitive=TRUE;
	}

	//Wheat, Barley
	if ((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))||
		(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA")))
    {
    	if (iStage==1)
			bVernalization=TRUE;

		if (iStage==2)
		{
			bDaylengthSensitive=TRUE;
			bVernalization=TRUE;
		}
    }




//	if(pDev->fCumDTT<=TDT[5])
//	{
		vT = RelativeTemperatureResponse_PLATHO(TempAkt,TempBase,TempOpt,TempMax);
			

		//===========================================================================================
		//		Calculate the Daylength effect factor (DF)
		//===========================================================================================
		if (bDaylengthSensitive==TRUE)		//This stage is photoperiod sensitive.
		{
			vDaylength = Daylength(pLo->pFarm->fLatitude, pTi->pSimTime->iJulianDay,3);
			vPh = Photoperiodism(vDaylength, (double)pGen->fOptDaylen, (double)pGen->fDaylenSensitivity, pGen->acDaylenType);
		}
		else 								//This stage is photoperiod unsensitive.
    		vPh=1.0;

		//===========================================================================================
		//		Calculate the vernalization effect factor (VF)
		//===========================================================================================
		if ((bVernalization==TRUE)&&(pPlant->pGenotype->fOptVernDays!=0))	//When vernalization exists
    		vVern=Vernalization(exp_p2);
		else                       //When no vernalization
			vVern=1.0;

		
		pDev->fDTT =  (float)(vT *min(vPh,vVern));
//	}
//	else
//		pDev->fDTT = (float)0.0;

	// O3 - Effekt
	//Verkürzung der biologischen Zeit bis zur Seneszenz (D4) in Abhängigkeit 
	//vom kumulativen Ozonstress (Berechnung in PLATHO_Stress.c)
	
	//Bei maximalem Stress (I=1)wird D4 auf (D3+D4,0)/2 reduziert:
	/*
	alphaO3 = (pGen->pStageParam->afThermalTime[3]-pGen->pStageParam->afThermalTime[2])
				/((float)2.0*pGen->pStageParam->afThermalTime[3]);
	
	vO3 = (float)1.0 - alphaO3*pPPltStress->vO3StressIntensity;
	*/
    if(pPlathoModules->iFlagO3==3)
        vO3 = 1.0 - pPPltStress->vO3Sensitivity*pPPltStress->vO3StressIntensity;
    else
    	vO3 = 1.0;

	if(pDev->fCumDTT<TDT[3])
		TDT[3] = max(pDev->fCumDTT,
							max(vO3*pGen->pStageParam->afThermalTime[3],
							     0.5*(pGen->pStageParam->afThermalTime[3]+pGen->pStageParam->afThermalTime[2])));


	TT1 = (iStage >= 1)? TDT[iStage-1] : 0;
	TT2 = (iStage == 6)? 9999 : TDT[iStage];

	if(iStage<6)
		pDev->fDevR = (float)(pDev->fDTT/(TT2-TT1));
		//pDev->fDevR = (float)min(TT2-pDev->fCumDTT,pDev->fDTT/(TT2-TT1));
	else
		pDev->fDevR = (float)0.0;

	pDev->fDevStage += pDev->fDevR;
	pDev->fCumDTT   += pDev->fDTT;

 
    //für Lysimeter-Versuch:
	if((!lstrcmp((LPSTR)pLo->pFieldplot->acWeatherStation,(LPSTR)"GSF-Lysimeter"))&&
		(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BE")))
	{
    //früher Blattaustrieb 2006 wird nicht simuliert, sondern in Messwerten vorgegeben.
 /*   if(pTi->pSimTime->lTimeDate==pPlant->pPMeasure->lDate)
    {
	    pDev->fDevStage = pPlant->pPMeasure->fDevStage/(float)10.0;
        pPDev->iStagePlatho = (int)pDev->fDevStage;
        iStage = pPDev->iStagePlatho;

	    TT1 = (iStage >= 1)? TDT[iStage-1] : 0;
	    TT2 = (iStage == 6)? 9999 : TDT[iStage];

	    pDev->fCumDTT = (float)(TT1	+ (pPlant->pDevelop->fDevStage-(float)iStage)
																		* (TT2-TT1));
    }
    */
 
    if(pTi->pSimTime->lTimeDate==110505)
    {
	    pDev->fDevStage = (float)1.3;
        pPDev->iStagePlatho = (int)pDev->fDevStage;
        iStage = pPDev->iStagePlatho;

	    TT1 = (iStage >= 1)? TDT[iStage-1] : 0;
	    TT2 = (iStage == 6)? 9999 : TDT[iStage];

	    pDev->fCumDTT = (float)(TT1	+ (pPlant->pDevelop->fDevStage-(float)iStage)
																		* (TT2-TT1));
    }
 
    if(pTi->pSimTime->lTimeDate==260406)
    {
	    pDev->fDevStage = (float)1.0;
        pPDev->iStagePlatho = (int)pDev->fDevStage;
        iStage = pPDev->iStagePlatho;

	    TT1 = (iStage >= 1)? TDT[iStage-1] : 0;
	    TT2 = (iStage == 6)? 9999 : TDT[iStage];

	    pDev->fCumDTT = (float)(TT1	+ (pPlant->pDevelop->fDevStage-(float)iStage)
																		* (TT2-TT1));
    }
	} // Ende 'für Lysimeter-Versuch'
 

    if(pDev->fCumDTT > TDT[5])
       pDev->fDevStage = (float)6.0;


	if(pDev->fCumDTT < TDT[0])
		pPDev->iStagePlatho  = 0;
	else if(pDev->fCumDTT < TDT[3])
	{
	//	pDev->iDayAftEmerg++;

		if(pPDev->iStagePlatho < 1)
		{
			pDev->fTimeOfStage1 = pTi->pSimTime->fTimeAct;
			pPDev->iStagePlatho = 1;
			pPlant->pDevelop->fDevStage = (float)1.0;
			TDT[0] = pGen->pStageParam->afThermalTime[0];

			pDev->fCumVernUnit = (float)0.0;

			HERBS // Initialisierung nach der Keimung
				PLATHO_StartValues(pPlant, iPlant, pSo, pMa);

		}
		if((pDev->fCumDTT > TDT[1])&&(pPDev->iStagePlatho < 2))
		{
			pDev->fTimeOfStage2 = pTi->pSimTime->fTimeAct;
			pPDev->iStagePlatho =2;
		}

		if((pDev->fCumDTT > TDT[2])&&(pPDev->iStagePlatho < 3))
		{
			pDev->fTimeOfStage3 = pTi->pSimTime->fTimeAct;
			pPDev->iStagePlatho =3;
		}

	}
	else if(pDev->fCumDTT < TDT[4])
	{
		if(pPDev->iStagePlatho < 4)
		{
			pDev->fTimeOfStage4 = pTi->pSimTime->fTimeAct;
			pPDev->iStagePlatho =4;
		}
	}
	else if(pDev->fCumDTT < TDT[5])
	{
		if(pPDev->iStagePlatho < 5)
		{
			pDev->fTimeOfStage5 = pTi->pSimTime->fTimeAct;
			pPDev->iStagePlatho = 5;
		}
	}
	else 
	{
		if(pPDev->iStagePlatho < 6)
			pDev->fTimeOfStage6 = pTi->pSimTime->fTimeAct;
			
		pPDev->iStagePlatho = 6;
	}


	//Initialisierungen zu Beginn der verschiedenen Entwicklungsstadien
	if(pPDev->iStagePlatho != iStageOld)
	{
		if(pPDev->iStagePlatho == 1)
		{
			TREES
			{
				SPRUCE{}
				else
				{
					pPMorph->vPlantLAI = 0.1;
				}
			}

            HERBS
            {
                pPlant->pCanopy->fPlantHeight = (float)0.01;
                pPMorph->vStemDiameter = pPlant->pCanopy->fPlantHeight/pPMorph->vHtoDmin;
            }
		}
			if(pPDev->iStagePlatho == 2)
		{
			HERBS
			{

				//initial value of stem weight
	//			pPlant->pBiomass->fStemWeight = (float)0.4 * (pPlant->pBiomass->fLeafWeight 
	//				+ pPlant->pBiomass->fRootWeight);

				//initial value of plant height
				/*
                pPlant->pCanopy->fPlantHeight = (float)pow(pow(pPMorph->vHtoDmin,2) * 4.0 *
					pBiom->fStemWeight/(PI*pPMorph->vStemDensity),1.0/3.0);
                   

				pPMorph->vStemDiameter = pPlant->pCanopy->fPlantHeight/pPMorph->vHtoDmin;
                 */
				PlantArea(pPlant, iPlant);

	//			pPlant->pBiomass->fLeafWeight *= (float)0.6;
	//			pPlant->pBiomass->fRootWeight *= (float)0.6;

/*				POTATO
				{
					pPBiom->vegAbvEnd1  =	pPlant->pBiomass->fStemWeight +
						pPlant->pBiomass->fLeafWeight +
						pBiom->fStemReserveWeight +
						pPlant->pPltCarbon->fCAssimilatePool;
				}*/

			}
		}
		
		if(pPDev->iStagePlatho == 3)
		{
			HERBS
			{
				/*pPlant->pBiomass->fFruitWeight = 
					(float)0.1*(pPlant->pBiomass->fStemWeight+pPlant->pBiomass->fLeafWeight);
				
				pPlant->pBiomass->fStemWeight *= (float)0.9;
				pPlant->pBiomass->fLeafWeight *= (float)0.9;
				*/

				pPBiom->vVegAbvEnd2  =	pPlant->pBiomass->fStemWeight +
										pPlant->pBiomass->fLeafWeight +
										pBiom->fStemReserveWeight +
										pPlant->pPltCarbon->fCAssimilatePool;
			}
		}

		if(pPDev->iStagePlatho == 4)
			pPBiom->vLeafEnd3 = pPlant->pBiomass->fLeafWeight;

		if(pPDev->iStagePlatho == 5)
			pPBiom->vFruitEnd4 = pPlant->pBiomass->fFruitWeight;

	}

	TREES
		pPlant->pDevelop->fAge += (float)(1.0/365.0);

	return 1;
}



double Photoperiodism(double vDaylength, double vOptDaylength, double vSensF, LPSTR acDaylengthType)
	{
		double 	vDaylenEffect, vChi;
		
		if (!lstrcmp((LPSTR)acDaylengthType,(LPSTR)"DNPs"))  
			vDaylenEffect=1.0;
		else 
		{
			if(!lstrcmp((LPSTR)acDaylengthType,(LPSTR)"LDPs"))
				vChi = 1.0;
			if(!lstrcmp((LPSTR)acDaylengthType,(LPSTR)"SDPs"))
				vChi = -1.0;

			if(vSensF!=0.0)
				vDaylenEffect=1.0-exp(-vChi*vSensF*(vDaylength-vOptDaylength+4.0*vChi/vSensF))
				/(1.0-exp(-4.0));
			else
				vDaylenEffect=1.0;
		}
  		   
		return max(0.1,min(1.0,vDaylenEffect));;
	}


double Vernalization(EXP_POINTER2) //aus CERES
	{
	//=========================================================================================
	//Variable Declaration and Intitiation
	//=========================================================================================
	double 	TempAveCrop;
	double 	VF,VD,VD1,VD2,CumVD;

	PDEVELOP pDev=pPlant->pDevelop;

    CumVD=(double)pDev->fCumVernUnit; //Cumulative vernalization till today.

	//=========================================================================================
	//Plant Temperature Calculation
	//=========================================================================================
	//TempAveCrop is the average plant crown temperature.

	TempAveCrop = (double)pCl->pWeather->fTempAve;
	if(TempAveCrop <= (float)-99.0)
		TempAveCrop = (pCl->pWeather->fTempMax + pCl->pWeather->fTempMin)/2.0;


	//=========================================================================================
	//Daily Vernalization Calculation
	//=========================================================================================
    if (pCl->pWeather->fTempMax<=(float)0.0) 		//if fTempMax lower than 0.0C, no vernalization occurs
    	VD=0.0;                 	//VD is the vernalization for a day.
    else                        	//if fTempMax>0.0, vernalization may or may not occur
     	{
		if (pCl->pWeather->fTempMin>(float)15.0)	//if fTempMin>15.0 C, no vernalization occurs.
			VD=0.0;
		else                        //if fTempMin<15.0 C, vernalization may occur.
			{
			if (!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))
				{                   //Vernalization for wheat crops
				VD1=1.4-0.0778*TempAveCrop;
				VD2=0.5+13.44*
							TempAveCrop/((pCl->pWeather->fTempMax-pCl->pWeather->fTempMin+3.0)*
							(pCl->pWeather->fTempMax-pCl->pWeather->fTempMin+3.0));
				}
			else                    //Vernalization for barly crops ?
				{
				VD1=1.0-0.285*(TempAveCrop-11.0);
				VD2=1.0-0.014*(TempAveCrop-9.0)*(TempAveCrop-9.0);
				}

			VD=max(0.0,min(1.0,min(VD1,VD2)));  //Daily vernalization value.
			}
		}

	//=========================================================================================
	//Cumulative Vernalization Days Calculation
	//=========================================================================================
	CumVD=CumVD+VD;	//Cumulative vernalization value (cumulative vernalization days).

	//=========================================================================================
	//Anti-Vernalization
	//=========================================================================================
    if ((CumVD<10.0)&&(pCl->pWeather->fTempMax>(float)30.0))
    	CumVD=CumVD-0.5*(pCl->pWeather->fTempMax-30.0);

    if (CumVD<0.0)
    	CumVD=0.0;

	//=========================================================================================
	//Vernalization Factor
	//=========================================================================================
    //SG/14/06/99:
	//	wegen pPl->pGenotype->iVernCoeff = 0 VF immer gleich 1.
	//VF=1.0-(double)pPl->pGenotype->iVernCoeff*(50.0-CumVD); //vernalization factor.
    VF=min(1.0,1.0-1.0/(double)pPlant->pGenotype->iVernCoeff*(pPlant->pGenotype->fOptVernDays-CumVD)); //vernalization factor.

	if (VF<=0.0) 	VF=0.0;
	if (VF>1.0) 	VF=1.0;

    pDev->fVernEff	=(float)VF;
    pDev->fVernUnit	=(float)VD;
    pDev->fCumVernUnit	=(float)CumVD;

	return VF;
   	}