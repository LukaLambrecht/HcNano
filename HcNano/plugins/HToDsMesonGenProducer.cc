/*
Custom analyzer class for investigating gen-level H -> Ds + X meson decays.
*/

// local include files
#include "PhysicsTools/HcNano/interface/HToDsMesonGenProducer.h"


// constructor //
HToDsMesonGenProducer::HToDsMesonGenProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name+"DecayType"); // singleton table of gen-level decay type
    produces<nanoaod::FlatTable>(name); // table of gen-particle kinematics
}

// destructor //
HToDsMesonGenProducer::~HToDsMesonGenProducer(){}

// descriptions //
void HToDsMesonGenProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void HToDsMesonGenProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get gen particles
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    if(!genParticles.isValid()){
        std::cout << "WARNING: genParticle collection not valid" << std::endl;
        return;
    }

    // find decay type
    int HGenDecayType = find_H_decay_type( *genParticles );

    // make the table
    auto decayTypeTable = std::make_unique<nanoaod::FlatTable>(1, name+"DecayType", true);
    decayTypeTable->addColumnValue<int>("", HGenDecayType, "");

    // add the table to the output
    iEvent.put(std::move(decayTypeTable), name+"DecayType");

    // find H -> Ds + X, Ds -> phi pi, phi -> K K
    std::vector< std::map< std::string, const reco::GenParticle* > > HToDsGenParticles;
    HToDsGenParticles = find_H_to_Ds_to_PhiPi_to_KKPi( *genParticles );

    // convert to format suitable for flat table
    std::map< std::string, std::vector<float> > variables;
    std::vector<std::string> particleNames = {"H", "Ds", "Phi", "Pi", "KPlus", "KMinus"};
    for( const auto& particleName: particleNames ){
        std::vector<float> pt;
        std::vector<float> eta;
        std::vector<float> phi;
        for(unsigned int idx=0; idx < HToDsGenParticles.size(); idx++){
            pt.push_back(HToDsGenParticles[idx].at(particleName)->pt());
            eta.push_back(HToDsGenParticles[idx].at(particleName)->eta());
            phi.push_back(HToDsGenParticles[idx].at(particleName)->phi());
        }
        std::string ptName = particleName + "_pt";
        variables[ptName] = pt;
        std::string etaName = particleName + "_eta";
        variables[etaName] = eta;
        std::string phiName = particleName + "_phi";
        variables[phiName] = phi;
    }

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(HToDsGenParticles.size(), name, false);
    for( const auto& pair : variables ){
        std::string name = pair.first;
        std::vector<float> values = pair.second;
        table->addColumn<float>(name, values, "");
    }

    // add the table to the output
    iEvent.put(std::move(table), name);
}

int HToDsMesonGenProducer::find_H_decay_type(
        const std::vector<reco::GenParticle>& genParticles){
    // find what type of event this is concerning the production and decay of H -> Ds.
    // the numbering convention is as follows:
    // 0: undefined, none of the below.
    // 1: at least one H -> Ds + X, Ds -> phi pi, phi -> K K (i.e. the decay of interest).
    // 2: at least one H -> Ds + X, Ds -> phi pi, but excluding the above.
    // 3: at least one H -> Ds + X, but excluding the above.
    // 4: at least one H -> c + cbar, but excluding the above.
    // 5: at least one H, but excluding the above.
    
    // initialize result
    int res = 99;

    // loop over all gen particles
    for( const reco::GenParticle& h : genParticles ){

        // check if it is a H boson
        bool isHBoson = (std::abs(h.pdgId()) == 25 && h.status()==62);
        if( !isHBoson ) continue;
        if(res > 5) res = 5;

        // find the decay products of the H boson
        std::vector<reco::GenParticle> hDaughters;
        for(unsigned int i=0; i < h.numberOfDaughters(); ++i){
            hDaughters.push_back( genParticles[h.daughterRef(i).key()] );
        }

        // check if they are c + cbar
        bool hToCC = (hDaughters.size()==2
                      && std::abs(hDaughters[0].pdgId())==4
                      && std::abs(hDaughters[1].pdgId())==4);
        if( !hToCC ) continue;
        if(res > 4) res = 4;

        // find the decay products of the c + cbar pair
        std::vector<reco::GenParticle> ccbarDaughters;
        ccbarDaughters = GenTools::getQuarkPairDaughters(hDaughters[0], hDaughters[1], genParticles);

        // loop over the decay products of the c + cbar pair
        for( const reco::GenParticle& ds : ccbarDaughters ){
            int pdgid = ds.pdgId();

            // check if it is a Ds meson
            if(std::abs(pdgid) != 431) continue;
            if(res > 3) res = 3;

            // find the decay products of the Ds meson
            std::vector<const reco::GenParticle*> dsDaughters;
            for(unsigned int i=0; i < ds.numberOfDaughters(); ++i){
                dsDaughters.push_back( &genParticles[ds.daughterRef(i).key()] );
            }

            // find if they are a pion and a phi meson
            if( dsDaughters.size()!=2 ) continue;
            const reco::GenParticle* phi;
            if( std::abs(dsDaughters.at(0)->pdgId())==333
                && std::abs(dsDaughters.at(1)->pdgId())==211 ){
                phi = dsDaughters.at(0);
            } else if( std::abs(dsDaughters.at(0)->pdgId())==211
                && std::abs(dsDaughters.at(1)->pdgId())==333 ){
                phi = dsDaughters.at(1);
            } else continue;
            if(res > 2) res = 2;

            // find the daughters of the phi
            std::vector<const reco::GenParticle*> phiDaughters;
            for(unsigned int i=0; i < phi->numberOfDaughters(); ++i){
                phiDaughters.push_back( &genParticles[phi->daughterRef(i).key()] );
            }

            // find if they are kaons
            if( phiDaughters.size()!=2 ) continue;
            if( std::abs(phiDaughters.at(0)->pdgId())==321
                && std::abs(phiDaughters.at(1)->pdgId())==321 ){
                // pass
            } else continue;

            // if all checks above succeeded,
            // we have a genuine H -> Ds + X, Ds -> phi pi, phi -> K K event
            if(res > 1) res = 1;
            break;
        }
    }
    if(res > 5) res = 0;
    return res;
}


