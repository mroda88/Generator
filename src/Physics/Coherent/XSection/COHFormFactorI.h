//____________________________________________________________________________
/*!

\class    genie::COHFormFactorI

\brief    Interface for COH Production model form factor

\author   Marco Roda <mroda@liverpool.ac.uk>
          University of Liverpool

\created  November 2019

\cpright  Copyright (c) 2003-2019, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _COH_FORM_FACTOR_I_H_
#define _COH_FORM_FACTOR_I_H_ 

#include "Framework/Algorithm/Algorithm.h"

namespace genie {

class COHFormFactorI : public Algorithm {

public:

  virtual ~COHFormFactorI() { ; } 

  virtual double ProtonFF ( double Q, int pdg ) const = 0 ;
  virtual double NeutronFF( double Q, int pdg ) const = 0 ;

  virtual bool HasNucleus( int pdg ) const = 0 ;

 protected:

  COHFormFactorI( string name );
  COHFormFactorI( string name, string config );


private:

  COHFormFactorI() {;}

};

}       // genie namespace
#endif  // #ifndef _COH_FORM_FACTOR_I_H_

