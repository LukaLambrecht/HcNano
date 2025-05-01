/*
Custom analyzer class for investigating gen-level b-hadron to D* decays.
*/

// local include files
#include "PhysicsTools/HcNano/interface/BToDStarMesonGenProducer.h"


// constructor //
BToDStarMesonGenProducer::BToDStarMesonGenProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name+"DecayType"); // singleton table of gen-level decay type
    produces<nanoaod::FlatTable>(name); // table of gen-particle kinematics
}

// destructor //
BToDStarMesonGenProducer::~BToDStarMesonGenProducer(){}

// descriptions //
void BToDStarMesonGenProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void BToDStarMesonGenProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get gen particles
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    if(!genParticles.isValid()){
        std::cout << "WARNING: genParticle collection not valid" << std::endl;
        return;
    }

    // find decay type
    int BsGenDecayType = find_B_decay_type( *genParticles );

    // make the table
    auto decayTypeTable = std::make_unique<nanoaod::FlatTable>(1, name+"DecayType", true);
    decayTypeTable->addColumnValue<int>("", BsGenDecayType, "");

    // add the table to the output
    iEvent.put(std::move(decayTypeTable), name+"DecayType");

    // find B -> D* X, D* -> D0 pi, D0 -> K pi
    std::vector< std::map< std::string, const reco::GenParticle* > > BGenParticles;
    BGenParticles = find_B_to_DStar( *genParticles );

    // convert to format suitable for flat table
    std::map< std::string, std::vector<float> > variables;
    std::vector<std::string> particleNames = {"BHadron", "DStar", "DZero", "Pi1", "K", "Pi2"};
    for( const auto& particleName: particleNames ){
        std::vector<float> pt;
        std::vector<float> eta;
        std::vector<float> phi;
        for(unsigned int idx=0; idx < BGenParticles.size(); idx++){
            pt.push_back(BGenParticles[idx].at(particleName)->pt());
            eta.push_back(BGenParticles[idx].at(particleName)->eta());
            phi.push_back(BGenParticles[idx].at(particleName)->phi());
        }
        std::string ptName = particleName + "_pt";
        variables[ptName] = pt;
        std::string etaName = particleName + "_eta";
        variables[etaName] = eta;
        std::string phiName = particleName + "_phi";
        variables[phiName] = phi;
    }

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(BGenParticles.size(), name, false);
    for( const auto& pair : variables ){
        std::string name = pair.first;
        std::vector<float> values = pair.second;
        table->addColumn<float>(name, values, "");
    }

    // add the table to the output
    iEvent.put(std::move(table), name);
}

int BToDStarMesonGenProducer::find_B_decay_type(
        const std::vector<reco::GenParticle>& genParticles){
    // find what type of event this is.
    // the numbering convention is as follows:
    // 0: undefined, none of the below.
    // 1: at least one b-hadron -> D* X, D* -> D0 pi, D0 -> K pi (i.e. the decay of interest).
    // 2: at least one b-hadron -> D* X, D* -> D0 pi, D0 -> other than above.
    // 3: at least one b-hadron -> D* X, D* -> other than above.
    // 4: at least one b-hadron -> c-meson (D0, Ds, D*, D+-), but excluding the above.
    // 5: at least one b-hadron in hard scattering, but excluding the above.
    
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

        // check if it is a b-hadron
        int pdgid = p->pdgId();
        bool bMeson = (std::abs(pdgid) > 500 && std::abs(pdgid) < 600);
        bool bBaryon = (std::abs(pdgid) > 5000 && std::abs(pdgid) < 6000);
        if( !(bMeson || bBaryon) ) continue;
        if(res > 5) res = 5;
        const reco::GenParticle* bHadron = p;

        // find the decay products of the b hadron
        std::vector<const reco::GenParticle*> bHadronDaughters;
        for(unsigned int i=0; i < bHadron->numberOfDaughters(); ++i){
            bHadronDaughters.push_back( &genParticles[bHadron->daughterRef(i).key()] );
        }
        
        // check if there is at least 1 charmed meson in b-hadron daughters particles
        bool hasCharm = false;
        for(const reco::GenParticle* daughter: bHadronDaughters){
            int absId = std::abs(daughter->pdgId());
            if( absId == 411 || absId == 421 || absId == 431 || absId == 413 ){
                hasCharm = true;
                break; // No need to continue if we found one
            }
        }
        if (!hasCharm) continue;
        if(res > 4) res = 4;

        // check if there is at least 1 D* meson
        const reco::GenParticle* dstar;
        bool hasDstar = false;
        for(const reco::GenParticle* daughter: bHadronDaughters){
            int absId = std::abs(daughter->pdgId());
            if (absId == 413) {
                hasDstar = true;
                dstar = daughter;
                break; // No need to continue if we found one
            }
        }
        if (!hasDstar || !dstar) continue;
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
        // we have a genuine b-hadron -> D* X, D* -> D0 pi, D0 -> K pi event
        if(res > 1) res = 1;
        break;
    }
    if(res > 5) res = 0;
    return res;
}


std::vector< std::map< std::string, const reco::GenParticle* > > BToDStarMesonGenProducer::find_B_to_DStar(
        const std::vector<reco::GenParticle>& genParticles){
    // find b-hadron -> Ds X, Ds -> D0 pi, D0 -> K pi at GEN level

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

        // check if it is a b-hadron
        int pdgid = p->pdgId();
        bool bMeson = (std::abs(pdgid) > 500 && std::abs(pdgid) < 600);
        bool bBaryon = (std::abs(pdgid) > 5000 && std::abs(pdgid) < 6000);
        if( !(bMeson || bBaryon) ) continue;
        const reco::GenParticle* bHadron = p;

        // find the decay products of the b hadron
        std::vector<const reco::GenParticle*> bHadronDaughters;
        for(unsigned int i=0; i < bHadron->numberOfDaughters(); ++i){
            bHadronDaughters.push_back( &genParticles[bHadron->daughterRef(i).key()] );
        }

        // find the Dstar
        const reco::GenParticle* dstar;
        bool hasDstar = false;
        for(const reco::GenParticle* daughter: bHadronDaughters){
            int absId = std::abs(daughter->pdgId());
            if (absId == 413) {
                hasDstar = true;
                dstar = daughter;
                break; // No need to continue if we found one
            }
        }
        if (!hasDstar || !dstar) continue;

        // find the daughters of the Dstar
        std::vector<const reco::GenParticle*> dstarDaughters;
        for(unsigned int i=0; i < dstar->numberOfDaughters(); ++i){
            dstarDaughters.push_back( &genParticles[dstar->daughterRef(i).key()] );
        }

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

        // find the daughters of the D0
        std::vector<const reco::GenParticle*> dzeroDaughters;
        for(unsigned int i=0; i<dzero->numberOfDaughters(); ++i){
            dzeroDaughters.push_back( &genParticles[dzero->daughterRef(i).key()] );
        }

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

        // set the particles in the output map
        std::map< std::string, const reco::GenParticle* > thisres = {
            {"BHadron", bHadron},
            {"DStar", dstar},
            {"DZero", dzero},
            {"Pi1", pi},
            {"Pi2", pi2},
            {"K", K}
        };
        res.push_back(thisres);
    }
    return res;
}

// define this as a plug-in
DEFINE_FWK_MODULE(BToDStarMesonGenProducer);
