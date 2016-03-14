/* Implementation of IsletSimulatorClass custom class
	Authors: William Fischer, Matt Wescott
*/

#include "islet-simulator.h"
#include "islet-file-handler.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <ostream>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <boost/lexical_cast.hpp>

using namespace std;

// Parallel processing core number

IsletSimulatorClass::IsletSimulatorClass(IsletFileHandlerClass tempHandler)
{	
	/* The following is a general implementation for pulling in a series 
		of user defined variable values from an input file. Not yet 
		implemented for variable assignment. So it doesn't do anything
		useful yet. - WLF
 	*/
	
	fileHandler = tempHandler;
		
	ifstream userVarFile;
	userVarFile.open(fileHandler.get_userVarsFile());
	
	for (int i = 0; !userVarFile.eof(); i++)
	{	
		/* The user defined variables are passed into a variable matrix
			"userVarMatrix" from the input file as string types. They are 
			error checked, converted into the appropriate data type, and 
			assigned in "setUserDefinedVars()". -WLF
		*/
		char buffer[20];
		stringstream varStream;
		userVarFile.getline(buffer, 20);
		varStream << buffer;
		varStream.getline(buffer, 10, '=');
		string strBuffer(buffer);
		userVarMatrix[0][i] = strBuffer;	
		varStream.getline(buffer, 10, ';');
		strBuffer.assign(buffer);
		userVarMatrix[1][i] = strBuffer;
	}
	
	IsletSimulatorClass::setDefaultVars();
	IsletSimulatorClass::setUserDefinedVars();
}

void IsletSimulatorClass::setDefaultVars()
{	
	// simulation vars:
	cellNumber = 1000;
	runTime = 500.0;
	stepTime = 0.18;
	Glucose = 11.0;
	
	IsletSimulatorClass::setInitialBetaCellVars();
	
	// Islet variables:
	islet.ktt = 0.05;
	islet.kdd = 0.01;
	islet.ktd = 0.026;
	islet.KRe = 0.000126;
	islet.Kfa = 0.0000063;
	islet.Stoichi = 2.5;
	islet.Rvol = 2.5;
	islet.kATPCa = 0.187;
	islet.kATP = 0.000062;
	islet.kADPf = 0.0002;
	islet.kADPb = 0.00002;
	islet.Naout = 140;
	islet.Kout = 5.4;
	islet.Caout = 2.6;
	islet.Cm = 6.158;
	islet.volER = 280;
	islet.fi = 0.01;
	islet.fer = 0.025;

	// Icav
	islet.RCaLNa = 0.0000185;
	islet.RCaLK = 0.000367;
	islet.PCaL = 48.9;

	// Ikslow
	islet.PKslow = 0.2;
	islet.nKslow = 2.2;
	islet.KdKslow = 0.00074;

	// Ikdr
	islet.pKDr = 2.1;
	islet.P_PMCA = 1.56;
	islet.K_PMCA = 0.00014;

	// ICRAN params
	islet.PCRAN = 0.00764;
	islet.KCaer = 0.003;
	islet.RNa_K_CRAN = 0.8;
	islet.pIbNSC = 0.00396;
	islet.KTRPM = 0.00076;
	islet.pTRPM = 0.0234;
	islet.RNa_K_TRPM = 0.8;

	//Ikatp params
	islet.gKATP = 2.31;

	// NA/Ca exchange params
	islet.KdNao = 87.5;
	islet.KdCao = 1.38;
	islet.KdNai = 20.75;
	islet.KdCai = 0.0184;
	islet.k3 = 1;
	islet.k4 = 1;

	// Na/K pump
	islet.Pii = 1.9;
	islet.Proton = 0.0001;
	islet.Kd_MgATP = 0.6;
	islet.Kd_Nao0 = 26.8;
	islet.Kd_Nai0 = 5.0;
	islet.Kd_Ko0 = 0.8;
	islet.Kd_Ki0 = 18.8;
	islet.delta_Nao = 0.44;
	islet.delta_Nai = -0.14;
	islet.delta_Ko = 0.23;
	islet.delta_Ki = -0.14;
	islet.k1_plus = 1.253;
	islet.k2_plus = 0.139;
	islet.k3_plus = 6.96;
	islet.k4_plus = 0.52;
	islet.k1_minus = 0.139;
	islet.k2_minus = 0.0139;
	islet.k3_minus = 13900;
	islet.k4_minus = 0.348;
	islet.PNaK = 350;

	//Metabolism
	islet.Nt = 10;

	//ER dynamics
	islet.KCarp = 0.0005;

	//Glycolysis and oxidative phosph;
	islet.KmATP = 0.5;
	islet.hgl = 2.5;
	islet.Kg = 13;

	//Check the Pop value
	islet.Kop = 0.02;

	// IKATP: ATP gated potassium channel current:
	islet.residual = 0.0;
	islet.kPrime = 1.0;

	// noise parameters
	islet.taup = 500;

	//exocytosis rates between pools
	islet.fusionMax = .030;
	islet.nFuse = 4;
	islet.K_I = 0.0022;

	// rate into immediately releasable pool
	islet.r1 = 0.020/1000;

	// rate out of IRP
	islet.r_1 = 0.025/1000;
	
	// rates into/out of primed and docked pools, and from reserve
	islet.r2 = 0.00012/1000;
	islet.r_2 = 0.0012/1000;
	islet.Rres = 0.00005/1000;
	islet.R_res = 0.00004/1000;

	// for cAMP later
	islet.CaN = 4;
	islet.Kp = 2.3E-4;
	islet.cN = 4;
	islet.cKi = 2.3E-3;

	//rate of movement from fusion to release pool (ms)
	islet.u2 = 0.003;

	//rate of release (ms)
	islet.u3 = 0.00004;
	islet.F_md = .01;
}

