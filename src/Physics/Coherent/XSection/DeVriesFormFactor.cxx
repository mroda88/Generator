//____________________________________________________________________________
/*
 Copyright (c) 2003-2019, The GENIE Collaboration
 For the full text of the license visit http://copyright.genie-mc.org
 or see $GENIE/LICENSE

 Author:

 For the class documentation see the corresponding header file.

 Important revisions after version 2.0.0 :
 @ Feb 09, 2009 - CA
   Moved into the new Coherent package from its previous location  (Elastic
   package)

*/
//____________________________________________________________________________

#include <TMath.h>

#include "Physics/Coherent/XSection/DeVriesFormFactor.h"
#include "Framework/Messenger/Messenger.h"

#include "Framework/Conventions/Constants.h"
#include "Framework/Conventions/Units.h"


using namespace genie;
using namespace genie::utils;

//____________________________________________________________________________
DeVriesFormFactor::DeVriesFormFactor() :
XSecAlgorithmI("genie::DeVriesFormFactor")
{

}
//____________________________________________________________________________
DeVriesFormFactor::DeVriesFormFactor(string config) :
XSecAlgorithmI("genie::DeVriesFormFactor", config)
{

}
//____________________________________________________________________________
DeVriesFormFactor::~DeVriesFormFactor()
{

}
//____________________________________________________________________________
double DeVriesFormFactor::FormFacotor( double Q ) const {

  double qr = Q * fRadius ;

  double aux_sum = 0.0, nu ;

  for (unsigned int i = 0 ; i < fFBCs.size() ; ++i ) {
     nu = i + 1. ;
     double pi_x_i = constants::kPi*nu ;
     aux_sum += pow( -1.0, i )*a[i]/( ( pi_x_i + qr )*( pi_x_i - qr ) ) ;
 }

 return 4.*constants::kPi*pow( R/units::fm, 3)*aux_sum*(sin(qr)/(qr) ) ;

}
//____________________________________________________________________________
void DeVriesFormFactor::Configure(const Registry & config)
{
  Algorithm::Configure(config);
  this->LoadConfig();
}
//____________________________________________________________________________
void DeVriesFormFactor::Configure(string config)
{
  Algorithm::Configure(config);
  this->LoadConfig();
}
//____________________________________________________________________________
void DeVriesFormFactor::LoadConfig(void)
{

  std::string raw ;
  std::vector<std::string> bits ;

  // load R33 parameters
  this -> GetParam( "DV-Coefficients", raw ) ;
  bits = utils::str::Split( raw, ";" ) ;

  if ( ! utils::str::Convert(bits, fFBCs) ) {
    LOG("DeVriesFormFactor", pFATAL) << "Failed to decode coeffictient string for nucleus " << Id().Config() ;
    LOG("DeVriesFormFactor", pFATAL) << "String " << raw ;
    invalid_configuration = true ;
  }

  GetParam( "DV-Radius", kRadius ) ;
  fRadius *= units::fm ;

  GetParam( "DV-Nucleus", fPDG ) ;

  LOG("DeVriesFormFactor", pINFO) << "Loaded " << fFBCs.size() << " coeffictients for nucleus " << fPDG ; 

}
//____________________________________________________________________________
