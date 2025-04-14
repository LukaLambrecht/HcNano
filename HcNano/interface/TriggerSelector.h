/*
Custom analyzer class for selecting events passing given triggers.
*/

#ifndef TriggerSelector_H
#define TriggerSelector_H

// system include files
#include <memory>
#include <unordered_map>

// root classes
#include <Math/Vector4D.h>

// general include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Common/interface/TriggerNames.h"

// specific include files
//#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/Common/interface/TriggerResults.h"


class TriggerSelector : public edm::stream::EDFilter<> {
  private:

    // attributes and variables
    std::vector<std::string> triggerNames;
    std::map<std::string, int> triggerIndex;
    bool reIndex = true;

    // help functions
    void makeIndex(const std::vector<std::string>&,
        const edm::TriggerNames&);

    // template member functions
    void beginRun(const edm::Run&, const edm::EventSetup&) override;
    bool filter(edm::Event&, const edm::EventSetup&) override;

    // tokens
    edm::EDGetTokenT<edm::TriggerResults> triggersToken;

  public:
    // constructor, destructor, and other meta-functions
    explicit TriggerSelector(const edm::ParameterSet&);
    ~TriggerSelector() override;
};

#endif
