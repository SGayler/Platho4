//===========================================================================================
//PLATHO_LIB.C-This is the interface for linking the plant process modules of the PLATHO model
//  	  to the Expert-N simulation system. 
//===========================================================================================
//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  <memory.h>

//Module defined include files
#include	"xinclexp.h"
#include	"xn_def.h"   
#include	"xh2o_def.h"  
#include	"Platho4.h"


extern int NewDay(PTIME);
extern int introduce(LPSTR lpName);								// from util_fct.c
extern int SimStart(PTIME);      								// aus util_fkt.c
extern int	WINAPI	Message(long, LPSTR);

#define SimulationStarted  		(SimStart(pTi))

//============================================================================================
//             DLL-Funktionen                       
//============================================================================================

int WINAPI  dllSGDevelopment_PLATHO(EXP_POINTER);
int WINAPI  dllSGRootSystem_PLATHO(EXP_POINTER);
int WINAPI  dllSGPhotosynthesis_PLATHO(EXP_POINTER);
int WINAPI  dllSGPotTransp_PLATHO(EXP_POINTER);
int WINAPI  dllSGActTransp_PLATHO(EXP_POINTER);
int WINAPI  dllSGBiomassGrowth_PLATHO(EXP_POINTER);
int WINAPI  dllSGCanopyForm_PLATHO(EXP_POINTER);
int WINAPI  dllSGOrganSenesz_PLATHO(EXP_POINTER);
int WINAPI  dllSGNitrogenDemand_PLATHO(EXP_POINTER);
int WINAPI  dllSGNitrogenDemand_PLATHO2(EXP_POINTER);
int WINAPI  dllSGNitrogenUptake_PLATHO(EXP_POINTER);

void SetAllPlantRateToZero(PPLANT pPlant,PSPROFILE pSo);
int DevelopmentCheckAndPostHarvestManagement(EXP_POINTER2);

//############################################################################################################
//	THE FOLLOWING FUNCTIONS FROM MODULES IN SUBDIRECTORY PLATHO3
//############################################################################################################
extern int Development_PLATHO(EXP_POINTER2,int iPlant);
extern int RootSystem3_PLATHO(EXP_POINTER2,int iPlant);
extern int Photosynthesis_PLATHO(EXP_POINTER2,int iPlant, int it);
extern int PotentialTranspiration_PLATHO(EXP_POINTER2,int iPlant);                                     
extern int ActualTranspiration_PLATHO(EXP_POINTER2,double,int iPlant);
extern int BiomassGrowth_PLATHO(EXP_POINTER2,int iPlant);
extern int Canopy_PLATHO(EXP_POINTER2,int iPlant);
extern int PlantNitrogenDemand_PLATHO(EXP_POINTER2,int iPlant);
extern int PlantNitrogenDemand_PLATHO2(EXP_POINTER2,int iPlant);
extern int NitrogenUptake_PLATHO(EXP_POINTER2,int iPlant);
extern int NitrogenUptake_PLATHO2(EXP_POINTER2,int iPlant);
extern int PlantNitrogenTranslocation_PLATHO(EXP_POINTER2, int iPlant);

extern int Senescence_PLATHO(EXP_POINTER2,float fPlantDens,int iPlant);
extern int DGL_PLATHO(EXP_POINTER2,int iPlant);

extern int Initialization_PLATHO(EXP_POINTER2,int iPlant);
extern int PLATHO_StartValues(PPLANT pPlant, int iPlant, PSPROFILE pSo, PMANAGEMENT pMa);
extern int Read_GrowthParameters(PPLANT pPlant, int iPlant);
extern int PLATHO_Simulation_Scenario(PTIME, PPLANT, int, PMSOWINFO, PPLATHOSCENARIO, PSPROFILE, PMANAGEMENT, PLOCATION);
extern int PLATHO_Genotype_Read(PPLANT, int);
extern int WINAPI ReadClimateData(PPLATHOCLIMATE pPlathoClimate, PTIME pTi);
extern int PLATHO_Modules();
extern int ReadOutputList(PPLATHOSCENARIO pPlathoScenario);
extern int ReadCropFactors(PWATER pWa, PPLANT pPlant);

extern int WINAPI freeAllocatedMemory(int, int);
extern int WINAPI freeClimate();
extern int WINAPI allocatePlathoVariables(PPLANT, int);
extern int WINAPI allocClimate();

extern	int Weight_kg_ha(PBIOMASS pBiom, float fSowDensity);
extern	int Weight_kg_plant(PBIOMASS pBiom, float fSowDensity);

