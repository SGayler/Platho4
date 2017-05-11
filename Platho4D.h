//////////////////////////////////////////////////////////
//	#defines und Variablen für PLATHO					//
//////////////////////////////////////////////////////////

//Funktionen
extern float AFGENERATOR(float fInput, RESPONSE* pfResp);
//extern double AVGENERATOR(double vInput, VRESPONSE* pvResp);

//Zusätzliche Definitionen und Variablen für das Modell PLATHO:

#define EXP_POINTER2 PCHEMISTRY pCh,PCLIMATE pCl,PGRAPHIC pGr,PHEAT pHe,PLOCATION pLo,PMANAGEMENT pMa,PPLANT pPlant,PSPROFILE pSo,PTIME pTi,PWATER pWa
#define exp_p2 pCh,pCl,pGr,pHe,pLo,pMa,pPlant,pSo,pTi,pWa

#define NewDayAndPlantGrowing2 	(NewDay(pTi)&&(pPlant->pDevelop->bPlantGrowth==TRUE)&&(pPlant->pDevelop->bMaturity==FALSE))
#define PlantIsGrowing2 		((pPlant->pDevelop->bPlantGrowth==TRUE)&&(pPlant->pDevelop->bMaturity==FALSE))
#define NewYear                 ((NewDay(pTi)&&(pTi->pSimTime->iJulianDay==1))&&(!SimulationStarted))

#define MAXPLANTS 20
#define MAXSOILLAYERS 100
#define LEAFLAYERS 20
#define MAXTIMESTEP 10000

#define	IDGASKONST 8.31441	// ideal gas constant needed for Baldocchi
#define	CONCO2 210.0		// O2 concentration in air
#define AIRPRESSURE 1013.0  //hPa

#define EPS 1e-15
#define NFAC_L (float)0.9
#define NFAC_S (float)0.6


#define BARLEY		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA"))
#define WHEAT		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))
#define POTATO		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"PT"))
#define SUNFLOWER	if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SF"))
#define MAIZE		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"MZ"))
#define BEECH		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BE"))
#define SPRUCE		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SP"))
#define LOLIUM		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"IR"))
#define ALFALFA		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"AL"))
#define APPLE		if(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"AP"))

#define HERBS		if((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BA"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"WH"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SF"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"IR"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"PT"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"MZ"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"AL")))
#define TREES		if((!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"BE"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"SP"))||(!lstrcmp((LPSTR)pPlant->pGenotype->acCropCode,(LPSTR)"AP")))

#define IF_PLANTGROWTH if((pPlathoPlant[0]!=NULL)&&(pPlathoPlant[0]->pPlathoDevelop->bHarvest==FALSE))

//Matrixpotentiale	
#define Psi1  -10.0		//ab hier Staunässe
#define Psi2  -630.0		//Beginn des Wasserstress
#define Psi3  -158489.3	//untere Grenze für Wasseraufnahme (Permanenter Welkepunkt)


//Dichte des Pflanzenbestandes
//float	fPlantDensity;
//Mittlerer Abstand zwischen den Individuen:
//float	MeanDistance;

typedef struct vResponse  * PVRESPONSE;
typedef struct vResponse
	{
	double	vInput;
	double	vOutput;
	}
	VRESPONSE;


//Struktur PPLATHOPLANT: Definition spezieller PLATHO - Variablen

	
typedef struct stPools * PPOOLS;
typedef struct stPools
{
	double  vCarbohydrates;
	double  vProteins;
	double  vLignins;
	double  vLipids;
	double  vOrganicAcids;
	double  vAminoAcids;
	double  vNucleotides;
	double  vMinerals;
	double  vFracC;
}
POOLS;


