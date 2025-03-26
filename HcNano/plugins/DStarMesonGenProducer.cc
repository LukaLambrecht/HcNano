/*
Custom analyzer class for investigating gen-level D* meson decays.
*/

// local include files
#include "PhysicsTools/HcNano/interface/DStarMesonGenProducer.h"


// constructor //
DStarMesonGenProducer::DStarMesonGenProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name+"DecayType"); // singleton table of gen-level decay type
    produces<nanoaod::FlatTable>(name); // table of gen-particle kinematics
}

// destructor //
DStarMesonGenProducer::~DStarMesonGenProducer(){}

// descriptions //
void DStarMesonGenProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void DStarMesonGenProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get gen particles
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    if(!genParticles.isValid()){
        std::cout << "WARNING: genParticle collection not valid" << std::endl;
        return;
    }

    // find decay type
    int DStarGenDecayType = find_DStar_decay_type( *genParticles );

    // make the table
    auto decayTypeTable = std::make_unique<nanoaod::FlatTable>(1, name+"DecayType", true);
    decayTypeTable->addColumnValue<int>("", DStarGenDecayType, "");

    // add the table to the output
    iEvent.put(std::move(decayTypeTable), name+"DecayType");

    // find D* -> pi D0 -> pi K pi
    std::vector< std::map< std::string, const reco::GenParticle* > > DStarGenParticles;
    DStarGenParticles = find_DStar_to_DZeroPi_to_KPiPi( *genParticles );

    // convert to format suitable for flat table
    std::map< std::string, std::vector<float> > variables;
    std::vector<std::string> particleNames = {"DStar", "DZero", "Pi1", "K", "Pi2"};
    for( const auto& particleName: particleNames ){
        std::vector<float> pt;
        std::vector<float> eta;
        std::vector<float> phi;
        for(unsigned int idx=0; idx < DStarGenParticles.size(); idx++){
            pt.push_back(DStarGenParticles[idx].at(particleName)->pt());
            eta.push_back(DStarGenParticles[idx].at(particleName)->eta());
            phi.push_back(DStarGenParticles[idx].at(particleName)->phi());
        }
        std::string ptName = particleName + "_pt";
        variables[ptName] = pt;
        std::string etaName = particleName + "_eta";
        variables[etaName] = eta;
        std::string phiName = particleName + "_phi";
        variables[phiName] = phi;
    }
    
    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(DStarGenParticles.size(), name, false);
    for( const auto& pair : variables ){
        std::string name = pair.first;
        std::vector<float> values = pair.second;
        table->addColumn<float>(name, values, "");
    }

    // add the table to the output
    iEvent.put(std::move(table), name);
}

int DStarMesonGenProducer::find_DStar_decay_type(
        const std::vector<reco::GenParticle>& genParticles){
    // find what type of event this is concerning the production and decay of D* mesons.
    // the numbering convention is as follows:
    // 0: undefined, none of the below.
    // 1: at least one D* -> D0 pi -> K pi pi (i.e. the decay of interest).
    // 2: at least one D* -> D0 pi, but excluding the above.
    // 3: at least one D*, but excluding the above.
    // 4: at least one charmed hadron, but excluding the above.
    
    // find all gen particles from the hard scattering
    // (implemented here as having a proton as their mother)
    std::vector<const reco::GenParticle*> hardScatterParticles;
    for( const reco::GenParticle& p : genParticles ){
        if( !p.isLastCopy() ) continue;
        int mompdgid = GenTools::getMotherPdgId(p, genParticles);
        if( std::abs(mompdgid)!=2212 ) continue;
        hardScatterParticles.push_back(&p);
    }
    if( hardScatterParticles.size() < 1 ) return 0;

    // initialize result
    int res = 99;

    // loop over hard scattering particles
    for( const reco::GenParticle* p : hardScatterParticles ){

        // check if it is a charmed hadron
        int pdgid = p->pdgId();
        bool cMeson = (std::abs(pdgid) > 400 && std::abs(pdgid) < 500);
        bool cBaryon = (std::abs(pdgid) > 4000 && std::abs(pdgid) < 5000);
        if( !(cMeson || cBaryon) ) continue;
        if(res > 4) res = 4;

        // check if it is a D* meson
        if(std::abs(pdgid) != 413) continue;
        const reco::GenParticle* dstar = p;
        if(res > 3) res = 3;

        // find the decay products of the D* meson
        std::vector<const reco::GenParticle*> dstarDaughters;
        for(unsigned int i=0; i < dstar->numberOfDaughters(); ++i){
            dstarDaughters.push_back( &genParticles[dstar->daughterRef(i).key()] );
        }

        // find if they are a D0 meson and a pion
        if( dstarDaughters.size()!=2 ) continue;
        const reco::GenParticle* dzero;
        if( std::abs(dstarDaughters.at(0)->pdgId())==421
            && std::abs(dstarDaughters.at(1)->pdgId())==211 ){
            dzero = dstarDaughters.at(0);
        } else if( std::abs(dstarDaughters.at(0)->pdgId())==211
            && std::abs(dstarDaughters.at(1)->pdgId())==421 ){
            dzero = dstarDaughters.at(1);
        } else continue;
        if(res > 2) res = 2;

        // find the daughters of the D0
        std::vector<const reco::GenParticle*> dzeroDaughters;
        for(unsigned int i=0; i < dzero->numberOfDaughters(); ++i){
            dzeroDaughters.push_back( &genParticles[dzero->daughterRef(i).key()] );
        }

        // find if they are a kaon and a pion
        if( dzeroDaughters.size()!=2 ) continue;
        if( std::abs(dzeroDaughters.at(0)->pdgId())==321
            && std::abs(dzeroDaughters.at(1)->pdgId())==211 ){
            // pass
        } else if( std::abs(dzeroDaughters.at(0)->pdgId())==211
            && std::abs(dzeroDaughters.at(1)->pdgId())==321 ){
            // pass
        } else continue;

        // if all checks above succeeded,
        // we have a genuine D* -> D0 pi -> K pi pi event
        if(res > 1) res = 1;
        break;
    }
    if(res > 4) res = 0;
    return res;
}