//=============================================================================================
// DLL-Function:	dllSGDevelopment_PLATHO
// Description:		Phasic development module
// Updating Date:  	                   
//=============================================================================================
int WINAPI  dllSGDevelopment_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR

	int i;

	PPLANT			pPlant;
	PMSOWINFO		pMaSowInfo;

	if SimulationStarted
	{
      	allocatePlathoVariables(pPl, pSo->iLayers-2);
      	
		//Einlesen der Modellkonfiguration
		PLATHO_Modules();
 		
        //Initalisierung
        pPlathoPlant[0]->pPlathoDevelop->bHarvest = FALSE;
 		
		//Einlesen der Szenariodaten
		PLATHO_Simulation_Scenario(pTi, pPl, 0, NULL, pPlathoScenario, pSo, pMa, pLo);

        //Einlesen der Outputliste
        ReadOutputList(pPlathoScenario);

		//Alternativ: Einlesen der hochaufgelösten Klimadaten	
		if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))
		{
			GlobalFree(pPlathoClimate);
			allocClimate();
			ReadClimateData(pPlathoClimate, pTi);
		}
      			
      	introduce((LPSTR)"dllSGDevelopment_PLATHO");
	}

	//Anzahl der Pflanzen im Simulationsobjekt (nPlants)
	if(pTi->pSimTime->fTimeAct == (float)pMa->pSowInfo->iDay)
	{
		for(pPlant=pPl, pMaSowInfo=pMa->pSowInfo, pPlathoScenario->nPlants=0; pPlant!=NULL; 
			pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext)
		{
			pPlathoScenario->nPlants++;
			
			if((pPlant->pBack!=NULL)&&(pPlant->pBack->pModelParam->iHarvestDay<pMaSowInfo->iDay))
				pPlathoScenario->nPlants--;
		}
		
        if(pPlathoScenario->iRows*pPlathoScenario->iColumns != pPlathoScenario->nPlants)
	    {
		    Message(1,"Error in plant number!!!");

		    pPlathoScenario->iRows= (int)max(1,sqrt((float)pPlathoScenario->nPlants));
		    pPlathoScenario->iColumns=pPlathoScenario->nPlants/pPlathoScenario->iRows;
	    }
    	
	    if(pPlathoScenario->nPlants==0) return 0;
 		
    }

    IF_PLANTGROWTH
    {
        // 5.12.2007:
        //Pflanzdichte aller Pflanzen jetzt aus Gitterabstand, der aus dem Szenario *.psc eingelesen wird
        //Grund: bei mehreren Individuen kann über die Datenbank keine Pflanzdichte eingegeben werden,
        //die der realen Dichte entspricht und den richtegen Gitterabstand liefert
        pPlathoScenario->vTotalPlantDensity = (float)(1.0/pow(pPlathoScenario->vLatticeSpacing,2));   //(plants/m^2)

	    //Mittlerer Abstand der Individuen. Verringert sich bei beginnendem Blatt-Wachstum einer neuen Pflanze
 /*       for(pPlant=pPl, pMaSowInfo=pMa->pSowInfo, pPlathoScenario->vTotalPlantDensity = (float)0.0, i=1; 
            (pPlant!=NULL)&&(i<=pPlathoScenario->nPlants); pPlant=pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext,i++)
	    {
		    if ((pPlant != NULL)&&(pMaSowInfo != NULL)&&
			    (pTi->pSimTime->fTimeAct == pMaSowInfo->iDay)) 
		    {
			    pPlathoScenario->vTotalPlantDensity += pMaSowInfo->fPlantDens;
			    if(pPlathoScenario->vTotalPlantDensity>(float)0.0)
                    pPlathoScenario->vLatticeSpacing = (float)(1.0/sqrt(pPlathoScenario->vTotalPlantDensity)); //[m]
		    }
	    }

        */

	    for(pPlant=pPl, pMaSowInfo=pMa->pSowInfo, i=0; (pPlant!=NULL)&&(i<=pPlathoScenario->nPlants-1); 
		    pPlant=pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext,i++)
	    {

		    ///////////////////////////////////////////////////////////////////////
		    //Hilfskonstruktion, da Wert der CSLib-Variablen bMaturity in SGLib nicht bekannt:
		    if ((pPlant->pBack!=NULL)&&(pPlant->pModelParam->pBack->lHarvestDate== pTi->pSimTime->lTimeDate))
			    pPlant->pDevelop->bMaturity = TRUE;


		    if ((pPlant != NULL)&&(pMaSowInfo != NULL)&&
			    (pTi->pSimTime->fTimeAct == pMaSowInfo->iDay)) 
		    {
			    /* Einlesen der genetischen und ökophysiologischen Parameter  */
			    PLATHO_Genotype_Read(pPlant, i);
			    /* Einlesen der Startwerte (Pflanze)  */
			    PLATHO_Simulation_Scenario(pTi, pPlant, i, pMaSowInfo, pPlathoScenario, pSo, pMa, pLo);

			    /* Schalter fuer Wachstumsmodell initialisieren  */
			    pPlant->pDevelop->bMaturity = FALSE;
			    pPlant->pDevelop->bPlantGrowth = TRUE;
		    }
	    }

	    ///////////////////////////////////////////////////////////////////////
	    if (NewDay(pTi)) //Einlesen des Stress-Szenarios
			    PLATHO_Simulation_Scenario(pTi, NULL, -1, NULL, pPlathoScenario, NULL, NULL, pLo);

        /* Einlesen der crop factors */
        if(pPlathoModules->iFlagCropFactors == 2)
        {
            if SimulationStarted
                ReadCropFactors(pWa, pPl);

            if NewYear
                ReadCropFactors(pWa, pPl);
        }

        for(pPlant=pPl, pMaSowInfo=pMa->pSowInfo, i=0; pPlant!=NULL; 
		    pPlant=pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext,i++)
	    {
		    if NewDayAndPlantGrowing2
		    {
    //			PLATHO_Simulation_Scenario(pTi, pPlant, i, pMaSowInfo, pPlathoScenario, pSo, pMa, pLo);
			    Development_PLATHO(exp_p2,i);
		    }

		    DevelopmentCheckAndPostHarvestManagement(exp_p2);
    	
	    }
    }
  	return 1;
	}




