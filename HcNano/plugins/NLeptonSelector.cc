/*
Custom analyzer class for selecting events with a given number of leptons.
*/


// local include files
#include "PhysicsTools/HcNano/interface/NLeptonSelector.h"

// constructor //
NLeptonSelector::NLeptonSelector(const edm::ParameterSet& iConfig)
  : minNLeptons(iConfig.getParameter<int>("minNLeptons")),
    electronsToken(consumes<std::vector<pat::Electron>>(
        iConfig.getParameter<edm::InputTag>("electronsToken"))),
    muonsToken(consumes<std::vector<pat::Muon>>(
        iConfig.getParameter<edm::InputTag>("muonsToken"))){
}

// destructor //
NLeptonSelector::~NLeptonSelector(){}

// filter (main method) //
bool NLeptonSelector::filter(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get all required objects from tokens
    edm::Handle<std::vector<pat::Electron>> electrons;
    iEvent.getByToken(electronsToken, electrons);
    edm::Handle<std::vector<pat::Muon>> muons;
    iEvent.getByToken(muonsToken, muons);

    // do selection
    int nElectrons = (*electrons).size();
    int nMuons = (*muons).size();
    int nLeptons = nElectrons + nMuons;
    if( nLeptons < minNLeptons ) return false;

    // default: return true
    return true;
}

// define this as a plug-in
DEFINE_FWK_MODULE(NLeptonSelector);
