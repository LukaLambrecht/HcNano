/*
Custom analyzer class for selecting events passing given triggers.
*/


// local include files
#include "PhysicsTools/HcNano/interface/TriggerSelector.h"

// constructor //
TriggerSelector::TriggerSelector(const edm::ParameterSet& iConfig)
  : triggerNames(iConfig.getParameter<std::vector<std::string>>("triggerNames")),
    triggersToken(consumes<edm::TriggerResults>(
        iConfig.getParameter<edm::InputTag>("triggersToken"))){
}

// destructor //
TriggerSelector::~TriggerSelector(){}

// begin new run
void TriggerSelector::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup){
    // set reIndex to true, as the names and ordering of triggers can change between runs
    reIndex = true;
}

// filter (main method) //
bool TriggerSelector::filter(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get all required objects from tokens
    edm::Handle<edm::TriggerResults> triggerResults;
    iEvent.getByToken(triggersToken, triggerResults);
    // keep event if trigger results are not valid,
    // so an appropriate handling can be done more downstream.
    if( !triggerResults.isValid() ) return true;
    if( triggerResults.failedToGet() ) return true;

    // re-index if needed
    if( reIndex ){
        const edm::TriggerNames& availableTriggers = iEvent.triggerNames(*triggerResults);
        makeIndex(triggerNames, availableTriggers);
        reIndex = false;
    }
    
    // selection
    for( const std::string& triggerName: triggerNames ){
        int idx = triggerIndex[triggerName];
        if( idx < 0 || static_cast<unsigned>(idx) >= triggerResults->size() ){
            std::cout << "WARNING in TriggerSelector::filter:";
            std::cout << " trigger " << triggerName << " not found in index." << std::endl;
            // return true in this case so this selection can be recovered downstream.
            return true;
        }
        if( triggerResults->accept(idx) ) return true;
    }

    // default: return false
    return false;
}

// helper functions
void TriggerSelector::makeIndex(
        const std::vector<std::string>& triggerNames,
        const edm::TriggerNames& availableTriggers){
    std::cout << "INFO: re-indexing triggers" << std::endl;
    // initialize the new index
    triggerIndex.clear();
    for(const std::string& triggerName: triggerNames) triggerIndex[triggerName] = -1;
    // find correct index for all requested triggers
    for( const std::string& triggerName: triggerNames ){
        for( unsigned int i = 0; i < availableTriggers.size(); i++){
            const std::string& fullName = availableTriggers.triggerName(i);
            if( fullName.rfind(triggerName+"_v", 0)==0 ){
                triggerIndex[triggerName] = i;
                break;
            }
        }
        if( triggerIndex[triggerName]==-1 ){
            std::cout << "WARNING in TriggerSelector::makeIndex:";
            std::cout << " trigger " << triggerName << " not found";
            std::cout << " in available triggers." << std::endl;
        }
    }
    std::cout << "INFO: done re-indexing triggers." << std::endl;
}

// define this as a plug-in
DEFINE_FWK_MODULE(TriggerSelector);
