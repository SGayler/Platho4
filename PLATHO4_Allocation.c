
//	Modul zum Allokieren von Speicherplatz für die 
//	PLATHO-spezifischen Variablenstrukturen
//
//	Definition der Strukturen siehe Platho4.h

//#include <stdio.h>
//#include <windows.h>
//#include <string.h>
//#include <stdlib.h>

//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "xinclexp.h"
#include "Platho4.h"


int WINAPI allocatePlathoVariables(PPLANT, int);
int WINAPI freeAllocatedMemory(int,int);

int WINAPI allocPlathoGenotype();
int WINAPI allocPltMaterial();
int WINAPI allocFalgePS();
int WINAPI allocFineRoot();
int WINAPI allocGrossRoot();
int WINAPI allocStem();
int WINAPI allocBranch();
int WINAPI allocLeaf();
int WINAPI allocFruit();
int WINAPI allocTuber();
int WINAPI allocPlathoDevelop();
int WINAPI allocPlathoBiomass();
int WINAPI allocPlathoPartitioning();
int WINAPI allocLeafLayer(int);
int WINAPI allocLeafSector();
int WINAPI allocPlathoRootLayer(int);
int WINAPI allocRootSector();
int WINAPI allocPlathoMorphology(int);
int WINAPI allocPlathoNitrogen();
int WINAPI allocPlantStress();
int WINAPI allocPlantNeighbours();
int WINAPI allocPlantStructures();
int WINAPI allocPlantLeft();
int WINAPI allocPlantRight();
int WINAPI allocPlantUpper();
int WINAPI allocPlantLower();
int	WINAPI allocGrowthEfficiency();
int	WINAPI allocCO2Required();
int	WINAPI allocCarbon();
int	WINAPI allocBiochemistry();
int WINAPI allocPlathoAllPlants();
int WINAPI allocPlathoScen();
int WINAPI allocClimate();
int WINAPI allocPlathoMod();

int WINAPI freePlathoGenotype(PPLATHOGENOTYPE);
int WINAPI freePltMaterial(PPLANTMATERIAL);
int WINAPI freeLeafLayer(PLEAFLAYER);
int WINAPI freePlathoRootLayer(PPLATHOROOTLAYER);
int WINAPI freePlathoMorphology(PPLATHOMORPHOLOGY,int);
int	WINAPI freeBiochemistry();
int WINAPI freeClimate();

PPOOLS				pFineRoot;
PPOOLS				pGrossRoot;
PPOOLS				pStem;
PPOOLS				pBranch;
PPOOLS				pLeaf;
PPOOLS				pFruit;
PPOOLS				pTuber;
PPLANTMATERIAL		pPltMaterial;
PFALGEPS            pFalgePS;
PPLATHOGENOTYPE		pPlathoGenotype;
PPLATHODEVELOP		pPlathoDevelop;
PPLATHOBIOMASS		pPlathoBiomass;
PPLATHOPARTITIONING pPlathoPartitioning;
PPLATHOROOTLAYER	pPlathoRootLayer;
PROOTSECTOR         pRootSector;
PLEAFLAYER          pLeafLayer;
PLEAFSECTOR         pLeafSector;
PPLATHOMORPHOLOGY	pPlathoMorphology;
PPLATHONITROGEN		pPlathoNitrogen;
PPLANTSTRESS		pPlantStress;
PPLANTNEIGHBOURS	pPlantNeighbours;
PPLANT				pPlantLeft;
PPLANT				pPlantRight;
PPLANT				pPlantUpper;
PPLANT				pPlantLower;
PGENOTYPE			pGenotype;
PDEVELOP			pDevelop;
PBIOMASS			pBiomass;
PCANOPY				pCanopy;
PROOT				pRoot;
PPLTCARBON			pPltCarbon;
PPLTWATER			pPltWater;
PPLTNITROGEN		pPltNitrogen;
PPLTCLIMATE			pPltClimate;	
PMODELPARAM			pModelParam;
PPLTMEASURE			pPMeasure;
PPLATHOPLANT		pPlathoPlant[MAXPLANTS];
PPLATHOALLPLANTS    pPlathoAllPlants;

