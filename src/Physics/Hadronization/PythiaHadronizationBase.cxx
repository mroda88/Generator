//____________________________________________________________________________
/*
 Copyright (c) 2003-2019, The GENIE Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author: Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
         University of Liverpool & STFC Rutherford Appleton Lab

         Changes required to implement the GENIE Boosted Dark Matter module
         were installed by Josh Berger (Univ. of Wisconsin)
*/
//____________________________________________________________________________

#include "Framework/Conventions/Constants.h"
#include "Framework/GHEP/GHepStatus.h"
#include "Framework/GHEP/GHepParticle.h"
#include "Framework/GHEP/GHepRecord.h"
#include "Framework/EventGen/EVGThreadException.h"
#include "Framework/GHEP/GHepFlags.h"
#include "Framework/Interaction/Interaction.h"
#include "Framework/Messenger/Messenger.h"
#include "Framework/Numerical/RandomGen.h"
#include "Framework/ParticleData/PDGCodes.h"
#include "Framework/ParticleData/PDGUtils.h"
#include "Framework/Utils/KineUtils.h"
#include "Physics/Hadronization/PythiaHadronizationBase.h"

using namespace genie;
using namespace genie::constants;

//____________________________________________________________________________
PythiaHadronizationBase::PythiaHadronizationBase() :
EventRecordVisitorI()
{

}
//____________________________________________________________________________
PythiaHadronizationBase::PythiaHadronizationBase(string name) :
EventRecordVisitorI(name)
{

}
//____________________________________________________________________________
PythiaHadronizationBase::PythiaHadronizationBase(string name, string config) :
EventRecordVisitorI(name, config)
{

}
//____________________________________________________________________________
PythiaHadronizationBase::~PythiaHadronizationBase()
{

}
//____________________________________________________________________________
void PythiaHadronizationBase::ProcessEventRecord(GHepRecord * event) const
{
  Interaction * interaction = event->Summary();

  bool ok = this->AssertValidity(interaction);
  if(!ok) {
     LOG("PythiaHad", pFATAL)
       << "Input interaction type is not allowed!!!";
     LOG("PythiaHad", pFATAL)
       << *event;
     gAbortingInErr = true;
    std::exit(1);
  }

  // Decide the leading quark and remnant diquark PDG codes for this event
  this->MakeQuarkDiquarkAssignments(interaction);

  // Copy original and set required PYTHIA decay flags
  this->CopyOriginalDecayFlags();
  this->SetDesiredDecayFlags();

  // Call PYTHIA6 or PYTHIA8 to obtain the fragmentation products
  TClonesArray * particle_list = this->Hadronize(interaction);

  // Restore
  this->RestoreOriginalDecayFlags();

  if(! particle_list ) {
    LOG("PythiaHad", pWARN)
      << "Hadronizer failure: A null particle list was returned!";

    event->EventFlags()->SetBitNumber(kHadroSysGenErr, true);

    genie::exceptions::EVGThreadException exception;
    exception.SetReason("Could not simulate the hadronic system");
    exception.SwitchOnFastForward();
    throw exception;

    return;
   }

  bool is_nucleus = interaction->InitState().Tgt().IsNucleus();
  GHepStatus_t istfin = is_nucleus ? kIStHadronInTheNucleus : kIStStableFinalState ;

  int mom = event->FinalStateHadronicSystemPosition();
  assert(mom!=-1);

  GHepParticle * neutrino  = event->Probe();
  const TLorentzVector & vtx = *(neutrino->X4());

  GHepParticle * p = 0;
  TIter particle_iter(particle_list);
  while ((p = (GHepParticle *) particle_iter.Next()))
  {
    int pdgc = p -> Pdg() ;

    // set the proper status according to a number of things:
    // interaction on a nucleaus or nucleon, particle type
    GHepStatus_t ist = ( p -> Status() ==1 ) ? istfin : kIStDISPreFragmHadronicState;
    // not that this status here ^ is the pythia state

    // handle gammas, and leptons that might come from internal pythia decays
    // mark them as final state particles
    bool not_hadron = ( pdgc == kPdgGamma ||
			pdg::IsNeutralLepton(pdgc) ||
			pdg::IsChargedLepton(pdgc) ) ;

    if( not_hadron )  { ist = kIStStableFinalState; }
    p -> SetStatus( ist ) ;

    p->SetFirstMother(mom + p->FirstMother() );
    p->SetLastMother( -1 );

    // In Pythia having no daughters means daughter == 0 hnce the following check
    int ifd = (p->FirstDaughter() <= 0 ) ? -1 : mom  + p->FirstDaughter();
    int ild = (p->LastDaughter()  <= 0 ) ? -1 : mom  + p->LastDaughter();
    p->SetFirstDaughter(ifd);
    p->SetLastDaughter (ild);

    // the Pythia particle position is overridden
    p -> SetPosition( vtx ) ;

    event->AddParticle(*p);
  }

  particle_list -> Delete() ;
  delete particle_list ;
}
//____________________________________________________________________________
void PythiaHadronizationBase::MakeQuarkDiquarkAssignments(
  const Interaction * interaction) const
{
  LOG("PythiaHad", pNOTICE)
    << "Making leading quark / remnant di-quark assignments";

  // get kinematics / init-state / process-info

  const Kinematics &   kinematics = interaction->Kine();
  const InitialState & init_state = interaction->InitState();
  const ProcessInfo &  proc_info  = interaction->ProcInfo();
  const Target &       target     = init_state.Tgt();

  assert(target.HitQrkIsSet());

  double W = kinematics.W();

  int  probe       = init_state.ProbePdg();
  int  hit_nucleon = target.HitNucPdg();
  int  hit_quark   = target.HitQrkPdg();
  bool from_sea    = target.HitSeaQrk();

  LOG("PythiaHad", pNOTICE)
    << "Hit nucleon pdgc = " << hit_nucleon << ", W = " << W;
  LOG("PythiaHad", pNOTICE)
    << "Selected hit quark pdgc = " << hit_quark
    << ((from_sea) ? "[sea]" : "[valence]");

  // check hit-nucleon assignment, input neutrino & interaction type
  bool isp  = pdg::IsProton           (hit_nucleon);
  bool isn  = pdg::IsNeutron          (hit_nucleon);
  bool isv  = pdg::IsNeutrino         (probe);
  bool isvb = pdg::IsAntiNeutrino     (probe);
//bool isl  = pdg::IsNegChargedLepton (probe);
//bool islb = pdg::IsPosChargedLepton (probe);
  bool iscc = proc_info.IsWeakCC      ();
  bool isnc = proc_info.IsWeakNC      ();
  bool isdm = proc_info.IsDarkMatter  ();
  bool isem = proc_info.IsEM          ();
  bool isu  = pdg::IsUQuark           (hit_quark);
  bool isd  = pdg::IsDQuark           (hit_quark);
  bool iss  = pdg::IsSQuark           (hit_quark);
  bool isub = pdg::IsAntiUQuark       (hit_quark);
  bool isdb = pdg::IsAntiDQuark       (hit_quark);
  bool issb = pdg::IsAntiSQuark       (hit_quark);

  //
  // Generate the quark system (q + qq) initiating the hadronization
  //

  int  leading_quark   = 0; // leading quark (hit quark after the interaction)
  int  remnant_diquark = 0; // remnant diquark (xF<0 at hadronic CMS)

  // Figure out the what happens to the hit quark after the interaction
  if (isnc || isem || isdm) {
    // NC, EM
    leading_quark = hit_quark;
  } else {
    // CC
    if      (isv  && isd ) leading_quark = kPdgUQuark;
    else if (isv  && iss ) leading_quark = kPdgUQuark;
    else if (isv  && isub) leading_quark = kPdgAntiDQuark;
    else if (isvb && isu ) leading_quark = kPdgDQuark;
    else if (isvb && isdb) leading_quark = kPdgAntiUQuark;
    else if (isvb && issb) leading_quark = kPdgAntiUQuark;
    else {
      LOG("PythiaHad", pERROR)
        << "Not allowed mode. Refused to make a leading quark assignment!";
      return;
    }
  }//CC

  // Figure out what the remnant diquark is.
  // Note from Hugh, following a conversation with his local HEP theorist
  // (Gary Goldstein): "I am told that the probability of finding the diquark
  // in the singlet vs. triplet states is 50-50."

  // hit quark = valence quark
  if(!from_sea) {
    if (isp && isu) remnant_diquark = kPdgUDDiquarkS1; /* u(->q) + ud */
    if (isp && isd) remnant_diquark = kPdgUUDiquarkS1; /* d(->q) + uu */
    if (isn && isu) remnant_diquark = kPdgDDDiquarkS1; /* u(->q) + dd */
    if (isn && isd) remnant_diquark = kPdgUDDiquarkS1; /* d(->q) + ud */
  }
  // hit quark = sea quark
  else {
    if(isp && isu) remnant_diquark = kPdgUDDiquarkS1; /* u(->q) + bar{u} uud (=ud) */
    if(isp && isd) remnant_diquark = kPdgUUDiquarkS1; /* d(->q) + bar{d} uud (=uu) */
    if(isn && isu) remnant_diquark = kPdgDDDiquarkS1; /* u(->q) + bar{u} udd (=dd) */
    if(isn && isd) remnant_diquark = kPdgUDDiquarkS1; /* d(->q) + bar{d} udd (=ud) */

    // The following section needs revisiting.

    // The lepton is scattered off a sea antiquark, materializing its quark
    // partner and leaving me with a 5q system ( <qbar + q> + qqq(valence) )
    // I will force few qbar+q annhilations below to get my quark/diquark system
    // Probably it is best to leave the qqq system in the final state and then
    // just do the fragmentation of the qbar q system? But how do I figure out
    // how to split the available energy?

    /* bar{u} (-> bar{d}) + u uud => u + uu */
    if(isp && isub && iscc) {
      leading_quark   = kPdgUQuark;
      remnant_diquark = kPdgUUDiquarkS1;
    }
    /* bar{u} (-> bar{u}) + u uud => u + ud */
    if(isp && isub && (isnc||isem||isdm)) {
      leading_quark   = kPdgUQuark;
      remnant_diquark = kPdgUDDiquarkS1;
    }
    /* bar{d} (-> bar{u}) + d uud => d + ud */
    if(isp && isdb && iscc) {
      leading_quark   = kPdgDQuark;
      remnant_diquark = kPdgUDDiquarkS1;
    }
    /* bar{d} (-> bar{d}) + d uud => d + uu */
    if(isp && isdb && (isnc||isem||isdm)) {
      leading_quark   = kPdgDQuark;
      remnant_diquark = kPdgUUDiquarkS1;
    }
    /* bar{u} (-> bar{d}) + u udd => u + ud */
    if(isn && isub && iscc) {
      leading_quark   = kPdgUQuark;
      remnant_diquark = kPdgUDDiquarkS1;
    }
    /* bar{u} (-> bar{u}) + u udd => u + dd */
    if(isn && isub && (isnc||isem||isdm)) {
      leading_quark   = kPdgUQuark;
      remnant_diquark = kPdgDDDiquarkS1;
    }
    /* bar{d} (-> bar{u}) + d udd => d + dd */
    if(isn && isdb && iscc) {
      leading_quark   = kPdgDQuark;
      remnant_diquark = kPdgDDDiquarkS1;
    }
    /* bar{d} (-> bar{d}) + d udd => d + ud */
    if(isn && isdb && (isnc||isem||isdm)) {
      leading_quark   = kPdgDQuark;
      remnant_diquark = kPdgUDDiquarkS1;
    }

    // The neutrino is scatterred off s or sbar sea quarks
    // For the time being I will handle s like d and sbar like dbar (copy & paste
    // from above) so that I conserve charge.

    if(iss || issb) {
       LOG("PythiaHad", pNOTICE)
         << "Can not really handle a hit s or sbar quark / Faking it";

       if(isp && iss) { remnant_diquark = kPdgUUDiquarkS1; }
       if(isn && iss) { remnant_diquark = kPdgUDDiquarkS1; }

       if(isp && issb && iscc) {
         leading_quark   = kPdgDQuark;
         remnant_diquark = kPdgUDDiquarkS1;
       }
       if(isp && issb && (isnc||isem||isdm)) {
         leading_quark   = kPdgDQuark;
         remnant_diquark = kPdgUUDiquarkS1;
       }
       if(isn && issb && iscc) {
         leading_quark   = kPdgDQuark;
         remnant_diquark = kPdgDDDiquarkS1;
       }
       if(isn && issb && (isnc||isem||isdm)) {
         leading_quark   = kPdgDQuark;
         remnant_diquark = kPdgUDDiquarkS1;
       }
    }

    // if the diquark is a ud, switch it to the singlet state with 50% probability
    if(remnant_diquark == kPdgUDDiquarkS1) {
      RandomGen * rnd = RandomGen::Instance();
      double Rqq = rnd->RndHadro().Rndm();
      if(Rqq<0.5) remnant_diquark = kPdgUDDiquarkS0;
    }
  }

  fLeadingQuark   = leading_quark;
  fRemnantDiquark = remnant_diquark;
}
//____________________________________________________________________________
bool PythiaHadronizationBase::AssertValidity(const Interaction * interaction) const {

  // check that there is no charm production
  // (GENIE uses a special model for these cases)
  if(interaction->ExclTag().IsCharmEvent()) {
    LOG("PythiaHad", pWARN) << "Can't hadronize charm events";
    return false;
  }
  // check the available mass
  double W = utils::kinematics::W(interaction);
  double Wmin = kNucleonMass+kPionMass;
  if(W < Wmin) {
    LOG("PythiaHad", pWARN) << "Low invariant mass, W = "
				<< W << " GeV!!";
    return false;
  }

  const InitialState & init_state = interaction->InitState();
  const ProcessInfo &  proc_info  = interaction->ProcInfo();
  const Target &       target     = init_state.Tgt();

  if( ! target.HitQrkIsSet() ) {
    LOG("PythiaHad", pWARN) << "Hit quark was not set!";
    return false;
  }

  int  probe       = init_state.ProbePdg();
  int  hit_nucleon = target.HitNucPdg();
  int  hit_quark   = target.HitQrkPdg();
  //bool from_sea    = target.HitSeaQrk();

  // check hit-nucleon assignment, input neutrino & weak current
  bool isp  = pdg::IsProton           (hit_nucleon);
  bool isn  = pdg::IsNeutron          (hit_nucleon);
  bool isv  = pdg::IsNeutrino         (probe);
  bool isvb = pdg::IsAntiNeutrino     (probe);
  bool isdm = pdg::IsDarkMatter         (probe);
  bool isl  = pdg::IsNegChargedLepton (probe);
  bool islb = pdg::IsPosChargedLepton (probe);
  bool iscc = proc_info.IsWeakCC      ();
  bool isnc = proc_info.IsWeakNC      ();
  bool isdmi = proc_info.IsDarkMatter  ();
  bool isem = proc_info.IsEM          ();
  if( !(iscc||isnc||isem||isdmi) ) {
    LOG("PythiaHad", pWARN)
      << "Can only handle electro-weak interactions";
    return false;
  }
  if( !(isp||isn) || !(isv||isvb||isl||islb||isdm) ) {
    LOG("PythiaHad", pWARN)
      << "Invalid initial state: probe = "
      << probe << ", hit_nucleon = " << hit_nucleon;
    return false;
  }

  // assert that the interaction mode is allowed
  bool isu  = pdg::IsUQuark     (hit_quark);
  bool isd  = pdg::IsDQuark     (hit_quark);
  bool iss  = pdg::IsSQuark     (hit_quark);
  bool isub = pdg::IsAntiUQuark (hit_quark);
  bool isdb = pdg::IsAntiDQuark (hit_quark);
  bool issb = pdg::IsAntiSQuark (hit_quark);

  bool allowed = (iscc && isv  && (isd||isub||iss))  ||
    (iscc && isvb && (isu||isdb||issb)) ||
    (isnc && (isv||isvb) && (isu||isd||isub||isdb||iss||issb)) ||
    (isdmi && isdm && (isu||isd||isub||isdb||iss||issb)) ||
    (isem && (isl||islb) && (isu||isd||isub||isdb||iss||issb));
  if(!allowed) {
    LOG("PythiaHad", pWARN)
      << "Impossible interaction type / probe / hit quark combination!";
    return false;
  }

  return true;
}
//____________________________________________________________________________
void PythiaHadronizationBase::Initialize(void)
{
  fLeadingQuark           = 0;
  fRemnantDiquark         = 0;
  fSSBarSuppression       = 0.;
  fGaussianPt2            = 0.;
  fNonGaussianPt2Tail     = 0.;
  fRemainingECutoff       = 0.;
  fDiQuarkSuppression     = 0.;
  fLightVMesonSuppression = 0.;
  fSVMesonSuppression     = 0.;
  fLunda                  = 0.;
  fLundb                  = 0.;
  fLundaDiq               = 0.;
  fOriDecayFlag_pi0       = false;
  fOriDecayFlag_K0        = false;
  fOriDecayFlag_K0b       = false;
  fOriDecayFlag_L0        = false;
  fOriDecayFlag_L0b       = false;
  fOriDecayFlag_Dm        = false;
  fOriDecayFlag_D0        = false;
  fOriDecayFlag_Dp        = false;
  fOriDecayFlag_Dpp       = false;
  fReqDecayFlag_pi0       = false;
  fReqDecayFlag_K0        = false;
  fReqDecayFlag_K0b       = false;
  fReqDecayFlag_L0        = false;
  fReqDecayFlag_L0b       = false;
  fReqDecayFlag_Dm        = false;
  fReqDecayFlag_D0        = false;
  fReqDecayFlag_Dp        = false;
  fReqDecayFlag_Dpp       = false;
}
//____________________________________________________________________________
void PythiaHadronizationBase::LoadConfig(void)
{
  // Get PYTHIA physics configuration parameters specified by GENIE
  this->GetParam( "PYTHIA-SSBarSuppression",       fSSBarSuppression       );
  this->GetParam( "PYTHIA-GaussianPt2",            fGaussianPt2            );
  this->GetParam( "PYTHIA-NonGaussianPt2Tail",     fNonGaussianPt2Tail     );
  this->GetParam( "PYTHIA-RemainingEnergyCutoff",  fRemainingECutoff       );
  this->GetParam( "PYTHIA-DiQuarkSuppression",     fDiQuarkSuppression     );
  this->GetParam( "PYTHIA-LightVMesonSuppression", fLightVMesonSuppression );
  this->GetParam( "PYTHIA-SVMesonSuppression",     fSVMesonSuppression     );
  this->GetParam( "PYTHIA-Lunda",                  fLunda                  );
  this->GetParam( "PYTHIA-Lundb",                  fLundb                  );
  this->GetParam( "PYTHIA-LundaDiq",               fLundaDiq               );

  // Set required PYTHIA decay flags
  fReqDecayFlag_pi0       = false; // don't decay pi0
  fReqDecayFlag_K0        = false; // don't decay K0
  fReqDecayFlag_K0b       = false; // don't decay \bar{K0}
  fReqDecayFlag_L0        = false; // don't decay Lambda0
  fReqDecayFlag_L0b       = false; // don't decay \bar{Lambda0}
  fReqDecayFlag_Dm        = true;  // decay Delta-
  fReqDecayFlag_D0        = true;  // decay Delta0
  fReqDecayFlag_Dp        = true;  // decay Delta+
  fReqDecayFlag_Dpp       = true;  // decay Delta++

  // Load Wcut determining the phase space area where the multiplicity prob.
  // scaling factors would be applied -if requested-
  double Wcut, Wmin ;
  this->GetParam( "Wcut",            Wcut );
  this->GetParam( "KNO2PYTHIA-Wmin", Wmin );

  if ( Wcut > Wmin ) {
    LOG("PythiaHad", pERROR)
       << "Wcut value too high and in conflict with the KNO2PYTHIA-Wmin!"
       << "\n  Wcut = " << Wcut
       << "\n  KNO2PYTHIA-Wmin = " << Wmin;
  }
}
//____________________________________________________________________________