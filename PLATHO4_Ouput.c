/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                                          %
% module      :   PLANT_Out.c                                                              %
% description :   Additional Output routines                                               %
%                                                                                          %
%                                                                                          %
% Date        :   04.08.99                                                                 %
%                                                                                          %
% Authors     :   Christian Haberbosch													   %
%				  Sebastian Gayler		                                                   %
%                                                                                          %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include <time.h>

#include "xinclexp.h"
#include "xh2o_def.h"
#include "xn_def.h"

#include "xlanguag.h"     

#include "Platho4.h"     

#define LONG_THIN_LINE "----------------------------------------------------------------------------------------------------------------------"
#define LONG_FAT_LINE  "======================================================================================================================"
 
/* from xfileout.c */  
extern int  fout_floatNorm(HANDLE,float,int,int);
extern int  fout_line(HANDLE, int);
extern int  fout_space(HANDLE, int);
extern int  fout_long(HANDLE, long far *);
//extern int  fout_double(HANDLE,double far *,int);
extern int  fout_int(HANDLE,int far *,int);
extern int  fout_string(HANDLE,LPSTR);
extern int FloatToCharacter(LPSTR,float,int);

extern int DateToString(long lDate,LPSTR lpDate);

extern HANDLE  OpenF(char far * lptname);
extern int CloseF(HANDLE);


/* from util_fct.c */
extern int WINAPI Message(long, LPSTR);
extern int SimStart(PTIME);		// from time1.c
extern int introduce(LPSTR lpName);	// from util_fct.c
extern int NewDay(PTIME);
extern int EndDay(PTIME);      
//extern int WINAPI PrintStart(EXP_POINTER, int);

extern	int Weight_kg_ha(PBIOMASS pBiom, float fSowDensity);
extern	int Weight_kg_plant(PBIOMASS pBiom, float fSowDensity);
//extern	int ReadOutputList(PPLATHOSCENARIO pPlathoScenario);

/* DLL-Funktion */            
int WINAPI _loadds dllSGOutput_PLATHO(EXP_POINTER);

int WINAPI PrintPLATHOResult(EXP_POINTER);

#ifndef MESSAGE_LENGTH
  #define MESSAGE_LENGTH  70
#endif  



/********************************************************************************
 * dll-Funktion:   dllSGOutput_PLATHO
 * Beschreibung:   
 *                 ch	21.7.97
 ********************************************************************************/
int WINAPI _loadds dllSGOutput_PLATHO(EXP_POINTER)
{ 

    if (SimStart(pTi)) 
    {
    introduce((LPSTR)"dllSGOutput_PLATHO");
	//ReadOutputList(pPlathoScenario);
 	//PrintStart(exp_p,1);
    }
        
	IF_PLANTGROWTH
    {
	    if(pPlathoScenario->abPlatho_Out[0]==TRUE)
	    {
	//	    if ((NewDay(pTi))&&(!SimStart(pTi)))
		    if (EndDay(pTi))
			    PrintPLATHOResult(exp_p);
	    }
	    else
        {
            //if(pTi->pSimTime->fTimeAct > (float)1095)
                PrintPLATHOResult(exp_p);
        }
    }
         
    return 1;
}   // Ende dllSGOutput_PLATHO


                                   

/*******************************************************************************************
 *
 *  Name     : PrintPLATHOResult()
 *  Funktion : Result File 
 *
 ******************************************************************************************
 */