PGROWTHEFFICIENCY	pGrowthEfficiency;
PCO2REQUIRED		pCO2Required;
PCARBON				pCarbon;
PBIOCHEMISTRY		pBiochemistry;

PPLATHOSCENARIO		pPlathoScenario;
PPLATHOCLIMATE		pPlathoClimate;
PPLATHOMODULES		pPlathoModules;


int WINAPI allocatePlathoVariables(PPLANT pPl, int iRootLayers)
{
	int i;
	PPLANT pPlant;

	GlobalFree(pPlathoAllPlants);
	allocPlathoAllPlants();

	GlobalFree(pPlathoScenario);
	allocPlathoScen();

	GlobalFree(pPlathoModules);
	allocPlathoMod();

	GlobalFree(pBiochemistry);
	allocBiochemistry();

	
		GlobalFree(pFineRoot);
		GlobalFree(pGrossRoot);
		GlobalFree(pStem);
		GlobalFree(pBranch);
		GlobalFree(pLeaf);
		GlobalFree(pFruit);
		GlobalFree(pTuber);
		GlobalFree(pFalgePS);
		GlobalFree(pPltMaterial);
		GlobalFree(pPlathoGenotype);
		GlobalFree(pPlathoDevelop);
		GlobalFree(pPlathoBiomass);
		GlobalFree(pPlathoPartitioning);
		GlobalFree(pLeafLayer);
		GlobalFree(pPlathoRootLayer);
		GlobalFree(pPlathoMorphology);
		GlobalFree(pPlathoNitrogen);
		GlobalFree(pPlantStress);
			
		GlobalFree(pPlantLeft);
		GlobalFree(pPlantRight);
		GlobalFree(pPlantUpper);
		GlobalFree(pPlantLower);
		GlobalFree(pPlantNeighbours);
	
		GlobalFree(pGenotype);
		GlobalFree(pDevelop);
		GlobalFree(pBiomass);
		GlobalFree(pCanopy);
		GlobalFree(pRoot);
		GlobalFree(pPltCarbon);
		GlobalFree(pPltWater);
		GlobalFree(pPltNitrogen);
		GlobalFree(pPltClimate);
		GlobalFree(pModelParam);
		GlobalFree(pPMeasure);




	// Speicher allokieren entsprechend der 
	// Pflanzenzahl im Simulationsprojekt:
	for(pPlant=pPl, i=0;pPlant!=NULL; pPlant=pPlant->pNext, i++)
	{

		GlobalFree(pPlathoPlant[i]);
		GlobalUnlock(pPlathoPlant[i]);
		pPlathoPlant[i] = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoPlant));
	

		allocPlathoGenotype();
		allocPlathoDevelop();
		allocPlathoBiomass();
		allocPlathoPartitioning();
		allocPlathoMorphology(iRootLayers);
		allocPlathoNitrogen();
		allocPlantStress();
		allocPlantNeighbours();

		pPlathoPlant[i]->pPlathoGenotype		= pPlathoGenotype;
		pPlathoPlant[i]->pPlathoDevelop			= pPlathoDevelop;
		pPlathoPlant[i]->pPlathoBiomass			= pPlathoBiomass;
		pPlathoPlant[i]->pPlathoPartitioning	= pPlathoPartitioning;
		pPlathoPlant[i]->pPlathoMorphology		= pPlathoMorphology;
		pPlathoPlant[i]->pPlathoNitrogen		= pPlathoNitrogen;
		pPlathoPlant[i]->pPlantStress			= pPlantStress;
		pPlathoPlant[i]->pPlantNeighbours		= pPlantNeighbours;
	}


	return 1;
}



int WINAPI allocPlathoGenotype()
{
	GlobalUnlock(pPlathoGenotype);

	pPlathoGenotype = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoGenotype));

	allocFalgePS();
    allocPltMaterial();

	pPlathoGenotype->pFalgePS = pFalgePS;
	pPlathoGenotype->pPltMaterial = pPltMaterial;

	return 1;
}