typedef struct stFalgePS *PFALGEPS;
typedef struct stFalgePS  
	{
        double ko298;
	    double kc298;
        double DeltaHaKo;
        double DeltaHaKc;
        double DeltaHaVcMax;
        double DeltaSVcMax;
        double DeltaHdVcMax;
        double darkresp298;
        double tau298;
        double DeltaHatau;
        double DeltaHaPml;
        double DeltaHdPml;
        double DeltaSPml;
        double DeltaHaresp;

	    double NetPhotoMol;       //=pnet;    die Nettophotosynthese in mol m^{-2} s^{-1}
	    double GrossPhotoMol;     //=pgross;  die Bruttophotosynthese in mol m^{-2} s^{-1}
	    double StomCond;          //=gpres;   stomatal conductance
	    double StomCondMax;       
	    double StomCondMin;       
	    double Transpiration;     //=tra;   Transpiration
	    double TranspR;           //=tra;   Transpiration rate

    	double K; //frost hardiness reduction parameter. K is between 0 and 1

        double laminarCond;	 //; =ga;   Leitfaehigkeit der laminaren Schicht
        double DarkResp;     //; =wd;
        double Gamma;        //; =gamma;
        double ewtp;         //; Wasserdampfpartialdruck  bei Taupunktstemperatur
        double ewair;        //; Wasserdampfpartialdruck   in der Luft
        double ewleaf;       //; Wasserdampfpartialdruck   im Blatt
        double ewr;          //; Hilfsvariablen
        double ewdr;
        double rtk;          //; Hilfsvariable ideale Gaskonstante * TempKelvin
        double dbl;          //; Dicke der laminaren Schicht
        double dwv;          //; Diffusionskoeffizient fuer Wasserdampf
        double tau;          //; =tau; enzymspezifischer Faktor
        double vpd;          //; =vpd; vapor pressure deficit
        double fac;          //; =fac; irgendein Faktor, der aus der Einstrahlung berechnet wird
        double Ko;           //; =ko;  Michaelis-Konstante fuer O2
        double Kc;           //; =kc;  Michaelis-Konstante fuer CO2
        double Pml;          //; =pml;
        double Pm;           //; =pm;
        double Vcmax;        //; =VcMax;
        double Cs;           //; CO2-Konz. an der Blattoberflaeche
        double Cint;         //; blattinterne CO2-Konz.
        double Hs;           //; relative Feuchte an der Blattoberflaeche

        double gfacbase;
        double gfac;
        double widthofleaf;
        double sigmaleaf;

	    double totaltoprojm2factor; // to account for needle surface to projected surface
       }
	FALGEPS;


typedef struct stPlantMaterial *PPLANTMATERIAL;
typedef struct stPlantMaterial  
	{
		PPOOLS	pFineRoot;
		PPOOLS	pGrossRoot;
		PPOOLS	pStem;
		PPOOLS	pBranch;
		PPOOLS	pLeaf;
		PPOOLS	pFruit;
		PPOOLS	pTuber;
	}
	PLANTMATERIAL;


