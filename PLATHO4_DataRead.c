///////////////////////////////////////////////////////////////////////////////
//	Sammlung der Einleseroutinen für PLATHO
//
//	PLATHO_Genotype_Read			platho\\PLATHO_PLANT.GTP
//	PLATHO_Simulation_Scenario		platho\\Platho_Scenario.XNM
//	ReadClimateData					platho\\Platho_Climate.HRC
//	
//////////////////////////////////////////////////////////////////////////////
//Standard C-Functions Librarys
#include <windows.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include  "xinclexp.h"

#include "Platho4.h"


extern int	WINAPI	Message(long, LPSTR);
extern int			ReadSymbol(HFILE,LPSTR);
extern int	 		ReadDate(HFILE,LPSTR);
extern int			ReadInt(HFILE);
extern long	 		ReadLong(HFILE);
extern float  		ReadFloat(HFILE);
extern double  		ReadDouble(HFILE);
extern long double 	ReadLongDouble(HFILE);
extern float 		AFGENERATOR(float fInput, RESPONSE* pfResp);


int PLATHO_Genotype_Read(PPLANT, int);
int PLATHO_Simulation_Scenario(PTIME, PPLANT, int, PMSOWINFO, PPLATHOSCENARIO, PSPROFILE, PMANAGEMENT, PLOCATION);
int WINAPI ReadClimateData(PPLATHOCLIMATE, PTIME);
int ReadOutputList(PPLATHOSCENARIO pPlathoScenario);
int PLATHO_Modules();
int ReadCropFactors(PWATER pWa, PPLANT pPlant);


extern int PLATHO_StartValues(PPLANT pPlant, int iPlant, PSPROFILE pSo, PMANAGEMENT pMa );