int WINAPI allocFalgePS()
{
    GlobalUnlock(pFalgePS);
	pFalgePS= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stFalgePS));

    return 1;
}

int WINAPI allocPltMaterial()
{

	GlobalUnlock(pPltMaterial);
	pPltMaterial= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlantMaterial));

	allocFineRoot();
	allocGrossRoot();
	allocStem();
	allocBranch();
	allocLeaf();
	allocFruit();
	allocTuber();

	pPltMaterial->pFineRoot		= pFineRoot;
	pPltMaterial->pGrossRoot	= pGrossRoot;
	pPltMaterial->pStem			= pStem;
	pPltMaterial->pBranch		= pBranch;
	pPltMaterial->pLeaf			= pLeaf;
	pPltMaterial->pFruit		= pFruit;
	pPltMaterial->pTuber		= pTuber;

	return 1;
}


int WINAPI allocFineRoot()
{
	GlobalUnlock(pFineRoot);
	pFineRoot= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}

int WINAPI allocGrossRoot()
{
	GlobalUnlock(pGrossRoot);
	pGrossRoot= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}

int WINAPI allocStem()
{
	GlobalUnlock(pStem);
	pStem= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}

int WINAPI allocBranch()
{
	GlobalUnlock(pBranch);
	pBranch= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}

int WINAPI allocLeaf()
{
	GlobalUnlock(pLeaf);
	pLeaf= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}

int WINAPI allocFruit()
{
	GlobalUnlock(pFruit);
	pFruit= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}

int WINAPI allocTuber()
{
	GlobalUnlock(pTuber);
	pTuber= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPools));
	return 1;
}


int WINAPI allocPlathoDevelop()
{
	GlobalUnlock(pPlathoDevelop);
	pPlathoDevelop = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoDevelop));
	return 1;
}


int WINAPI allocPlathoBiomass()
{
	GlobalUnlock(pPlathoBiomass);
	pPlathoBiomass = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoBiomass));
	return 1;
}


int WINAPI allocPlathoPartitioning()
{
	GlobalUnlock(pPlathoPartitioning);
	pPlathoPartitioning = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoPartitioning));
	return 1;
}


int WINAPI allocPlathoMorphology(int iRootLayers)
{
	GlobalUnlock(pPlathoMorphology);
	pPlathoMorphology = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoMorphology));
//	pPlathoMorphology = GlobalAlloc(GMEM_ZEROINIT,2988);

	allocPlathoRootLayer(iRootLayers);
	allocLeafLayer(LEAFLAYERS);

	pPlathoMorphology->pPlathoRootLayer	= pPlathoRootLayer;
	pPlathoMorphology->pLeafLayer		= pLeafLayer;

	return 1;
}

int	WINAPI allocLeafLayer(int iLeafLayers)
{
	int i, j;
	PLEAFLAYER	pLLayer[LEAFLAYERS], pLL;

	GlobalUnlock(pLeafLayer);


	for(i=0;i<iLeafLayers;i++)//Anzahl der Einträge
	{
		pLLayer[i] = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stLeafLayer));

		if(i==0)
		{
			pLeafLayer		= pLLayer[i];

            for(j=0;j<4;j++)//4 Sectors
            {
                allocLeafSector();
                pLeafLayer->apLeafSector[j] = pLeafSector;
            }
			
			pLeafLayer->pBack	= NULL;
			pLeafLayer->pNext	= NULL;
		}
		else
		{
			pLL = pLeafLayer->pNext = pLLayer[i];
			pLL->pBack = pLeafLayer;

            for(j=0;j<4;j++)//4 Sectors
            {
                allocLeafSector();
                pLL->apLeafSector[j] = pLeafSector;
            }
			
            pLeafLayer = pLL;
			pLL=pLL->pNext;
		}
	}

	pLeafLayer = pLLayer[0];

	return 1;
}

int WINAPI allocLeafSector()
{
 	GlobalUnlock(pLeafSector);
	pLeafSector = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stLeafSector));
    return 1;
}