int WINAPI PrintPLATHOResult(EXP_POINTER)
{ 
	DECLARE_COMMON_VAR
	DECLARE_POINTER  

    PPLANT		pPlant;
	PWLAYER		pSLW=pWa->pWLayer->pNext; 
	PCLAYER 	pSLN=pCh->pCLayer->pNext;
	PMSOWINFO	pMaSowInfo;
    PLEAFLAYER  pLLayer;


  int iPlant, i, iSector;
  int iVarCounter=1;
  int L;

  static HANDLE hFile;

  int iRet =0;           
  char *acFileName;                           
  char *acDummy2, *acDummy3; 


  N_ZERO_LAYER
  
  		acDummy2 = malloc(sizeof(char)*200);  // Speicher allokieren.
  		acDummy3 = malloc(sizeof(char)*(MESSAGE_LENGTH+10+2));  // Speicher allokieren.

       if (!hFile)
       {  
  		acFileName      = malloc(sizeof(char)*(MESSAGE_LENGTH+10+2));  // Speicher allokieren.

        if (strlen(pLo->pFieldplot->acName)<2)
        {
        lstrcpyn(acFileName,"expertn",7);
        }
        else
        {
        lstrcpyn(acFileName,pLo->pFieldplot->acName,7);
        }
        
        itoa( (int)(pTi->pSimTime->lStopDate % 100),acDummy2,10);
        lstrcat(acFileName,acDummy2);

    
     for (;iRet<13;iRet++) acDummy2[iRet] = '\0'; iRet=0;
     strcpy(acDummy2,"result//");
     strcat(acDummy2,acFileName);
     strcat(acDummy2,".rfu");   
     hFile = OpenF(acDummy2);


	if (hFile)  
	 {        
	 
     strcat(acDummy2," opened PLATHO result file.");
	 Message(0, acDummy2);


       fout_line(hFile, 1);     

	   fout_string(hFile, "EXPERT-N : PLATHO Result File.");

       fout_line(hFile, 1);     
	   fout_string(hFile, "---> userdefined <---");
       fout_line(hFile, 1);     
//	   fout_string(hFile, Date);
       fout_line(hFile, 1);     
//	   fout_string(hFile, Time);
       fout_line(hFile, 1);     
       fout_line(hFile, 1);     

       fout_string(hFile, "(1)  Date              [DDMMYY] ");
       fout_line(hFile, 1);     


	   for(pPlant=pPl,iPlant=1; pPlant!=NULL; pPlant=pPlant->pNext,iPlant++)
	   {

	   if(pPlathoScenario->abPlatho_Out[1]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Development stage ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [-]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

       TREES
	   {
		if(pPlathoScenario->abPlatho_Out[2]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Fine root biomass ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [g]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
		   
		if(pPlathoScenario->abPlatho_Out[3]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Gross root biomass ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [g]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	   } //end TREES
	   else
	   {
   		if(pPlathoScenario->abPlatho_Out[2]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Root biomass ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [g]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	   } // end HERBS

	   if(pPlathoScenario->abPlatho_Out[4]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Stem biomass");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }


	TREES
	{
		if(pPlathoScenario->abPlatho_Out[5]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Branch biomass ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [g]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	
		if(pPlathoScenario->abPlatho_Out[6]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Stem+branch biomass");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [g]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	
		if(pPlathoScenario->abPlatho_Out[7]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Wood biomass");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [g]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	   } // end TREES
	   
	   
	   if(pPlathoScenario->abPlatho_Out[8]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Leaf biomass");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   POTATO
	   {
	   if(pPlathoScenario->abPlatho_Out[9]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Fruit biomass ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	   }
	   else
	   {
	   if(pPlathoScenario->abPlatho_Out[9]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Fruit biomass ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	   }
		
	   if(pPlathoScenario->abPlatho_Out[10]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Total biomass ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  		fout_string(hFile, acDummy2);
		fout_line(hFile, 1);
	   }
	 
	   if(pPlathoScenario->abPlatho_Out[11]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Assimilate pool ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[12]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Storage ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[13]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Defensive Compounds ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[14]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Defens. Comp. Conc. (Leaves) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g/g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[15]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Defens. Comp. Conc. (roots) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g/g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[16]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Gross Photosynthesis ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(CO2)/d]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[17]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Maintenance respiration ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(CO2)/d]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[18]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Growth respiration ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(CO2)/d]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[19]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Costs for growth ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(CH2O)/d]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[20]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Costs for defense ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(CH2O)/d]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[21]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Costs for maintenance ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(CH2O)/d]  ");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[22]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  plant height ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[23]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  stem diameter ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[24]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Crown area ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[98]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Crown volume ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [cm^3]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[25]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Leaf area ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[26]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Leaf area density (layer) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		//strcat(acDummy2,"    [m^2/m^2]  [##.###]");   
		strcat(acDummy2,"    [m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[27]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Leaf area (sector) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
//		strcat(acDummy2,"    [m^2/m^2]  [##.###]");   
		strcat(acDummy2,"    [m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[28]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Spec. leaf weight distrib. ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g/m^2]]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[29]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Leaf nitrogen distrib. ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g/g]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[30]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  LAI (Plant) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m^2/m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[90]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  LAI (Canopy) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m^2/m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[31]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Root depth ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [cm]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[97]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Root zone area ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[96]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Root volume ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [cm^3]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[32]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Root length density (layer) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m/m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[33]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Root length density (sector) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [m/m^2]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[34]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Competition coefficient ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [-]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[35]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  carbon availability ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [-]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	
	   if(pPlathoScenario->abPlatho_Out[36]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  nitrogen availability ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [-]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
	

	   if(pPlathoScenario->abPlatho_Out[37]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Leaf internal O3 ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [µg/kg]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[38]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  O3-Stress intens. ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [0-1]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[39]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Water-Shortage ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [0-1]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[40]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Potential plant water uptake ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [cm^3]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[41]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Actual plant water uptake ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [cm^3]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[42]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Nitrogen demand ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(N)/d]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[43]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Pot. nitrogen uptake ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(N)]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   if(pPlathoScenario->abPlatho_Out[44]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Act. nitrogen uptake ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [g(N)]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }
		
	   TREES
	   {
		if(pPlathoScenario->abPlatho_Out[45]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Act. nitrogen conc. (fine roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[46]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Min. nitrogen conc. (fine roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[47]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Opt. nitrogen conc. (fine roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[48]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Act. nitrogen conc. (gross roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[49]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Min. nitrogen conc. (gross roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[50]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Opt. nitrogen conc. (gross roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	   } // end TREES
	   else
	   {
		if(pPlathoScenario->abPlatho_Out[45]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Act. nitrogen conc. (roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
		if(pPlathoScenario->abPlatho_Out[46]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Min. nitrogen conc. (roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
		if(pPlathoScenario->abPlatho_Out[47]==TRUE)
		{
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Opt. nitrogen conc. (roots) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	   } // end HERBS
	
	   if(pPlathoScenario->abPlatho_Out[51]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Act. nitrogen conc. (stem) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [%/100]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[52]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Min. nitrogen conc. (stem) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [%/100]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   if(pPlathoScenario->abPlatho_Out[53]==TRUE)
	   {
		iVarCounter++;
		strcpy(acDummy2,"(");
		itoa(iVarCounter,acDummy3,10);
		strcat(acDummy2,acDummy3);
		strcat(acDummy2,")  Opt. nitrogen conc. (stem) ");   
 		strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
		strcat(acDummy2,"    [%/100]");   
  	    fout_string(hFile, acDummy2);
        fout_line(hFile, 1);
	   }

	   TREES
	   {
		if(pPlathoScenario->abPlatho_Out[54]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Act. nitrogen conc. (branches) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[55]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Min. nitrogen conc. (branches) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		if(pPlathoScenario->abPlatho_Out[56]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Opt. nitrogen conc. (branches) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}
	   }
	
	   if(pPlathoScenario->abPlatho_Out[57]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Act. nitrogen conc. (leaves) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

	   if(pPlathoScenario->abPlatho_Out[58]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Min. nitrogen conc. (leaves) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

	   if(pPlathoScenario->abPlatho_Out[59]==TRUE)
		{
			iVarCounter++;
			strcpy(acDummy2,"(");
			itoa(iVarCounter,acDummy3,10);
			strcat(acDummy2,acDummy3);
			strcat(acDummy2,")  Opt. nitrogen conc. (leaves) ");   
 			strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
			strcat(acDummy2,"    [%/100]");   
  			fout_string(hFile, acDummy2);
			fout_line(hFile, 1);
		}

		POTATO
		{
			if(pPlathoScenario->abPlatho_Out[60]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Act. nitrogen conc. (tubers) ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [%/100]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}
			if(pPlathoScenario->abPlatho_Out[61]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Min. nitrogen conc. (tubers) ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [%/100]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}
			if(pPlathoScenario->abPlatho_Out[62]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Opt. nitrogen conc. (tubers) ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [%/100]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}
		} // end POTATO
		else
		{
			if(pPlathoScenario->abPlatho_Out[60]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Act. nitrogen conc. (fruits) ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [%/100]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[61]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Min. nitrogen conc. (fruits) ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [%/100]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[62]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Opt. nitrogen conc. (fruits) ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [%/100]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}
		} // end else

			if(pPlathoScenario->abPlatho_Out[63]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Photosynthesis ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(CO2)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[64]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Growth Respiration ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(CO2)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[65]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Maintenance Respiration ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(CO2)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[66]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Litter Loss ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(C)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[67]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Cost for Growth ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(CH2O)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[68]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Cost for Defense ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(CH2O)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[69]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  cum. Cost for Maintenance ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(CH2O)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[70]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  C-Balance ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(C)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[71]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  N-Balance ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(N)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[72]==TRUE)
			{
				iVarCounter++;
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Allocation to S ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2," [g(N)]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[73]==TRUE)
			{
				iVarCounter++;
                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") LLayer LAICum ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [m^2/m^2]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

			}

			if(pPlathoScenario->abPlatho_Out[74]==TRUE)
			{
				iVarCounter++;
                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") LLayer GrossPSDay ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [g(CO2)/d]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

			}

			if(pPlathoScenario->abPlatho_Out[75]==TRUE)
			{
 				iVarCounter++;
                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") LLayer NettoPSDay ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [g(CO2)/d]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
			}

			if(pPlathoScenario->abPlatho_Out[76]==TRUE)
			{

			    if(pPlathoScenario->abPlatho_Out[77]==TRUE)
			    {
				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  PAR ");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [mol/m2/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[78]==TRUE)
			    {
				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Intercepted PAR ");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [mol/m2/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[79]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  LLayer Intercepted PAR ");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [mol/m2/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[80]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Intercepted PAR Cumulative");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [mol/m2/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[81]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Fine root growth rate");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[82]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Fine root loss rate");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[83]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Leaf growth rate");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[84]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Leaf loss rate");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }
 
			    if(pPlathoScenario->abPlatho_Out[85]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Costs for induced defense-related compounds");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g(CH2O)]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }
                }

			if(pPlathoScenario->abPlatho_Out[86]==TRUE)
			{

			    if(pPlathoScenario->abPlatho_Out[87]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Leaf Rubisco concentration");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g/g]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[88]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  O3 Uptake Rate");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g/d]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[89]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Cumulative O3 Uptake");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [mmol/m^2]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[91]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  cum. costs for Rubisco repair ");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g(CH2O)]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[92]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  cumu. costs for ozone detoxification ");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [g(CH2O)]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

			    if(pPlathoScenario->abPlatho_Out[93]==TRUE)
			    {
 				    iVarCounter++;
                    strcpy(acDummy2,"(");
				    itoa(iVarCounter,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,")  Stomatal conductance per leaf layer");   
 				    strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				    strcat(acDummy2," [mmol (H2O)/m^2/s]");   
  				    fout_string(hFile, acDummy2);
				    fout_line(hFile, 1);
                }

                
                
                /*
				strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Intercepted PAR Cum ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [J]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  Cum O3 Uptake ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [µg]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,")  O3 Uptake Rate ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [µg/d]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") % Defense d.m. ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [µg]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") Cum. O3 Upt. ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [µg/d]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") O3 conc. crit. ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [µg/d]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

         

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") LLayer LAICum ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [m^2/m^2]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") LLayer GrossPSDay ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [g(CO2)/d]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);

                strcpy(acDummy2,"(");
				itoa(iVarCounter,acDummy3,10);
				strcat(acDummy2,acDummy3);
				strcat(acDummy2,") LLayer NettoPSDay ");   
 				strcat(acDummy2,pPlant->pGenotype->acVarietyName);   
				strcat(acDummy2,"    [g(CO2)/d]  [##.###]");   
  				fout_string(hFile, acDummy2);
				fout_line(hFile, 1);
    */
            }
			
	} //end iPlant


       fout_line(hFile, 1);
       fout_line(hFile, 1);


	   	strcpy(acDummy2,"-1");

/*	   for (iPlant=1, pPlant=pPl;pPlant!=NULL;pPlant=pPlant->pNext,iPlant++)
	   {  
		   for(i=1;i<=10;i++)
		   {
			   strcat(acDummy2,"\t-");
			   itoa((int)10.0*(iPlant-(int)1)+i+(int)1,acDummy3,10);
			   strcat(acDummy2,acDummy3);   
			   //strcat(acDummy2,"\t\t\t\t\t");
			   fout_string(hFile, acDummy2);
		   }
	   }
*/
	   fout_line(hFile, 1);


       fout_string(hFile,"Date\t  ");

	   for(pPlant=pPl; pPlant!=NULL; pPlant=pPlant->pNext)
	   {	
			if(pPlathoScenario->abPlatho_Out[1]==TRUE)
			{
			strcpy(acDummy2,"DevStage (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			TREES
			{
			if(pPlathoScenario->abPlatho_Out[2]==TRUE)
			{
				strcpy(acDummy2,"Fine roots (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
				
			if(pPlathoScenario->abPlatho_Out[3]==TRUE)
			{
				strcpy(acDummy2,"Gross roots (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			} // end TREES
			else
			{
			if(pPlathoScenario->abPlatho_Out[2]==TRUE)
			{
				strcpy(acDummy2,"Roots (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			}

			if(pPlathoScenario->abPlatho_Out[4]==TRUE)
			{
			strcpy(acDummy2,"Stem (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			TREES
			{
			if(pPlathoScenario->abPlatho_Out[5]==TRUE)
			{
				strcpy(acDummy2,"Branches (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[6]==TRUE)
			{
				strcpy(acDummy2,"Stem+Branches (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[7]==TRUE)
			{
				strcpy(acDummy2,"Wood (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			} // end TREES

			if(pPlathoScenario->abPlatho_Out[8]==TRUE)
			{
			strcpy(acDummy2,"Leaves (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			
			POTATO
			{
			if(pPlathoScenario->abPlatho_Out[9]==TRUE)
			{
				strcpy(acDummy2,"Tubers (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			}
			else
			{
			if(pPlathoScenario->abPlatho_Out[9]==TRUE)
			{
				strcpy(acDummy2,"Fruits (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			}

			if(pPlathoScenario->abPlatho_Out[10]==TRUE)
			{
			strcpy(acDummy2,"Total Biomass (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[11]==TRUE)
			{
			strcpy(acDummy2,"Assimilates (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[12]==TRUE)
			{
			strcpy(acDummy2,"Storage (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}


			if(pPlathoScenario->abPlatho_Out[13]==TRUE)
			{
			strcpy(acDummy2,"Defens. comp. (cont.) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}


			if(pPlathoScenario->abPlatho_Out[14]==TRUE)
			{
			strcpy(acDummy2,"Def. comp. conc. (leaves) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
			
			if(pPlathoScenario->abPlatho_Out[15]==TRUE)
			{
			strcpy(acDummy2,"Def. comp. conc. (roots) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[16]==TRUE)
			{
			strcpy(acDummy2,"Gross Photosynthesis (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[17]==TRUE)
			{
			strcpy(acDummy2,"Maintenance respiration (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
			
			if(pPlathoScenario->abPlatho_Out[18]==TRUE)
			{
			strcpy(acDummy2,"Growth respiration (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
			
			if(pPlathoScenario->abPlatho_Out[19]==TRUE)
			{
			strcpy(acDummy2,"Cost for Growth (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
			
			if(pPlathoScenario->abPlatho_Out[20]==TRUE)
			{
			strcpy(acDummy2,"Cost for Defense (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[21]==TRUE)
			{
			strcpy(acDummy2,"Cost for Maintenance (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[22]==TRUE)
			{
			strcpy(acDummy2,"PlantHeight (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[23]==TRUE)
			{
			strcpy(acDummy2,"StemDiameter (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[24]==TRUE)
			{
			strcpy(acDummy2,"CrownArea (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[98]==TRUE)
			{
			strcpy(acDummy2,"CrownVolume (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[25]==TRUE)
			{
			strcpy(acDummy2,"LeafArea (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[26]==TRUE)
			{
			    for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"Leaf area[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[27]==TRUE)
			{
			    for(i=1;i<=LEAFLAYERS; i++)
			    {
				    for(iSector=1; iSector<=4; iSector++)
                    {
                        strcpy(acDummy2,"LeafAreaSec[");
				        itoa(i,acDummy3,10);
				        strcat(acDummy2,acDummy3);
				        strcat(acDummy2,",");
				        itoa(iSector,acDummy3,10);
				        strcat(acDummy2,acDummy3);
				        strcat(acDummy2,"] (");
				        strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				        strcat(acDummy2,")\t");   
				        fout_string(hFile,acDummy2);
                    }
			    }
			}

			if(pPlathoScenario->abPlatho_Out[28]==TRUE)
			{
			    for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"SpecLeafWeight[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[29]==TRUE)
			{
			    for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"Leaf nitrogen conc.[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[30]==TRUE)
			{
			strcpy(acDummy2,"LAI (plant) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[90]==TRUE)
			{
			strcpy(acDummy2,"LAI (Canopy) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[31]==TRUE)
			{
			strcpy(acDummy2,"RootDepth (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[97]==TRUE)
			{
			strcpy(acDummy2,"RootZoneArea (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[96]==TRUE)
			{
			strcpy(acDummy2,"RootVolume (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[32]==TRUE)
			{
			    for (L=1;L<=pSo->iLayers-2;L++)
			    {
				    strcpy(acDummy2,"RootDens (");
                    itoa(L,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[33]==TRUE)
			{
			    for (L=1;L<=pSo->iLayers-2;L++)
			    {
				    for(iSector=1; iSector<=4; iSector++)
                    {
				        strcpy(acDummy2,"RootLengthSec[");
                        itoa(L,acDummy3,10);
				        strcat(acDummy2,acDummy3);
				        strcat(acDummy2,",");
				        itoa(iSector,acDummy3,10);
				        strcat(acDummy2,acDummy3);
				        strcat(acDummy2,"] (");
				        strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				        strcat(acDummy2,")\t");   
				        fout_string(hFile,acDummy2);
                    }
			    }
			}

			if(pPlathoScenario->abPlatho_Out[34]==TRUE)
			{
			strcpy(acDummy2,"Comp. coeff. (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[35]==TRUE)
			{
			strcpy(acDummy2,"C-availability (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[36]==TRUE)
			{
			strcpy(acDummy2,"N-availability (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[37]==TRUE)
			{
			strcpy(acDummy2,"Leaf internal O3 conc. (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[38]==TRUE)
			{
			strcpy(acDummy2,"O3 Stress Intens. (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[39]==TRUE)
			{
			strcpy(acDummy2,"Water shortage (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[40]==TRUE)
			{
			strcpy(acDummy2,"Pot. transpiration (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[41]==TRUE)
			{
			strcpy(acDummy2,"Act. transpiration (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[42]==TRUE)
			{
			strcpy(acDummy2,"Nitrogen demand (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[43]==TRUE)
			{
			strcpy(acDummy2,"Pot. nitrogen uptake (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[44]==TRUE)
			{
			strcpy(acDummy2,"Act. nitrogen uptake (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			TREES
			{
			if(pPlathoScenario->abPlatho_Out[45]==TRUE)
			{
			strcpy(acDummy2,"Act. N-conc. (fine roots) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[46]==TRUE)
			{
			strcpy(acDummy2,"Min. N-conc. (fine roots) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[47]==TRUE)
			{
			strcpy(acDummy2,"Opt. N-conc. (fine roots)  (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[48]==TRUE)
			{
			strcpy(acDummy2,"Act. N-conc. (gross roots)  (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[49]==TRUE)
			{
			strcpy(acDummy2,"Min. N-conc. (gross roots) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[50]==TRUE)
			{
			strcpy(acDummy2,"Opt. N-conc. (gross roots)  (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
			} // end TREES
			else
			{	
			if(pPlathoScenario->abPlatho_Out[45]==TRUE)
			{
			strcpy(acDummy2,"Act. N-conc. (roots)  (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[46]==TRUE)
			{
			strcpy(acDummy2,"Min. N-conc. (roots)  (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[47]==TRUE)
			{
			strcpy(acDummy2,"Opt. N-conc. (roots)  (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			} // end HERBS
			
			if(pPlathoScenario->abPlatho_Out[51]==TRUE)
			{
			strcpy(acDummy2,"Act. N-conc. (stem) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[52]==TRUE)
			{
			strcpy(acDummy2,"Min. N-conc. (stem) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[53]==TRUE)
			{
			strcpy(acDummy2,"Opt. N-conc. (stem) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}

			TREES
			{
			if(pPlathoScenario->abPlatho_Out[54]==TRUE)
			{
			strcpy(acDummy2,"Act. N-conc. (branches) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[55]==TRUE)
			{
			strcpy(acDummy2,"Min. N-conc. (branches) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
		
			if(pPlathoScenario->abPlatho_Out[56]==TRUE)
			{
			strcpy(acDummy2,"Opt. N-conc. (branches) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
			} // end TREES
		
			if(pPlathoScenario->abPlatho_Out[57]==TRUE)
			{
			strcpy(acDummy2,"Act. N-conc. (leaves) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
				
			if(pPlathoScenario->abPlatho_Out[58]==TRUE)
			{
			strcpy(acDummy2,"Min. N-conc. (leaves) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
				
			if(pPlathoScenario->abPlatho_Out[59]==TRUE)
			{
			strcpy(acDummy2,"Opt. N-conc. (leaves) (");
			strcat(acDummy2, pPlant->pGenotype->acVarietyName);
			strcat(acDummy2,")\t");   
			fout_string(hFile,acDummy2);
			}
				
		POTATO
			{
			if(pPlathoScenario->abPlatho_Out[60]==TRUE)
				{
					strcpy(acDummy2,"Act. N-conc. (tubers) (");
					strcat(acDummy2, pPlant->pGenotype->acVarietyName);
					strcat(acDummy2,")\t");   
					fout_string(hFile,acDummy2);
				}

				if(pPlathoScenario->abPlatho_Out[61]==TRUE)
				{
					strcpy(acDummy2,"Min. N-conc. (tubers) (");
					strcat(acDummy2, pPlant->pGenotype->acVarietyName);
					strcat(acDummy2,")\t");   
					fout_string(hFile,acDummy2);
				}

				if(pPlathoScenario->abPlatho_Out[62]==TRUE)
				{
					strcpy(acDummy2,"Opt. N-conc. (tubers) (");
					strcat(acDummy2, pPlant->pGenotype->acVarietyName);
					strcat(acDummy2,")\t");   
					fout_string(hFile,acDummy2);
				}

			} // end POTATO
			else
			{
				if(pPlathoScenario->abPlatho_Out[60]==TRUE)
				{
					strcpy(acDummy2,"Act. N-conc. (fruits) (");
					strcat(acDummy2, pPlant->pGenotype->acVarietyName);
					strcat(acDummy2,")\t");   
					fout_string(hFile,acDummy2);
				}

				if(pPlathoScenario->abPlatho_Out[61]==TRUE)
				{
					strcpy(acDummy2,"Min. N-conc. (fruits) (");
					strcat(acDummy2, pPlant->pGenotype->acVarietyName);
					strcat(acDummy2,")\t");   
					fout_string(hFile,acDummy2);
				}

				if(pPlathoScenario->abPlatho_Out[62]==TRUE)
				{
					strcpy(acDummy2,"Opt. N-conc. (fruits) (");
					strcat(acDummy2, pPlant->pGenotype->acVarietyName);
					strcat(acDummy2,")\t");   
					fout_string(hFile,acDummy2);
				}
			} // end else

			if(pPlathoScenario->abPlatho_Out[63]==TRUE)
			{
				strcpy(acDummy2,"cum. Photosynthesis (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[64]==TRUE)
			{
				strcpy(acDummy2,"cum. Growth Resp. (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[65]==TRUE)
			{
				strcpy(acDummy2,"cum. Maintenance Resp. (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[66]==TRUE)
			{
				strcpy(acDummy2,"cum. Litter Loss (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			if(pPlathoScenario->abPlatho_Out[67]==TRUE)
			{
				strcpy(acDummy2,"cum. Cost for Growth (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			if(pPlathoScenario->abPlatho_Out[68]==TRUE)
			{
				strcpy(acDummy2,"cum. Cost for Defense (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			if(pPlathoScenario->abPlatho_Out[69]==TRUE)
			{
				strcpy(acDummy2,"cum. Cost for Maintenance (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}
			if(pPlathoScenario->abPlatho_Out[70]==TRUE)
			{
				strcpy(acDummy2,"C-Balance (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[71]==TRUE)
			{
				strcpy(acDummy2,"N-Balance (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[72]==TRUE)
			{
				strcpy(acDummy2,"S/Gpot (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
			}

			if(pPlathoScenario->abPlatho_Out[73]==TRUE)
			{
               for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"LAICum[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[74]==TRUE)
			{
			    for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"LLayer GrossPSDay[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[75]==TRUE)
			{
                for(i = 0;i<LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"LLayer NettoPSDay[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			}

			if(pPlathoScenario->abPlatho_Out[76]==TRUE)
			{
                if(pPlathoScenario->abPlatho_Out[77]==TRUE)
	            {
                    strcpy(acDummy2,"PAR (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[78]==TRUE)
	            {
                    strcpy(acDummy2,"PARIntDay (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[79]==TRUE)
	            {
                    for(i = 0;i<LEAFLAYERS;i++)
			        {
				        strcpy(acDummy2,"LLayerPARIntDay[");
				        itoa(i,acDummy3,10);
				        strcat(acDummy2,acDummy3);
				        strcat(acDummy2,"] (");
				        strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				        strcat(acDummy2,")\t");   
				        fout_string(hFile,acDummy2);
			        }
                }

                if(pPlathoScenario->abPlatho_Out[80]==TRUE)
	            {
                    strcpy(acDummy2,"PARIntCum (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[81]==TRUE)
	            {
                    strcpy(acDummy2,"FineRootGrowthR (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[82]==TRUE)
	            {
                    strcpy(acDummy2,"FineRootLossR (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[83]==TRUE)
	            {
                    strcpy(acDummy2,"LeafGrowthR (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[84]==TRUE)
	            {
                    strcpy(acDummy2,"LeafLossR (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }
 
                if(pPlathoScenario->abPlatho_Out[85]==TRUE)
	            {
                    strcpy(acDummy2,"Costs for induced defense-related Compounds (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
                }
            }

            	
            if(pPlathoScenario->abPlatho_Out[86]==TRUE)
			{

                if(pPlathoScenario->abPlatho_Out[87]==TRUE)
	            {
                strcpy(acDummy2,"RubiscoConc (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[88]==TRUE)
	            {
 				strcpy(acDummy2,"O3-Uptake_Day (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[89]==TRUE)
	            {
				strcpy(acDummy2,"O3-Uptake_Cum (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[91]==TRUE)
	            {
				strcpy(acDummy2,"CostRepairCum (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                }
                
                if(pPlathoScenario->abPlatho_Out[92]==TRUE)
	            {
				strcpy(acDummy2,"CostDetocCum (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                }

                if(pPlathoScenario->abPlatho_Out[93]==TRUE)
	            {
                    for(i = 0;i<LEAFLAYERS;i++)
			        {
				        strcpy(acDummy2,"LLayer g_H2O[");
				        itoa(i,acDummy3,10);
				        strcat(acDummy2,acDummy3);
				        strcat(acDummy2,"] (");
				        strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				        strcat(acDummy2,")\t");   
				        fout_string(hFile,acDummy2);
			        }
                }

/*               
                strcpy(acDummy2,"BlwgCover (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);


 				strcpy(acDummy2,"ET_pot Day (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"ET_pot dt (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"E_pot Day (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                
				strcpy(acDummy2,"E_pot dt (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"E_act Day (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
                
				strcpy(acDummy2,"E_act dt (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"T_pot Day (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"T_pot dt (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"T_act_Day (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);


				strcpy(acDummy2,"T_act (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);
 */
/*				strcpy(acDummy2,"x (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);

				strcpy(acDummy2,"y (");
				strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				strcat(acDummy2,")\t");   
				fout_string(hFile,acDummy2);



               for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"LAICum[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
			    for(i=1;i<=LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"LLayer GrossPSDay[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }

                for(i = 0;i<LEAFLAYERS;i++)
			    {
				    strcpy(acDummy2,"LLayer NettoPSDay[");
				    itoa(i,acDummy3,10);
				    strcat(acDummy2,acDummy3);
				    strcat(acDummy2,"] (");
				    strcat(acDummy2, pPlant->pGenotype->acVarietyName);
				    strcat(acDummy2,")\t");   
				    fout_string(hFile,acDummy2);
			    }
           */           

            }

		} // end iPlant

	       

       fout_line(hFile, 1);
       fout_string(hFile, LONG_THIN_LINE);
        
       fout_line(hFile, 1);
      } // file opened
      else
      {
      Message(1,FILEOPEN_ERROR_TXT);
      }
	}
	
  if (hFile)
	{
     
	if(pPlathoScenario->abPlatho_Out[0]==TRUE)
	{ // Ein Ausgabewert pro Tag
		itoa((int)pTi->pSimTime->lTimeDate,acDummy2,10);
		if(strlen(acDummy2)==5)fout_string(hFile, "0");
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	else
	{ //ein Ausgabewert pro Zeitschritt
		gcvt((double)pTi->pSimTime->fTimeAct,12,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");

  /*  	gcvt((double)pTi->pSimTime->fTimeDay,12,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");

        gcvt((double)pTi->pTimeStep->fAct,12,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
        */
}


   for(pPlant=pPl, pMaSowInfo=pMa->pSowInfo, iPlant=1;pPlant!=NULL;
		pPlant=pPlant->pNext, pMaSowInfo=pMaSowInfo->pNext,iPlant++)
   {
	   	PLAYERROOT 			pLR	     = pPlant->pRoot->pLayerRoot;
		PPLATHOPARTITIONING pPPart   = pPlathoPlant[iPlant-1]->pPlathoPartitioning;
		PPLATHOMORPHOLOGY   pPMorph  = pPlathoPlant[iPlant-1]->pPlathoMorphology;
		PPLATHOBIOMASS      pPBiom   = pPlathoPlant[iPlant-1]->pPlathoBiomass;
		PPLATHONITROGEN     pPPltN   = pPlathoPlant[iPlant-1]->pPlathoNitrogen;
	    PPLATHOROOTLAYER	pPRL	 = pPMorph->pPlathoRootLayer;
		PPLANTSTRESS        pPStress = pPlathoPlant[iPlant-1]->pPlantStress;
        PFALGEPS            pFlgPS   = pPlathoPlant[iPlant-1]->pPlathoGenotype->pFalgePS;

	if(pPlathoScenario->abPlatho_Out[99]==TRUE)
	{
		Weight_kg_plant(pPlant->pBiomass,pMaSowInfo->fPlantDens);
	}

	if(pPlathoScenario->abPlatho_Out[1]==TRUE)
	{
	gcvt((double)pPlant->pDevelop->fDevStage,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[2]==TRUE)
	{
	gcvt((double)(pPlant->pBiomass->fRootWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	   
	TREES
	{
	if(pPlathoScenario->abPlatho_Out[3]==TRUE)
	{
		gcvt((double)(pPlant->pBiomass->fGrossRootWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	}

	if(pPlathoScenario->abPlatho_Out[4]==TRUE)
	{
	gcvt((double)(pPlant->pBiomass->fStemWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");	   
	}


	TREES
	{
	if(pPlathoScenario->abPlatho_Out[5]==TRUE)
	{
		gcvt((double)((pPlant->pBiomass->fBranchWeight)*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");	
	}

	if(pPlathoScenario->abPlatho_Out[6]==TRUE)
	{
		gcvt((double)((pPlant->pBiomass->fStemWeight+pPlant->pBiomass->fBranchWeight)*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[7]==TRUE)
	{
		gcvt((double)(pPlant->pBiomass->fWoodWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	}
	
	if(pPlathoScenario->abPlatho_Out[8]==TRUE)
	{
	gcvt((double)((pPlant->pBiomass->fLeafWeight)*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");	
	}


	POTATO
	{
	if(pPlathoScenario->abPlatho_Out[9]==TRUE)
	{
	  gcvt((double)(pPlant->pBiomass->fTuberWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");	   
	}
	}
	else
	{
	if(pPlathoScenario->abPlatho_Out[9]==TRUE)
	{
	  gcvt((double)(pPlant->pBiomass->fFruitWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	}


	if(pPlathoScenario->abPlatho_Out[10]==TRUE)
	{
		gcvt((double)(pPlant->pBiomass->fTotalBiomass*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000:1)),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[11]==TRUE)
	{
	gcvt((double)(pPlant->pPltCarbon->fCAssimilatePool*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000
															:pMaSowInfo->fPlantDens*1e4)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[12]==TRUE)
	{
	gcvt((double)(pPlant->pBiomass->fStemReserveWeight*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000
															:pMaSowInfo->fPlantDens*1e4)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}


	if(pPlathoScenario->abPlatho_Out[13]==TRUE)
	{
		gcvt((double)(pPStress->vDefCompCont*(pPlathoScenario->abPlatho_Out[99]==TRUE?1000
															:pMaSowInfo->fPlantDens*1e4)),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[14]==TRUE)
	{
		gcvt((double)(pPStress->vLeafDefConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[15]==TRUE)
	{
		gcvt((double)(pPStress->vFineRootDefConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[16]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPPart->vDailyGrossPhotosynthesis*1000.0,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
			gcvt((double)pPlant->pPltCarbon->fGrossPhotosynR*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
	}

	if(pPlathoScenario->abPlatho_Out[17]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPPart->vDailyMaintResp*1000.0,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
			gcvt((double)pPlant->pPltCarbon->fMaintRespR*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
	}

	if(pPlathoScenario->abPlatho_Out[18]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPPart->vDailyGrowthResp*1000.0,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
			gcvt((double)pPlant->pPltCarbon->fGrowthRespR*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
	}


	if(pPlathoScenario->abPlatho_Out[19]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPPart->vDailyGrowth*1000.0,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
			gcvt((double)pPPart->dCostsForBiomassGrowth*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
	}

	if(pPlathoScenario->abPlatho_Out[20]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPPart->vDailyDefense*1000.0,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
			gcvt((double)pPPart->dCostsForDefense*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
	}

	if(pPlathoScenario->abPlatho_Out[21]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPPart->vDailyMaintenance*1000.0,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
			gcvt((double)pPPart->dCostsForMaintenance*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
	}


	if(pPlathoScenario->abPlatho_Out[22]==TRUE)
	{
	gcvt((double)pPlant->pCanopy->fPlantHeight,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[23]==TRUE)
	{
	gcvt((double)pPMorph->vStemDiameter,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[24]==TRUE)
	{
	gcvt((double)pPMorph->vCrownArea,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[98]==TRUE)
	{
	gcvt((double)pPMorph->dCrownVolume,6,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[25]==TRUE)
	{
	gcvt((double)pPlant->pCanopy->fPlantLA,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[26]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        //gcvt((double)(pLLayer->vLeafAreaLay/pPMorph->vCrownArea),5,acDummy2);
	        gcvt((double)(pLLayer->vLeafAreaLay),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
	}

	if(pPlathoScenario->abPlatho_Out[27]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        for(iSector=0;iSector<4;iSector++)
            {
 //               gcvt((double)(pLLayer->apLeafSector[iSector]->vLeafAreaSec/pPMorph->vCrownArea),5,acDummy2);
                gcvt((double)(pLLayer->apLeafSector[iSector]->vLeafAreaSec),5,acDummy2);
                fout_string(hFile, acDummy2);
                fout_string(hFile,"\t");
            }
            pLLayer = pLLayer->pNext;
	    }
	}

	if(pPlathoScenario->abPlatho_Out[28]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        gcvt((double)(pLLayer->vSpecLeafWeight*1000),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
	}

	if(pPlathoScenario->abPlatho_Out[29]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        gcvt((double)(pLLayer->vLeafNConc),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
	}

	if(pPlathoScenario->abPlatho_Out[30]==TRUE)
	{
       gcvt((double)pPMorph->vPlantLAI,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[90]==TRUE)
	{
       gcvt((double)pPl->pCanopy->fLAI,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[31]==TRUE)
	{
	gcvt((double)pPlant->pRoot->fDepth,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[97]==TRUE)
	{
	gcvt((double)pPMorph->vRootZoneArea,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[96]==TRUE)
	{
	gcvt((double)pPMorph->dRootVolume,6,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[32]==TRUE)
	{
	for (L=1;L<=pSo->iLayers-2;L++)
	{
		gcvt((double)pLR->fLengthDens/pPMorph->vCrownArea,4,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");

		pLR =pLR ->pNext;
	}
	}

    if(pPlathoScenario->abPlatho_Out[33]==TRUE)
	{
        pPRL	 = pPMorph->pPlathoRootLayer;
	    for(L=1;L<=pSo->iLayers-2;L++)
	    {
	        for(iSector=0;iSector<4;iSector++)
            {
                gcvt((double)(pPRL->apRootSector[iSector]->vFRLengthDens/pPMorph->vCrownArea),5,acDummy2);
                fout_string(hFile, acDummy2);
                fout_string(hFile,"\t");
            }
            pPRL =pPRL->pNext;
	    }
	}



	if(pPlathoScenario->abPlatho_Out[34]==TRUE)
	{
	gcvt((double)(pPStress->vCompCoeff),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[35]==TRUE)
	{
	   gcvt((double)(pPStress->vCShortage),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	   
	if(pPlathoScenario->abPlatho_Out[36]==TRUE)
	{
	gcvt((double)(pPStress->vNShortage),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	   
	if(pPlathoScenario->abPlatho_Out[37]==TRUE)
	{
	gcvt((double)pPStress->vO3ConcLeaf,5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[38]==TRUE)
	{
	gcvt((double)(pPStress->vO3StressIntensity),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[39]==TRUE)
	{
	//gcvt((double)(pPlant->pPltWater->fStressFacPhoto),8,acDummy2);
	gcvt((double)(pPStress->dWaterShortage),8,acDummy2);
	//gcvt((double)(pPStress->dWaterShortageDay/(pTi->pSimTime->fTimeDay+DeltaT)),8,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

/*	if(pPlathoScenario->abPlatho_Out[40]==TRUE)
	{
	gcvt((double)(pPlant->pPltWater->fPotTranspdt * pPMorph->vRootZoneArea * (float)1000),5,acDummy2); //cm^3
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[41]==TRUE)
	{
	gcvt((double)(pPlant->pPltWater->fActTranspdt * pPMorph->vRootZoneArea * (float)1000),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
*/
    if(pPlathoScenario->abPlatho_Out[40]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
            gcvt(pPPart->vPotTranspDay,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
        else
	    {
            gcvt(pPPart->vPotTranspR,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
    }


    if(pPlathoScenario->abPlatho_Out[41]==TRUE)
	{
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
            gcvt(pPPart->vActTranspDay,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
        else
	    {
            gcvt(pPPart->vActTranspR,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
    }


	if(pPlathoScenario->abPlatho_Out[42]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fTotalDemand),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	   
	if(pPlathoScenario->abPlatho_Out[43]==TRUE)
	{
	    if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
	        gcvt(pPPltN->dPotNUptDay*1000.0,5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
	    else
		{
	        gcvt((double)(pPlant->pPltNitrogen->fPotNUpt*1000.0),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
	}
	   
	if(pPlathoScenario->abPlatho_Out[44]==TRUE)
	{
	    if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
            gcvt((double)(pPPltN->dActNUptDay*1000.0),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
        else
		{
            gcvt((double)(pPlant->pPltNitrogen->fActNUpt*1000.0),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
	}
	   	

	if(pPlathoScenario->abPlatho_Out[45]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fRootActConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	   
	if(pPlathoScenario->abPlatho_Out[46]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fRootMinConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	   
	if(pPlathoScenario->abPlatho_Out[47]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fRootOptConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	   
	TREES
	{
		if(pPlathoScenario->abPlatho_Out[48]==TRUE)
		{
		gcvt((double)(pPlant->pPltNitrogen->fGrossRootActConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
		}
		if(pPlathoScenario->abPlatho_Out[49]==TRUE)
		{
		gcvt((double)(pPlant->pPltNitrogen->fGrossRootMinConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
		}
		if(pPlathoScenario->abPlatho_Out[50]==TRUE)
		{
		gcvt((double)(pPlant->pPltNitrogen->fGrossRootOptConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
		}
	}
	   
	if(pPlathoScenario->abPlatho_Out[51]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fStemActConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[52]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fStemMinConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[53]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fStemOptConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}


	TREES
	{
	if(pPlathoScenario->abPlatho_Out[54]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fBranchActConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[55]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fBranchMinConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[56]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fBranchOptConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	}
	   
	   
	if(pPlathoScenario->abPlatho_Out[57]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fLeafActConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	
	if(pPlathoScenario->abPlatho_Out[58]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fLeafMinConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	
	if(pPlathoScenario->abPlatho_Out[59]==TRUE)
	{
	gcvt((double)(pPlant->pPltNitrogen->fLeafOptConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	

	POTATO
	{
	if(pPlathoScenario->abPlatho_Out[60]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fTuberActConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[61]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fTuberMinConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[62]==TRUE)
	{
		gcvt((double)(pPlant->pPltNitrogen->fTuberOptConc),5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
	}
	}
	else
	{
	if(pPlathoScenario->abPlatho_Out[60]==TRUE)
	{
	  gcvt((double)(pPlant->pPltNitrogen->fFruitActConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[61]==TRUE)
	{
	  gcvt((double)(pPlant->pPltNitrogen->fFruitMinConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[62]==TRUE)
	{
	  gcvt((double)(pPlant->pPltNitrogen->fFruitOptConc),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	}//end else


	if(pPlathoScenario->abPlatho_Out[63]==TRUE)
	{
	  gcvt((double)(pPPart->vCumPhotosynthesis)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[64]==TRUE)
	{
	  gcvt((double)(pPPart->vCumGrowthRespiration)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[65]==TRUE)
	{
	  gcvt((double)(pPPart->vCumMaintenanceRespiration)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[66]==TRUE)
	{
	  gcvt((double)(pPPart->vCumLitterLossC)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[67]==TRUE)
	{
	  gcvt((double)(pPPart->dCostsForBiomassGrowthCum)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[68]==TRUE)
	{
	  gcvt((double)(pPPart->dCostsForDefenseCum)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[69]==TRUE)
	{
	  gcvt((double)(pPPart->dCostsForMaintenanceCum)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[70]==TRUE)
	{
	  gcvt((double)(pPPart->vCBalance)*1e3,7,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}
	if(pPlathoScenario->abPlatho_Out[71]==TRUE)
	{
	  gcvt((double)(pPPart->vNBalance)*1e3,5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[72]==TRUE)
	{
	  gcvt((double)(pPPart->dCostsForDefense/(pPPart->vTotalDemandForGrowthAndDefense+EPSILON)),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
	}

	if(pPlathoScenario->abPlatho_Out[73]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        gcvt((double)(pLLayer->vLAICum),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }

	}

	if(pPlathoScenario->abPlatho_Out[74]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        gcvt((double)(pLLayer->vGrossPsDay),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
	}

	if(pPlathoScenario->abPlatho_Out[75]==TRUE)
	{
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
            gcvt((double)(pLLayer->vNettoPsDay),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
	}


	if(pPlathoScenario->abPlatho_Out[76]==TRUE)
	{
        if(pPlathoScenario->abPlatho_Out[77]==TRUE)
	    {
            //  gcvt((double)(pCl->pWeather->pBack->fSolRad*0.5*1E6)*min(pPMorph->vCrownArea,PI/(4.0*pPlathoScenario->vTotalPlantDensity)),5,acDummy2);    // [J]
            //  gcvt((double)(pCl->pWeather->pBack->fSolRad*0.5),6,acDummy2);    // [MJ/m2/d]
                gcvt((double)(pCl->pWeather->fSolRad*0.5),6,acDummy2);    // [MJ/m2/d]
            // gcvt((double)(pCl->pWeather->pBack->fSolRad*0.5*1E6)*pPMorph->vCrownArea,5,acDummy2);    // [J]
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[78]==TRUE)
	    {
            gcvt(pPPart->vPARabsorbedDay,6,acDummy2);       // [J]
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[79]==TRUE)
	    {
            pLLayer = pPMorph->pLeafLayer;
	        for(i = 0;i<LEAFLAYERS;i++)
	        {
                gcvt((double)(pLLayer->vPARabsorbedDay),6,acDummy2);
                fout_string(hFile, acDummy2);
                fout_string(hFile,"\t");
                pLLayer = pLLayer->pNext;
	        }
        }

        if(pPlathoScenario->abPlatho_Out[80]==TRUE)
	    {
            gcvt(pPPart->vPARabsorbedCum,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }



        if(pPlathoScenario->abPlatho_Out[81]==TRUE)
	    {
            gcvt(pPBiom->vFineRootGrowDay*1000.0,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[82]==TRUE)
	    {
            gcvt(pPBiom->vFineRootLossDay*1000.0,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[83]==TRUE)
	    {
            gcvt(pPBiom->vLeafGrowDay*1000.0,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[84]==TRUE)
	    {
            gcvt(pPBiom->vLeafLossDay*1000.0,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[85]==TRUE)
	    {
		    if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		    {
			    gcvt((double)pPPart->dDailyDefInduced*1000.0,5,acDummy2);
			    fout_string(hFile, acDummy2);
			    fout_string(hFile,"\t");
		    }
		    else
		    {
			    gcvt(pPPart->dCostsForDefInduced*1000.0/pTi->pTimeStep->fAct,5,acDummy2);
			    fout_string(hFile, acDummy2);
			    fout_string(hFile,"\t");
		    }
        }
    }

			

       ////////////////////////////////////////////////////////////////
        //
        // Achtung:  pPlathoScenario->abPlatho_Out[90] vergeben bei
        //           Bestandes-LAI (pPlathoScenario->abPlatho_Out[30])
        //
        ////////////////////////////////////////////////////////////////
                        
 	if(pPlathoScenario->abPlatho_Out[86]==TRUE)
	{
       if(pPlathoScenario->abPlatho_Out[87]==TRUE)
	    {
            gcvt(pPStress->dRubiscoConc,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[88]==TRUE)
	    {
            gcvt(pPStress->vO3UptakeDay,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

         if(pPlathoScenario->abPlatho_Out[89]==TRUE)
	    {
            gcvt(pPStress->vO3UptakeCum,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[91]==TRUE)
	    {
            gcvt(pPStress->dCostsRepairCum,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        if(pPlathoScenario->abPlatho_Out[92]==TRUE)
	    {
            gcvt(pPStress->dCostForDetoxCum,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

        
   /*
		if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
            gcvt(pPPart->vAveStomCondDay,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }
        else
        {

            gcvt(pPPart->vAveStomCond,7,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

 */       

        if(pPlathoScenario->abPlatho_Out[93]==TRUE)
	    {
		    if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		    {
                pLLayer = pPMorph->pLeafLayer;
	            for(i = 0;i<LEAFLAYERS;i++)
                {
                    gcvt(pLLayer->vStomCondDay,7,acDummy2);
                    fout_string(hFile, acDummy2);
                    fout_string(hFile,"\t");
                    pLLayer = pLLayer->pNext;
                }
            }
            else
            {
                pLLayer = pPMorph->pLeafLayer;
	            for(i = 0;i<LEAFLAYERS;i++)
                {
                    gcvt(pLLayer->vStomCond,7,acDummy2);
                    fout_string(hFile, acDummy2);
                    fout_string(hFile,"\t"); 
                    pLLayer = pLLayer->pNext;
                }
            }
        }


        

       //pot. Evapotranspiration

 /*       gcvt(pWa->fPotETDay,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pWa->fPotETdt,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pWa->pEvap->fPotDay,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pWa->pEvap->fPotR*DeltaT,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pWa->pEvap->fActDay,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pWa->pEvap->fActR*DeltaT,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pPlant->pPltWater->fPotTranspDay,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pPlant->pPltWater->fPotTranspdt,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pPl->pRoot->fUptakeR,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

          gcvt(pPlant->pPltWater->fActTranspdt,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

 */             
         
 /*       gcvt(pWa->fGrdWatLevel,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pWa->fGrdWatLvlPot,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");


        gcvt(pPPart->dCostsForDetoxAndRepairCum*1000,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pPStress->dRubiscoConc,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");
                                    

        gcvt(pPBiom->vKLeaf,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt(pPBiom->vKFRoot,7,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

                                      */
        
 /*       gcvt((double)pPMorph->vPlantLAI,4,acDummy2);
 //  gcvt((double)pPlant->pCanopy->fLAI,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");
 
    gcvt((double)pPMorph->vHeightOfCrownOnset,4,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");

 	  gcvt((double)(pPPart->vDarkRespR),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");

 	  gcvt((double)(pPPart->vActTranspR),5,acDummy2);
       fout_string(hFile, acDummy2);
       fout_string(hFile,"\t");

       gcvt((double)(pPStress->PhiO3),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

       gcvt((double)(pPPart->dCostsForDefense/(pPPart->dCostsForBiomassGrowth+pPPart->dCostsForDefense+EPSILON)),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

       

  	    
        gcvt((double)(pPPart->vO3Cond),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");
   
	    gcvt((double)(pPStress->vO3UptakeCum),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

	    gcvt((double)(pPStress->vO3ConcLeafMes),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

	    gcvt((double)(pPStress->vPotO3Detox),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

        gcvt((double)(pPStress->vO3Detox),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");   */

/*	    gcvt((double)(pPlant->pBiomass->fStemReserveGrowRate),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

	    gcvt((double)(pPlant->pBiomass->fReserveTransRate),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

	    gcvt((double)(pPPart->vReservesGrowDay),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

	    gcvt((double)(pPPart->vReservesTransDay),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");
 
	    gcvt((double)(pPPart->vCumReservesGrow),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");

	    gcvt((double)(pPPart->vCumReservesTrans),5,acDummy2);
        fout_string(hFile, acDummy2);
        fout_string(hFile,"\t");   


    	if(pPlathoScenario->abPlatho_Out[0]==TRUE)
		{
			gcvt((double)pPStress->vO3UptakeDay,5,acDummy2);
			fout_string(hFile, acDummy2);
			fout_string(hFile,"\t");
		}
		else
		{
 	        gcvt((double)(pPStress->vO3Uptake/DeltaT),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
        }

 */

 /*       gcvt((!lstrcmp((LPSTR)pPlathoScenario->acResolution,(LPSTR)"high"))?
			pPlathoClimate->vCO2:pPlathoScenario->vCO2Day,5,acDummy2);
		fout_string(hFile, acDummy2);
		fout_string(hFile,"\t");
   	
    
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        gcvt((double)(pLLayer->vLAICum),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }

  

        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
	        gcvt((double)(pLLayer->vGrossPsDay),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
   
        pLLayer = pPMorph->pLeafLayer;
	    for(i = 0;i<LEAFLAYERS;i++)
	    {
            gcvt((double)(pLLayer->vNettoPsDay),5,acDummy2);
            fout_string(hFile, acDummy2);
            fout_string(hFile,"\t");
            pLLayer = pLLayer->pNext;
	    }
   */
    }
                

	if(pPlathoScenario->abPlatho_Out[99]==TRUE)
	{
		Weight_kg_ha(pPlant->pBiomass, pMaSowInfo->fPlantDens);
	}

   }

	

	fout_line(hFile, 1);

} /* data output */

     
		//hFile = CloseF(acDummy2);

//   	free(acDummy2);  // Speicher frei geben
//   	free(acDummy3);  // Speicher frei geben
 //		if(!hFile)
//			free(acFileName);


return 1;
}

                                     