/////////////////////////////////////////////////////////////////////////////////////////////////
//						SPAMC Crop Data Reading
//							
//									Enli Wang   
//									
//	    							22.03.1995
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//		SPAMC Initialization routine: Genotype Data read 
///////////////////////////////////////////////////////////////////////////////////////////////
int PLATHO_Genotype_Read(PPLANT pPlant, int iPlant)
	{
    	PPLATHOPARTITIONING pPPart		= pPlathoPlant[iPlant]->pPlathoPartitioning;
	    PPLATHOBIOMASS		pPBiom		= pPlathoPlant[iPlant]->pPlathoBiomass;
	    PPLATHOMORPHOLOGY	pPMorph		= pPlathoPlant[iPlant]->pPlathoMorphology;
	    PPLATHOGENOTYPE		pPGen		= pPlathoPlant[iPlant]->pPlathoGenotype;
	    PPLATHODEVELOP		pPDev		= pPlathoPlant[iPlant]->pPlathoDevelop;
	    PPLATHONITROGEN		pPPltN		= pPlathoPlant[iPlant]->pPlathoNitrogen;
	    PPLANTSTRESS		pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
	    PGROWTHEFFICIENCY	pPGrwEff	= pBiochemistry->pGrowthEfficiency;
	    PCO2REQUIRED		pPCO2Req	= pBiochemistry->pCO2Required;

  	    OFSTRUCT	ofStruct;
	    HFILE		hFile;
    //	HWND 		hwndRead;
    //	HDC  		hdc;

        int			i,iValue;
	    long 		longCode;
	    char  		cChar[20];
    	
	    //===================================================================================
	    //The genotype file name
	    //===================================================================================
	    char	cGenFileName[50];

		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_WHEAT.GTP\0");
		else if((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA"))
				||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"S")))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_BARLEY.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"PT"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_POTATO.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SF"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_SUNFLOWER.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"MZ"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_MAIZE.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"AL"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_ALFALFA.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"IR"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_LOLIUM.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"AP"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_APPLE.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BE"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_BEECH.GTP\0");
		else if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SP"))
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_SPRUCE.GTP\0");
		else
		{
			lstrcpy((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH");
			lstrcpy(cGenFileName,"Platho4Input\\PLATHO_PLANT.GTP\0");
		}

    
	//===================================================================================
	//Read Genotype data
	//===================================================================================
	hFile=OpenFile(cGenFileName,(LPOFSTRUCT)&ofStruct, OF_EXIST);
   // i=_lclose(hFile);

	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	//Display elements
	if (hFile==HFILE_ERROR)
        {
		Message(1,"Genotype file not found ! File Open Error");
		return 0;
        }                         
       
		Message(1,"Reading PLATHO-Genotype file ");
    
	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
	//=======================================================================
	//	Get the pointer, open the file
	//=======================================================================

	hFile=OpenFile(cGenFileName,(LPOFSTRUCT)&ofStruct, OF_READ);
	longCode=0;

	//=======================================================================
	//	Name and Type
	//=======================================================================
	while (longCode!=2000001) longCode=ReadLong(hFile);
    
    iValue = ReadInt(hFile);
    if (iValue>0)
    	{
		ReadSymbol(hFile,cChar);
		if(lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,cChar))
			Message(1,"Plant warning ");

		lstrcpy(pPlant->pGenotype->acCropCode,  	cChar);

		ReadSymbol(hFile,cChar); // lstrcpy(pPl->pGenotype->acCropName,  	cChar);
		}

	//=======================================================================
	//	Photosynthesis Response
	//=======================================================================
	//Ps parameters


	while (longCode!=2000010)	longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSu[0].vInput =iValue;

	pPlant->pGenotype->fMaxGrossPs 	= ReadFloat(hFile);

	if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSu[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSu[0].vOutput=(double)iValue;

        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSu[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSu[i].vOutput=	ReadDouble(hFile);
		}


	while (longCode!=2000011)	longCode=ReadLong(hFile);
    
	iValue = ReadInt(hFile);
    pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSh[0].vInput =iValue;
 
	if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSh[0].vInput =iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSh[0].vOutput=iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSh[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MaxGrossPsSh[i].vOutput=	ReadDouble(hFile);
		}


	while (longCode!=2000012)	longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    if (iValue>0)
    	{
		pPlant->pGenotype->fLightUseEff		= ReadFloat(hFile);
		pPlant->pGenotype->fMinRm			= ReadFloat(hFile);
		pPlant->pGenotype->fCO2CmpPoint		= ReadFloat(hFile);
		pPlant->pGenotype->fCiCaRatio		= ReadFloat(hFile);
		}
	//----------------------------------------------------------
	//	Photosynthesis Temperature Response
	while (longCode!=2000013) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->PmaxTemp[0].fInput =(float)iValue;

	pPlant->pGenotype->fTempMinPs 	= ReadFloat(hFile);
	pPlant->pGenotype->fTempOptPs 	= ReadFloat(hFile);
	pPlant->pGenotype->fTempMaxPs 	= ReadFloat(hFile);

    if (iValue>0)
    	{   
    	pPlant->pGenotype->PmaxTemp[0].fInput =(float)iValue;
    	pPlant->pGenotype->PmaxTemp[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->PmaxTemp[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->PmaxTemp[i].fOutput=	ReadFloat(hFile);
		}

			
	TREES
	{
	//Parameters for the Farquhar model
	while (longCode!=2000014)	longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSu[0].vInput =(double)iValue;

	pPlathoPlant[iPlant]->pPlathoGenotype->vVcmaxSu 	= ReadDouble(hFile);

	if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSu[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSu[0].vOutput=(double)iValue;

        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSu[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSu[i].vOutput=	ReadDouble(hFile);
		}


	while (longCode!=2000015)	longCode=ReadLong(hFile);
    
	iValue = ReadInt(hFile);
    pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSh[0].vInput =(double)iValue;

	pPlathoPlant[iPlant]->pPlathoGenotype->vVcmaxSh 	= ReadDouble(hFile);

 	if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSh[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSh[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSh[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avVcmaxSh[i].vOutput=	ReadDouble(hFile);
		}

	while (longCode!=2000016)	longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSu[0].vInput =(double)iValue;

	pPlathoPlant[iPlant]->pPlathoGenotype->vJmaxSu 	= ReadDouble(hFile);

	if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSu[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSu[0].vOutput=(double)iValue;

        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSu[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSu[i].vOutput=	ReadDouble(hFile);
		}


	while (longCode!=2000017)	longCode=ReadLong(hFile);
    
	iValue = ReadInt(hFile);
    pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSh[0].vInput =(double)iValue;

	pPlathoPlant[iPlant]->pPlathoGenotype->vJmaxSh 	= ReadDouble(hFile);

 	if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSh[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSh[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSh[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->avJmaxSh[i].vOutput=	ReadDouble(hFile);
		}


	while (longCode!=2000018)	longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    if (iValue>0)
    	{
		pPlathoPlant[iPlant]->pPlathoGenotype->vFCalphaSu		= ReadDouble(hFile);
		pPlathoPlant[iPlant]->pPlathoGenotype->vFCalphaSh		= ReadDouble(hFile);
		pPlathoPlant[iPlant]->pPlathoGenotype->vTheta			= ReadDouble(hFile);
		}
	}//end Trees


	//=======================================================================
	//	Maintenance and Growth
	//=======================================================================
	while (longCode!=2000020) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

		pPlathoPlant[iPlant]->pPlathoGenotype->vMaxPlantGrowR	= ReadDouble(hFile); 
		pPlathoPlant[iPlant]->pPlathoGenotype->vLeafFlushR		= ReadDouble(hFile); 
		pPlathoPlant[iPlant]->pPlathoGenotype->vFineRootFlushR	= ReadDouble(hFile); 
		pPlathoPlant[iPlant]->pPlathoGenotype->vAssUseRate		= ReadDouble(hFile); 
		pPlant->pGenotype->fRelResvGrwRate						= ReadFloat(hFile);
		pPlathoPlant[iPlant]->pPlathoGenotype->vNTransRate		= ReadDouble(hFile); 

		for(i=1;i<iValue;i++)
		{
			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
				pPlathoPlant[iPlant]->pPlathoGenotype->vMaxPlantGrowR	= ReadDouble(hFile); 
				pPlathoPlant[iPlant]->pPlathoGenotype->vLeafFlushR		= ReadDouble(hFile); 
				pPlathoPlant[iPlant]->pPlathoGenotype->vFineRootFlushR	= ReadDouble(hFile); 
				pPlathoPlant[iPlant]->pPlathoGenotype->vAssUseRate		= ReadDouble(hFile); 
				pPlant->pGenotype->fRelResvGrwRate						= ReadFloat(hFile);
				pPlathoPlant[iPlant]->pPlathoGenotype->vNTransRate		= ReadDouble(hFile); 
			}
			else
			{
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadFloat(hFile);
				ReadDouble(hFile);
			}
		}	//end for
	 }	//end if iValue

		
	//Maintenance Respiration Coefficients
	while (longCode!=2000021) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    if (iValue>0)
    	{
			pPlathoPlant[iPlant]->pPlathoGenotype->vProteinsTurnoverR	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->vLipidsTurnoverR	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->vMineralsLeakageR	= ReadDouble(hFile);
		}


	while (longCode!=2000022) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    if (iValue>0)
    	{
			pPlathoPlant[iPlant]->pPlathoGenotype->vMaintMetabol	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->vMaintMetabolTimeDelay	= ReadDouble(hFile);
		}



	//Biochemical fractions of new plant material
	//
	while (longCode!=2000029) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
    if (iValue>0)
    {
			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vCarbohydrates	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vLipids			= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vOrganicAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vNucleotides	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFineRoot->vMinerals		= ReadDouble(hFile);
		
		TREES
		{
			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vCarbohydrates = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vLipids		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vOrganicAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vNucleotides	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pGrossRoot->vMinerals		= ReadDouble(hFile);
		}
			
			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vCarbohydrates	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vLipids			= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vOrganicAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vNucleotides	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pStem->vMinerals		= ReadDouble(hFile);
		

		TREES
		{
			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vCarbohydrates= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vLipids		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vOrganicAcids = ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vNucleotides= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pBranch->vMinerals		= ReadDouble(hFile);
		}
		

			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vCarbohydrates	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vLipids			= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vOrganicAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vNucleotides	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pLeaf->vMinerals		= ReadDouble(hFile);
		
		POTATO
		{
			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vCarbohydrates = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vLipids		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vOrganicAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vNucleotides	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pTuber->vMinerals		= ReadDouble(hFile);
		}
		else
		{
			ReadSymbol(hFile,cChar);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vCarbohydrates = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vProteins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vLipids		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vLignins		= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vOrganicAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vAminoAcids	= ReadDouble(hFile);
			//pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vNucleotides	= ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->pPltMaterial->pFruit->vMinerals		= ReadDouble(hFile);
		}
					
	}
		


	//=======================================================================
	//	Phenological Development
	//=======================================================================
	while (longCode!=2000030) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

		ReadSymbol(hFile,cChar);	lstrcpy(pPlant->pGenotype->acDaylenType, 	cChar);
		pPlant->pGenotype->fDaylenSensitivity	= ReadFloat(hFile);
		pPlant->pGenotype->fOptDaylen			= ReadFloat(hFile);

		if ((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))||
			(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA")))
    	{
			pPlant->pGenotype->fOptVernDays	= ReadFloat(hFile);
			pPlant->pGenotype->iVernCoeff	= ReadInt(hFile);
		}


		for(i=1;i<iValue;i++)
		{
			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
				ReadSymbol(hFile,cChar);	lstrcpy(pPlant->pGenotype->acDaylenType, 	cChar);
				pPlant->pGenotype->fDaylenSensitivity	= ReadFloat(hFile);
				pPlant->pGenotype->fOptDaylen			= ReadFloat(hFile);
						
				if ((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))||
					(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA")))
    				{
						pPlant->pGenotype->fOptVernDays	= ReadFloat(hFile);
						pPlant->pGenotype->iVernCoeff	= ReadInt(hFile);
					}
			}
			else
			{
				ReadSymbol(hFile,cChar);
				ReadFloat(hFile);
				ReadFloat(hFile);

				if ((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))||
					(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA")))
    				{
						ReadFloat(hFile);
						ReadInt(hFile);
					}
			} //end if
		}	//end for
	 }	//end if iValue

	//----------------------------------------------------------
	//Stage dependend parameters for the CERES - model

	while (longCode!=2000036) longCode=ReadLong(hFile);

        for (i=0;i<=5;i++)
	    	pPlant->pGenotype->pStageParam->afTempMax[i] = ReadFloat(hFile);
        for (i=0;i<=5;i++)
	    	pPlant->pGenotype->pStageParam->afTempOpt[i] = ReadFloat(hFile);
        for (i=0;i<=5;i++)
	    	pPlant->pGenotype->pStageParam->afTempBase[i] = ReadFloat(hFile);
        for (i=0;i<=5;i++)
	    	pPlant->pGenotype->pStageParam->afThermalTime[i] = ReadFloat(hFile);
	

	//=======================================================================
	//	Morphology
	//=======================================================================
	
	while (longCode!=2000042)	longCode=ReadLong(hFile);

	iValue = ReadInt(hFile);
    if (iValue>0)
    	{
			ReadSymbol(hFile,cChar);

			pPlant->pGenotype->fPlantHeight=(float)0.01*ReadFloat(hFile); //m
			ReadSymbol(hFile,cChar);	lstrcpy(pPlant->pGenotype->acLvAngleType, cChar);  
			pPlathoPlant[iPlant]->pPlathoGenotype->vRelHeightMaxLeaf = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->vKDiffuse = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->vLAImax = ReadDouble(hFile);

			for(i=1;i<iValue;i++)
			{
				ReadSymbol(hFile,cChar);
			
				if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
				{
					pPlant->pGenotype->fPlantHeight=(float)0.01*ReadFloat(hFile); //m
					ReadSymbol(hFile,cChar);	lstrcpy(pPlant->pGenotype->acLvAngleType, cChar);  
					pPlathoPlant[iPlant]->pPlathoGenotype->vRelHeightMaxLeaf = ReadDouble(hFile);
					pPlathoPlant[iPlant]->pPlathoGenotype->vKDiffuse = ReadDouble(hFile);
					pPlathoPlant[iPlant]->pPlathoGenotype->vLAImax = ReadDouble(hFile);
				}
				else
				{
					ReadFloat(hFile);
					ReadSymbol(hFile,cChar);	
					ReadDouble(hFile);
					ReadDouble(hFile);
					ReadDouble(hFile);
				}
			}
		}

	while (longCode!=2000043)	longCode=ReadLong(hFile);

	 iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

		TREES
			pPlathoPlant[iPlant]->pPlathoMorphology->vHtoDmax = ReadDouble(hFile);
			
		pPlathoPlant[iPlant]->pPlathoMorphology->vHtoDmin	  = ReadDouble(hFile);
		pPlant->pGenotype->fSpecLfWeight					  = ReadFloat(hFile);
		pPlathoPlant[iPlant]->pPlathoMorphology->vStemDensity = ReadDouble(hFile);


		for(i=1;i<iValue;i++)
		{
			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
				TREES
					pPlathoPlant[iPlant]->pPlathoMorphology->vHtoDmax = ReadDouble(hFile);
					
				pPlathoPlant[iPlant]->pPlathoMorphology->vHtoDmin	  = ReadDouble(hFile);
				pPlant->pGenotype->fSpecLfWeight					  = ReadFloat(hFile);
				pPlathoPlant[iPlant]->pPlathoMorphology->vStemDensity = ReadDouble(hFile);
			}
			else
			{
				TREES
					ReadDouble(hFile);

				ReadDouble(hFile);
				ReadFloat(hFile);
				ReadDouble(hFile);
			}
		}	//end for
	 }	//end if iValue


	HERBS
	{
	while (longCode!=2000044) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlathoPlant[iPlant]->pPlathoMorphology->vHtoDmax = ReadDouble(hFile);

    if (iValue>0)
    	{   
		pPlathoPlant[iPlant]->pPlathoMorphology->vHDmax[0].vInput  = (double)iValue;
		pPlathoPlant[iPlant]->pPlathoMorphology->vHDmax[0].vOutput = (double)iValue;
    	
        for (i=1;i<=iValue;i++)
			pPlathoPlant[iPlant]->pPlathoMorphology->vHDmax[i].vInput  = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
			pPlathoPlant[iPlant]->pPlathoMorphology->vHDmax[i].vOutput  = ReadDouble(hFile);
		}  
	}



	while (longCode!=2000045)	longCode=ReadLong(hFile);

	iValue = ReadInt(hFile);
    if (iValue>0)
    	{
			ReadSymbol(hFile,cChar);

			HERBS
			{
				pPlathoPlant[iPlant]->pPlathoMorphology->vFineRootToLeafRatio = ReadDouble(hFile);
				pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToStemRatio = ReadDouble(hFile);
				pPlant->pGenotype->fRelStorageFillRate = ReadFloat(hFile);
				pPlathoPlant[iPlant]->pPlathoBiomass->vSeedWeight = ReadDouble(hFile);
			}
			
			TREES
			{			
                        pPlathoPlant[iPlant]->pPlathoGenotype->vLa = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoGenotype->vLb = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoGenotype->vFRa = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoGenotype->vFRb = ReadDouble(hFile);
//						pPlathoPlant[iPlant]->pPlathoMorphology->vMaxLeafToWoodRatio = ReadDouble(hFile);
//						pPlathoPlant[iPlant]->pPlathoMorphology->vFineRootToLeafRatio = ReadDouble(hFile);
//						pPlathoPlant[iPlant]->pPlathoMorphology->vFruitToLeafRatio = ReadDouble(hFile);

                APPLE
                    pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToBranchRatio = ReadDouble(hFile);
			}

			for(i=1;i<iValue;i++)
			{
				ReadSymbol(hFile,cChar);
			
				if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
				{
					HERBS
					{
						pPlathoPlant[iPlant]->pPlathoMorphology->vFineRootToLeafRatio = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToStemRatio = ReadDouble(hFile);
						pPlant->pGenotype->fRelStorageFillRate = ReadFloat(hFile);
						pPlathoPlant[iPlant]->pPlathoBiomass->vSeedWeight = ReadDouble(hFile);
					}
					
					TREES
					{			
                        pPlathoPlant[iPlant]->pPlathoGenotype->vLa = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoGenotype->vLb = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoGenotype->vFRa = ReadDouble(hFile);
						pPlathoPlant[iPlant]->pPlathoGenotype->vFRb = ReadDouble(hFile);
//						pPlathoPlant[iPlant]->pPlathoMorphology->vMaxLeafToWoodRatio = ReadDouble(hFile);
//						pPlathoPlant[iPlant]->pPlathoMorphology->vFineRootToLeafRatio = ReadDouble(hFile);
//						pPlathoPlant[iPlant]->pPlathoMorphology->vFruitToLeafRatio = ReadDouble(hFile);

                        APPLE
                            pPlathoPlant[iPlant]->pPlathoMorphology->vLeafToBranchRatio = ReadDouble(hFile);
					}
				}
				else
				{
					ReadDouble(hFile);
					ReadDouble(hFile);
					ReadDouble(hFile);
//					HERBS
                        ReadDouble(hFile);
                    APPLE
                        ReadDouble(hFile);
				} //end else
			} //end for
		} //end iValue > 0


	while (longCode!=2000046)	longCode=ReadLong(hFile);

	iValue = ReadInt(hFile);
    if (iValue>0)
    	{
			ReadSymbol(hFile,cChar);

			pPlathoPlant[iPlant]->pPlathoMorphology->vPlasticity = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoMorphology->vCRegulation = ReadDouble(hFile);
			pPlathoPlant[iPlant]->pPlathoMorphology->vNRegulation = ReadDouble(hFile);

			for(i=1;i<iValue;i++)
			{
				ReadSymbol(hFile,cChar);
			
				if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
				{
				    pPlathoPlant[iPlant]->pPlathoMorphology->vPlasticity = ReadDouble(hFile);
					pPlathoPlant[iPlant]->pPlathoMorphology->vCRegulation = ReadDouble(hFile);
					pPlathoPlant[iPlant]->pPlathoMorphology->vNRegulation = ReadDouble(hFile);
				}
				else
				{
					ReadDouble(hFile);
					ReadDouble(hFile);
					ReadDouble(hFile);
				} //end else
			} //end for
		} //end iValue > 0


	TREES
	{
		while (longCode!=2000047) longCode=ReadLong(hFile);

		iValue = ReadInt(hFile);
		if (iValue>0)
		{   
				pPlathoPlant[iPlant]->pPlathoMorphology->vUGrWoodFrac[0].vInput = (double)iValue;
				pPlathoPlant[iPlant]->pPlathoMorphology->vBranchFrac[0].vInput = (double)iValue;
				pPlathoPlant[iPlant]->pPlathoMorphology->vUGrWoodFrac[0].vOutput = (double)iValue;
				pPlathoPlant[iPlant]->pPlathoMorphology->vBranchFrac[0].vOutput = (double)iValue;
    		
			for (i=1;i<=iValue;i++)
			{
	    		pPlathoPlant[iPlant]->pPlathoMorphology->vUGrWoodFrac[i].vInput = 
					pPlathoPlant[iPlant]->pPlathoMorphology->vBranchFrac[i].vInput = ReadDouble(hFile);
			}
			for (i=1;i<=iValue;i++)
	    		pPlathoPlant[iPlant]->pPlathoMorphology->vUGrWoodFrac[i].vOutput= ReadDouble(hFile);
			for (i=1;i<=iValue;i++)
	    		pPlathoPlant[iPlant]->pPlathoMorphology->vBranchFrac[i].vOutput= ReadDouble(hFile);
		}  
	}
	
	//=======================================================================
	//	TISSUE NITROGEN CONTENT
	//=======================================================================
	//Leaf Opt Nc
	while (longCode!=2000070) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->OptLvNc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->OptLvNc[0].fInput =(float)iValue;
    	pPlant->pGenotype->OptLvNc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->OptLvNc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->OptLvNc[i].fOutput= ReadFloat(hFile);
		}  
	//----------------------------------------------------------
	TREES
	{
		//Branches Opt Nc
		while (longCode!=2000071) longCode=ReadLong(hFile);

		iValue = ReadInt(hFile);
		pPlathoPlant[iPlant]->pPlathoGenotype->OptBrNc[0].vInput =(double)iValue;
			
		if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->OptBrNc[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->OptBrNc[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->OptBrNc[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->OptBrNc[i].vOutput=	ReadDouble(hFile);
		}
	}
	//----------------------------------------------------------
	//Stem Opt Nc
	while (longCode!=2000072) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->OptStNc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->OptStNc[0].fInput =(float)iValue;
    	pPlant->pGenotype->OptStNc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->OptStNc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->OptStNc[i].fOutput= ReadFloat(hFile);
		}  
	//----------------------------------------------------------
	//Fine Root Opt Nc
	while (longCode!=2000073) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->OptRtNc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->OptRtNc[0].fInput =(float)iValue;
    	pPlant->pGenotype->OptRtNc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->OptRtNc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->OptRtNc[i].fOutput= ReadFloat(hFile);
		}  
	//----------------------------------------------------------
	TREES
	{
	//Gross Root Opt Nc
	while (longCode!=2000074) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->OptGRtNc[0].vInput =(double)iValue;

    if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->OptGRtNc[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->OptGRtNc[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->OptGRtNc[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->OptGRtNc[i].vOutput= ReadDouble(hFile);
		}  
	}
  	//----------------------------------------------------------
	POTATO
	{
	//Tuber Opt Nc
	while (longCode!=20000745) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->OptTuberNc[0].vInput =(double)iValue;

    if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->OptTuberNc[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->OptTuberNc[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->OptTuberNc[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->OptTuberNc[i].vOutput= ReadDouble(hFile);
		}  
	}
	else
	{
	//Fruit Opt Nc
		while (longCode!=20000745) longCode=ReadLong(hFile);

		iValue = ReadInt(hFile);
		pPlathoPlant[iPlant]->pPlathoGenotype->OptFruitNc[0].vInput =(double)iValue;

		if (iValue>0)
    		{   
    		pPlathoPlant[iPlant]->pPlathoGenotype->OptFruitNc[0].vInput =(double)iValue;
    		pPlathoPlant[iPlant]->pPlathoGenotype->OptFruitNc[0].vOutput=(double)iValue;
	    	
			for (i=1;i<=iValue;i++)
	    		pPlathoPlant[iPlant]->pPlathoGenotype->OptFruitNc[i].vInput = ReadDouble(hFile);
			for (i=1;i<=iValue;i++)
	    		pPlathoPlant[iPlant]->pPlathoGenotype->OptFruitNc[i].vOutput= ReadDouble(hFile);
			}  
	}
  	//----------------------------------------------------------
	//Leaf Min Nc
	while (longCode!=2000075) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->MinLvNc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->MinLvNc[0].fInput =(float)iValue;
    	pPlant->pGenotype->MinLvNc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->MinLvNc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->MinLvNc[i].fOutput= ReadFloat(hFile);
		}  
  	//----------------------------------------------------------
	TREES
	{
	//Branches Min Nc
	while (longCode!=2000076) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->MinBrNc[0].vInput =(double)iValue;

    if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->MinBrNc[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->MinBrNc[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MinBrNc[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MinBrNc[i].vOutput=	ReadDouble(hFile);
		}   
	}
	//----------------------------------------------------------
	//Stem Min Nc
	while (longCode!=2000077) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->MinStNc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->MinStNc[0].fInput =(float)iValue;
    	pPlant->pGenotype->MinStNc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->MinStNc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->MinStNc[i].fOutput= ReadFloat(hFile);
		}  
   	//----------------------------------------------------------
	//Fine Root Min Nc
	while (longCode!=2000078) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->MinRtNc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->MinRtNc[0].fInput =(float)iValue;
    	pPlant->pGenotype->MinRtNc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->MinRtNc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->MinRtNc[i].fOutput= ReadFloat(hFile);
		}  

   	//----------------------------------------------------------
	TREES
	{
	//Gross Root Min Nc
	while (longCode!=2000079) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->MinGRtNc[0].vInput =(double)iValue;

    if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->MinGRtNc[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->MinGRtNc[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MinGRtNc[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MinGRtNc[i].vOutput= ReadDouble(hFile);
		}  
	}
//----------------------------------------------------------
	POTATO
	{
	//Tuber Min Nc
	while (longCode!=20000795) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->MinTuberNc[0].vInput =(double)iValue;

    if (iValue>0)
    	{   
    	pPlathoPlant[iPlant]->pPlathoGenotype->MinTuberNc[0].vInput =(double)iValue;
    	pPlathoPlant[iPlant]->pPlathoGenotype->MinTuberNc[0].vOutput=(double)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MinTuberNc[i].vInput = ReadDouble(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlathoPlant[iPlant]->pPlathoGenotype->MinTuberNc[i].vOutput= ReadDouble(hFile);
		}  
	}
	else
	{
	//Fruit Min Nc
		while (longCode!=20000795) longCode=ReadLong(hFile);

		iValue = ReadInt(hFile);
		pPlathoPlant[iPlant]->pPlathoGenotype->MinFruitNc[0].vInput =(double)iValue;

		if (iValue>0)
    		{   
    		pPlathoPlant[iPlant]->pPlathoGenotype->MinFruitNc[0].vInput =(double)iValue;
    		pPlathoPlant[iPlant]->pPlathoGenotype->MinFruitNc[0].vOutput=(double)iValue;
	    	
			for (i=1;i<=iValue;i++)
	    		pPlathoPlant[iPlant]->pPlathoGenotype->MinFruitNc[i].vInput = ReadDouble(hFile);
			for (i=1;i<=iValue;i++)
	    		pPlathoPlant[iPlant]->pPlathoGenotype->MinFruitNc[i].vOutput= ReadDouble(hFile);
			}  
	}
	//=======================================================================
	//	Root Growth and Activity
	//=======================================================================
	//Parameters
	while (longCode!=2000080) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
    	{
			ReadSymbol(hFile,cChar);

			pPlant->pGenotype->fMaxRootExtRate	= ReadFloat(hFile);
			pPlant->pGenotype->fMaxRootDepth 	= ReadFloat(hFile);
			pPlathoPlant[iPlant]->pPlathoGenotype->vRelDepthMaxRoot = ReadDouble(hFile);
			
			for(i=1;i<iValue;i++)
			{
				ReadSymbol(hFile,cChar);
			
				if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
				{
					pPlant->pGenotype->fMaxRootExtRate	= ReadFloat(hFile);
					pPlant->pGenotype->fMaxRootDepth 	= ReadFloat(hFile);
					pPlathoPlant[iPlant]->pPlathoGenotype->vRelDepthMaxRoot = ReadDouble(hFile);
				}
				else
				{
					ReadFloat(hFile);
					ReadFloat(hFile);
					ReadDouble(hFile);
				}
			}
		}
	//----------------------------------------------------------
	//Parameters
	while (longCode!=2000081) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
    	{
            ReadSymbol(hFile,cChar);

            pPlant->pGenotype->fRootLengthRatio	= ReadFloat(hFile);

		    TREES
			    pPlathoPlant[iPlant]->pPlathoGenotype->vSpecGrossRootLength	= ReadDouble(hFile);

		    pPlant->pGenotype->fMaxWuptRate 	= ReadFloat(hFile);
		    pPlant->pGenotype->fMaxNuptRate		= ReadFloat(hFile);

            for(i=1;i<iValue;i++)
			{
				ReadSymbol(hFile,cChar);
			
				if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
				{
                    pPlant->pGenotype->fRootLengthRatio	= ReadFloat(hFile);

		            TREES
			            pPlathoPlant[iPlant]->pPlathoGenotype->vSpecGrossRootLength	= ReadFloat(hFile);

		            pPlant->pGenotype->fMaxWuptRate 	= ReadFloat(hFile);
		            pPlant->pGenotype->fMaxNuptRate		= ReadFloat(hFile);
				}
				else
				{
					ReadFloat(hFile);
                    
                    TREES
                        ReadDouble(hFile);

					ReadFloat(hFile);
					ReadFloat(hFile);
				}
			}

		}
	//----------------------------------------------------------
	//Root Extension Rate vs Temperature
	while (longCode!=2000082) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->RootExtRateTemp[0].fInput =(float)iValue;

   	pPlathoPlant[iPlant]->pPlathoGenotype->vRootExtTempMin 	= ReadDouble(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->vRootExtTempOpt 	= ReadDouble(hFile);
	pPlathoPlant[iPlant]->pPlathoGenotype->vRootExtTempMax 	= ReadDouble(hFile);

    if (iValue>0)
    	{   
    	pPlant->pGenotype->RootExtRateTemp[0].fInput =(float)iValue;
    	pPlant->pGenotype->RootExtRateTemp[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->RootExtRateTemp[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->RootExtRateTemp[i].fOutput= ReadFloat(hFile);
		}  

	//----------------------------------------------------------
	//Root Extension Rate vs Soil moisture
	while (longCode!=2000083) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
	pPlant->pGenotype->RootExtRateWc[0].fInput =(float)iValue;

    if (iValue>0)
    	{   
    	pPlant->pGenotype->RootExtRateWc[0].fInput =(float)iValue;
    	pPlant->pGenotype->RootExtRateWc[0].fOutput=(float)iValue;
    	
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->RootExtRateWc[i].fInput = ReadFloat(hFile);
        for (i=1;i<=iValue;i++)
	    	pPlant->pGenotype->RootExtRateWc[i].fOutput= ReadFloat(hFile);
		}  

	//=======================================================================
	//	Senesence
	//=======================================================================
	//Parameters
	while (longCode!=2000084) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
		pPlant->pGenotype->fBeginShadeLAI = ReadFloat(hFile); 

	 //------------------------------------------------------------
	 //Loss rates
	while (longCode!=2000085) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
    {
		 HERBS
		 {
			pPlathoPlant[iPlant]->pPlathoGenotype->vFineRootLossR 	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vStemLossR		= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vLeafLossR 		= ReadDouble(hFile); 
			
			POTATO
		 		pPlathoPlant[iPlant]->pPlathoGenotype->vTuberLossR 		= ReadDouble(hFile); 
			else
				pPlathoPlant[iPlant]->pPlathoGenotype->vFruitLossR 		= ReadDouble(hFile); 

			 
		 }
		 TREES
		 {
			pPlathoPlant[iPlant]->pPlathoGenotype->vFineRootLossR 	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vGrossRootLossR 	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vStemLossR 		= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vBranchLossR 	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vLeafLossR 		= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vFruitLossR 		= ReadDouble(hFile); 
		 }
	}


	 //------------------------------------------------------------
	 //Relocation fractions of dying organs
	 while (longCode!=2000086) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
    	{
		 HERBS
		 {
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcFineRoot 	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcStem		= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcLeaf		= ReadDouble(hFile); 
			
			POTATO
				 pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcTuber		= ReadDouble(hFile);
			else
				pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcFruit		= ReadDouble(hFile); 

			 
		 }
		 TREES
		 {
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcFineRoot	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcGrossRoot	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcStem 		= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcBranch	= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcLeaf		= ReadDouble(hFile); 
			pPlathoPlant[iPlant]->pPlathoGenotype->vAssRlcFruit		= ReadDouble(hFile); 
		 }
		 
		}


	//----------------------------------------------------------
	//Relative death rate temperature leaves
	while (longCode!=2000087) longCode=ReadLong(hFile);
	
    iValue = ReadInt(hFile);  
	pPlant->pGenotype->LvDeathRateTemp[0].fInput =(float)iValue;

	if (iValue>0)
		{   
		pPlant->pGenotype->LvDeathRateTemp[0].fInput =(float)iValue;
		pPlant->pGenotype->LvDeathRateTemp[0].fOutput=(float)iValue;
		    	
		for (i=1;i<=iValue;i++)
			pPlant->pGenotype->LvDeathRateTemp[i].fInput = ReadFloat(hFile);
		for (i=1;i<=iValue;i++)
			pPlant->pGenotype->LvDeathRateTemp[i].fOutput= ReadFloat(hFile);
		}  

	//----------------------------------------------------------
	//Relative death rate temperature roots
	while (longCode!=2000088) longCode=ReadLong(hFile);
	
    iValue = ReadInt(hFile);     
	pPlant->pGenotype->RtDeathRateTemp[0].fInput =(float)iValue;

	if (iValue>0)
		{   
		pPlant->pGenotype->RtDeathRateTemp[0].fInput =(float)iValue;
		pPlant->pGenotype->RtDeathRateTemp[0].fOutput=(float)iValue;
		    	
		for (i=1;i<=iValue;i++)
			pPlant->pGenotype->RtDeathRateTemp[i].fInput = ReadFloat(hFile);
		for (i=1;i<=iValue;i++)
			pPlant->pGenotype->RtDeathRateTemp[i].fOutput= ReadFloat(hFile);
		}  


	//------------------------------------------------------------
    // DEFENCE:
    //------------------------------------------------------------

	while (longCode!=2000090) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

		pPPltStress->vConstDefenseBase = ReadDouble(hFile);
		pPPltStress->vConstDefense    = ReadDouble(hFile); 

		for(i=1;i<iValue;i++)
		{
			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
		        pPPltStress->vConstDefenseBase = ReadDouble(hFile);
		        pPPltStress->vConstDefense    = ReadDouble(hFile); 
			}
			else
			{
				ReadDouble(hFile);
				ReadDouble(hFile);
			}
		}	//end for
	 }	//end if iValue

 	while (longCode!=2000091) longCode=ReadLong(hFile);

		pBiochemistry->pCO2Required->vDefComp		= ReadDouble(hFile);
		pBiochemistry->pGrowthEfficiency->vDefComp	= ReadDouble(hFile);
		pPPltStress->vDefCompTurnoverR0	            = ReadDouble(hFile);


     //Ozone
	while (longCode!=2000092) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

		pPPltStress->vO3IndDefense	    = ReadDouble(hFile);
		pPPltStress->vO3DamageRate	    = ReadDouble(hFile);
		pPPltStress->vO3DefenseEff	    = ReadDouble(hFile);
		pPPltStress->vO3Sensitivity	    = ReadDouble(hFile);

		for(i=1;i<iValue;i++)
		{
			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
                pPPltStress->vO3IndDefense	    = ReadDouble(hFile);
		        pPPltStress->vO3DamageRate	    = ReadDouble(hFile);
		        pPPltStress->vO3DefenseEff	    = ReadDouble(hFile);
		        pPPltStress->vO3Sensitivity	    = ReadDouble(hFile);
			}
			else
			{
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
			}
		}	//end for
	 }	//end if iValue

    if(pPlathoModules->iFlagO3 == 4)
    {
        //Ozone van Oijen
	    while (longCode!=2000925) longCode=ReadLong(hFile);

        iValue = ReadInt(hFile);
        if (iValue>0)
	    {
		    ReadSymbol(hFile,cChar);

		   pPGen->cdetox	    = ReadDouble(hFile);
		   pPGen->crepair	    = ReadDouble(hFile);
		   pPGen->fdetox	    = ReadDouble(hFile);
		   pPGen->frepair	    = ReadDouble(hFile);
		   pPGen->dO3damage	    = ReadDouble(hFile);
		   pPGen->dRubiscoConcCrit = ReadDouble(hFile);

		    for(i=1;i<iValue;i++)
		    {
			    ReadSymbol(hFile,cChar);

			    if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			    {
		            pPGen->cdetox	    = ReadDouble(hFile);
		            pPGen->crepair	    = ReadDouble(hFile);
		            pPGen->fdetox	    = ReadDouble(hFile);
		            pPGen->frepair	    = ReadDouble(hFile);
		            pPGen->dO3damage	= ReadDouble(hFile);
		            pPGen->dRubiscoConcCrit = ReadDouble(hFile);
			    }
			    else
			    {
				    ReadDouble(hFile);
				    ReadDouble(hFile);
				    ReadDouble(hFile);
				    ReadDouble(hFile);
				    ReadDouble(hFile);
				    ReadDouble(hFile);
			    }
		    }	//end for
	    }	//end if iValue
    }


	//Leaf Pathogenes
	while (longCode!=2000093) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

        pPPltStress->vLfPathIndDefense	    = ReadDouble(hFile);
		pPPltStress->vLfPathDamageRate	    = ReadDouble(hFile);
		pPPltStress->vLfPathDefenseEff	    = ReadDouble(hFile);
		pPPltStress->vLfPathBeta			= ReadDouble(hFile);

		for(i=1;i<iValue;i++)
		{

			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
		        pPPltStress->vLfPathIndDefense	    = ReadDouble(hFile);
		        pPPltStress->vLfPathDamageRate	    = ReadDouble(hFile);
		        pPPltStress->vLfPathDefenseEff	    = ReadDouble(hFile);
		        pPPltStress->vLfPathBeta			= ReadDouble(hFile);
			}
			else
			{
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
			}
		}	//end for
	 }	//end if iValue


	//Root Pathogenes
	while (longCode!=2000094) longCode=ReadLong(hFile);

    iValue = ReadInt(hFile);
     if (iValue>0)
	 {
		ReadSymbol(hFile,cChar);

		pPPltStress->vRtPathIndDefense	    = ReadDouble(hFile);
		pPPltStress->vRtPathDamageRate	    = ReadDouble(hFile);
		pPPltStress->vRtPathDefenseEff	    = ReadDouble(hFile);
		pPPltStress->vRtPathBeta			= ReadDouble(hFile);

		for(i=1;i<iValue;i++)
		{

			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
		        pPPltStress->vRtPathIndDefense	    = ReadDouble(hFile);
		        pPPltStress->vRtPathDamageRate	    = ReadDouble(hFile);
		        pPPltStress->vRtPathDefenseEff	    = ReadDouble(hFile);
		        pPPltStress->vRtPathBeta			= ReadDouble(hFile);
			}
			else
			{
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
			}
		}	//end for
	 }	//end if iValue


	_lclose(hFile);

//	ReleaseDC(hwndRead,hdc);
//  DestroyWindow(hwndRead);

	return 1;
	}
	





int PLATHO_Simulation_Scenario(PTIME pTi, PPLANT pPlant, 
						 int iPlant, PMSOWINFO pMaSowInfo, PPLATHOSCENARIO pPlathoScenario, 
						 PSPROFILE pSo, PMANAGEMENT pMa, PLOCATION pLo)
{
	////////////////////////////////////////////////////////////////////
	//	Einlesen der Szenariodaten für PLATHO
	//
	//	zu Beginn:
	//		Anzahl der Reihen und Spalten im Versuchsaufbau (iRows, iColumns)
	//		zeitliche Auflösung des Szenarios (daily/high)
	//
	//	bei beginnendem Pflanzenwachstum:
	//		Alter
	//		Entwicklungsstadium
	//		Pflanzenhöhe
	//		Biomasseverteilung
	//		verfügbare Reserven
	//		N-Status
	//
	//	täglich:
	//		vCO2				:CO2-Gehalt der Atmosphäre
	//		vO3ConcMin			:min. O3-Gehalt der Atmosphäre
	//		vO3ConcMax			:max. O3-Gehalt der Atmosphäre
	//		fLeafPathogenes		:Befall durch Pathogene (Blatt)
	//		fRootPathogenes		:Befall durch Pathogene (Wwurzel)
	////////////////////////////////////////////////////////////////////

	OFSTRUCT	ofStruct;
	HFILE		hFile;
	long 		longCode;
	static int  iCounter;
	char		cChar[20];

	float	fBranchFrac, fUGrWoodFrac;
	float	fFRootFrac,fGRootFrac;
	float	fFinalBranchF = (float)0.1;
	float	wood1;

    int		i,iValue, iValue2, iValue3;
    int     iSector;
	double	test;

	//===================================================================================
	//The simulation project file name
	//===================================================================================
    char	cScenFileName[50];

	//*.xnm --> platho\*.psc
        
	strcpy(cScenFileName,"Platho4Input\\");
	strcat(cScenFileName,pLo->pFieldplot->acModFileName);
	iValue3=strlen(cScenFileName);
	cScenFileName[iValue3-3]='\0'; 	
	lstrcat(cScenFileName,"psc");

	
	//===================================================================================
	//Read simulation project data
	//===================================================================================
	hFile=OpenFile(cScenFileName,(LPOFSTRUCT)&ofStruct, OF_EXIST);

	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	//Display elements
	if (hFile==HFILE_ERROR)
        {
		Message(1,"Platho simulation scenario not found ! File Open Error");
		return 0;
        }        

	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
	//=======================================================================
	//	Get the pointer, open the file
	//=======================================================================

	hFile=OpenFile(cScenFileName,(LPOFSTRUCT)&ofStruct, OF_READ);
	longCode=0;

	if (iPlant>=0)
	{
	PPLATHOBIOMASS	pPBiom = pPlathoPlant[iPlant]->pPlathoBiomass;
	PPLANTSTRESS	pPPltStress = pPlathoPlant[iPlant]->pPlantStress;
	PPLATHOMORPHOLOGY	pPMorph	= pPlathoPlant[iPlant]->pPlathoMorphology;

	while (longCode!=4000001) longCode=ReadLong(hFile);
	if((pTi->pSimTime->lTimeDate==pTi->pSimTime->lStartDate)&&(iCounter==0))
	{
//		if(pPlathoScenario->nPlants == 0)
		if(pPlant == NULL)
		{
			Message(1,"No plant data in data base Xn_daten.mdb!!!");
			return 0;
		}

		iValue = ReadInt(hFile);
		if (iValue>0)
    	{
			pPlathoScenario->iRows			= ReadInt(hFile);
			pPlathoScenario->iColumns		= ReadInt(hFile);

            pPlathoScenario->vLatticeSpacing = ReadDouble(hFile);
		}

				
	
	while (longCode!=4000008) longCode=ReadLong(hFile);
		ReadSymbol(hFile,cChar);
		lstrcpy(pPlathoScenario->acResolution,cChar);

        if(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"constant")
            ||(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"Constant"))
            ||(!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"const")))
        {            
            pPlathoScenario->vPhotoperiod = ReadDouble(hFile);
        }

		_lclose(hFile);

		iCounter =1;

		Message(1,"Reading scenario file");

		return 1;

	}


	if(pTi->pSimTime->fTimeAct == (float)pMaSowInfo->iDay)//start of plant growth
	{
		// Null setzen der in der ModLib initialisierten Variablen
		pPMorph->vPlantLAI		        = (float)0.0;
		pPlant->pCanopy->fPlantLA	= (float)0.0;

		pPlant->pPltNitrogen->fRootActConc = (float)0.0;
		pPlant->pPltNitrogen->fStemActConc = (float)0.0;
		pPlant->pPltNitrogen->fLeafActConc = (float)0.0;
		
		pPlant->pBiomass->fRootWeight = (float)0.0;
		pPlant->pBiomass->fStemWeight = (float)0.0;
		pPlant->pBiomass->fLeafWeight = (float)0.0;
		pPlant->pBiomass->fStovWeight = (float)0.0;

		while (longCode!=4000002) longCode=ReadLong(hFile);
		iValue = ReadInt(hFile);

		if (iValue>0)
    	{
			ReadSymbol(hFile,cChar);

			TREES
    			pPlant->pDevelop->fAge =ReadFloat(hFile);

			pPlant->pDevelop->fDevStage =ReadFloat(hFile);
			pPlant->pCanopy->fPlantHeight=ReadFloat(hFile);

			TREES
				pPMorph->vCrownDiameter = ReadDouble(hFile);

			pPlant->pRoot->fDepth=ReadFloat(hFile);

			TREES
				pPMorph->vRootZoneDiameter = ReadDouble(hFile);

            for(i=1;i<iValue;i++)
			{

				ReadSymbol(hFile,cChar);


				if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
				{
					TREES
						pPlant->pDevelop->fAge		= ReadFloat(hFile);

					pPlant->pDevelop->fDevStage		= ReadFloat(hFile);
					pPlant->pCanopy->fPlantHeight	= ReadFloat(hFile);
					
					TREES
						pPMorph->vCrownDiameter		= ReadDouble(hFile);

					pPlant->pRoot->fDepth			= ReadFloat(hFile);

                    TREES
				        pPMorph->vRootZoneDiameter = ReadDouble(hFile);
				}
				else
				{
					ReadFloat(hFile);
					ReadFloat(hFile);
					ReadFloat(hFile);
					
					TREES
					{
						ReadDouble(hFile);
						ReadFloat(hFile);
						ReadDouble(hFile);
					}
				}
			} //end varieties 
		}// end iValue > 0

		TREES //von der Pflanze bedeckte Bodenfläche [m^2]
		{
			if(pPMorph->vCrownDiameter==(float)0.0)
				pPMorph->vCrownArea = (float)1.0/pPlathoScenario->vTotalPlantDensity;
			else
				pPMorph->vCrownArea =(float)(PI/4.0 * 
						pow((double)pPMorph->vCrownDiameter,2.0));

			//Begrenzung des maximalen Kronendurchmessers
            if(pPMorph->vCrownArea > (float)PI/pPlathoScenario->vTotalPlantDensity)
				pPMorph->vCrownArea = (float)PI/pPlathoScenario->vTotalPlantDensity;	
		}

	while (longCode!=4000003) longCode=ReadLong(hFile);
		    
	iValue2 = ReadInt(hFile);
	if (iValue2>0)
    {
		ReadSymbol(hFile,cChar);

		pPlant->pBiomass->fRootWeight=ReadFloat(hFile);

		TREES
			pPlant->pBiomass->fGrossRootWeight=ReadFloat(hFile);

		pPlant->pBiomass->fStemWeight=ReadFloat(hFile);

		TREES
		{
			pPlant->pBiomass->fBranchWeight=ReadFloat(hFile);
				
			pPlant->pBiomass->fWoodWeight = pPlant->pBiomass->fStemWeight
				+ pPlant->pBiomass->fBranchWeight + pPlant->pBiomass->fGrossRootWeight;
		}

		pPlant->pBiomass->fLeafWeight=ReadFloat(hFile);


			
		for(i=1;i<iValue2;i++)
		{

			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
				pPlant->pBiomass->fRootWeight=ReadFloat(hFile);

				TREES
					pPlant->pBiomass->fGrossRootWeight=ReadFloat(hFile);
					
				pPlant->pBiomass->fStemWeight=ReadFloat(hFile);

				TREES
				{
					pPlant->pBiomass->fBranchWeight=ReadFloat(hFile);
					
					pPlant->pBiomass->fWoodWeight = pPlant->pBiomass->fStemWeight
						+ pPlant->pBiomass->fBranchWeight + pPlant->pBiomass->fGrossRootWeight;
				}

				pPlant->pBiomass->fLeafWeight=ReadFloat(hFile);
			}
			else
			{
				ReadFloat(hFile);
				ReadFloat(hFile);
				ReadFloat(hFile);
				
				TREES
				{
					ReadFloat(hFile);
					ReadFloat(hFile);
				}
			}
		} //end varieties 
	}

	TREES	//modifiziert nach FAGUS
	{
		if(iValue2 == 0)
		{
			//Zweig/Äste-Anteil
			fBranchFrac	= (float)(fFinalBranchF + (1.0 - fFinalBranchF) * exp(-pow(5.0,pPlant->pDevelop->fAge/150. -1)));

			//underground wood fraction
			fUGrWoodFrac = (float)(0.8 * exp(-0.37 * pPlant->pDevelop->fAge) + 0.15);	


			//wood1 in [t(CH2O)/ha]
			wood1 = (float)0.001*pPlant->pBiomass->fStemWeight/((float)1.0-fUGrWoodFrac)/((float)1.0-fBranchFrac);
		
			test = 1.0;

			while(test>0.1)
			{
			//gross root fraction
			if(wood1<(float)5.0)
				fGRootFrac = (float)0.0736*wood1-(float)0.00673*wood1*wood1;
			else
				fGRootFrac = (float)(0.12+0.08*exp(-0.00035*((double)wood1-5.0)*((double)wood1-5.0)));
			//fine root fraction
			fFRootFrac = (float)(0.015+0.253*exp(-0.4224*(double)wood1));

			fUGrWoodFrac = fGRootFrac + fFRootFrac;	//with the used formula ugwdf until Wood = 5 is 0.25,
													//then it decreases succesively to 0.14
			pPlant->pBiomass->fWoodWeight = (float)0.001*pPlant->pBiomass->fStemWeight/((float)1.0-fUGrWoodFrac)/((float)1.0-fBranchFrac); //[t(CH2O)/ha]

			test = fabs((double)(pPlant->pBiomass->fWoodWeight-wood1)); //Abbruchkriterium

			wood1 = pPlant->pBiomass->fWoodWeight;
			}

		
		//-----------------------------------------------------------------------------------------

		//Umrechnung auf [kg(CH2O)/ha]:
		pPlant->pBiomass->fWoodWeight = pPlant->pBiomass->fWoodWeight*(float)1000.0;

		//Grobwurzeln [kg(CH2O)/ha]:
		pPlant->pBiomass->fGrossRootWeight = pPlant->pBiomass->fWoodWeight * fGRootFrac;	 
		//Feinwurzeln [kg(CH2O)/ha]:
		pPlant->pBiomass->fRootWeight = pPlant->pBiomass->fWoodWeight * fFRootFrac;	 
		//Äste/Zweige [kg(CH2O)/ha]:
		pPlant->pBiomass->fBranchWeight = pPlant->pBiomass->fWoodWeight * ((float)1.0-fUGrWoodFrac)*fBranchFrac;	 
	}

	pPlant->pBiomass->fBiomassAbvGround = pPlant->pBiomass->fStemWeight + pPlant->pBiomass->fBranchWeight + pPlant->pBiomass->fLeafWeight;
    pPlant->pBiomass->fTotRootWeight    = pPlant->pBiomass->fRootWeight + pPlant->pBiomass->fGrossRootWeight;
	pPlant->pBiomass->fTotalBiomass     = pPlant->pBiomass->fBiomassAbvGround + pPlant->pBiomass->fTotRootWeight;
	}


    //Startwerte "Aboveground biomass distribution"
	while (longCode!=4000004) longCode=ReadLong(hFile);
				
	iValue = ReadInt(hFile);
	if (iValue>0)
	{
		ReadSymbol(hFile,cChar);

		for(iSector=0;iSector<4;iSector++)
            pPMorph->pLeafLayer->apLeafSector[iSector]->vAbvgDistrFac = ReadDouble(hFile);

		for(i=1;i<iValue;i++)
		{

			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
		        for(iSector=0;iSector<4;iSector++)
                    pPMorph->pLeafLayer->apLeafSector[iSector]->vAbvgDistrFac = ReadDouble(hFile);
			}
			else
			{
				ReadFloat(hFile);
				ReadFloat(hFile);
				ReadFloat(hFile);
				ReadFloat(hFile);
			}
		}
	}
		
    //Startwerte "below ground biomass distribution"
	while (longCode!=4000005) longCode=ReadLong(hFile);
				
	iValue = ReadInt(hFile);
	if (iValue>0)
	{
		ReadSymbol(hFile,cChar);

		for(iSector=0;iSector<4;iSector++)
            pPMorph->pPlathoRootLayer->apRootSector[iSector]->vBlwgDistrFac = ReadDouble(hFile);

		for(i=1;i<iValue;i++)
		{

			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
		        for(iSector=0;iSector<4;iSector++)
                    pPMorph->pPlathoRootLayer->apRootSector[iSector]->vBlwgDistrFac = ReadDouble(hFile);
			}
			else
			{
				ReadFloat(hFile);
				ReadFloat(hFile);
				ReadFloat(hFile);
				ReadFloat(hFile);
			}
		}
	}
		

    //Startwerte "Defensive Compounds"

	while (longCode!=4000006) longCode=ReadLong(hFile);
				
	iValue = ReadInt(hFile);
	if (iValue>0)
	{
		ReadSymbol(hFile,cChar);

		pPPltStress->vFineRootDefConc = ReadDouble(hFile);
		pPPltStress->vLeafDefConc = ReadDouble(hFile);
		pPPltStress->vStemDefConc = ReadDouble(hFile);

        POTATO
	    	pPPltStress->vTuberDefConc = ReadDouble(hFile);
        else
            pPPltStress->vFruitDefConc = ReadDouble(hFile);


		for(i=1;i<iValue;i++)
		{

			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{
		        pPPltStress->vFineRootDefConc = ReadDouble(hFile);
		        pPPltStress->vLeafDefConc = ReadDouble(hFile);
		        pPPltStress->vStemDefConc = ReadDouble(hFile);

                POTATO
	    	        pPPltStress->vTuberDefConc = ReadDouble(hFile);
                else
                    pPPltStress->vFruitDefConc = ReadDouble(hFile);
			}
			else
			{
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
				ReadDouble(hFile);
			}
		}

        pPPltStress->vGrossRootDefConc = pPPltStress->vStemDefConc;
        pPPltStress->vBranchDefConc = pPPltStress->vStemDefConc;
	}
		


	while (longCode!=4000007) longCode=ReadLong(hFile);

	iValue = ReadInt(hFile);
	if (iValue>0)
    {
		ReadSymbol(hFile,cChar);

		pPlant->pPltCarbon->fCAssimilatePool = ReadFloat(hFile);
		pPlant->pBiomass->fStemReserveWeight=ReadFloat(hFile);
		pPlant->pPltNitrogen->fNStressFac=ReadFloat(hFile);

		for(i=1;i<iValue;i++)
		{
			ReadSymbol(hFile,cChar);

			if(!(lstrcmp((LPSTR)pPlant->pGenotype->acVarietyName,cChar)))
			{			
				pPlant->pPltCarbon->fCAssimilatePool = ReadFloat(hFile);
				pPlant->pBiomass->fStemReserveWeight=ReadFloat(hFile);
				pPlant->pPltNitrogen->fNStressFac=ReadFloat(hFile);
			}
			else
			{
				ReadFloat(hFile);
				ReadFloat(hFile);
				ReadFloat(hFile);
			}
		}
				
		TREES
			pPlant->pBiomass->fSeedReserv =(float)0.0; 
		HERBS
		{
			if(pPlant->pDevelop->fDevStage < (float)2.0)
				pPlant->pBiomass->fSeedReserv =(float)(pPBiom->vSeedWeight/1000);
			else
				pPlant->pBiomass->fSeedReserv =(float)0.0; //sind bei iStagePLATHO=2 wirklich schon alle Seed-Reserven leer?
		}
	}
	else
	{
		TREES
		{
			pPlant->pPltCarbon->fCAssimilatePool = (float)0.05*pPlant->pBiomass->fStemWeight+(float)0.1*pPlant->pBiomass->fLeafWeight;
			pPlant->pBiomass->fStemReserveWeight=(float)0.15*pPlant->pBiomass->fStemWeight;
			pPlant->pPltNitrogen->fNStressFac=(float)1.0;
			pPlant->pBiomass->fSeedReserv =(float)0.0; 
		}
		HERBS
		{
			if(pPlant->pDevelop->fDevStage <= (float)1.0)
			{
				// Daten aus Souci et al. 1989
				WHEAT
				{
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.012*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.67*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.02*pPBiom->vSeedWeight/1000);
				}
				BARLEY
				{
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.022*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.73*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.017*pPBiom->vSeedWeight/1000);
				}
				MAIZE
				{
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.185*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.70*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.015*pPBiom->vSeedWeight/1000);
				}
				SUNFLOWER
				{
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.01*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.132*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.0365*pPBiom->vSeedWeight/1000);
				}
				POTATO
				{
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.032*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.64*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.014*pPBiom->vSeedWeight/1000);
				}
				ALFALFA
				{	//wie Weizen
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.012*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.67*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.02*pPBiom->vSeedWeight/1000);
				}
				LOLIUM
				{	//wie Weizen
					pPlant->pPltCarbon->fCAssimilatePool=(float)(0.012*pPBiom->vSeedWeight/1000);
					pPlant->pBiomass->fStemReserveWeight=(float)(0.67*pPBiom->vSeedWeight/1000);
					pPlant->pPltNitrogen->fNPool=(float)(0.02*pPBiom->vSeedWeight/1000);
				}

				pPlant->pBiomass->fSeedReserv =(float)(pPBiom->vSeedWeight/1000);
				pPlant->pPltNitrogen->fNStressFac=(float)1.0;
				
			}
			else
			{
				pPlant->pPltCarbon->fCAssimilatePool = (float)0.05*pPlant->pBiomass->fStemWeight+(float)0.1*pPlant->pBiomass->fLeafWeight;
				pPlant->pBiomass->fStemReserveWeight=(float)0.15*pPlant->pBiomass->fStemWeight;
				pPlant->pBiomass->fSeedReserv =(float)0.0; //sind bei iStagePLATHO=1 wirklich schon alle Seed-Reserven leer?
				pPlant->pPltNitrogen->fNStressFac=(float)1.0;
			}
		}
	}


	TREES
		PLATHO_StartValues(pPlant, iPlant, pSo, pMa);
	HERBS
	{
		if(pPlant->pDevelop->fDevStage >= 1)
			PLATHO_StartValues(pPlant, iPlant, pSo, pMa);
	}

	}//end Start of plant growth
}
else
{

	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////

	
	//	tägliches Einlesen des Stress-Szenarios
	while (longCode!=pTi->pSimTime->lTimeDate) longCode=ReadLong(hFile);
		pPlathoScenario->vCO2Day = ReadDouble(hFile);
	    pPlathoScenario->vO3ConcMin = ReadDouble(hFile);
	    pPlathoScenario->vO3ConcMax = ReadDouble(hFile);
	    pPlathoScenario->vLeafPathogenes = ReadDouble(hFile);
	    pPlathoScenario->vRootPathogenes = ReadDouble(hFile);
}


	_lclose(hFile);

	return 1;
}