int	WINAPI allocPlathoRootLayer(int iRootLayers)
{
	int i, j;
	PPLATHOROOTLAYER	pPRLayer[MAXSOILLAYERS], pPRL;

	GlobalUnlock(pPlathoRootLayer);


	for(i=0;i<iRootLayers;i++)//Anzahl der Einträge
	{
		pPRLayer[i] = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoRootLayer));

		if(i==0)
		{
			pPlathoRootLayer		= pPRLayer[i];

            for(j=0;j<4;j++)//4 Sectors
            {
                allocRootSector();
                pPlathoRootLayer->apRootSector[j] = pRootSector;
            }
			
			pPlathoRootLayer->pBack	= NULL;
			pPlathoRootLayer->pNext	= NULL;
		}
		else
		{
			pPRL = pPlathoRootLayer->pNext = pPRLayer[i];
			pPRL->pBack = pPlathoRootLayer;

            for(j=0;j<4;j++)//4 Sectors
            {
                allocRootSector();
                pPRL->apRootSector[j] = pRootSector;
            }
			
            pPlathoRootLayer = pPRL;
			pPRL=pPRL->pNext;
		}
	}

	pPlathoRootLayer = pPRLayer[0];

	return 1;
}


int WINAPI allocRootSector()
{
 	GlobalUnlock(pRootSector);
	pRootSector = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stRootSector));
    return 1;
}

int WINAPI allocPlathoNitrogen()
{
	GlobalUnlock(pPlathoNitrogen);
	pPlathoNitrogen = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoNitrogen));
	return 1;
}


int WINAPI allocPlantStress()
{
	GlobalUnlock(pPlantStress);
	pPlantStress = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlantStress));
	return 1;
}

int WINAPI allocPlantNeighbours()
{
	GlobalUnlock(pPlantNeighbours);
	pPlantNeighbours = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlantNeighbours));

/*	allocPlantLeft();
	allocPlantRight();
	allocPlantUpper();
	allocPlantLower();

	pPlantNeighbours->pPlantLeft	= pPlantLeft;
	pPlantNeighbours->pPlantRight	= pPlantRight;
	pPlantNeighbours->pPlantUpper	= pPlantUpper;
	pPlantNeighbours->pPlantLower	= pPlantLower;    */

	return 1;
}


int WINAPI allocPlantLeft()
{
	GlobalUnlock(pPlantLeft);
	pPlantLeft = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplant));

	allocPlantStructures();

	pPlantLeft->pGenotype	= pGenotype;
	pPlantLeft->pDevelop	= pDevelop;
	pPlantLeft->pBiomass	= pBiomass;
	pPlantLeft->pCanopy		= pCanopy;
	pPlantLeft->pRoot		= pRoot;
	pPlantLeft->pPltCarbon	= pPltCarbon;
	pPlantLeft->pPltWater	= pPltWater;
	pPlantLeft->pPltNitrogen= pPltNitrogen;
	pPlantLeft->pPltClimate	= pPltClimate;
	pPlantLeft->pModelParam	= pModelParam;
	pPlantLeft->pPMeasure	= pPMeasure;

	return 1;
}

int WINAPI allocPlantRight()
{
	GlobalUnlock(pPlantRight);
	pPlantRight = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplant));

	allocPlantStructures();

	pPlantRight->pGenotype		= pGenotype;
	pPlantRight->pDevelop		= pDevelop;
	pPlantRight->pBiomass		= pBiomass;
	pPlantRight->pCanopy		= pCanopy;
	pPlantRight->pRoot			= pRoot;
	pPlantRight->pPltCarbon		= pPltCarbon;
	pPlantRight->pPltWater		= pPltWater;
	pPlantRight->pPltNitrogen	= pPltNitrogen;
	pPlantRight->pPltClimate	= pPltClimate;
	pPlantRight->pModelParam	= pModelParam;
	pPlantRight->pPMeasure		= pPMeasure;

	return 1;
}

