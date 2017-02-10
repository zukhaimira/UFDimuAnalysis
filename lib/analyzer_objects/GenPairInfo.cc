
#include "GenPairInfo.h"

void GenPairInfo::init() {

  iMu1      = -999;
  iMu2      = -999;
  mother_ID = -999;
  postFSR   = -999;

  mass   = -999;
  pt     = -999;
  eta    = -999;
  y      = -999;
  phi    = -999;
  angle  = -999;
  
} // End void GenPairInfo::init()
///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

Float_t GenPairInfo::getMass()
{
    return mass;
}

///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

TLorentzVector GenPairInfo::get4vec()
{
    TLorentzVector v;
    v.SetPtEtaPhiM(pt, eta, phi, getMass());
    return v;
}

///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

TString GenPairInfo::outputInfo()
{
    TString s = Form("pt: %7.3f, eta: %7.3f, phi: %7.3f, mass: %7.3f", 
                      pt, eta, phi, mass);
    return s;
}

///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

Double_t GenPairInfo::iso()
{
    return 0.0;
}