void IsletSimulatorClass::setInitialBetaCellVars()
{
	// populate initial beta cell parameters from external files
	ifstream cellVarsFile (fileHandler.get_cellPropertiesFile());
	ifstream RVFile(fileHandler.get_randomVarsFile());
	ifstream NNFile(fileHandler.get_nnFile());
	
	if (cellVarsFile.is_open() && RVFile.is_open() && NNFile.is_open())
	{
		for (int cellIndex = 0; cellIndex < cellNumber; cellIndex++)
		{
			// create temporary beta cell variable.
			BetaCellStructure cell;
						
			for (int varFileIndex = 0; varFileIndex < 30; varFileIndex++)
			{	// get initial values for cellular variables from an input file
				cellVarsFile >> cell.x[varFileIndex];
			}
			
			// populate beta cell constants from random variable file
			RVFile >> cell.gKATPar;
			RVFile >> cell.gCoup;
			RVFile >> cell.gKtoar;
			RVFile >> cell.PCaER;
			RVFile >> cell.gKCaBKar;
			RVFile >> cell.PNACAar;
			RVFile >> cell.Prelar;
			RVFile >> cell.Popar;
			RVFile >> cell.ATPar;
			RVFile >> cell.KRev;
			RVFile >> cell.RandomSeed;
			RVFile >> cell.gChR2;
			
			// populate nearest neighbor array from nn file.
		
			for (int nnIndex = 0; nnIndex < 15; nnIndex++)
			{
				int  nnID;
				NNFile >> nnID;
				if (nnID != -1)
				{	/* the NNFile is formatted so "-1" values are placed to fill
						each row with 15 total integers after the list of actual
						nearest neighbor cell #'s. This only fills the vector with
						actual values.
					*/
					cell.nnVector.push_back(nnID);
				}
			}
			cell.nnCount = cell.nnVector.size();	
			betaCells.push_back(cell);
		}
		cellVarsFile.close();
		RVFile.close();
		NNFile.close();
	}
}