int WINAPI allocPlantUpper()
{
	GlobalUnlock(pPlantUpper);
	pPlantUpper = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplant));

	allocPlantStructures();

	pPlantUpper->pGenotype		= pGenotype;
	pPlantUpper->pDevelop		= pDevelop;
	pPlantUpper->pBiomass		= pBiomass;
	pPlantUpper->pCanopy		= pCanopy;
	pPlantUpper->pRoot			= pRoot;
	pPlantUpper->pPltCarbon		= pPltCarbon;
	pPlantUpper->pPltWater		= pPltWater;
	pPlantUpper->pPltNitrogen	= pPltNitrogen;
	pPlantUpper->pPltClimate	= pPltClimate;
	pPlantUpper->pModelParam	= pModelParam;
	pPlantUpper->pPMeasure		= pPMeasure;

	return 1;
}

int WINAPI allocPlantLower()
{
	GlobalUnlock(pPlantLower);
	pPlantLower= GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplant));

	allocPlantStructures();

	pPlantLower->pGenotype		= pGenotype;
	pPlantLower->pDevelop		= pDevelop;
	pPlantLower->pBiomass		= pBiomass;
	pPlantLower->pCanopy		= pCanopy;
	pPlantLower->pRoot			= pRoot;
	pPlantLower->pPltCarbon		= pPltCarbon;
	pPlantLower->pPltWater		= pPltWater;
	pPlantLower->pPltNitrogen	= pPltNitrogen;
	pPlantLower->pPltClimate	= pPltClimate;
	pPlantLower->pModelParam	= pModelParam;
	pPlantLower->pPMeasure		= pPMeasure;

	return 1;
}


int WINAPI allocPlantStructures()
{
	GlobalUnlock(pGenotype);
	pGenotype = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stgenotype));

	GlobalUnlock(pDevelop);
	pDevelop = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stdevelop));

	GlobalUnlock(pBiomass);
	pBiomass = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stbiomass));

	GlobalUnlock(pCanopy);
	pCanopy = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stcanopy));

	GlobalUnlock(pRoot);
	pRoot = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stroot));

	GlobalUnlock(pPltCarbon);
	pPltCarbon = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplantCarbon));

	GlobalUnlock(pPltWater);
	pPltWater = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplantWater));

	GlobalUnlock(pPltNitrogen);
	pPltNitrogen = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplantNitrogen));

	GlobalUnlock(pPltClimate);
	pPltClimate = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stplantClimate));

	GlobalUnlock(pModelParam);
	pModelParam = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stModelParam));

	GlobalUnlock(pPMeasure);
	pPMeasure = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPltMeasure));

	return 1;
}


int WINAPI 	allocBiochemistry()
{
	//Stellt Speicherplatz bereit für Tabelle "Kosten der Biosynthese"

	GlobalUnlock(pBiochemistry);
	pBiochemistry = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stBiochemistry));

	allocGrowthEfficiency();
	pBiochemistry->pGrowthEfficiency = pGrowthEfficiency;

	allocCO2Required();
	pBiochemistry->pCO2Required = pCO2Required;

	allocCarbon();
	pBiochemistry->pCarbon = pCarbon;

	return 1;
}

int WINAPI allocGrowthEfficiency()
{
	GlobalUnlock(pGrowthEfficiency);
	pGrowthEfficiency = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stGrowthEfficiency));
	return 1;
}

int WINAPI allocCO2Required()
{
	GlobalUnlock(pCO2Required);
	pCO2Required = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stCO2Required));
	return 1;
}

int WINAPI allocCarbon()
{
	GlobalUnlock(pCarbon);
	pCarbon = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stCarbon));
	return 1;
}

int WINAPI allocPlathoAllPlants()
{
	//Stellt Speicherplatz bereit für PLATHO-Szenario (Tageswerte)
	GlobalUnlock(pPlathoAllPlants);
	pPlathoAllPlants = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoAllPlants));
	return 1;
}

int WINAPI allocPlathoScen()
{
	//Stellt Speicherplatz bereit für PLATHO-Szenario (Tageswerte)
	GlobalUnlock(pPlathoScenario);
	pPlathoScenario = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoScenario));
	return 1;
}