//=============================================================================================
// DLL-Function:	dllEWRootSystem_SGSPASS
// Description:		Root system formation module for all crops (Wang,1997)
// Updating Date:  	12.08.97                   
//=============================================================================================
int WINAPI  dllSGRootSystem_PLATHO(EXP_POINTER)
	{
 	DECLARE_COMMON_VAR

	int i;
    
	PPLANT				pPlant; 

	if SimulationStarted
		introduce((LPSTR)"dllSGRootSystem_PLATHO");
	
    IF_PLANTGROWTH
    {
	    for(pPlant = pPl, i=0; pPlant!=NULL; pPlant = pPlant->pNext, i++)
	    {

	        if PlantIsGrowing2
	        {
		        HERBS
		        {
			        if(pPlant->pDevelop->iDayAftEmerg > 0)
				        RootSystem3_PLATHO(exp_p2,i);
		        }
		        TREES
			        RootSystem3_PLATHO(exp_p2,i);
	        }
	    }
    }

   	return 1;
	}



//=============================================================================================
// DLL-Function:	dllSGPhotosynthesis_PLATHO 
//=============================================================================================
int WINAPI  dllSGPhotosynthesis_PLATHO(EXP_POINTER)
	{
		DECLARE_COMMON_VAR
		int i,it;
    
		PPLANT	pPlant;
	 
		if SimulationStarted
			introduce((LPSTR)"dllSGPhotosynthesis_PLATHO");

			
        IF_PLANTGROWTH
        {
			for(pPlant = pPl, i=0; pPlant!=NULL; pPlant = pPlant->pNext, i++)
			{
				if PlantIsGrowing2
				{
					if(pTi->pTimeStep->fAct == (float) 1.0 )
					{
						for (it=1;it<=5;it++)
							Photosynthesis_PLATHO(exp_p2,i,it);
					}
					else
						Photosynthesis_PLATHO(exp_p2,i,0);

				}  // end if PlantIsGrowing2
			} //end for(...)
        } //end IF_PLANTGROWTH
		
		return 1;
	}


int WINAPI  dllSGPotTransp_PLATHO(EXP_POINTER)
	{
	    DECLARE_COMMON_VAR
	    int i;
        
	    PPLANT				pPlant;
	    PMSOWINFO			pMaSowInfo;
     
	    if SimulationStarted
        introduce((LPSTR)"dllSGPotTransp_PLATHO");

	    //Weiterschieben der hochaufgelösten Klimadaten
	    while((pPlathoClimate!=NULL)&&(pPlathoClimate->pNext->vSimTime<=pTi->pSimTime->fTimeAct)) 
		    pPlathoClimate = pPlathoClimate->pNext;

    	
        IF_PLANTGROWTH
        {
            if (pTi->pSimTime->fTimeAct == (float)(pPl->pModelParam->iHarvestDay))
	        {	
                pPlathoPlant[0]->pPlathoDevelop->bHarvest = TRUE;
            }


	        pPlathoAllPlants->dAbvgSpaceOccupation = 0.0;
	        pPlathoAllPlants->dBlwgSpaceOccupation = 0.0;
            pPlathoAllPlants->dPlantsInSimulation = 0.0;

	        //Anteil der von Pflanzen bedeckten Fläche
	        for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo,i=0; pPlant!=NULL; 
			        pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext, i++)
	        {
		        if PlantIsGrowing2
		        {                   
			        PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[i]->pPlathoMorphology;
			        pPlathoAllPlants->dAbvgSpaceOccupation += pPMorph->vCrownArea*pMaSowInfo->fPlantDens; // m^2/m^2
			        pPlathoAllPlants->dBlwgSpaceOccupation += pPMorph->vRootZoneArea*pMaSowInfo->fPlantDens; // m^2/m^2
			        pPlathoAllPlants->dPlantsInSimulation += pMaSowInfo->fPlantDens; // 1/m^2
		        } //end PlantIsGrowing
	        } //end iPlant
                 
            if (pPlathoAllPlants->dPlantsInSimulation> (float)0.0)
            {
                pPlathoAllPlants->dAbvgSpaceOccupation *= pPlathoScenario->vTotalPlantDensity/pPlathoAllPlants->dPlantsInSimulation;
                pPlathoAllPlants->dBlwgSpaceOccupation *= pPlathoScenario->vTotalPlantDensity/pPlathoAllPlants->dPlantsInSimulation;
            }


            for(pPlant=pPl, pMaSowInfo=pMa->pSowInfo, i=0; pPlant!=NULL; 
		        pPlant=pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext,i++)
	        {

	            if PlantIsGrowing2
		        {
			        Weight_kg_plant(pPlant->pBiomass,pMaSowInfo->fPlantDens);
			        pPlant->pDevelop->fDevStage /= (float)10.0;

			        PotentialTranspiration_PLATHO(exp_p2,i);   
 		        }

	        } //end for
        }

   	    return 1;
	}


   