typedef struct stPlathoGenotype * PPLATHOGENOTYPE;
typedef struct stPlathoGenotype
	{
		double vMaxPlantGrowR; // [1/d]
		double vLeafFlushR; // [1/d]
		double vFineRootFlushR; // [1/d]

		double vVcmaxSu;
		double vJmaxSu;
		double vVcmaxSh;
		double vJmaxSh;
		VRESPONSE avVcmaxSu[21];
		VRESPONSE avJmaxSu[21];
		VRESPONSE avVcmaxSh[21];
		VRESPONSE avJmaxSh[21];
		double vFCalphaSu;
		double vFCalphaSh;
		double vTheta;
        
        PFALGEPS pFalgePS;

		double vFineRootLossR;
		double vGrossRootLossR;
		double vStemLossR;
		double vBranchLossR;
		double vLeafLossR;
		double vFruitLossR;
		double vTuberLossR;

		double vAssRlcFineRoot;
		double vAssRlcGrossRoot;
		double vAssRlcStem;
		double vAssRlcBranch;
		double vAssRlcLeaf;
		double vAssRlcFruit;
		double vAssRlcTuber;

		double vAssUseRate; //relocation rate of CH2O from assimilate pool [1/d]
		double vNTransRate; //relocation rate of mobile nitrogen [1/d]
		
		VRESPONSE MaxGrossPsSu[21];
		VRESPONSE MaxGrossPsSh[21];

		VRESPONSE MinBrNc[21];
		VRESPONSE MinGRtNc[21];
		VRESPONSE MinTuberNc[21];
		VRESPONSE MinFruitNc[21];
		VRESPONSE OptBrNc[21];
		VRESPONSE OptGRtNc[21];
		VRESPONSE OptTuberNc[21];
		VRESPONSE OptFruitNc[21];
		VRESPONSE MaxBrNc[21];
		VRESPONSE MaxGRtNc[21];
		VRESPONSE MaxTuberNc[21];
		VRESPONSE MaxFruitNc[21];

		PPLANTMATERIAL pPltMaterial;

		double  vProteinsTurnoverR;
		double  vLipidsTurnoverR;
		double  vNucleotidesTurnoverR;
		double  vMineralsLeakageR;
		double  vMaintMetabol;
		double  vMaintMetabolTimeDelay;
		double  vO3DefCompTurnoverR;
		double  vLfPathDefCompTurnR;
		double  vRtPathDefCompTurnR;

		double  vSpecGrossRootLength;
		double  vRootExtTempMin;
		double  vRootExtTempOpt;
		double  vRootExtTempMax;

		double  vRelHeightMaxLeaf;
		double  vRelDepthMaxRoot;

        double  vLa;
        double  vLb;
        double  vFRa;
        double  vFRb;

		double  vKDiffuse;
		double  vLAImax;

        double  cdetox;
        double  crepair;
        double  fdetox;
        double  frepair;
        double  dO3damage;
        double  dRubiscoConcCrit;

	}
	PLATHOGENOTYPE;


typedef struct stPlathoBiomass * PPLATHOBIOMASS;
typedef struct stPlathoBiomass
	{
		double vLivingBiomass;
        double vMaxLeafWeight;
        double vMaxFRootWeight;
        double vWoodInc;
		double vFRootFrac;
		double vGRootFrac;
		double vLeafFrac;
		double vBranchDeathRate;
		double vGRootDeathRate;
		double vSeedWeight;
		double vKFRoot;
		double vKGRoot;
		double vKStem;
		double vKBranch;
		double vKLeaf;
		double vKFruit;
		double vKTuber;
//		double vVegAbvEnd1;
		double vVegAbvEnd2;
		double vLeafEnd3;
		double vFruitEnd4;
		double vAssRelocR;
		double vNRelocR;
		double vStarchGrowR;
        double vFracRespWood;
        double vLeafDamageO3;
        double vLeafDamagePath;
        double vRootDamage;
        double vFineRootGrowDay;
        double vLeafGrowDay;
        double vFineRootLossDay;
        double vLeafLossDay;
	}          
	PLATHOBIOMASS;


typedef struct stPlathoDevelop * PPLATHODEVELOP;
typedef struct stPlathoDevelop  
	{
		int	iStagePlatho;
        BOOL bHarvest;
	}
	PLATHODEVELOP;



typedef struct stRootSector *PROOTSECTOR; 
typedef struct stRootSector
	{
	    double	vFRSecBiomass;
	    double	vGRSecBiomass;

        double	vFRLengthDens;
	    double	vFRLengthDensR;
	    double	vFRLengthDensFac;
	    double	vGRLengthDens;
	    double	vGRLengthDensR;
	    double	vGRLengthDensFac;
	    double	vRootArea;

        double  vSecCompCoeff;

        double  vActSecWatUpt;
        double  vPotSecWatUpt;
        double  vActSecNO3Upt;
        double  vActSecNH4Upt;
        double  vPotSecNO3Upt;
        double  vPotSecNH4Upt;
	    double	vWCompCoeff;
	    double	vNCompCoeff;
        		
        double	vDegMycoShort;
		double	vDegMycoMedium;
		double	vDegMycoLong;
		double	vCostMycoShort;
		double	vCostMycoMedium;
		double	vCostMycoLong;
		double	vBenefitMycoShort;
		double	vBenefitMycoMedium;
		double	vBenefitMycoLong;

        double  vBlwgDistrFac;
	}
	ROOTSECTOR;