int WINAPI allocPlathoMod()
{
	//Stellt Speicherplatz bereit für PLATHO-Szenario (Tageswerte)
	GlobalUnlock(pPlathoScenario);
	pPlathoModules = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoModules));
	return 1;
}

int WINAPI allocClimate()
{
	//Stellt Speicherplatz bereit für PLATHO-Szenario (Tageswerte)
	FILE		*pFile;
	char		cGenFileName[50];
	int			c,cc;
	int			i, iLines;

	PPLATHOCLIMATE	pClimate[MAXTIMESTEP], pPC;
	

	GlobalUnlock(pPlathoClimate);
	//pPlathoClimate = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoClimate));


	//Zeilen Zählen (ohne die beiden ersten!)

	lstrcpy(cGenFileName,"Platho4Input\\Platho_Climate.hrc\0");
	pFile = fopen(cGenFileName,"r");
	//fopen_s(&pFile,cGenFileName,"r");

	iLines = -2;
	while ((c = getc(pFile)) !=EOF)
	{
		if(c == '\n')
		{
			iLines++;

			if((cc = getc(pFile) == '\n')||(cc = getc(pFile) == 'E')||
				(cc = getc(pFile) == 'e')||(cc = getc(pFile) == 'e'))
				break;
		}
	}


	for(i=0;i<iLines;i++)//Anzahl der Einträge
	{
		pClimate[i] = GlobalAlloc(GMEM_ZEROINIT,sizeof(struct stPlathoClimate));

		if(i==0)
		{
			//pPlathoClimate =  (PPLATHOCLIMATE)GlobalLock(pClimate[i]);
			pPlathoClimate =  pClimate[i];
			pPlathoClimate->pBack = NULL;
			pPlathoClimate->pNext = NULL;
		}
		else
		{
			//pPC = pPlathoClimate->pNext = (PPLATHOCLIMATE)GlobalLock(pClimate[i]);
			pPC = pPlathoClimate->pNext = pClimate[i];
			pPC->pBack = pPlathoClimate;
			pPlathoClimate = pPC;
			pPC=pPC->pNext;
		}
	}

	pPlathoClimate = pClimate[0];
	

	fclose(pFile);


	return 1;
}


/*

int WINAPI freeAllocatedMemory(int iPlants)
{
	int i;
		
	GlobalFree(pPlathoClimate);
	GlobalFree(pPlathoScenario);
	GlobalFree(pBiochemistry);
	GlobalFree(pPlantNeighbours);
	GlobalFree(pPlathoRootLayer);

	for(i=0; i<iPlants; i++)
		GlobalFree(pPlathoPlant[i]);

		
	return 1;
}
*/


