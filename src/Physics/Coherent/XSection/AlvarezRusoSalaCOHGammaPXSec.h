//____________________________________________________________________________
/*!

  \class    genie::AlvarezRusoSalaCOHGammaPXSec

\brief    Implementation of the Alvarez-Ruso Sala coherent gamma production model
          Is a concrete implementation of the XSecAlgorithmI interface.

\ref

\author   Marco Roda
          University of Liverpool

          Jon Sensenig
          University of Pennsylvania

\cpright  Copyright (c) 2003-2020, The GENIE Collaboration
          For the full text of the license visit http://copyright.genie-mc.org
          or see $GENIE/LICENSE
*/
//____________________________________________________________________________

#ifndef _ALVAREZ_RUSO_SALA_COH_GAMMA_XSEC_H_
#define _ALVAREZ_RUSO_SALA_COH_GAMMA_XSEC_H_

#include <vector>

#include "Framework/EventGen/XSecAlgorithmI.h"
#include "Physics/XSectionIntegration/XSecIntegratorI.h"

#include "Physics/Coherent/XSection/COHHadronicCurrentI.h"
#include "Physics/Coherent/XSection/COHFormFactorI.h"

namespace genie {

class AlvarezRusoSalaCOHGammaPXSec : public XSecAlgorithmI {

public:
  AlvarezRusoSalaCOHGammaPXSec();
  AlvarezRusoSalaCOHGammaPXSec( string config );
  virtual ~AlvarezRusoSalaCOHGammaPXSec();

  //-- XSecAlgorithmI interface implementation

  double XSec            ( const Interaction * i, KinePhaseSpace_t k) const override ;
  double Integral        ( const Interaction * i) const override ;
  bool   ValidProcess    ( const Interaction * i) const override ;


  //-- overload the Algorithm::Configure() methods to load private data
  //   members from configuration options
  void Configure(const Registry & config) override ;
  void Configure(string config) override ;

protected:
  void LoadConfig(void);

  utils::math::GTrace TotalHadronicCurrent ( const Interaction * interaction ) const;
  double NeutrinoHadronContraction ( const Interaction * i ) const ;
  double AntiNeutrinoHadronContraction ( const Interaction * i ) const ; 

private:

  std::vector<const COHHadronicCurrentI *> fCurrents ;
  const COHFormFactorI * fFormFactors ;

  const XSecIntegratorI * fXSecIntegrator;

};

}      // genie namespace

#endif  // _ALVAREZ_RUSO_SALA_COH_GAMMA_XSEC_H_