int WINAPI ReadClimateData(PPLATHOCLIMATE pPlathoClimate, PTIME pTi)
{
	//	Einleseroutine für Klimadaten, die genauer 
	//	aufgelöst sind als Tageswerte

	
	HFILE		hFile;	
	OFSTRUCT	ofStruct;
	
	PPLATHOCLIMATE pPC;

	long	longCode;
	//float	tt;
	//===================================================================================
	//The simulation project file name
	//===================================================================================
    
	char	cGenFileName[50];

	lstrcpy(cGenFileName,"Platho4Input\\Platho_Climate.hrc\0");

	hFile=OpenFile(cGenFileName,(LPOFSTRUCT)&ofStruct, OF_EXIST);


	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	//Display elements
	if (hFile==HFILE_ERROR)
        {
		//Message(1,"Platho climate data not found ! File Open Error");
		return 0;
        }                         
       
		//Message(1,"Reading climate file");
    
	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
	//=======================================================================
	//	Get the pointer, open the file
	//=======================================================================

	hFile=OpenFile(cGenFileName,(LPOFSTRUCT)&ofStruct, OF_READ);

	longCode =0;
	//tt = 0.0f;

	
	pPC = pPlathoClimate;

	while(longCode!=1000000) longCode=ReadLong(hFile);

	for (pPC = pPlathoClimate;(pPC!=NULL);pPC = pPC->pNext) 
	{
		pPC->vSimTime	= ReadFloat(hFile);
		if((pPC->pBack!=NULL)&&(pPC->vSimTime<=pPC->pBack->vSimTime))
			break;
		//pPC->vRad		= ReadDouble(hFile);
		pPC->vPAR		= ReadDouble(hFile);
		pPC->vTemp		= ReadDouble(hFile);
		pPC->vCO2		= ReadDouble(hFile);
		pPC->vHumidity	= ReadDouble(hFile);
		//pPC->vO3		= ReadDouble(hFile);

	}
	
	
/*
		while(longCode!=1000000) longCode=ReadLong(hFile);

		while(pPlathoClimate->fSimTime<=pTi->pSimTime->fTimeAct) 
		{
			pPlathoClimate->fSimTime	= ReadFloat(hFile);
			if(pPlathoClimate->fSimTime <= pTi->pSimTime->fTimeAct)
				tt = pPlathoClimate->fSimTime;
			else
			{
				pPlathoClimate->fSimTime = tt;
				break;
			}

			//pPlathoClimate->fRad		= ReadFloat(hFile);
			pPlathoClimate->fPAR		= ReadFloat(hFile);
			pPlathoClimate->fTemp		= ReadFloat(hFile);
			pPlathoClimate->vCO2		= ReadFloat(hFile);
			pPlathoClimate->fHumidity	= ReadFloat(hFile);
			//pPlathoClimate->fO3		= ReadFloat(hFile);
	}

*/
	
	_lclose(hFile);

	return 1;
}