//=============================================================================================
// DLL-Function:	dllSGActTransp_PLATHO
//=============================================================================================
int WINAPI  dllSGActTransp_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR  
	int i, L;
    
	PPLANT		pPlant; 
	PLAYERROOT 	pLR;
	PMSOWINFO	pMaSowInfo;

	if SimulationStarted
		introduce((LPSTR)"dllSGActTransp_PLATHO");

    IF_PLANTGROWTH
    {
	    pPlathoAllPlants->dAbvgSpaceOccupation = 0.0;
	    pPlathoAllPlants->dBlwgSpaceOccupation = 0.0;
        pPlathoAllPlants->dPlantsInSimulation = 0.0;


	    for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo,i=0; pPlant!=NULL; 
		    pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext, i++)
	    {

	    if PlantIsGrowing2
	    {                   
            PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[i]->pPlathoMorphology;

		    pLR	= pPlant->pRoot->pLayerRoot;

		    HERBS
		    {
			    if(pPlant->pDevelop->fDevStage > 1)
				    ActualTranspiration_PLATHO(exp_p2,(double)pMaSowInfo->fPlantDens,i);
		    }
		    TREES
		    {
			  //if(pPlant->pCanopy->fLAI > (float)0.0)   //15052008
                if(pPMorph->vPlantLAI > (float)0.0)
			        ActualTranspiration_PLATHO(exp_p2,(double)pMaSowInfo->fPlantDens,i);
		    }

		    if(pPlant!=pPl) //bei mehreren Pflanzen Aufnahme addieren
			    pPl->pRoot->fUptakeR += pPlant->pRoot->fUptakeR;

	    } //end PlantIsGrowing
	    } //end iPlant

	    //Wasserentnahme aus den Bodenschichten
	    for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo,i=0; pPlant!=NULL; 
		    pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext, i++)
	    {
		    if PlantIsGrowing2
    	    {

				    PSLAYER		pSL	 = pSo->pSLayer->pNext;
				    PWLAYER		pSLW = pWa->pWLayer->pNext; 
				    PLAYERROOT	pLR	 = pPlant->pRoot->pLayerRoot;

				    PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[i]->pPlathoMorphology;

			        for (L=1;L<=pSo->iLayers-2;L++)
			        {
                        //in cm^3/cm^3
                        pSLW->fContAct -=pLR->fActLayWatUpt*pMaSowInfo->fPlantDens/((float)1e3*pSL->fThickness);
                       // pSLW->fContAct -=pLR->fActLayWatUpt*(float)(1e-3*pMaSowInfo->fPlantDens*pPlathoAllPlants->dBlwgSpaceOccupation)/(pSL->fThickness);
                        
			      	    pSLW=pSLW->pNext;
			      	    pLR =pLR ->pNext;
				    }
            } //end if plant is growing   	
	    } //end iPlants
    }
	
	return 1;
}


  
//=============================================================================================
// DLL-Function:	dllEWBiomassGrowth_SGSPASS
// Description:		Maitenance and biomass growth (Wang,1997) 			                            
// Updating Date:  	12.08.97                   
//=============================================================================================
int WINAPI  dllSGBiomassGrowth_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR
	int i;
    
	PPLANT		pPlant;

	if SimulationStarted
       introduce((LPSTR)"dllSGBiomassGrowth_PLATHO");

    IF_PLANTGROWTH
    {
	    for(pPlant = pPl, i=0; pPlant!=NULL; pPlant = pPlant->pNext, i++)
	    {
	        if PlantIsGrowing2
	        {
		        HERBS
		        {
			        if(pPlant->pDevelop->iDayAftEmerg > 0)
				        BiomassGrowth_PLATHO(exp_p2,i);
		        }
		        TREES
			        BiomassGrowth_PLATHO(exp_p2,i);
	        }
	    } //end for
    }

   	return 1;
	}