typedef struct stPlathoRootLayer *PPLATHOROOTLAYER;
typedef struct stPlathoRootLayer
	{
        double vDepthOfLayer;
	    double vDiameterOfLayer;
        double vThicknessOfLayer;
        double vVolumeOfLayer;

	    double	vFRLayBiomass;
	    double	vGRLayBiomass;

        double	vFRLengthDens;
	    double	vFRLengthDensR;
	    double	vFRLengthDensFac;
	    double	vGRLengthDens;
	    double	vGRLengthDensR;
	    double	vGRLengthDensFac;
	    double	vRootArea;
	    double	vWCompCoeff;
	    double	vNCompCoeff;
        		
        double	vDegMycoShort;
		double	vDegMycoMedium;
		double	vDegMycoLong;
		double	vCostMycoShort;
		double	vCostMycoMedium;
		double	vCostMycoLong;
		double	vBenefitMycoShort;
		double	vBenefitMycoMedium;
		double	vBenefitMycoLong;

        double  RootAreaNbg[4]; //Root area of neighbours

        PROOTSECTOR apRootSector[4];
        
	    PPLATHOROOTLAYER pNext;
	    PPLATHOROOTLAYER pBack;
	}
	PLATHOROOTLAYER;

typedef struct stLeafSector *PLEAFSECTOR;
typedef struct stLeafSector
	{
        double vLeafWeightSec;
        double vLeafAreaSec; //m^2(leaf area)

		double vSLWeightSec;
		double vLeafAreaCumSec;
		double vLAICumSec;
		double vLeafWeightCumSec;

        double vSecCompCoeff;
		
        double vLeafNConc;
        double vCint;           // ppm
        double vGrossPsR;       // g(CO2)/h
        double vNettoPsR;       // g(CO2)/h
        double vDarkRespR;      // g(CO2)/h
        double vStomCond;       // cm/s
        double vActTranspR;     // mm/h   
        double vGrossPsDay;     // g(CO2)
        double vNettoPsDay;     // g(CO2)
        double vDarkRespDay;    // g(CO2)
        double vStomCondDay;    // cm/s
        double vActTranspDay;   // mm

        double vAbvgDistrFac;  // %/100
	}
	LEAFSECTOR;

typedef struct stLeafLayer *PLEAFLAYER;
typedef struct stLeafLayer
	{
        double vLeafWeightLay;
		double vLeafAreaLay; //m^2(leaf area)

        double vSpecLeafWeight;
		double vLeafAreaCum;
		double vLAICum;
		double vLeafWeightCum;

		double vLeafNConc;
        double vCint;           // ppm
        double vGrossPsR;       // g(CO2)/h
        double vNettoPsR;       // g(CO2)/h
        double vDarkRespR;      // g(CO2)/h
        double vStomCond;       // cm/s
        double vActTranspR;     // mm/h   
        double vGrossPsDay;     // g(CO2)
        double vNettoPsDay;     // g(CO2)
        double vDarkRespDay;    // g(CO2)
        double vStomCondDay;    // cm/s
        double vActTranspDay;   // mm

        double vPARabsorbed;    // µmol/s (Farquhar) bzw. J/s (Goudriaan)
        double vPARabsorbedDay;    // J
        double vFracCPL;        // fraction of central leaves in layer 
        double vLeafAreaNbg[4]; //Leaf area of neighbours

        PLEAFSECTOR apLeafSector[4];

        PLEAFLAYER pNext;
        PLEAFLAYER pBack;
	}
	LEAFLAYER;


