//____________________________________________________________________________
/*!

\class    genie::COHHadronicCurrentI

\brief    Interface for COH Hadronic Current for COH NC Gamma production

\author   Marco Roda <mroda@liverpool.ac.uk>
          University of Liverpool

\created  November 2019

\cpright  Copyright (c) 2003-2019, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _COH_HADRONIC_CURRENT_I_H_
#define _COH_HADRONIC_CURRENT_I_H_

#include "Framework/Algorithm/Algorithm.h"
#include "Framework/ParticleData/BaryonResonance.h"
#include "Framework/Interaction/Interaction.h" 
#include "Framework/Numerical/GTrace.h"

#include "Physics/Coherent/XSection/COHFormFactorI.h"

#include <complex>

namespace genie {

class COHHadronicCurrentI : public Algorithm {

public:

  virtual ~COHHadronicCurrentI() { ; } 

  virtual GTrace R( const Interaction * i, const COHFormFactorI * ff ) const = 0 ;

  virtual Resonance_t Resonance() const { return kNoResonance ; } 

 protected:

  COHHadronicCurrentI( string name );
  COHHadronicCurrentI( string name, string config );


private:

  COHHadronicCurrentI() {;}

};

}       // genie namespace
#endif  // #ifndef _COH_HADRONIC_CURRENT_I_H_