//=============================================================================================
// DLL-Function:	dllEWCanopyForm_SGSPASS
// Description:		Canopy formation module for cereals (Wang,1997) 			                            
// Updating Date:  	12.08.97                   
//=============================================================================================
int WINAPI  dllSGCanopyForm_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR
	int i;
    
	PPLANT		pPlant;
	PMSOWINFO	pMaSowInfo;

    extern int LightCompetitionFactor(PPLANT, int);
 
	if SimulationStarted
       introduce((LPSTR)"dllSGCanopyForm_PLATHO");

    IF_PLANTGROWTH
    {
    	
	    for(pPlant = pPl, i=0; pPlant!=NULL; pPlant = pPlant->pNext, i++)
	    {

		    if PlantIsGrowing2
		    {		
			    HERBS
			    {
				    if(pPlant->pDevelop->iDayAftEmerg > 0)
					    Canopy_PLATHO(exp_p2,i);
			    }
			    TREES
				    Canopy_PLATHO(exp_p2,i);
		    }
	    } //end for


	    //Calculation of new light copmpetion factors:
	    for(pPlant = pPl, i=0; pPlant!=NULL; pPlant = pPlant->pNext, i++)
	    {
		    if PlantIsGrowing2
		    {
			    LightCompetitionFactor(pPlant,i);
		    }
	    }

        //Calculation of total LAI:
	    pPl->pCanopy->fLAI = (float)0.0;
    	
	    for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo; 
		    (pPlant!=NULL)&&(pTi->pSimTime->fTimeAct>=pMaSowInfo->iDay);
		    pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext)
	    {
		    if PlantIsGrowing2
		    {
			    pPl->pCanopy->fLAI += pPlant->pCanopy->fPlantLA*pMaSowInfo->fPlantDens;
		    }
	    }


    }

	return 1;
	}


//=============================================================================================
// DLL-Function:	dllEWOrganSenesz_SGSPASS
// Description:		Organ senescence (Wang,1997) 			                            
// Updating Date:  	12.08.97                   
//=============================================================================================
int WINAPI  dllSGOrganSenesz_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR
	int i, iPlant, iPlants;
	int	  iPlantLeft, iPlantRight, iPlantUpper, iPlantLower;

    
	PPLANT		pPlant, pPlantLeft, pPlantRight, pPlantUpper, pPlantLower;
	PMSOWINFO	pMaSowInfo;

	extern	int GetPlantNeighbours(PPLANT pPlant, int iPlant, int *iPlantLeft, int *iPlantRight, int *iPlantUpper, int *iPlantLower);
	extern	int	Competition(PPLANT pPlant, PPLANT pPlantUpper, PPLANT pPlantLeft, PPLANT pPlantRight,
					PPLANT pPlantLower, int iPlant, int iPlantUpper, int iPlantLeft, int iPlantRight, 
					int iPlantLower, PSPROFILE pSo);

	if SimulationStarted
       introduce((LPSTR)"dllSGOrganSenesz_PLATHO");

    IF_PLANTGROWTH
    {
	    for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo, i=0;
		    (pPlant!=NULL)&&(pTi->pSimTime->fTimeAct>=pMaSowInfo->iDay);
		    pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext, i++)
	    {
	        if PlantIsGrowing2
	        {
		        HERBS
		        {
			        if(pPlant->pDevelop->iDayAftEmerg > 0)
			        {
			        Senescence_PLATHO(exp_p2, pMaSowInfo->fPlantDens, i);
			        DGL_PLATHO(exp_p2,i);
			        }
		        }
		        TREES
		        {
			        Senescence_PLATHO(exp_p2,pMaSowInfo->fPlantDens,i);
			        DGL_PLATHO(exp_p2,i);
		        }

		        Weight_kg_ha(pPlant->pBiomass,pMaSowInfo->fPlantDens);
		        pPlant->pDevelop->fDevStage *= (float)10.0;
	        }
	    } //end for



	    for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo, iPlant=0; pPlant!=NULL;
		    pPlant = pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext, iPlant++)
	    {
		    if PlantIsGrowing2
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

		    GetPlantNeighbours(pPlant, iPlant, &iPlantLeft, &iPlantRight, 
													    &iPlantUpper, &iPlantLower);

		    pPlantLeft	= 	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLeft;
		    pPlantRight	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantRight;
		    pPlantUpper	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantUpper;
		    pPlantLower	=	pPlathoPlant[iPlant]->pPlantNeighbours->pPlantLower ;



		    Competition(pPlant, pPlantUpper, pPlantLeft, pPlantRight, pPlantLower, 
					    iPlant, iPlantUpper, iPlantLeft, iPlantRight, iPlantLower, pSo);
		    }
	    }
    }
	//////////////////////////////////////////////////////////////////////////////////

	if ((pTi->pSimTime->fTimeAct+pTi->pTimeStep->fAct>=(float)(pTi->pSimTime->iSimDuration)))
	{
		//Speicher freigeben, falls hochaufgelöste Klimadaten eingelesen wurden:
		if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))
			freeClimate();

		// Freigeben der übrigen PLATHO-Variablen:
		iPlants = pPlathoScenario->iRows*pPlathoScenario->iColumns;
		freeAllocatedMemory(iPlants,pSo->iLayers-2);
	}

	return 1;
	}