typedef struct stPlathoMorphology * PPLATHOMORPHOLOGY;
typedef struct stPlathoMorphology  
	{
        double  vPlantLAI;

		double	vStemDiameter;
		double	vStemDiameterOld;
		double	vCrownArea;
		double	vCrownDiameter;
		double	vRootZoneArea;
		double	vRootZoneDiameter;
        double  vHeightOfCrownOnset;
        double  dCrownVolume;
        double  dRootVolume;

        double  vBlwgCover;
        double  vAbvgCover;

        double	AbvgCoeffComp[4];
		double	BlwgCoeffComp[4];
        double  SectorCompCoeff_abvg[4];
        double  SectorCompCoeff_blwg[4];

		VRESPONSE	vBranchFrac[21];
		VRESPONSE	vUGrWoodFrac[21];
		double	vStemDensity;
		double	dStemFormFactor;
		double	vStemHeightR;
		double	vSpecStemLength;
		double	vMinSpecStemLength;
		double	vMaxSpecStemLength;
		double	vHeightToDiameter;
		double	vHtoDmax;
		VRESPONSE vHDmax[21];
		double	vHtoDmin;
		double	dSpecLfWeightShade;
		double	vMeanSpecLfWeight;
		double	vCrownStemRatio;
		double	vCrownRootRatio;
		double	vCRegulation;
		double	vNRegulation;
		double	vFineRootToLeafRatio;
		double	vLeafToStemRatio;
        double  vLeafToBranchRatio;
		double	vMaxLeafToWoodRatio;
		double	vFruitToLeafRatio;
        double  vPlasticity; //Flexibility of biomassdistribution to Sectors
		PLEAFLAYER   pLeafLayer;
		PPLATHOROOTLAYER  pPlathoRootLayer;
	}
	PLATHOMORPHOLOGY;


typedef struct stPlathoNitrogen * PPLATHONITROGEN;
typedef struct stPlathoNitrogen 
	{
		double  vNTotalOptConc;
		double	vNTotalActConc;
		double  vNDemandForGrowth;

		double	vFruitMinConc;

		double	vGrossRootTransNw;
		double	vBranchTransNw;
		double	vFruitTransNw;
		double	vTuberTransNw;
		double	vNSeedReserves;

		double	vFruitNtransRate;
		double	vRNO3U[MAXSOILLAYERS];
		double	vRNH4U[MAXSOILLAYERS];

		double	dPotNUpt;
		double	dPotNUptDay;
		double	dActNUptDay;
		double	dTotalTransNw;
		double	dNPool;
		double	dTotalDemand;
	}
	PLATHONITROGEN;


