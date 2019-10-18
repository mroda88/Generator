//____________________________________________________________________________
/*!

\class    genie::Pythia8Hadronization

\brief    Provides access to the PYTHIA hadronization models. \n
          Is a concrete implementation of the EventRecordVisitorI interface.

\author   Shivesh Mandalia <s.p.mandalia@qmul.ac.uk>
          Queen Mary University of London

          Costas Andreopoulos <costas.andreopoulos \at stfc.ac.uk>
          University of Liverpool & STFC Rutherford Appleton Lab

\created  October 17, 2019

\cpright  Copyright (c) 2003-2019, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _PYTHIA8_HADRONIZATION_H_
#define _PYTHIA8_HADRONIZATION_H_

#include "Framework/Conventions/GBuild.h"
#include "Framework/Interaction/Interaction.h"
#include "Physics/Hadronization/PythiaHadronizationBase.h"

#ifdef __GENIE_PYTHIA8_ENABLED__
#include "Pythia8/Pythia.h"
#endif

namespace genie {

class GHepParticle;

class Pythia8Hadronization : public PythiaHadronizationBase {

public:
  Pythia8Hadronization();
  Pythia8Hadronization(string config);
  virtual ~Pythia8Hadronization();

  // Implement the EventRecordVisitorI interface
  void ProcessEventRecord(GHepRecord * event) const;

  // Overload the Algorithm::Configure() methods to load private data
  // members from configuration options
  void Configure(const Registry & config);
  void Configure(string config);

private:
  TClonesArray * Hadronize (const Interaction*) const;

  void CopyOriginalDecayFlags     (void) const;
  void SetDesiredDecayFlags       (void) const;
  void RestoreOriginalDecayFlags  (void) const;

  void LoadConfig (void);
  void Initialize (void);

#ifdef __GENIE_PYTHIA8_ENABLED__
  mutable Pythia8::Pythia * fPythia; ///< PYTHIA8 instance
#endif

};

}         // genie namespace

#endif    // _PYTHIA8_HADRONIZATION_H_