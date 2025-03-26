/*
Simple analyzer to print some gen particle info.
For testing and debugging, not meant to be used in production.
*/

// local include files
#include "PhysicsTools/HcNano/interface/GenParticlePrinter.h"


// constructor //
GenParticlePrinter::GenParticlePrinter(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    // (none since this analyzer only prints some info,
    // does not produce any output).
    // (note: to test whether this analyzer actually runs,
    // it might be optimized away if it does not produce any output)
};

// destructor //
GenParticlePrinter::~GenParticlePrinter(){}

// descriptions //
void GenParticlePrinter::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void GenParticlePrinter::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get gen particles
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    if(!genParticles.isValid()){
        std::cout << "WARNING: genParticle collection not valid" << std::endl;
        return;
    }

    // print a few relevant gen particles in the event
    for( const reco::GenParticle& p : *genParticles ){
        if(!p.isLastCopy()) continue;
        int pdgid = p.pdgId();
        if(std::abs(pdgid) < 400 || std::abs(pdgid) > 500) continue;
        int mompdgid = GenTools::getMotherPdgId(p, *genParticles);
        std::cout << "Particle " << pdgid << std::endl;
        std::cout << "  kinematics: " << p.pt() << " " << p.eta() << " " << p.phi() << std::endl;
        std::cout << "  mass: " << p.mass() << std::endl;
        std::cout << "  mom pdg id: " << mompdgid << std::endl;
    }
}

// define this as a plug-in
DEFINE_FWK_MODULE(GenParticlePrinter);