typedef struct stPlathoPartitioning * PPLATHOPARTITIONING;
typedef struct stPlathoPartitioning 
	{
		double  vTotalDemandForGrowthAndDefense;

		double  dCostsForMaintenance;
		double  dCostsForBiomassGrowth;
		double  dCostsForMetabolism;
		double  dCostsForDefense;
		double  dCostsForDefConstitutive;
		double  dCostsForDefInduced;
        double  dCostsForMycorrhiza;

		double  dCostsForMaintenanceCum;
		double  dCostsForBiomassGrowthCum;
		double  dCostsForDefenseCum;
		double  dCostsForDetoxAndRepairCum;
        double  dCostsForMycorrhizaCum;

		double  vCumPhotosynthesis;
		double  vCumMaintenanceRespiration;
		double  vCumGrowthRespiration;
		double  vCumTotalRespiration;
        double  vCumAssReloc;
		double  vReservesGrowDay;
		double  vReservesTransDay;
		double  vCumReservesGrow;
		double  vCumReservesTrans;

		double	dCumNUptake;

		double  dCumLitterLossC;
		double  dCumLitterLossN;

		double  vDailyGrossPhotosynthesis;
		double  vDailyNettoPhotosynthesis;
		double  vDailyDarkResp;
		double  vDailyMaintResp;
		double  vDailyGrowthResp;

        double  vPARabsorbedCan; // [J]
        double  vPARabsorbedDay; // [J]
        double  vPARabsorbedCum; // [J]

        double  vH2OCond;
        double  vCO2Cond;
        double  vO3Cond;
 
        double  vAveStomCond;
        double  vAveStomCondDay;
        double  vActTranspR;
        double  vActTranspDay;
        double  vPotTranspDay;
        double  vPotTranspR;

        double  vNettoPhotosynR;
        double  vDarkRespR;

		double  vDailyGrowth;
		double  vDailyMaintenance;
		double  vDailyDefense;
		double  dDailyDefInduced;

		double  vDemandForDefenseBase;
		double  vDemandForDefense;
        double  dDemandForDefConstitutive;
        double  dDemandForDefInduced;

		double  vDetLeaf;
		double  vPotFru;
		double  vDetJohannis;

		double dAssR;
		double dResR;

		double dFineRootAssCont;
		double dCoarseRootAssCont;
		double dStemAssCont;
		double dBranchAssCont;
		double dLeafAssCont;
		double dFruitAssCont;
		double dTuberAssCont;

		double dFineRootAssConc;
		double dCoarseRootAssConc;
		double dStemAssConc;
		double dBranchAssConc;
		double dLeafAssConc;
		double dFruitAssConc;
		double dTuberAssConc;

		double dFineRootResCont;
		double dCoarseRootResCont;
		double dStemResCont;
		double dBranchResCont;
		double dLeafResCont;
		double dFruitResCont;
		double dTuberResCont;

		double dFineRootResConc;
		double dCoarseRootResConc;
		double dStemResConc;
		double dBranchResConc;
		double dLeafResConc;
		double dFruitResConc;
		double dTuberResConc;

		double dCH2OFRootRelocR;
		double dCH2OGRootRelocR;
		double dCH2OStemRelocR;
		double dCH2OBranchRelocR;
		double dCH2OLeafRelocR;
		double dCH2OFruitRelocR;
		double dCH2OTuberRelocR;

		double dNFRootRelocR;
		double dNGRootRelocR;
		double dNStemRelocR;
		double dNBranchRelocR;
		double dNLeafRelocR;
		double dNFruitRelocR;
		double dNTuberRelocR;

		double  dCBalance;
		double  dNBalance;
	}
	PLATHOPARTITIONING;