void IsletSimulatorClass::setUserDefinedVars()
{
	/*	This function over-rides default variables with any user defined
	in the input file. Easily expansible to any commonly manipulated
	variables. -WLF
	*/

	for(int i = 0; userVarMatrix[0][i] !=   ""; i++)
	{
		/* If a given variable is on the list, set the value. Used boost
			lexical_cast to convert string to double because stod, atof,
			and strod aren't correctly implemented in cygwin. -WLF
		*/
		if(userVarMatrix[0][i]   ==   "ktt")
		{
			islet.ktt = boost::lexical_cast<double>(userVarMatrix[1][i]);
			cout << "ktt value derived from user input: " << islet.ktt << endl;
		}

		if(userVarMatrix[0][i]   ==   "kdd")
		{
			islet.kdd = boost::lexical_cast<double>(userVarMatrix[1][i]);
			cout << "kdd value derived from user input: " << islet.kdd << endl;
		}

		if(userVarMatrix[0][i]   ==   "ktd")
		{
			islet.ktd = boost::lexical_cast<double>(userVarMatrix[1][i]);
			cout << "ktd value derived from user input: " << islet.ktd << endl;
		}

		if(userVarMatrix[0][i]   ==   "runTime")
		{
			runTime = boost::lexical_cast<double>(userVarMatrix[1][i]);
			cout << "runTime value derived from user input: " << runTime << endl;
		}

		if(userVarMatrix[0][i]   ==   "stepTime")
		{
			stepTime = boost::lexical_cast<double>(userVarMatrix[1][i]);
			cout << "stepTime value derived from user input: " << stepTime << endl;
		}
	}
}

