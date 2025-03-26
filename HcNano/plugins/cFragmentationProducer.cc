/*
Custom analyzer class for investigating c-quark fragmentation.
*/

// local include files
#include "PhysicsTools/HcNano/interface/cFragmentationProducer.h"


// constructor //
cFragmentationProducer::cFragmentationProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name);
};

// destructor //
cFragmentationProducer::~cFragmentationProducer(){}

// descriptions //
void cFragmentationProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void cFragmentationProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get gen particles
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    if(!genParticles.isValid()){
        std::cout << "WARNING: genParticle collection not valid" << std::endl;
        return;
    }

    // find all gen particles from the hard scattering
    // (implemented here as having a proton as their mother)
    std::vector<const reco::GenParticle*> hardScatterParticles;
    for( const reco::GenParticle& p : *genParticles ){
        if( !p.isLastCopy() ) continue;
        int mompdgid = GenTools::getMotherPdgId(p, *genParticles);
        if( std::abs(mompdgid)!=2212 ) continue;
        hardScatterParticles.push_back(&p);
    }
    if( hardScatterParticles.size() < 1 ) return;

    // find charmed hadrons
    int cFragmentationPdgId = 0;
    int cBarFragmentationPdgId = 0;
    for( const reco::GenParticle* p : hardScatterParticles ){
        int pdgid = p->pdgId();
        if( (std::abs(pdgid) > 400 && std::abs(pdgid) < 500)
            || (std::abs(pdgid) > 4000 && std::abs(pdgid) < 5000) ){
            if(pdgid > 0) cFragmentationPdgId = pdgid;
            else cBarFragmentationPdgId = pdgid;
        }
    }

    // printouts
    /*if( cFragmentationPdgId==0 || cBarFragmentationPdgId==0 ){
        std::cout << "WARNING in cFragmentationAnalyzer:";
        std::cout << " no c-meson and/or cbar-meson found." << std::endl;
        std::cout << "Pdgids of hard scattering particles are:" << std::endl;
        for( const reco::GenParticle* p : hardScatterParticles ){
            int pdgid = p->pdgId();
            std::cout << pdgid << " ";
        }
        std::cout << std::endl;
    }*/

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(1, name, true);
    table->addColumnValue<int>("c", cFragmentationPdgId, "");
    table->addColumnValue<int>("cbar", cBarFragmentationPdgId, "");

    // add the table to the output
    iEvent.put(std::move(table), name);
}

// define this as a plug-in
DEFINE_FWK_MODULE(cFragmentationProducer);