typedef struct stPlantStress * PPLANTSTRESS;
typedef struct stPlantStress
	{
		//competition for light and nutrients
		double  dWaterShortage;
		double  dWaterShortageDay;
        double  vCompCoeff;
		//float	fWCC;
		double  vNCC;
		double  vNShortage;
		double  vCShortage;

        // Defensive Compounds
		double  vConstDefenseBase;	//[%/100]
		double  vConstDefense;	    //[%/100]
		double  dInducedDefense;	//[%/100]
		double  vDefCompTurnoverR;  //[1/d]
		double  vDefCompTurnoverR0;  //[1/d]

		double  vDefCompCont;		//[kg]
		double  vDefCompR;		    //[kg]
		double  vFineRootDefConc;	//[kg/kg(dry matter)]
		double  vFineRootDefCont;	//[kg]
		double  vGrossRootDefConc;	//[kg/kg(dry matter)]
		double  vGrossRootDefCont;	//[kg]
		double  vStemDefConc;		//[kg/kg(dry matter)]
		double  vStemDefCont;		//[kg]
		double  vBranchDefConc;		//[kg/kg(dry matter)]
		double  vBranchDefCont;		//[kg]
		double  vLeafDefConc;		//[kg/kg(dry matter)]
		double  vLeafDefCont;		//[kg]
		double  vFruitDefConc;		//[kg/kg(dry matter)]
		double  vFruitDefCont;		//[kg]
		double  vTuberDefConc;		//[kg/kg(dry matter)]
		double  vTuberDefCont;		//[kg]

		//O3-Stress
		double  vO3StressIntensity;
		double  vO3Sensitivity;
		double  vO3ConcLeaf;
		double  vO3ConcLeafMes;
		double  vO3LeafConcCum;
		double  vO3LeafConcCrit;
		double  vO3LeafConcCumCrit;
		double  vO3Detox;		    //[µg(O3)/kg(TS)]
		double  vO3DetoxCum;		//[µg(O3)/kg(TS)]
		double  vPotO3Detox;		//[µg(O3)/kg(TS)]
		double  vPotO3DetoxCum;		//[µg(O3)/kg(TS)]

		double  vO3IndDefense;		//[%/100/(µg(O3)/kg(TS))]
		double  vO3DamageRate;		//[(1/d)/(µg(O3)/kg(TS))]
		double  vO3DefenseEff;		//[µg(O3)/kg]
		double  vO3Beta;			//[-]

        double  vO3Uptake;         //µg
        double  vO3UptakeDay;      //µg
        double  vO3UptakeCum;      //µg

		//leaf pathogenes
		double  vLfPathIndDefense;	//[(%/100)/(m2(infected)/m2)]
		double  vLfPathDamageRate;	//[(1/d)/(m2(infected)/m2)]
		double  vLfPathDefenseEff;	//[(1/d)/(kg(def.comp.)/kg(TS))]
		double  vLfPathBeta;			//[-]
		double  vCritLfPath;			//[m2(infected)/m2(leaf)]

		//root pathogenes
		double  vRtPathIndDefense;	//[(%/100)/(m2(infected)/m2)]
		double  vRtPathDamageRate;	//[(1/d)/(m2(infected)/m2)]
		double  vRtPathDefenseEff;	//[(1/d)/(kg(def.comp.)/kg(TS))]
		double  vRtPathBeta;			//[-]
		double  vCritRtPath;			//[cm(infected)/cm(root)]

        //stress factors
        double PhiO3;

        //O3-stress (van Oijen)
        double dCostForDetox; //amount of assimilates used for detoxification
        double dCostsRepair; //amount of assimilates used for repair
        double dCostForDetoxCum; //amount of assimilates used for detoxification
        double dCostsRepairCum; //amount of assimilates used for repair
        double dRepairR; //repair rate of Rubisco
        double dRubiscoConc;  //concentration of Rubisco in leaves
        double dRubiscoCont;   //content of Rubisco in leaves
        double dRubiscoContCrit;   //critical content of Rubisco in leaves
	}
	PLANTSTRESS;


typedef struct stPlantNeighbours * PPLANTNEIGHBOURS;
typedef struct stPlantNeighbours
	{
	 PPLANT	pPlantLeft;
	 PPLANT	pPlantRight;
	 PPLANT	pPlantUpper;
	 PPLANT	pPlantLower;
	}
	PLANTNEIGHBOURS;


typedef struct stPlathoPlant * PPLATHOPLANT;
typedef struct stPlathoPlant
{
	PPLATHOGENOTYPE			pPlathoGenotype;
	PPLATHODEVELOP			pPlathoDevelop;
	PPLATHOBIOMASS			pPlathoBiomass;
	PPLATHOPARTITIONING		pPlathoPartitioning;
	PPLATHOMORPHOLOGY		pPlathoMorphology;
	PPLATHONITROGEN			pPlathoNitrogen;
	PPLANTSTRESS			pPlantStress;
	PPLANTNEIGHBOURS		pPlantNeighbours;
//	PPLATHOPLANT			pNext;
//	PPLATHOPLANT			pBack;
}
PLATHOPLANT;

typedef struct stPlathoAllPlants * PPLATHOALLPLANTS;
typedef struct stPlathoAllPlants
{
	double	dPlantsInSimulation;
	double	dAbvgSpaceOccupation;
	double	dBlwgSpaceOccupation;
    double  dActTranspR;
    double  dActTranspDay;
    double  dPotTranspDay;
    double  dPotTranspR;
}
PLATHOALLPLANTS;