std::vector< std::map< std::string, const reco::GenParticle* > > HToDsMesonGenProducer::find_H_to_Ds_to_PhiPi_to_KKPi(
        const std::vector<reco::GenParticle>& genParticles){
    // find H -> Ds + X, Ds -> phi pi, phi -> K K at GEN level

    // initialize output
    std::vector< std::map< std::string, const reco::GenParticle* > > res;

    // loop over all gen particles
    for( const reco::GenParticle& h : genParticles ){

        // check if it is a H boson
        bool isHBoson = (std::abs(h.pdgId()) == 25 && h.status()==62);
        if( !isHBoson ) continue;

        // find the decay products of the H boson
        std::vector<reco::GenParticle> hDaughters;
        for(unsigned int i=0; i < h.numberOfDaughters(); ++i){
            hDaughters.push_back( genParticles[h.daughterRef(i).key()] );
        }

        // check if they are c + cbar
        bool hToCC = (hDaughters.size()==2
                      && std::abs(hDaughters[0].pdgId())==4
                      && std::abs(hDaughters[1].pdgId())==4);
        if( !hToCC ) continue;

        // find the decay products of the c + cbar pair
        std::vector<reco::GenParticle> ccbarDaughters;
        ccbarDaughters = GenTools::getQuarkPairDaughters(hDaughters[0], hDaughters[1], genParticles);

        // loop over the decay products of the c + cbar pair
        for( const reco::GenParticle& ds : ccbarDaughters ){
            int pdgid = ds.pdgId();

            // check if it is a Ds meson
            if(std::abs(pdgid) != 431) continue;

            // find its daughters
            std::vector<const reco::GenParticle*> dsDaughters;
            for(unsigned int i=0; i<ds.numberOfDaughters(); ++i){
                dsDaughters.push_back( &genParticles[ds.daughterRef(i).key()] );
            }

            // find the pion and phi
            if( dsDaughters.size()!=2 ) continue;
            const reco::GenParticle* pi;
            const reco::GenParticle* phi;
            if( std::abs(dsDaughters.at(0)->pdgId())==333
                && std::abs(dsDaughters.at(1)->pdgId())==211 ){
                phi = dsDaughters.at(0);
                pi = dsDaughters.at(1);
            } else if( std::abs(dsDaughters.at(0)->pdgId())==211
                && std::abs(dsDaughters.at(1)->pdgId())==333 ){
                phi = dsDaughters.at(1);
                pi = dsDaughters.at(0);
            } else continue;

            // find the daughters of the phi
            std::vector<const reco::GenParticle*> phiDaughters;
            for(unsigned int i=0; i<phi->numberOfDaughters(); ++i){
                phiDaughters.push_back( &genParticles[phi->daughterRef(i).key()] );
            }

            // find the kaons
            const reco::GenParticle* K1;
            const reco::GenParticle* K2;
            const reco::GenParticle* KPlus;
            const reco::GenParticle* KMinus;
            if( phiDaughters.size()!=2 ) continue;
            if( std::abs(phiDaughters.at(0)->pdgId())==321
                && std::abs(phiDaughters.at(1)->pdgId())==321 ){
                K1 = phiDaughters.at(0);
                K2 = phiDaughters.at(1);
            } else continue;

            // find which one is the positive and which one the negative
            if( K1->charge() > 0 && K2->charge() < 0 ){
                KPlus = K1; 
                KMinus = K2;
            }
            else{
                KPlus = K2;
                KMinus = K1;
            }

            // set the particles in the output map
            std::map< std::string, const reco::GenParticle* > thisres = {
                {"H", &h},
                {"Ds", &ds},
                {"Phi", phi},
                {"Pi", pi},
                {"KPlus", KPlus},
                {"KMinus", KMinus}
            };
            res.push_back(thisres);
        }
    }
    return res;
}

// define this as a plug-in
DEFINE_FWK_MODULE(HToDsMesonGenProducer);