int ReadOutputList(PPLATHOSCENARIO pPlathoScenario)
{
  	OFSTRUCT	ofStruct;
	HFILE		hFile;

	long 		longCode;
	char  		cChar[20];
	
   
	//===================================================================================
	//Open output variables list
	//===================================================================================
	hFile=OpenFile("Platho4Input\\Platho.out\0",(LPOFSTRUCT)&ofStruct, OF_EXIST);
   // i=_lclose(hFile);

	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	//Display elements
	if (hFile==HFILE_ERROR)
        {
		Message(1,"Output variables list not found! File Open Error");
		return 0;
        }                         
       
		Message(1,"Reading Output variables list ");
    
	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
	//=======================================================================
	//	Get the pointer, open the file
	//=======================================================================

	hFile=OpenFile("Platho4Input\\Platho.out\0",(LPOFSTRUCT)&ofStruct, OF_READ);
	longCode=0;

	//=======================================================================
	//	"d": one value per day, "h" one value per time step 
	//=======================================================================
	while (longCode!=50000) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(h)"))||(!lstrcmp(cChar,(LPSTR)"(H)")))
		pPlathoScenario->abPlatho_Out[0]=FALSE;
	else
		pPlathoScenario->abPlatho_Out[0]=TRUE;

	//=======================================================================
	//	"h": biomass in kg/ha, "p" biomass in kg/plant 
	//=======================================================================
	while (longCode!=59999) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(h)"))||(!lstrcmp(cChar,(LPSTR)"(H)")))
		pPlathoScenario->abPlatho_Out[99]=FALSE;
	else
		pPlathoScenario->abPlatho_Out[99]=TRUE;

	//=======================================================================
	//	development stage 
	//=======================================================================
	while (longCode!=50001) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[1]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[1]=FALSE;

	//=======================================================================
	//	fine roots 
	//=======================================================================
	while (longCode!=50002) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[2]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[2]=FALSE;

	//=======================================================================
	//	gross roots 
	//=======================================================================
	while (longCode!=50003) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[3]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[3]=FALSE;

	//=======================================================================
	//	stem  
	//=======================================================================
	while (longCode!=50004) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[4]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[4]=FALSE;

	//=======================================================================
	//	branches  
	//=======================================================================
	while (longCode!=50005) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[5]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[5]=FALSE;

	//=======================================================================
	//	stem+branches 
	//=======================================================================
	while (longCode!=50006) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[6]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[6]=FALSE;

	//=======================================================================
	//	wood  
	//=======================================================================
	while (longCode!=50007) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[7]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[7]=FALSE;

	//=======================================================================
	//	leaves  
	//=======================================================================
	while (longCode!=50008) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[8]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[8]=FALSE;

	//=======================================================================
	//	fruits/tubers
	//=======================================================================
	while (longCode!=50009) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[9]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[9]=FALSE;

	//=======================================================================
	//	total biomass 
	//=======================================================================
	while (longCode!=50010) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[10]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[10]=FALSE;

	//=======================================================================
	//	assimilate pool 
	//=======================================================================
	while (longCode!=50011) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[11]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[11]=FALSE;

	//=======================================================================
	//	storage 
	//=======================================================================
	while (longCode!=50012) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[12]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[12]=FALSE;

	//=======================================================================
	//	defensive compounds 
	//=======================================================================
	while (longCode!=50013) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[13]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[13]=FALSE;

	//=======================================================================
	//	defensive compounds concentration (leaves) 
	//=======================================================================
	while (longCode!=50014) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[14]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[14]=FALSE;

	//=======================================================================
	//	defensive compounds concentration (roots)
	//=======================================================================
	while (longCode!=50015) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[15]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[15]=FALSE;

	//=======================================================================
	//	gross photosynthesis 
	//=======================================================================
	while (longCode!=50016) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[16]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[16]=FALSE;

	//=======================================================================
	//	maintenance respiration 
	//=======================================================================
	while (longCode!=50017) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[17]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[17]=FALSE;

	//=======================================================================
	//	growth respiration 
	//=======================================================================
	while (longCode!=50018) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[18]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[18]=FALSE;

	//=======================================================================
	//	cost for growth
	//=======================================================================
	while (longCode!=50019) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[19]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[19]=FALSE;

	//=======================================================================
	//	cost for defense 
	//=======================================================================
	while (longCode!=50020) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[20]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[20]=FALSE;

	//=======================================================================
	//	cost for maintenance 
	//=======================================================================
	while (longCode!=50021) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[21]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[21]=FALSE;

	//=======================================================================
	//	plant height 
	//=======================================================================
	while (longCode!=50022) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[22]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[22]=FALSE;

	//=======================================================================
	//	stem diameter 
	//=======================================================================
	while (longCode!=50023) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[23]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[23]=FALSE;

	//=======================================================================
	//	crown area 
	//=======================================================================
	while (longCode!=50024) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[24]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[24]=FALSE;

	//=======================================================================
	//	crown volume 
	//=======================================================================
	while (longCode!=50124) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[98]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[98]=FALSE;

	//=======================================================================
	//	leaf area
	//=======================================================================
	while (longCode!=50025) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[25]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[25]=FALSE;

	//=======================================================================
	//	leaf area per layer 
	//=======================================================================
	while (longCode!=50026) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[26]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[26]=FALSE;

	//=======================================================================
	//	leaf area per sector 
	//=======================================================================
	while (longCode!=50027) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[27]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[27]=FALSE;

	//=======================================================================
	//	distribution of specific leaf weight 
	//=======================================================================
	while (longCode!=50028) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[28]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[28]=FALSE;

	//=======================================================================
	//	distribution of leaf nitrogen
	//=======================================================================
	while (longCode!=50029) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[29]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[29]=FALSE;

	//=======================================================================
	//	LAI  
	//=======================================================================
	while (longCode!=50030) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[30]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[30]=FALSE;

	while (longCode!=500302)
    {
        longCode=ReadLong(hFile);
        if(longCode == 50031) goto label_50031;
    }
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[90]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[90]=FALSE;

	//=======================================================================
	//	root depth
	//=======================================================================
	while (longCode!=50031) longCode=ReadLong(hFile);
	
label_50031:
    ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[31]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[31]=FALSE;

	//=======================================================================
	//	root zone area 
	//=======================================================================
	while (longCode!=50131) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[97]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[97]=FALSE;

	//=======================================================================
	//	root volume 
	//=======================================================================
	while (longCode!=50231) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[96]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[96]=FALSE;

	//=======================================================================
	//	root length per layer 
	//=======================================================================
	while (longCode!=50032) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[32]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[32]=FALSE;

	//=======================================================================
	//	root length per sector 
	//=======================================================================
	while (longCode!=50033) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[33]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[33]=FALSE;

	//=======================================================================
	//	competition coefficient 
	//=======================================================================
	while (longCode!=50034) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[34]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[34]=FALSE;

	//=======================================================================
	//	carbon availability 
	//=======================================================================
	while (longCode!=50035) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[35]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[35]=FALSE;

	//=======================================================================
	//	nitrogen availability 
	//=======================================================================
	while (longCode!=50036) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[36]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[36]=FALSE;

	//=======================================================================
	//	leaf internal O3-concentration 
	//=======================================================================
	while (longCode!=50037) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[37]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[37]=FALSE;

	//=======================================================================
	//	O3 stress
	//=======================================================================
	while (longCode!=50038) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[38]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[38]=FALSE;

	//=======================================================================
	//	water shortage 
	//=======================================================================
	while (longCode!=50039) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[39]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[39]=FALSE;

	//=======================================================================
	//	potential transpiration 
	//=======================================================================
	while (longCode!=50040) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[40]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[40]=FALSE;

	//=======================================================================
	//	actual transpiration
	//=======================================================================
	while (longCode!=50041) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[41]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[41]=FALSE;

	//=======================================================================
	//	nitrogen demand 
	//=======================================================================
	while (longCode!=50042) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[42]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[42]=FALSE;

	//=======================================================================
	//	potential nitrogen uptake  
	//=======================================================================
	while (longCode!=50043) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[43]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[43]=FALSE;

	//=======================================================================
	//	actual nitrogen uptake
	//=======================================================================
	while (longCode!=50044) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[44]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[44]=FALSE;

	//=======================================================================
	//	actual nitrogen concentration (fine roots))
	//=======================================================================
	while (longCode!=50045) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[45]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[45]=FALSE;

	//=======================================================================
	//	minimal nitrogen concentration (fine roots)
	//=======================================================================
	while (longCode!=50046) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[46]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[46]=FALSE;

	//=======================================================================
	//	optimal nitrogen concentration (fine roots)
	//=======================================================================
	while (longCode!=50047) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[47]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[47]=FALSE;

	//=======================================================================
	//	actual nitrogen concentration (gross roots) 
	//=======================================================================
	while (longCode!=50048) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[48]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[48]=FALSE;

	//=======================================================================
	//	minimal nitrogen concentration (gross roots) 
	//=======================================================================
	while (longCode!=50049) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[49]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[49]=FALSE;

	//=======================================================================
	//	optimal nitrogen concentration (gross roots) 
	//=======================================================================
	while (longCode!=50050) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[50]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[50]=FALSE;

	//=======================================================================
	//	actual nitrogen concentration (stem) 
	//=======================================================================
	while (longCode!=50051) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[51]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[51]=FALSE;

	//=======================================================================
	//	minimal nitrogen concentration (stem) 
	//=======================================================================
	while (longCode!=50052) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[52]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[52]=FALSE;

	//=======================================================================
	//	optimal nitrogen concentration (stem)
	//=======================================================================
	while (longCode!=50053) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[53]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[53]=FALSE;

	//=======================================================================
	//	actual nitrogen concentration (branches)
	//=======================================================================
	while (longCode!=50054) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[54]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[54]=FALSE;

	//=======================================================================
	//	minimal nitrogen concentration (branches)
	//=======================================================================
	while (longCode!=50055) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[55]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[55]=FALSE;

	//=======================================================================
	//	optimal nitrogen concentration (branches)
	//=======================================================================
	while (longCode!=50056) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[56]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[56]=FALSE;

	//=======================================================================
	//	actual nitrogen concentration (leaves)
	//=======================================================================
	while (longCode!=50057) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[57]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[57]=FALSE;

	//=======================================================================
	//	minimal nitrogen concentration (leaves)
	//=======================================================================
	while (longCode!=50058) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[58]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[58]=FALSE;

	//=======================================================================
	//	optimal nitrogen concentration (leaves)
	//=======================================================================
	while (longCode!=50059) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[59]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[59]=FALSE;

	//=======================================================================
	//	actual nitrogen concentration (fruits/tubers)
	//=======================================================================
	while (longCode!=50060) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[60]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[60]=FALSE;

	//=======================================================================
	//	minimal nitrogen concentration (fruits/tubers)
	//=======================================================================
	while (longCode!=50061) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[61]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[61]=FALSE;

	//=======================================================================
	//	optimal nitrogen concentration (fruits/tubers)
	//=======================================================================
	while (longCode!=50062) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[62]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[62]=FALSE;

	//=======================================================================
	//	cumulative gross photosynthesis 
	//=======================================================================
	while (longCode!=50063) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[63]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[63]=FALSE;

	//=======================================================================
	//	cumulative growth respiration 
	//=======================================================================
	while (longCode!=50064) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[64]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[64]=FALSE;

	//=======================================================================
	//	cumulative maintenance respiration 
	//=======================================================================
	while (longCode!=50065) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[65]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[65]=FALSE;

	//=======================================================================
	//	cumulative littering
	//=======================================================================
	while (longCode!=50066) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[66]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[66]=FALSE;

	//=======================================================================
	//	cumulative cost for growth
	//=======================================================================
	while (longCode!=50067) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[67]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[67]=FALSE;


	//=======================================================================
	//	cumulative cost for defense 
	//=======================================================================
	while (longCode!=50068) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[68]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[68]=FALSE;

	//=======================================================================
	//	cumulative cost for maintenance 
	//=======================================================================
	while (longCode!=50069) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[69]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[69]=FALSE;


	//=======================================================================
	//	carbon balance
	//=======================================================================
	while (longCode!=50070) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[70]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[70]=FALSE;


	//=======================================================================
	//	nitrogen balance
	//=======================================================================
	while (longCode!=50071) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[71]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[71]=FALSE;


	//=======================================================================
	//	allocation to defense
	//=======================================================================
	while (longCode!=50072) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[72]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[72]=FALSE;


	//=======================================================================
	//	Extras
	//=======================================================================
	while (longCode!=50073) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[73]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[73]=FALSE;

	while (longCode!=50074) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[74]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[74]=FALSE;

	while (longCode!=50075) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[75]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[75]=FALSE;

	while (longCode!=50076) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[76]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[76]=FALSE;

    if(pPlathoScenario->abPlatho_Out[76]==TRUE)
    {
        while (longCode!=500761) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[77]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[77]=FALSE;

        while (longCode!=500762) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[78]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[78]=FALSE;

        while (longCode!=500763) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[79]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[79]=FALSE;

        while (longCode!=500764) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[80]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[80]=FALSE;

        while (longCode!=500765) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[81]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[81]=FALSE;

        while (longCode!=500766) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[82]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[82]=FALSE;

        while (longCode!=500767) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[83]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[83]=FALSE;

        while (longCode!=500768) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[84]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[84]=FALSE;

        while (longCode!=500769) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[85]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[85]=FALSE;
    }

     
 	while (longCode!=50077) longCode=ReadLong(hFile);
	ReadSymbol(hFile,cChar);
	if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		pPlathoScenario->abPlatho_Out[86]=TRUE;
	else
		pPlathoScenario->abPlatho_Out[86]=FALSE;

   if(pPlathoScenario->abPlatho_Out[86]==TRUE)
    {
        while (longCode!=500771) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[87]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[87]=FALSE;

        while (longCode!=500772) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[88]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[88]=FALSE;

        while (longCode!=500773) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[89]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[89]=FALSE;

        while (longCode!=500774) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[91]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[91]=FALSE;

        while (longCode!=500775) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[92]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[92]=FALSE;

        while (longCode!=500776) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[93]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[93]=FALSE;




 /*       while (longCode!=500769) longCode=ReadLong(hFile);
	    ReadSymbol(hFile,cChar);
	    if((!lstrcmp(cChar,(LPSTR)"(x)"))||(!lstrcmp(cChar,(LPSTR)"(X)")))
		    pPlathoScenario->abPlatho_Out[85]=TRUE;
	    else
		    pPlathoScenario->abPlatho_Out[85]=FALSE;
            */

    }

	_lclose(hFile);
	
	return 1;
}

