//____________________________________________________________________________
/*
 Copyright (c) 2003-2020, The GENIE Collaboration
 For the full text of the license visit http://copyright.genie-mc.org

 Costas Andreopoulos <constantinos.andreopoulos \at cern.ch>
 University of Liverpool & STFC Rutherford Appleton Laboratory
*/
//____________________________________________________________________________

#include <TMath.h>
#include <TVector3.h>

#include "Framework/Conventions/Constants.h"
#include "Physics/Coherent/EventGen/COHPrimaryLeptonGenerator.h"
#include "Framework/GHEP/GHepParticle.h"
#include "Framework/GHEP/GHepRecord.h"
#include "Framework/Interaction/Interaction.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Numerical/RandomGen.h"
#include "Framework/EventGen/XSecAlgorithmI.h"
#include "Framework/EventGen/EVGThreadException.h"
#include "Framework/EventGen/EventGeneratorI.h"
#include "Framework/EventGen/RunningThreadInfo.h"

using namespace genie;
using namespace genie::constants;

//___________________________________________________________________________
COHPrimaryLeptonGenerator::COHPrimaryLeptonGenerator() :
  PrimaryLeptonGenerator("genie::COHPrimaryLeptonGenerator")
{

}
//___________________________________________________________________________
COHPrimaryLeptonGenerator::COHPrimaryLeptonGenerator(string config) :
  PrimaryLeptonGenerator("genie::COHPrimaryLeptonGenerator", config)
{

}
//___________________________________________________________________________
COHPrimaryLeptonGenerator::~COHPrimaryLeptonGenerator()
{

}
//___________________________________________________________________________
void COHPrimaryLeptonGenerator::ProcessEventRecord(GHepRecord * evrec) const
{
  
  const Interaction * interaction = evrec -> Summary() ;
  const XclsTag & xcls = interaction -> ExclTag() ;

  if ( xcls.NPions() ==1 ) {

    //-- Access cross section algorithm for running thread
    RunningThreadInfo * rtinfo = RunningThreadInfo::Instance();
    const EventGeneratorI * evg = rtinfo->RunningThread();
    const XSecAlgorithmI *fXSecModel = evg->CrossSectionAlg();
    
    // In Rein and Berger-Sehgal, no modification is required to the standard impl.
    if (fXSecModel->Id().Name() == "genie::ReinSehgalCOHPiPXSec") {
      PrimaryLeptonGenerator::ProcessEventRecord(evrec);
    }
    else if ((fXSecModel->Id().Name() == "genie::BergerSehgalCOHPiPXSec2015")) {
      PrimaryLeptonGenerator::ProcessEventRecord(evrec);
    }
    else if ((fXSecModel->Id().Name() == "genie::BergerSehgalFMCOHPiPXSec2015")) {
      PrimaryLeptonGenerator::ProcessEventRecord(evrec);
    }
    else if ( fXSecModel->Id().Name() == "genie::AlvarezRusoCOHPiPXSec" ) {
      CalculatePrimaryLepton_AlvarezRuso(evrec);
    }
    else {
      LOG("COHPrimaryLeptonGenerator",pFATAL) <<
	"ProcessEventRecord >> Cannot calculate primary lepton for " <<
	fXSecModel->Id().Name();
    }
  }
  else if ( xcls.NSingleGammas() == 1 ) {
    
    const Kinematics & kinematics = interaction->Kine();

    const TLorentzVector * nu_p4 = evrec->Probe() -> P4() ;
    TVector3 nu_dir = nu_p4 -> Vect().Unit() ; 
    
    TVector3 lep_p3 = kinematics.FSLeptonP4().Vect();
    lep_p3.RotateUz( nu_dir ) ;
    
    TLorentzVector lep_p4( lep_p3, kinematics.FSLeptonP4().E() ) ;
    
    int pdgc = interaction->FSPrimLepton()->PdgCode();

    evrec -> AddParticle( pdgc, kIStStableFinalState, 
			  evrec->ProbePosition(), -1, // mothers
			  -1, -1,                     //  daughters
			  lep_p4, * evrec->Probe() -> X4() ) ;
  }

}
//___________________________________________________________________________
void COHPrimaryLeptonGenerator::CalculatePrimaryLepton_AlvarezRuso(GHepRecord * evrec) const
{
  Interaction * interaction = evrec->Summary();
  const Kinematics & kinematics = interaction->Kine();
  TLorentzVector p4l = kinematics.FSLeptonP4();
  int pdgc = interaction->FSPrimLepton()->PdgCode();
  this->AddToEventRecord(evrec, pdgc, p4l);
  this->SetPolarization( evrec );
}
//___________________________________________________________________________