// Defininition spezieller PLATHO-Variablen:
typedef struct stPlathoClimate   * PPLATHOCLIMATE;
typedef struct stPlathoClimate  
	{
	double   vSimTime;
	double   vRad;
	double   vPAR;
	double   vTemp;
	double   vHumidity;
	double   vCO2;
	double   vO3;

	PPLATHOCLIMATE    pNext;
	PPLATHOCLIMATE    pBack;
	}
	PLATHOCLIMATE;



typedef struct stPlathoScenario * PPLATHOSCENARIO;
typedef struct stPlathoScenario  
	{
		int		iRows;
		int		iColumns;
		int		nPlants;
        double  vLatticeSpacing;    //(mean) distance between individuals [m]
        double  vTotalPlantDensity; //total number of plants per square meter
        int     iLeafLayers;
		char	acResolution[5];

		double  vPhotoperiod;
        double  vCO2Day;
		double  vO3ConcMin;
		double  vO3ConcMax;
		double  vLeafPathogenes;	//[m2(infected)/m2(leaf)]
		double  vRootPathogenes;	//[cm(infected)/cm(root)]

		BOOL	abPlatho_Out[110];
	}
	PLATHOSCENARIO;



typedef struct stGrowthEfficiency * PGROWTHEFFICIENCY;
typedef struct stGrowthEfficiency
	{
		double vCarbohydrates;
		double vProteins;
		double vLipids;
		double vLignins;
		double vOrganicAcids;
		//double vAminoAcids;
		//double vNucleotides;
		double vMinerals;
		double vProteinsN;
		double vDefComp;

		double vProteinsNitRed;
		double vAminoAcidsNitRed;
		double vNucleotidesNitRed;
	}
	GROWTHEFFICIENCY;

typedef struct stCO2Required * PCO2REQUIRED;
typedef struct stCO2Required
	{
		double vCarbohydrates;
		double vProteins;
		double vLipids;
		double vLignins;
		double vOrganicAcids;
		//double vAminoAcids;
		//double vNucleotides;
		double vMinerals;
		double vProteinsN;
		double vDefComp;

		double vProteinsNitRed;
		double vAminoAcidsNitRed;
		double vNucleotidesNitRed;
	}
	CO2REQUIRED;


typedef struct stCarbon * PCARBON;
typedef struct stCarbon
	{
		double vCarbohydrates;
		double vProteins;
		double vLipids;
		double vLignins;
		double vOrganicAcids;
		//double vAminoAcids;
		//double vNucleotides;
		double vMinerals;
		double vO3DefComp;
		double vLfPathDefComp;
		double vRtPathDefComp;
	}
	CARBON;


typedef struct stBiochemistry * PBIOCHEMISTRY;
typedef struct stBiochemistry
	{
		PGROWTHEFFICIENCY	pGrowthEfficiency;
		PCO2REQUIRED		pCO2Required;
		PCARBON				pCarbon;
	}
	BIOCHEMISTRY;

typedef struct stPlathoModules * PPLATHOMODULES;
typedef struct stPlathoModules  
	{
		int		iFlagPhotosynthesis;
		int		iFlagCi;
		int		iFlagLightInterception;
		int		iFlagStomCond;
		int		iFlagNitrogenDistribution;
		int		iFlagSpecLeafArea;
        int		iFlagElasticity;
        int		iFlagH2OEffectPhot;
        int		iFlagNEffectPhot;
        int		iFlagO3;
        int		iFlagCH2OEffectPhot;
        int		iFlagNEffectGrw;
        int     iFlagPriority;
        int		iFlagLeafPathEffPhot;
        int		iFlagRootPathEffUpt;
        int		iFlagMycorrhiza;
        int		iFlagSectors;
        int     iFlagCropFactors;
	}
	PLATHOMODULES;



	
PPLATHOSCENARIO			pPlathoScenario;
PPLATHOMODULES			pPlathoModules;
PPLATHOCLIMATE			pPlathoClimate;
PPLATHOPLANT			pPlathoPlant[MAXPLANTS];
PPLATHOALLPLANTS		pPlathoAllPlants;
PBIOCHEMISTRY			pBiochemistry;

