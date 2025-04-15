/*
Custom analyzer class for selecting events with a given number of leptons.
*/

#ifndef NLeptonSelector_H
#define NLeptonSelector_H

// system include files
#include <memory>
#include <unordered_map>
#include <algorithm>

// root classes
#include <Math/Vector4D.h>

// general include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// specific include files
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"


class NLeptonSelector : public edm::stream::EDFilter<> {
  private:

    // attributes and variables
    const int minNLeptons;

    // template member functions
    bool filter(edm::Event&, const edm::EventSetup&) override;

    // tokens
    edm::EDGetTokenT<std::vector<pat::Electron>> electronsToken;
    edm::EDGetTokenT<std::vector<pat::Muon>> muonsToken;

  public:
    // constructor, destructor, and other meta-functions
    explicit NLeptonSelector(const edm::ParameterSet&);
    ~NLeptonSelector() override;
};

#endif