int PLATHO_Modules()
{
	OFSTRUCT	ofStruct;
	HFILE		hFile;

	long 		longCode;
   
	//===================================================================================
	//Open modules list
	//===================================================================================
	hFile=OpenFile("Platho4Input\\Platho.mod\0",(LPOFSTRUCT)&ofStruct, OF_EXIST);
   // i=_lclose(hFile);

	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
	//Display elements
	if (hFile==HFILE_ERROR)
        {
		Message(1,"Platho-model configuration file not found! File Open Error");
		return 0;
        }                         
       
		Message(1,"Reading model configuration");
    
	//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
    
	//=======================================================================
	//	Get the pointer, open the file
	//=======================================================================

	hFile=OpenFile("Platho4Input\\Platho.mod\0",(LPOFSTRUCT)&ofStruct, OF_READ);
	longCode=0;

	//=======================================================================
	//	Photosynthesis 
	//=======================================================================
	while (longCode!=60010) longCode=ReadLong(hFile);
		pPlathoModules->iFlagPhotosynthesis = ReadInt(hFile);

	while (longCode!=60011) longCode=ReadLong(hFile);
		pPlathoModules->iFlagCi = ReadInt(hFile);

	while (longCode!=60012) longCode=ReadLong(hFile);
		pPlathoModules->iFlagLightInterception = ReadInt(hFile);

	while (longCode!=60013) longCode=ReadLong(hFile);
		pPlathoModules->iFlagStomCond = ReadInt(hFile);

	//=======================================================================
	//	Physiology 
	//=======================================================================
    while (longCode!=60014) longCode=ReadLong(hFile);
		pPlathoModules->iFlagH2OEffectPhot=ReadInt(hFile);

    while (longCode!=60015) longCode=ReadLong(hFile);
		pPlathoModules->iFlagNEffectPhot=ReadInt(hFile);

    while (longCode!=60016) longCode=ReadLong(hFile);
		pPlathoModules->iFlagO3=ReadInt(hFile);

    while (longCode!=60017) longCode=ReadLong(hFile);
		pPlathoModules->iFlagLeafPathEffPhot=ReadInt(hFile);

    while (longCode!=60018) longCode=ReadLong(hFile);
		pPlathoModules->iFlagCH2OEffectPhot=ReadInt(hFile);

    while (longCode!=60020) longCode=ReadLong(hFile);
		pPlathoModules->iFlagNEffectGrw=ReadInt(hFile);

    while (longCode!=60024) longCode=ReadLong(hFile);
		pPlathoModules->iFlagPriority=ReadInt(hFile);

    while (longCode!=60025) longCode=ReadLong(hFile);
		pPlathoModules->iFlagRootPathEffUpt=ReadInt(hFile);

	//=======================================================================
	//	Mycorrhiza 
	//=======================================================================
    while (longCode!=60030) longCode=ReadLong(hFile);
		pPlathoModules->iFlagMycorrhiza=ReadInt(hFile);

	//=======================================================================
	//	Morphology 
	//=======================================================================
	while (longCode!=60040) longCode=ReadLong(hFile);
		pPlathoModules->iFlagNitrogenDistribution=ReadInt(hFile);

	while (longCode!=60041) longCode=ReadLong(hFile);
		pPlathoModules->iFlagSpecLeafArea=ReadInt(hFile);

	while (longCode!=60042) longCode=ReadLong(hFile);
		pPlathoModules->iFlagElasticity=ReadInt(hFile);

	while (longCode!=60043) longCode=ReadLong(hFile);
		pPlathoModules->iFlagSectors=ReadInt(hFile);

	//=======================================================================
	//	Crop factors for PM-Equation 
	//=======================================================================
	while (longCode!=60044) longCode=ReadLong(hFile);
		pPlathoModules->iFlagCropFactors=ReadInt(hFile);

	return 1;
}

 int    ReadCropFactors(PWATER pWa, PPLANT pPlant)
 {
	OFSTRUCT	ofStruct;
    HFILE		hFile;
    int			i,iValue,iAgeLine;
	long 		longCode;
	char  		cChar[20];

	//===================================================================================
	//Open CropFactors.txt
	//===================================================================================
	hFile=OpenFile("Platho4Input\\CropFactors.txt\0",(LPOFSTRUCT)&ofStruct, OF_EXIST);
	
    //Display elements
	if (hFile==HFILE_ERROR)
    {
		Message(1,"crop factors file not found! File Open Error");
		return 0;
    }                         
       
    Message(1,"Reading crop factors");

    hFile=OpenFile("Platho4Input\\CropFactors.txt\0",(LPOFSTRUCT)&ofStruct, OF_READ);
	longCode=0;


     	while (longCode!=40003) longCode=ReadLong(hFile);

        iValue = ReadInt(hFile);
        		
        ReadSymbol(hFile,cChar);
        pWa->pEvap->fCropBasalIni = ReadFloat(hFile);
        pWa->pEvap->fCropBasalMid = ReadFloat(hFile);
        pWa->pEvap->fCropBasalEnd = ReadFloat(hFile);
        pWa->pEvap->fCropTimeIni = ReadFloat(hFile);
        pWa->pEvap->fCropTimeDev = ReadFloat(hFile);
        pWa->pEvap->fCropTimeMid = ReadFloat(hFile);
        pWa->pEvap->fCropTimeEnd = ReadFloat(hFile);

		for(i=1;i<iValue;i++)
		{
            iAgeLine = ReadInt(hFile);

            if((int)pPlant->pDevelop->fAge != iAgeLine)
            {
                ReadFloat(hFile);
                ReadFloat(hFile);
                ReadFloat(hFile);
                ReadFloat(hFile);
                ReadFloat(hFile);
                ReadFloat(hFile);
                ReadFloat(hFile);
            }
            else
            {
                pWa->pEvap->fCropBasalIni = ReadFloat(hFile);
                pWa->pEvap->fCropBasalMid = ReadFloat(hFile);
                pWa->pEvap->fCropBasalEnd = ReadFloat(hFile);
                pWa->pEvap->fCropTimeIni = ReadFloat(hFile);
                pWa->pEvap->fCropTimeDev = ReadFloat(hFile);
                pWa->pEvap->fCropTimeMid = ReadFloat(hFile);
                pWa->pEvap->fCropTimeEnd = ReadFloat(hFile);
            }
        }
 
     return 1;
 }