int WINAPI freeAllocatedMemory(int iPlants, int iRootLayers)
{
    PLEAFLAYER pLLayer;
    PPLATHOROOTLAYER pPRLayer;
    int i, j, l, n;

    HGLOBAL hGlobal;

	for(i=iPlants; i>0; i--)
	{
        //PLATHOGENOTYPE-----------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pFineRoot);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pFineRoot);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pFineRoot = NULL;
    	
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pGrossRoot);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pGrossRoot);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pGrossRoot = NULL;
   	
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pStem);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pStem);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pStem = NULL;
   	
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pBranch);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pBranch);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pBranch = NULL;
    	
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pLeaf);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pLeaf);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pLeaf = NULL;
    	
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pFruit);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pFruit);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pFruit = NULL;
    	
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pTuber);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pTuber);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial->pTuber = NULL;

        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial);
        pPlathoPlant[i-1]->pPlathoGenotype->pPltMaterial = NULL;
    
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype->pFalgePS);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype->pFalgePS);
        pPlathoPlant[i-1]->pPlathoGenotype->pFalgePS = NULL;
    
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoGenotype);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoGenotype);
        pPlathoPlant[i-1]->pPlathoGenotype = NULL;
        //------------------------------------------------------
 

        //PLATHODEVELOP-----------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoDevelop);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoDevelop);
        pPlathoPlant[i-1]->pPlathoDevelop = NULL;
        //------------------------------------------------------

        //PLATHOBIOMASS-----------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoBiomass);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoBiomass);
        pPlathoPlant[i-1]->pPlathoBiomass = NULL;
       //------------------------------------------------------

        //PLATHOPARTITIONING-----------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoPartitioning);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoPartitioning);
        pPlathoPlant[i-1]->pPlathoPartitioning = NULL;
        //------------------------------------------------------

                 
        //PLATHOMORPHOLOGY-----------------------------------------
        // N Leaf layers
        pLLayer = pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer;

        //Zur obersten Blattschicht zeigen:
	    for(n=1;n<=LEAFLAYERS-1;n++)
		    pLLayer=pLLayer->pNext;

        // rückwärts freigeben
   	    for(n=LEAFLAYERS;n>1;n--)
        {
            //4 Sectoren freigeben
            for(j=0;j<4;j++)
            {
                GlobalUnlock(pLLayer->apLeafSector[j]);
                hGlobal = GlobalFree(pLLayer->apLeafSector[j]);
                pLLayer->apLeafSector[j] = NULL;
            }
			
  	        //Blattschichten freigeben
            pLLayer = pLLayer->pBack;
            GlobalUnlock(pLLayer->pNext);
            hGlobal = GlobalFree(pLLayer->pNext);
            pLLayer->pNext = NULL;
        }

        //letzte Schicht:
        for(j=0;j<4;j++)
        {
            GlobalUnlock(pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer->apLeafSector[j]);
            hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer->apLeafSector[j]);
            pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer->apLeafSector[j] = NULL;
        }   
		
  	    //Schicht freigeben
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer);
        pPlathoPlant[i-1]->pPlathoMorphology->pLeafLayer = NULL;


        // L root layers
        pPRLayer = pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer;

        //Zur untersten Wurzelschicht zeigen:
	    for(l=1;l<=iRootLayers-1;l++)
		    pPRLayer=pPRLayer->pNext;

        // rückwärts freigeben
   	    for(l=iRootLayers;l>1;l--)
        {
            //4 Sectoren freigeben
            for(j=0;j<4;j++)
            {
                GlobalUnlock(pPRLayer->apRootSector[j]);
                hGlobal = GlobalFree(pPRLayer->apRootSector[j]);
                pPRLayer->apRootSector[j] = NULL;
           }     
			
  	        //Blattschichten freigeben
            pPRLayer = pPRLayer->pBack;
            GlobalUnlock(pPRLayer->pNext);
            hGlobal = GlobalFree(pPRLayer->pNext);
            pPRLayer->pNext = NULL;
        }

        //letzte Schicht:
        for(j=0;j<4;j++)
        {
            GlobalUnlock(pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer->apRootSector[j]);
            hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer->apRootSector[j]);
            pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer->apRootSector[j] = NULL;
        }    
		
  	    //Schicht freigeben
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer);
        pPlathoPlant[i-1]->pPlathoMorphology->pPlathoRootLayer = NULL;


        GlobalUnlock(pPlathoPlant[i-1]->pPlathoMorphology);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoMorphology);
        pPlathoPlant[i-1]->pPlathoMorphology = NULL;
        //------------------------------------------------------
 
        //PPLATHONITROGEN---------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]->pPlathoNitrogen);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlathoNitrogen);
        pPlathoPlant[i-1]->pPlathoNitrogen = NULL;
        //------------------------------------------------------

        //PPLANTSTRESS---------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]->pPlantStress);
        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlantStress);
        pPlathoPlant[i-1]->pPlantStress = NULL;
        //------------------------------------------------------

        //PPLANTNEIGHBOURS---------------------------------------

        //!!! Speicherplatz der Nachbarn muss nicht freigegeben werden,
        //!!! weil identisch mit Speicherplatz von pPl, pPl->pNext, ...
        //!!! aus der ModLib
        //!!! (wird in der ModLib freigegeben!!!