void IsletSimulatorClass::simulationLoop()
{
	double tOld2 = 0;
	for(double t = 0; t < runTime; t = t + stepTime)
	{	// Loop the simulation through each time step

		for(int cellIndex = 0; cellIndex < cellNumber; cellIndex++)
		{	
			/* Populate variables with their values from the previous time step of the linear 
				approximation. These variables  are used in intermediate calculations before
				the next time step. -WLF
			*/
			BetaCellStructure& cell =  betaCells[cellIndex];
			

		}

		for(int cellIndex = 0; cellIndex < cellNumber; cellIndex++)
		{	// Loop through each beta cell and perform calculations
			BetaCellStructure& cell =  betaCells[cellIndex];

			double& Vm = cell.x[0];
			double& Nai = cell.x[1];
			double& Ki = cell.x[2];
			double& Cai = cell.x[3];
			double& Caer = cell.x[4];
			double& ATP = cell.x[5];
			double& MgADP = cell.x[6];
			double& Re = cell.x[7];
			double& q_KDr = cell.x[8];
			double& d_CaL = cell.x[9];
			double& U_CaL = cell.x[10];
			double& fus = cell.x[11];
			double& p_KDr = cell.x[12];
			double& m_Kto = cell.x[13];
			double& h_Kto = cell.x[14];
			double& E1_tota = cell.x[15];
			double& I1 = cell.x[16];
			double& I2 = cell.x[17];
			//double& O1 = cell.x[18];
			//double& O2 = cell.x[19];
			//double& C1 = cell.x[20];
			//double& C2 = cell.x[21];
			double& IRP = cell.x[22];
			double& PP = cell.x[23];
			double& DP = cell.x[24];
			double& RES = cell.x[25];
			double& FIP = cell.x[26];
			double& RIP = cell.x[27];
			//double& Cap = cell.x[28];
			double& Pns = cell.x[29];

			/* Intermediate calculations between previous time step and next
				This takes the current data set, which is transferred into BetaCellStructure variables from
				the cell.x[] array (seen above) and makes intermediate calculations which feed into the 
				calculations for the next time step. -WLF
			*/
			
			double ADPb=cell.ATPar-ATP-MgADP/0.55;
			double voli=1.049*exp(0.456*cell.nnCount)+738.7;
			// This resets the Caer values imported from above... Why is that, and how does it effect outcome?
			Caer=yini4+(islet.fer*voli/2/islet.volER)*(islet.Cm/F/voli*(Vm-yini0)-(Nai-yini1)-(Ki-yini2)-2/islet.fi*(Cai-yini3)); 
			
			double Jserca = cell.PCaER*Cai*Cai/(Cai*Cai+islet.KCarp*islet.KCarp);
			double Jout = cell.Prelar*(Caer-Cai);
			
			// Glycolysis and oxidative phosph;
			double fGlu = ATP/(islet.KmATP+ATP)*pow(Glucose, islet.hgl)/(pow(islet.Kg, islet.hgl)+pow(Glucose, islet.hgl));
			
			//Check the Pop value
			double JOP = cell.Popar*Re*pow(MgADP,2)/(pow(MgADP,2)+pow(islet.Kop,2));
			
			//Constant field equations
			double NaCF = (Vm/RTF)/(1-exp(-Vm/RTF))*(Nai-islet.Naout*exp(-Vm/RTF));
			double KCF = (Vm/RTF)/(1-exp(-Vm/RTF))*(Ki-islet.Kout*exp(-Vm/RTF));
			double CaCF = (Vm/RTF2)/(1-exp(-Vm/RTF2))*(Cai-islet.Caout*exp(-Vm/RTF2));
			
			//reverse potentials
			double EK = RTF*log(islet.Kout/Ki);
			double ENa = RTF*log(islet.Naout/Nai);
			double ECa = RTF*log(islet.Caout/Cai)/2;
			double IbNSC1 = islet.pIbNSC*NaCF;
			double IbNSC2 = 0.01*KCF;
			double IbNSC0 = IbNSC1+IbNSC2;
			double dalpha = 1/(0.88*exp(-(Vm-3)/50)+0.09*exp(-(Vm-3)/600));
			double dbeta = 1/(5.48*exp((Vm-3)/12)+1.245*exp((Vm-3)/30));
			double VpOpen = pow(d_CaL,2);
			
			//Calcium gating function
			double SingleiCaL = 0.0676*CaCF;
			double Ualpha = 0.0042*2;													
			double Ubeta = 0.1159*(-1.15*SingleiCaL*VpOpen+Cai)*2;

			//Ultraslow gate
			double usalpha = 1/(75000*exp(Vm/34));
			double usbeta = 1/(5000*exp(-Vm/19)+500*exp(-Vm/100));	

			// ICaL
			double RundownATP = 1/(1+pow((1.4/ATP),3));
			double pO = (VpOpen*U_CaL*(0.4+0.6*fus))*RundownATP;
			double ICaL1 = islet.RCaLNa*islet.PCaL*pO*NaCF;
			double ICaL2 = islet.RCaLK*islet.PCaL*pO*KCF;
			double ICaL3 = islet.PCaL*pO*CaCF;
			double ICaL0 = ICaL1+ICaL2+ICaL3;
			
			// Ikdr
			double alphap = 1.1/(25*exp(-(Vm-3)/8)+1*exp(-(Vm-3)/100));
			double betap = 1.1/(25*exp(Vm/100));
			double alphaq = 1/800;
			double betaq = 1/(1000*exp(-Vm/8)+100*exp(-Vm/100));
			double IKDr2 = islet.pKDr*p_KDr*(0.6*q_KDr+0.4)*KCF;
			double IKDr0 = IKDr2;	// Both of these are used in separate equations elsewhere. Keep it like this
													// in case this formula needs to be expanded later (to avoid confusion)
			
			// Ikto
			double alpham = 0.4/(5.46*exp(-Vm/20));
			double betam = 0.4/(2.48*exp(Vm/60));
			double alphah = 1.7/(969*exp(Vm/500));
			double betah = 1.7/(13.2*exp(-Vm/9)+6.93*exp(-Vm/1000));
			double IKto2 = cell.gKtoar*m_Kto*h_Kto*(Vm-EK);
			double IKto0 = IKto2;	// Both of these are used in separate equations elsewhere. Keep it like this
													// in case this formula needs to be expanded later (to avoid confusion)		

			// ITRPM
			double PoTRPM = 1/(1+pow((islet.KTRPM/Cai),1.7));
			double ITRPM1 = islet.pTRPM*islet.RNa_K_TRPM*NaCF*PoTRPM;
			double ITRPM2 = islet.pTRPM*KCF*PoTRPM;
			double ITRPM0 = ITRPM1+ITRPM2;
			
			// IKATP: ATP gated potassium channel current:
			double PoATP = (islet.residual*0.5)+((1-islet.residual)*(0.08*(1+2*MgADP/islet.kdd)+0.89*pow((MgADP/islet.kdd),2))/
										pow((1+MgADP/islet.kdd),2)/(1+0.45*MgADP/islet.ktd+ATP/(islet.kPrime*islet.ktt)));
			
			double IChR2 = 0;

			/*
			// These don't do anything at the moment. -WLF
			//double dO1 = 0;
			//double dO2 = 0;
			//double dC1 = 0;
			//double dC2 = 0;
			
			//IChR2 = ChR2Current(10000,dt,15000);
			//double IChR2 = gChR2[j]*Vm*0.3*(O1+0.04*O2);
			
			//	Disabled: KATP Mutant equation
			if(t>1000)
			{
				if (j<cellNumber*pMutant)
				{
					PoATP = 0.5*PoATP+(1-0.5)*pKATP[j];
					if (PoATP<0)
					{
						PoATP = 0;
					}
				}
			}
			*/

			double IKATP2 = cell.gKATPar*(1+Pns)*PoATP*(Vm-EK);
			double IKATP0 = IKATP2;
			
			//INaK
			double fVm = F*Vm/(R*Tem);
			double Kd_Nao = islet.Kd_Nao0*exp(islet.delta_Nao*fVm);
			double Kd_Nai = islet.Kd_Nai0*exp(islet.delta_Nai*fVm);
			double Kd_Ko = islet.Kd_Ko0*exp(islet.delta_Ko*fVm);
			double Kd_Ki = islet.Kd_Ki0*exp(islet.delta_Ki*fVm);
			double Nai_ = Nai/Kd_Nai;
			double Naout_ = islet.Naout/Kd_Nao;
			double Ki_ = Ki/Kd_Ki;
			double Kout_ = islet.Kout/Kd_Ko;
			double MgATP_ = ATP/islet.Kd_MgATP;

			double a1_plus = (islet.k1_plus*pow(Nai_,3.0))/(pow((1+Nai_),3)+pow((1+Ki_),2)-1);
			double a2_plus = islet.k2_plus;
			double a3_plus = islet.k3_plus*pow(Kout_,2)/(pow((1+Naout_),2)+pow((1+Kout_),2)-1);
			double a4_plus = islet.k4_plus*MgATP_/(1+MgATP_);
			double a1_minus = islet.k1_minus*MgADP;
			double a2_minus = islet.k2_minus*pow(Naout_,3)/(pow((1+Naout_),3)+pow((1+Kout_),2)-1);
			double a3_minus = islet.k3_minus*islet.Pii*islet.Proton/(1+MgATP_);
			double a4_minus = islet.k4_minus*pow(Ki_,2)/(pow((1+Nai_),3)+pow((1+Ki_),2)-1);
			
			double denom = (a1_minus+a1_plus)*a2_minus*a3_minus+a1_plus*a2_plus*(a3_plus+a3_minus)+a2_plus*a3_plus*(a4_plus+a4_minus)+(a2_plus+a2_minus)*a3_minus*a4_minus+(a1_minus+a1_plus)*a3_plus*a4_plus+a1_minus*(a3_plus+a3_minus)*a4_minus+a1_plus*(a2_plus+a2_minus)*a4_plus+a1_minus*a2_minus*(a4_plus+a4_minus);
			double numer = a1_plus*a2_plus*a3_plus*a4_plus-a1_minus*a2_minus*a3_minus*a4_minus;
			double iglc = (0.4+0.6*exp(-Glucose/5.84));
			double vcyc = (numer/denom)*iglc;
			double INaK0 = islet.PNaK*vcyc;
			double INaK1 = 3*INaK0;
			double INaK2 = -2*INaK0;

			// INaCa slow
			double pE1Na = 1/(1+pow((islet.KdNai/Nai),3)*(1+Cai/islet.KdCai));
			double pE1Ca = 1/(1+(islet.KdCai/Cai)*(1+pow((Nai/islet.KdNai),3)));
			double pE2Na = 1/(1+pow((islet.KdNao/islet.Naout),3)*(1+islet.Caout/islet.KdCao));
			double pE2Ca = 1/(1+(islet.KdCao/islet.Caout)*(1+pow((islet.Naout/islet.KdNao),3)));
			double k1 = exp(0.32*Vm/RTF);
			double k2 = exp((0.32-1)*Vm/RTF);
			double fCa = Cai/(Cai+0.004);
			double alpha1 = pE1Na*(fCa*0.002+(1-fCa)*0.0015);
			double beta1 = fCa*0.0012+(1-fCa)*0.0000005;
			double alpha2 = fCa*0.00003+(1-fCa)*0.01;
			double beta2 = fCa*0.09+(1-fCa)*0.0001;

			// IPMCA
			double IPMCA0 = islet.P_PMCA*pow(Cai,2)/(pow(Cai,2)+pow(islet.K_PMCA,2));
			double IPMCA1 = -IPMCA0;
			double IPMCA3 = 2*IPMCA0;
			double kf = k2*pE2Na+islet.k4*pE2Ca;
			double kb = k1*pE1Na+islet.k3*pE1Ca;
			double E2_tot = 1-E1_tota-I1-I2;
			double INaCa0 = cell.PNACAar*(k1*pE1Na*E1_tota-k2*pE2Na*E2_tot);
			double INaCa1 = 3*INaCa0;
			double INaCa3 = -2*INaCa0;

			//IKSLOW
			double PoKslow = 1/(1+pow((islet.KdKslow/Cai),islet.nKslow));
			double IKslow2 = islet.PKslow*PoKslow*KCF;
			double IKslow0 = IKslow2;	// Both of these are used in separate equations elsewhere. Keep it like this
															// in case this formula needs to be expanded later (to avoid confusion)		

			double PoCRAN = 1/(1+exp((Caer-islet.KCaer)/0.003));
			double ICRAN1 = islet.PCRAN*islet.RNa_K_CRAN*PoCRAN*NaCF;
			double ICRAN2 = islet.PCRAN*PoCRAN*KCF;
			double ICRAN3 = islet.PCRAN*PoCRAN*CaCF*20;
			double ICRAN0 = ICRAN1+ICRAN2+ICRAN3;
						
			double Icoup=0;
			for (int nnIndex = 0; nnIndex < cell.nnVector.size(); nnIndex++)
			{	// get cell data for each couple nearest neighbor cell
				int nnID = cell.nnVector[nnIndex];
				BetaCellStructure neighbor = betaCells[nnID];
				Icoup=Icoup+((cell.gCoup+neighbor.gCoup)/2)*(Vm-neighbor.x[0]);			// x[0] is the nearest neighbor's stored Vm value.
																																			// Vm can't be used because it points to the current cell's x[0]
			}
			
			double Itot = IbNSC0+IKDr0+IKto0+IKATP0+ITRPM0+ICaL0+INaK0+INaCa0+IPMCA0+IKslow0+ICRAN0+Icoup+IChR2;
			double INatot = IbNSC1+ITRPM1+ICaL1+INaK1+INaCa1+IPMCA1+ICRAN1+Icoup/3+IChR2/2;
			double IKtot = IbNSC2+IKDr2+IKto2+IKATP2+ITRPM2+ICaL2+INaK2+IKslow2+ICRAN2+Icoup/3;
			double ICatot = ICaL3+INaCa3+IPMCA3+ICRAN3+Icoup/3+IChR2/2;
			
			double JGlyc = cell.KRev*fGlu*(islet.Nt-Re);
			double JBox = islet.Kfa*(islet.Nt-Re);

			// noise parameters
			double noiseRand = (rand() % 3) - 1;
			double noisey = noiseRand / 80;
			
			// FUSION POOL EXOCYTOSIS
			//Hill equation for fusion with plasma membrane
			long double fusion_I = islet.fusionMax * (pow(Cai,islet.nFuse))/(pow(Cai,islet.nFuse) + pow(islet.K_I,islet.nFuse));

			long double Lflux = 5.18E-15*ICaL3/(.00383E-3);
			/* Calc dxdt steps for each differential equation at each time 
				point.
			*/
				
			cell.dxdt[0] = -Itot/islet.Cm;
			cell.dxdt[1] = -INatot/(F*voli);
			cell.dxdt[2] = -IKtot/(F*voli);
			cell.dxdt[3] = islet.fi*(-ICatot/(2*F)-Jserca+Jout)/voli;
			cell.dxdt[4] = islet.fer*(Jserca-Jout)/islet.volER;
			cell.dxdt[5] = JOP-((INaK0+IPMCA0)/F+Jserca/2)/voli-(islet.kATP+islet.kATPCa*Cai)*ATP;
			cell.dxdt[6] = -0.55*(JOP-((INaK0+IPMCA0)/F+Jserca/2)/voli-(islet.kATP+islet.kATPCa*Cai)*ATP)+0.55*islet.kADPb*ADPb-islet.kADPf*MgADP;
			cell.dxdt[7] = JGlyc+JBox-JOP*islet.Rvol/islet.Stoichi;
			cell.dxdt[8] = alphaq*(1-q_KDr)-betaq*q_KDr;
			cell.dxdt[9] = dalpha*(1-d_CaL)-dbeta*d_CaL;
			cell.dxdt[10] = Ualpha*(1-U_CaL)-Ubeta*U_CaL;
			cell.dxdt[11] = usalpha*(1-fus)-usbeta*fus;
			cell.dxdt[12] = alphap*(1-p_KDr)-betap*p_KDr;
			cell.dxdt[13] = alpham*(1-m_Kto)-betam*m_Kto;
			cell.dxdt[14] = alphah*(1-h_Kto)-betah*h_Kto;
			cell.dxdt[15] = E2_tot*kf+I1*beta1+I2*beta2-E1_tota*(kb+alpha1+alpha2);
			cell.dxdt[16] = E1_tota*alpha1-I1*beta1;
			cell.dxdt[17] = E1_tota*alpha2-I2*beta2;
				//cell.dxdt[18] = dO1;
				//cell.dxdt[19] = dO2;
				//cell.dxdt[20] = dC1;
				//cell.dxdt[21] = dC2;
			//exocytosis ODEs
			cell.dxdt[22] = islet.r1 * PP - islet.r_1 * IRP - fusion_I * IRP;
			cell.dxdt[23] = islet.r_1* IRP - (islet.r1+islet.r_2)*PP +islet.r2*DP;
			cell.dxdt[24] = islet.r_2* PP - islet.r2*DP + islet.Rres*RES - islet.R_res*RES;
			cell.dxdt[25] = islet.Rres*RES-islet.R_res*RES;
			cell.dxdt[26] = fusion_I*IRP - islet.u2*FIP;
			cell.dxdt[27] = islet.u2*FIP-islet.u3*RIP;
			cell.dxdt[28] = .0035*fusion_I*IRP;
			cell.dxdt[29] = (-Pns)/islet.taup-(Pns/islet.taup)+noisey;
		}

		for(int cellIndex = 0; cellIndex < cellNumber; cellIndex++)
		{	// Loop back through to use dxdt for linear approximation.
			for(int i = 0; i < 18; i++)
			{
				betaCells[cellIndex].x[i] = betaCells[cellIndex].x[i] + betaCells[cellIndex].dxdt[i] * stepTime;				
			}
			
			for(int i = 22; i < 30; i++)
			{
				betaCells[cellIndex].x[i] = betaCells[cellIndex].x[i] + betaCells[cellIndex].dxdt[i] * stepTime;				
			}
		}
		
		//fileHandler.ObjectiveWriteOutputsOutputs(betaCells, cellNumber);
		
		if ((t - tOld2 ) > 100)
		{
			cout << "Objective Version Time: " << t << endl;
			tOld2=t;
		}
	}
}





// Getters
double IsletSimulatorClass::get_ktt() const
{
	return islet.ktt;
}

double IsletSimulatorClass::get_kdd() const
{
	return islet.kdd;
}

double IsletSimulatorClass::get_ktd() const
{
	return islet.ktd;
}

double IsletSimulatorClass::get_runTime() const
{
	return runTime;
}

double IsletSimulatorClass::get_stepTime() const
{
	return stepTime;
}