//=============================================================================================
// DLL-Function:	dllSGNitrogenDemand_PLATHO
// Description:		Plant nitrogen demand module 		                            
//=============================================================================================
int WINAPI  dllSGNitrogenDemand_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR
	int i;
    
	PPLANT		pPlant;
	 
  //  if(pPlathoModules->iFlagNEffectGrw==2)// N effect on growth included
  //  {
	    if SimulationStarted
		    introduce((LPSTR)"dllSGNitrogenDemand_PLATHO");

        IF_PLANTGROWTH
        {
	        for(pPlant = pPl, i=0; pPlant!=NULL; pPlant = pPlant->pNext, i++)
	        {
	            if PlantIsGrowing2
	            {		
		            HERBS
		            {
			            if(pPlant->pDevelop->fDevStage > 0)
				            PlantNitrogenDemand_PLATHO(exp_p2,i);
		            }
		            TREES
			            PlantNitrogenDemand_PLATHO(exp_p2,i);
	            }
	        } //end for
        }
   // } // end iFlagNEffectGrw==1

	return 1;
	}



 
//=============================================================================================
// DLL-Function:	dllSGNitrogenUptake_PLATHO
// Description:		Plant nitrogen demand module (Wang,1997) 			                            
// Updating Date:  	12.08.97                   
//=============================================================================================
int WINAPI  dllSGNitrogenUptake_PLATHO(EXP_POINTER)
	{
	DECLARE_COMMON_VAR
	int i;
    
	PPLANT		pPlant;
	PMSOWINFO	pMaSowInfo;

  //  if(pPlathoModules->iFlagNEffectGrw==2)// N effect on growth included
  //  {

	    if SimulationStarted
		    introduce((LPSTR)"dllSGNitrogenUptake_PLATHO2");

        IF_PLANTGROWTH
        {
	        for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo, i=0; pPlant!=NULL; 
			        pPlant = pPlant->pNext,pMaSowInfo=pMaSowInfo->pNext, i++)
	        {

	            if PlantIsGrowing2
	            {		
		            HERBS
		            {
			            if(pPlant->pDevelop->fDevStage > 1)
				            NitrogenUptake_PLATHO(exp_p2,i);
		            }
		            TREES
			            NitrogenUptake_PLATHO(exp_p2,i);
	            }

	            if PlantIsGrowing2
	            {		
		            HERBS
		            {
			            if(pPlant->pDevelop->iDayAftEmerg > 0)
				            PlantNitrogenTranslocation_PLATHO(exp_p2,i);
		            }
		            TREES
			            PlantNitrogenTranslocation_PLATHO(exp_p2,i);
	            }
	        }

	        for(pPlant = pPl, pMaSowInfo=pMa->pSowInfo, i=0; pPlant!=NULL; 
			        pPlant = pPlant->pNext,pMaSowInfo=pMaSowInfo->pNext, i++)
	        {
        		
	        if PlantIsGrowing2
    	        {
		        int L;
		        PLAYERROOT	pLR	=pPlant->pRoot->pLayerRoot;
		        PCLAYER 	pSLN=pCh->pCLayer->pNext;

		        for (L=1;L<=pSo->iLayers-2;L++)
			        {
		            //Check the whether there are roots in this layer:
		            if (pLR->fLengthDens==(float)0.0)		break;

			        //Nitrogen in layer L: SNO3,SNH4 (kg N/ha)
			        pSLN->fNO3N=pSLN->fNO3N-pLR->fActLayNO3NUpt*pMaSowInfo->fPlantDens*(float)1e4;
			        pSLN->fNH4N=pSLN->fNH4N-pLR->fActLayNH4NUpt*pMaSowInfo->fPlantDens*(float)1e4;

			        pLR =pLR ->pNext;
			        pSLN=pSLN->pNext;
			        }
    	        }

	        } //end for

    						
	        // Aktuelle N-Aufnahmerate durch alle Pflanzen [kg/ha] (für Bilanzierung)
	        pPl->pPltNitrogen->fActNUptR = 
				        pPl->pPltNitrogen->fActNUptR*pMa->pSowInfo->fPlantDens * (float)1e4;

	        for(pPlant = pPl->pNext, pMaSowInfo=pMa->pSowInfo->pNext, i=0; pPlant!=NULL; 
			        pPlant = pPlant->pNext,pMaSowInfo=pMaSowInfo->pNext, i++)
	        {
		        if PlantIsGrowing2
		        {
			        pPl->pPltNitrogen->fActNUptR += 
				        pPlant->pPltNitrogen->fActNUptR*pMaSowInfo->fPlantDens * (float)1e4;
		        }
	        }
        }  //end IF_PLANTGROWTH
    //} // end iFlagNEffectGrw=2

	return 1;
	}

	