/*        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pGenotype);
        pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pGenotype = NULL;
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pDevelop);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pBiomass);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pCanopy);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pRoot);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pPltCarbon);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pPltWater);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pPltNitrogen);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pPltClimate);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pModelParam);
        GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight->pPMeasure);  


        if(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pGenotype !=NULL)
        {
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pGenotype);
            pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pGenotype = NULL;
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pDevelop);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pBiomass);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pCanopy);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pRoot);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pPltCarbon);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pPltWater);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pPltNitrogen);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pPltClimate);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pModelParam);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower->pPMeasure);     

        }

        if(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pGenotype !=NULL)
        {
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pGenotype);
            pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pGenotype = NULL;
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pDevelop);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pBiomass);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pCanopy);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pRoot);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pPltCarbon);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pPltWater);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pPltNitrogen);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pPltClimate);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pModelParam);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft->pPMeasure);  

        }

        if(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pGenotype !=NULL)
        {
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pGenotype);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pDevelop);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pBiomass);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pCanopy);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pRoot);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pPltCarbon);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pPltWater);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pPltNitrogen);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pPltClimate);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pModelParam);
            GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper->pPMeasure);      
        }

        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight);
        pPlathoPlant[i-1]->pPlantNeighbours->pPlantRight = NULL;

        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower);
        pPlathoPlant[i-1]->pPlantNeighbours->pPlantLower = NULL;

        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft);
        pPlathoPlant[i-1]->pPlantNeighbours->pPlantLeft = NULL;

        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper);
        pPlathoPlant[i-1]->pPlantNeighbours->pPlantUpper = NULL;
 */

        hGlobal = GlobalFree(pPlathoPlant[i-1]->pPlantNeighbours);
        pPlathoPlant[i-1]->pPlantNeighbours = NULL;
       //------------------------------------------------------
  
        //PPLATHOPLANT--------------------------------------
        GlobalUnlock(pPlathoPlant[i-1]);
        hGlobal = GlobalFree(pPlathoPlant[i-1]);
        pPlathoPlant[i-1] = NULL;
        //------------------------------------------------------
    }

        //PPLATHOSCENARIO--------------------------------------
        GlobalUnlock(pPlathoScenario);
        hGlobal = GlobalFree(pPlathoScenario);
        pPlathoScenario = NULL;
        //------------------------------------------------------
	        
        //PPLATHOMODULES--------------------------------------
        GlobalUnlock(pPlathoModules);
        hGlobal = GlobalFree(pPlathoModules);
        pPlathoModules = NULL;
        //------------------------------------------------------

        //PBIOCHEMISTRY--------------------------------------
        GlobalUnlock(pBiochemistry->pGrowthEfficiency);
        hGlobal = GlobalFree(pBiochemistry->pGrowthEfficiency);
        pBiochemistry->pGrowthEfficiency = NULL;

        GlobalUnlock(pBiochemistry->pCO2Required);
        hGlobal = GlobalFree(pBiochemistry->pCO2Required);
        pBiochemistry->pCO2Required = NULL;

        GlobalUnlock(pBiochemistry->pCarbon);
        hGlobal = GlobalFree(pBiochemistry->pCarbon);
        pBiochemistry->pCarbon = NULL;

        GlobalUnlock(pBiochemistry);
        hGlobal = GlobalFree(pBiochemistry);
        pBiochemistry = NULL;
        //------------------------------------------------------
    


	return 1;
}


int WINAPI freeClimate()
{
	int i,istep;

	PPLATHOCLIMATE pPClimate = pPlathoClimate;

	//Zum vorletzten Eintrag zeigen:
	for(istep=0;pPClimate->pNext->pNext!=NULL;istep++)
		pPClimate=pPClimate->pNext;

	//Speicher freigeben
	for(i=0;i<istep;i++,pPClimate=pPClimate->pBack)//Alle Einträge
		GlobalFree(pPClimate->pNext);

	GlobalFree(pPlathoClimate);

	return 1;
}