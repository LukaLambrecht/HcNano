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

    // count electrons passing selection
    // note: apply loose selection,
    //       make sure it is nowhere potentially tighter
    //       than later downstream selections.
    int nElectrons = 0;
    for( const pat::Electron& electron : *electrons ){
        if( electron.pt() < 5 ) continue;
        if( std::fabs(electron.eta()) > 3 ) continue;
        if( std::fabs(electron.dB(pat::Electron::PV2D)) > 1 ) continue;
        if( std::fabs(electron.dB(pat::Electron::PVDZ)) > 2 ) continue;
        nElectrons++;
    }

    // count muons passing selection
    // note: apply loose selection,
    //       make sure it is nowhere potentially tighter
    //       than later downstream selections.
    int nMuons = 0;
    for( const pat::Muon& muon : *muons ){
        if( muon.pt() < 3 ) continue;
        if( std::fabs(muon.eta()) > 3 ) continue;
        if( std::fabs(muon.dB(pat::Muon::PV2D)) > 1 ) continue;
        if( std::fabs(muon.dB(pat::Muon::PVDZ)) > 2 ) continue;
        nMuons++;
    }

    // do selection on total number of leptons
    int nLeptons = nElectrons + nMuons;
    if( nLeptons < minNLeptons ) return false;

    // default: return true
    return true;
}

// define this as a plug-in
DEFINE_FWK_MODULE(NLeptonSelector);