//=============================================================================================
// Internal funciton:	SetAllPlantRateToZero
// Description:			If plant matures all rates are set to zero
// Updating Date:  		12.10.97                   
//=============================================================================================
void SetAllPlantRateToZero(PPLANT pPlant, PSPROFILE pSo)
	{        
	int L;
	PLAYERROOT	pLayRoot;
	//--------------------------------------------------------------------------------------
	//Development Stage
	//--------------------------------------------------------------------------------------
		pPlant->pDevelop->fDevR		=(float)0.0;

	//--------------------------------------------------------------------------------------
	//Canopy
	//--------------------------------------------------------------------------------------
		pPlant->pCanopy->fLAGrowR		=(float)0.0;
		pPlant->pCanopy->fLeafSenesR	=(float)0.0;

	//--------------------------------------------------------------------------------------
	//Biomass
	//--------------------------------------------------------------------------------------
		pPlant->pBiomass->fLeafGrowR	=(float)0.0;
		pPlant->pBiomass->fStemGrowR	=(float)0.0;
		pPlant->pBiomass->fRootGrowR	=(float)0.0;
		pPlant->pBiomass->fGrainGrowR	=(float)0.0;
		pPlant->pBiomass->fGrossRootGrowR	=(float)0.0;
		pPlant->pBiomass->fBranchGrowR	=(float)0.0;
		pPlant->pBiomass->fTuberGrowR	=(float)0.0;

		pPlant->pBiomass->fStemReserveGrowRate	=(float)0.0;

		pPlant->pBiomass->fLeafDeathRate		=(float)0.0;
		pPlant->pBiomass->fRootDeathRate		=(float)0.0;
		pPlant->pBiomass->fStemDeathRate		=(float)0.0;
		pPlant->pBiomass->fFruitDeathRate		=(float)0.0;
		pPlant->pBiomass->fGrossRootDeathRate		=(float)0.0;
		pPlant->pBiomass->fBranchDeathRate		=(float)0.0;
		pPlant->pBiomass->fTuberDeathRate		=(float)0.0;
       
	//--------------------------------------------------------------------------------------
	//Transpiration
	//--------------------------------------------------------------------------------------
		pPlant->pPltWater->fPotTranspdt =(float)0.0;
		pPlant->pPltWater->fActTranspdt =(float)0.0;

	//--------------------------------------------------------------------------------------
	//Plant water and nitrogen information
	//--------------------------------------------------------------------------------------
		pPlant->pPltWater->fActTranspdt	=(float)0.0;
		pPlant->pRoot->fUptakeR 			=(float)0.0;
		
		pPlant->pPltNitrogen->fActNUptR	=(float)0.0;
		pPlant->pPltNitrogen->fActNUpt		=(float)0.0;
		pPlant->pPltNitrogen->fActNO3NUpt	=(float)0.0;
		pPlant->pPltNitrogen->fActNH4NUpt	=(float)0.0;

		pPlant->pPltNitrogen->fLeafNinc		=(float)0.0;
		pPlant->pPltNitrogen->fStemNinc		=(float)0.0;
		pPlant->pPltNitrogen->fRootNinc		=(float)0.0;
		pPlant->pPltNitrogen->fGrainNinc	=(float)0.0;
		pPlant->pPltNitrogen->fGrossRootNinc=(float)0.0;
		pPlant->pPltNitrogen->fBranchNinc	=(float)0.0;
		pPlant->pPltNitrogen->fTuberNinc	=(float)0.0;

		pPlant->pPltNitrogen->fLeafNtransRate		=(float)0.0;
		pPlant->pPltNitrogen->fStemNtransRate		=(float)0.0;
		pPlant->pPltNitrogen->fRootNtransRate		=(float)0.0;
		pPlant->pPltNitrogen->fBranchNtransRate	=(float)0.0;
		pPlant->pPltNitrogen->fGrossRootNtransRate=(float)0.0;



	//--------------------------------------------------------------------------------------
	//Root Information
	//--------------------------------------------------------------------------------------
		pPlant->pRoot->fDepthGrowR	=(float)0.0;

		pLayRoot = pPlant->pRoot->pLayerRoot;
		for (L=1;L<=pSo->iLayers-2;L++)
			{
			pLayRoot->fLengthDensR	=(float)0.0;
//			pRt->fDieLength[L]		=(float)0.0;

			pLayRoot->fActLayWatUpt	=(float)0.0;
			pLayRoot->fActLayNO3NUpt=(float)0.0;
			pLayRoot->fActLayNH4NUpt=(float)0.0;
			pLayRoot->fActLayNUpt	=(float)0.0;
			

			
			pLayRoot = pLayRoot->pNext;
			} 


	return;
	} 
					   
	int DevelopmentCheckAndPostHarvestManagement(EXP_POINTER2)
	{
		PDEVELOP pDev = pPlant->pDevelop;

		if NewDayAndPlantGrowing2
		{
			if (pDev->fDevStage>=(float)1.0)
				pDev->iDayAftEmerg++;
				
			HERBS
			{
				if (pDev->fDevStage>=(float)6.0)
				{
				pPlant->pDevelop->bMaturity=TRUE;
				SetAllPlantRateToZero(pPlant,pSo);
				}
			}
		}		
  		

//	if (pTi->pSimTime->fTimeAct == (float)(pPlant->pModelParam->iHarvestDay-1))
	if (pTi->pSimTime->fTimeAct == (float)(pPlant->pModelParam->iHarvestDay))
	{	
		float *pMat1Local,factor,amount,actDepth,RootProp,RootSum;
		int    i1,i2;
		PSLAYER pSL;								    
		PCLAYER pCL;

		pPlant->pDevelop->bPlantGrowth = FALSE;
		pPlant->pDevelop->bMaturity = TRUE;
		i1 = (int)99; //aktuelle Bodenschichten
		RootSum = actDepth = (float)0.0;


		pMat1Local        = (float *) malloc(i1 * sizeof(float));
		memset(pMat1Local,0x0,(i1 * sizeof(float)));
		
		// The aboveground biomass at iHarvestDay must be written 
		// to the surfacepools and the rootbiomass to the Litterpools
		if(pPlant->pModelParam->cResidueCarryOff==0)
		{
		pCh->pCProfile->fDryMatterLitterSurf += pPlant->pBiomass->fBiomassAbvGround-pPlant->pBiomass->fGrainWeight;
		pCh->pCProfile->fNLitterSurf += pPlant->pPltNitrogen->fCumActNUpt;
		pCh->pCProfile->fCLitterSurf += (pPlant->pBiomass->fBiomassAbvGround-pPlant->pBiomass->fGrainWeight) * (float)0.45 ; // 45% C in Biomasse
		
		if (pCh->pCProfile->fNLitterSurf > (float)0.0)
		pCh->pCProfile->fCNLitterSurf = pCh->pCProfile->fCLitterSurf /
													pCh->pCProfile->fNLitterSurf; 
		}

		pMa->pLitter->fRootC = pPlant->pBiomass->fRootWeight;


		if (pPlant->pPltNitrogen->fRootCont > (float) 0)
		{
		 pMa->pLitter->fRootCN = pPlant->pBiomass->fRootWeight/pPlant->pPltNitrogen->fRootCont;
		}
		else
		{
		 pMa->pLitter->fRootCN =(float)0.1;// ep 250399 fRootCN=0.1 overestimates NLitter
		}


	   // Berechnen schichtmaessiger Anteil. Zur Vereinfachung 
	   // Variable pCL->fCLitter benutzt um Anteile zu uebergeben. 

		for (pSL = pSo->pSLayer->pNext,
    	 pCL = pCh->pCLayer->pNext,i2=0;
                         ((pSL->pNext != NULL)&&
                          (pCL->pNext != NULL));
         pSL = pSL->pNext,
         pCL = pCL->pNext,i2++)
		{
		  actDepth = actDepth + (pSL->fThickness / 10);
		  RootProp =	(float)exp((float)-3.0 * actDepth / (pSo->fDepth / (float)10.0));
		  RootSum += RootProp;
	   // pCL->fCLitter wurde auf -99 initialisiert; wenn nein in Modell-
	   // parameter-File eingelesen und sollte nicht ueberschrieben werden   
		  pMat1Local[i2] = RootProp;
		}

		// Anteilsmaessige Verteilung auf die Bodenschichten. 
    for (pSL = pSo->pSLayer->pNext,
    	 pCL = pCh->pCLayer->pNext,i2=0;
                         ((pSL->pNext != NULL)&&
                          (pCL->pNext != NULL));
         pSL = pSL->pNext,
         pCL = pCL->pNext,i2++)
		{
		  factor = pMat1Local[i2] / RootSum;
          amount = pMa->pLitter->fRootC * factor;
		// wurde auf -99 initialisiert; wenn nein in Modellparameter-
	    //  File eingelesen und sollte nicht ueberschrieben werden   

		  pCL->fCLitter += amount;
		  pCL->fNLitter += amount / pMa->pLitter->fRootCN;
		  
		  if((pCL->fCLitter>EPSILON)&&(pCL->fNLitter>EPSILON))  
		  	{
		  	pCL->fLitterCN = pCL->fCLitter / pCL->fNLitter;          
		  	}             
		  	else
		  	pCL->fLitterCN =(float)0.1;

		}

		SetAllPlantRateToZero(pPlant,pSo);
	}

	return 1;
	}