std::vector< std::map< std::string, const reco::GenParticle* > > DStarMesonGenProducer::find_DStar_to_DZeroPi_to_KPiPi(
        const std::vector<reco::GenParticle>& genParticles){
    // find D* -> D0 pi -> K pi pi at GEN level

    // initialize output
    std::vector< std::map< std::string, const reco::GenParticle* > > res;

    // find all gen particles from the hard scattering
    // (implemented here as having a proton as their mother)
    std::vector<const reco::GenParticle*> hardScatterParticles;
    for( const reco::GenParticle& p : genParticles ){
        if( !p.isLastCopy() ) continue;
        int mompdgid = GenTools::getMotherPdgId(p, genParticles);
        if( std::abs(mompdgid)!=2212 ) continue;
        hardScatterParticles.push_back(&p);
    }
    if( hardScatterParticles.size() < 1 ) return res;

    // loop over all hard scattering particles
    for( const reco::GenParticle* p : hardScatterParticles ){
    
        // check if it is a D* meson
        int pdgid = p->pdgId();
        if(std::abs(pdgid) != 413) continue;
        const reco::GenParticle* dstar = p;

        // find its daughters
        std::vector<const reco::GenParticle*> dstarDaughters;
        for(unsigned int i=0; i<dstar->numberOfDaughters(); ++i){
            dstarDaughters.push_back( &genParticles[dstar->daughterRef(i).key()] );
        }

        // printouts
        //std::cout << "D* daughters:" << std::endl;
        //for( const reco::GenParticle* p: dstarDaughters ){ std::cout << p->pdgId() << " "; }
        //std::cout << std::endl;

        // find the D0 meson and the pion
        if( dstarDaughters.size()!=2 ) continue;
        const reco::GenParticle* dzero;
        const reco::GenParticle* pi;
        if( std::abs(dstarDaughters.at(0)->pdgId())==421
            && std::abs(dstarDaughters.at(1)->pdgId())==211 ){
            dzero = dstarDaughters.at(0);
            pi = dstarDaughters.at(1);
        } else if( std::abs(dstarDaughters.at(0)->pdgId())==211
            && std::abs(dstarDaughters.at(1)->pdgId())==421 ){
            dzero = dstarDaughters.at(1);
            pi = dstarDaughters.at(0);
        } else continue;

        // printouts
        //std::cout << "  -> found D* -> D0 + pi" << std::endl;

        // find the daughters of the D0
        std::vector<const reco::GenParticle*> dzeroDaughters;
        for(unsigned int i=0; i<dzero->numberOfDaughters(); ++i){
            dzeroDaughters.push_back( &genParticles[dzero->daughterRef(i).key()] );
        }

        // printouts
        //std::cout << "D0 daughters" << std::endl;
        //for( const reco::GenParticle* p: dzeroDaughters ){ std::cout << p->pdgId() << " "; } 
        //std::cout << std::endl;

        // find the kaon and the pion
        const reco::GenParticle* K;
        const reco::GenParticle* pi2;
        if( dzeroDaughters.size()!=2 ) continue;
        if( std::abs(dzeroDaughters.at(0)->pdgId())==321
            && std::abs(dzeroDaughters.at(1)->pdgId())==211 ){
            K = dzeroDaughters.at(0);
            pi2 = dzeroDaughters.at(1);
        } else if( std::abs(dzeroDaughters.at(0)->pdgId())==211
            && std::abs(dzeroDaughters.at(1)->pdgId())==321 ){
            K = dzeroDaughters.at(1);
            pi2 = dzeroDaughters.at(0);
        } else continue;

        // print kinematics
        /*std::cout << "D* kinematics:" << std::endl;
        std::cout << dstar.pt() << " " << dstar.eta() << " " << dstar.phi() << std::endl;
        std::cout << "pion kinematics:" << std::endl;
        std::cout << pi->pt() << " " << pi->eta() << " " << pi->phi() << std::endl;
        std::cout << "D0 kinematics:" << std::endl;
        std::cout << dzero->pt() << " " << dzero->eta() << " " << dzero->phi() << std::endl;
        std::cout << "kaon kinematics:" << std::endl;
        std::cout << K->pt() << " " << K->eta() << " " << K->phi() << std::endl;
        std::cout << "pion2 kinematics:" << std::endl;
        std::cout << pi2->pt() << " " << pi2->eta() << " " << pi2->phi() << std::endl;*/

        // set the particles in the output map
        std::map< std::string, const reco::GenParticle* > thisres = {
            {"DStar", dstar},
            {"DZero", dzero},
            {"Pi1", pi},
            {"K", K},
            {"Pi2", pi2}
        };
        res.push_back(thisres);
    }
    return res;
}

// define this as a plug-in
DEFINE_FWK_MODULE(DStarMesonGenProducer);
