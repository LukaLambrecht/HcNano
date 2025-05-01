/*
Custom analyzer class for investigating gen-level Bs meson decays.
*/

// local include files
#include "PhysicsTools/HcNano/interface/BsMesonGenProducer.h"


// constructor //
BsMesonGenProducer::BsMesonGenProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name+"DecayType"); // singleton table of gen-level decay type
    produces<nanoaod::FlatTable>(name); // table of gen-particle kinematics
}

// destructor //
BsMesonGenProducer::~BsMesonGenProducer(){}

// descriptions //
void BsMesonGenProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void BsMesonGenProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get gen particles
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    if(!genParticles.isValid()){
        std::cout << "WARNING: genParticle collection not valid" << std::endl;
        return;
    }

    // find decay type
    int BsGenDecayType = find_Bs_decay_type( *genParticles );

    // make the table
    auto decayTypeTable = std::make_unique<nanoaod::FlatTable>(1, name+"DecayType", true);
    decayTypeTable->addColumnValue<int>("", BsGenDecayType, "");

    // add the table to the output
    iEvent.put(std::move(decayTypeTable), name+"DecayType");

    // find Bs -> Ds X
    std::vector< std::map< std::string, const reco::GenParticle* > > BsGenParticles;
    BsGenParticles = find_Bs_to_Ds( *genParticles );

    // convert to format suitable for flat table
    std::map< std::string, std::vector<float> > variables;
    std::vector<std::string> particleNames = {"Bs", "DStar", "DZero", "Pi1", "K", "Pi2"};
    for( const auto& particleName: particleNames ){
        std::vector<float> pt;
        std::vector<float> eta;
        std::vector<float> phi;
        for(unsigned int idx=0; idx < BsGenParticles.size(); idx++){
            pt.push_back(BsGenParticles[idx].at(particleName)->pt());
            eta.push_back(BsGenParticles[idx].at(particleName)->eta());
            phi.push_back(BsGenParticles[idx].at(particleName)->phi());
        }
        std::string ptName = particleName + "_pt";
        variables[ptName] = pt;
        std::string etaName = particleName + "_eta";
        variables[etaName] = eta;
        std::string phiName = particleName + "_phi";
        variables[phiName] = phi;
    }

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(BsGenParticles.size(), name, false);
    for( const auto& pair : variables ){
        std::string name = pair.first;
        std::vector<float> values = pair.second;
        table->addColumn<float>(name, values, "");
    }

    // add the table to the output
    iEvent.put(std::move(table), name);
}

int BsMesonGenProducer::find_Bs_decay_type(
        const std::vector<reco::GenParticle>& genParticles){
    // find what type of event this is concerning the production and decay of Bs mesons.
    // the numbering convention is as follows:
    // 0: undefined, none of the below.
    //-2: at least one D* -> D0 pi -> K pi pi (i.e. the decay of interest).
    //-1: at least one D* -> D0 pi, but excluding the above.
    // 1: at least 1 D* meson
    // 2: at least one charmed meson in B daughter particles (D0, Ds, D*, D+-)
    // 3: at least one Bs, B0 or B+- (but now being commented out)
    // 4: if there's B meson/baryon in hard scattering
    
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

        // check if it is a bottom meson/hadron
        int pdgid = p->pdgId();
        bool bMeson = (std::abs(pdgid) > 500 && std::abs(pdgid) < 600);
        bool bBaryon = (std::abs(pdgid) > 5000 && std::abs(pdgid) < 6000);
        if( !(bMeson || bBaryon) ) continue;
        if(res > 4) res = 4;

        // check if it is a Bs, B0 or B+- meson
        // now, not requiring it to be any specific B meson, so now the largest res value is 3
        // if( std::abs(pdgid) != 531 && std::abs(pdgid) != 521 && std::abs(pdgid) != 511 ) continue;
        const reco::GenParticle* bs = p;
        if(res > 3) res = 3;

        // find the decay products of the B mesons
        std::vector<const reco::GenParticle*> bsDaughters;
        for(unsigned int i=0; i < bs->numberOfDaughters(); ++i){
            bsDaughters.push_back( &genParticles[bs->daughterRef(i).key()] );
        }
        
        // check if there's at least 1 charmed meson in b daughters particles
        bool hasCharm = false;
        // const reco::GenParticle* bMeson = p;
        for(unsigned int i=0; i < bs->numberOfDaughters(); ++i){
            const auto& daughter = bsDaughters.at(i);
            int absId = std::abs(daughter->pdgId());
            if (absId == 411 || absId == 421 || absId == 431 || absId == 413) {
                hasCharm = true;
                break; // No need to continue if we found one
            }
        }
        if (!hasCharm) continue;
        if(res > 2) res = 2;

        // if there's at least 1 dstar meson
        const reco::GenParticle* dstar;
        bool hasDstar = false;
        for(unsigned int i=0; i < bs->numberOfDaughters(); ++i){
            const auto& daughter = bsDaughters.at(i);
            int absId = std::abs(daughter->pdgId());
            if (absId == 413) {
                hasDstar = true;
                dstar = bsDaughters.at(i);
                break; // No need to continue if we found one
            }
        }
        if (!hasDstar || !dstar) continue;
        if(res > 1) res = 1;

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
        if(res > -1) res = -1;

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
        if(res > -2) res = -2;
        break;
    }
    if(res > 4) res = 0;
    return res;
}

std::vector< std::map< std::string, const reco::GenParticle* > > BsMesonGenProducer::find_Bs_to_Ds(
        const std::vector<reco::GenParticle>& genParticles){
    // find Bs -> Ds X -> phi pi -> K K pi at GEN level

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

        // check if it is a B meson
        int pdgid = p->pdgId();
        if( std::abs(pdgid) != 531 && std::abs(pdgid) != 521 && std::abs(pdgid) != 511 ) continue;
        const reco::GenParticle* bs = p;

        // find its daughters
        std::vector<const reco::GenParticle*> bsDaughters;
        for(unsigned int i=0; i<bs->numberOfDaughters(); ++i){
                bsDaughters.push_back( &genParticles[bs->daughterRef(i).key()] );
        }
    
        // printouts
        //std::cout << "ds daughters:" << std::endl;
        //for( const reco::GenParticle* p: dsDaughters ){ std::cout << p->pdgId() << " "; }
        //std::cout << std::endl;

        //find Dstar
        //if( bsDaughters.size()!=2 ) continue;
        const reco::GenParticle* dstar;
        bool hasDstar = false;
        for(unsigned int i=0; i < bs->numberOfDaughters(); ++i){
            const auto& daughter = bsDaughters.at(i);
            int absId = std::abs(daughter->pdgId());
            if (absId == 413) {
                hasDstar = true;
                dstar = bsDaughters.at(i);
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

        // printouts
        //std::cout << "phi daughters" << std::endl;
        //for( const reco::GenParticle* p: phiDaughters ){ std::cout << p->pdgId() << " "; } 
        //std::cout << std::endl;

        //find the kaon and the pion
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
            {"DStar", dstar},
            {"DZero", dzero},
            {"Bs", bs},
            {"Pi1", pi},
            {"Pi2", pi2},
            {"K", K}
        };
        res.push_back(thisres);
    }
    return res;
}

// define this as a plug-in
DEFINE_FWK_MODULE(BsMesonGenProducer);
