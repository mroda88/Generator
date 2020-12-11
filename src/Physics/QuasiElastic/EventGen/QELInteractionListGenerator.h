//____________________________________________________________________________
/*!

\class    genie::QELInteractionListGenerator

\brief    Concrete implementations of the InteractionListGeneratorI interface.
          Generates a list of all the Interaction (= event summary) objects that
          can be generated by the QEL EventGenerator.

\author   Costas Andreopoulos <constantinos.andreopoulos \at cern.ch>
          University of Liverpool & STFC Rutherford Appleton Laboratory

\created  May 13, 2005

\cpright  Copyright (c) 2003-2020, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org        
*/
//____________________________________________________________________________

#ifndef _QEL_INTERACTION_GENERATOR_H_
#define _QEL_INTERACTION_GENERATOR_H_

#include "Framework/EventGen/InteractionListGeneratorI.h"

namespace genie {

class QELInteractionListGenerator : public InteractionListGeneratorI {

public :
  QELInteractionListGenerator();
  QELInteractionListGenerator(string config);
 ~QELInteractionListGenerator();

  // implement the InteractionListGeneratorI interface
  InteractionList * CreateInteractionList(const InitialState & init) const;

  // overload the Algorithm::Configure() methods to load private data
  // members from configuration options
  void Configure(const Registry & config);
  void Configure(string config);

private:

  void LoadConfigData(void);

  InteractionList * CreateInteractionListCC        (const InitialState & init) const;
  InteractionList * CreateInteractionListNC        (const InitialState & init) const;
  InteractionList * CreateInteractionListEM        (const InitialState & init) const;
  InteractionList * CreateInteractionListCharmCC   (const InitialState & init) const;
  InteractionList * CreateInteractionListStrangeCC (const InitialState & init) const;

  bool fIsCC;
  bool fIsNC;
  bool fIsEM;
  bool fIsCharm;
  bool fIsStrange;
};

}      // genie namespace

#endif // _QEL_INTERACTION_GENERATOR_H_
