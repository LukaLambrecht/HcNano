/*
Custom analyzer class for investigating gen-level H -> D* + X decays.
*/

// local include files
#include "PhysicsTools/HcNano/interface/HToDStarMesonGenProducer.h"


// constructor //
HToDStarMesonGenProducer::HToDStarMesonGenProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    genParticlesToken(consumes<reco::GenParticleCollection>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))) {
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name+"DecayType"); // singleton table of gen-level decay type
    produces<nanoaod::FlatTable>(name); // table of gen-particle kinematics
}

// destructor //
HToDStarMesonGenProducer::~HToDStarMesonGenProducer(){}

// descriptions //
void HToDStarMesonGenProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void HToDStarMesonGenProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

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

    // find H -> D* + X, D* -> pi D0, D0 -> K pi
    std::vector< std::map< std::string, const reco::GenParticle* > > HtoDStarGenParticles;
    HtoDStarGenParticles = find_H_to_DStar_to_DZeroPi_to_KPiPi( *genParticles );

    // convert to format suitable for flat table
    std::map< std::string, std::vector<float> > variables;
    std::vector<std::string> particleNames = {"H", "DStar", "DZero", "Pi1", "K", "Pi2"};
    for( const auto& particleName: particleNames ){
        std::vector<float> pt;
        std::vector<float> eta;
        std::vector<float> phi;
        for(unsigned int idx=0; idx < HtoDStarGenParticles.size(); idx++){
            pt.push_back(HtoDStarGenParticles[idx].at(particleName)->pt());
            eta.push_back(HtoDStarGenParticles[idx].at(particleName)->eta());
            phi.push_back(HtoDStarGenParticles[idx].at(particleName)->phi());
        }
        std::string ptName = particleName + "_pt";
        variables[ptName] = pt;
        std::string etaName = particleName + "_eta";
        variables[etaName] = eta;
        std::string phiName = particleName + "_phi";
        variables[phiName] = phi;
    }
    
    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(HtoDStarGenParticles.size(), name, false);
    for( const auto& pair : variables ){
        std::string name = pair.first;
        std::vector<float> values = pair.second;
        table->addColumn<float>(name, values, "");
    }

    // add the table to the output
    iEvent.put(std::move(table), name);
}

int HToDStarMesonGenProducer::find_H_decay_type(
        const std::vector<reco::GenParticle>& genParticles){
    // find what type of event this is concerning the production and decay of H -> D*.
    // the numbering convention is as follows:
    // 0: undefined, none of the below.
    // 1: at least one H -> D* + X, D* -> D0 pi, D0 -> K pi (i.e. the decay of interest).
    // 2: at least one H -> D* + X, D* -> D0 pi, but excluding the above.
    // 3: at least one H -> D* + X, but excluding the above.
    // 4: at least one H -> c + cbar, but excluding the above.
    // 5: at least one H, but excluding the above
    
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
        for( const reco::GenParticle& dstar : ccbarDaughters ){
            int pdgid = dstar.pdgId();

            // check if it is a D* meson
            if(std::abs(pdgid) != 413) continue;
            if(res > 3) res = 3;

            // find the decay products of the D* meson
            std::vector<const reco::GenParticle*> dstarDaughters;
            for(unsigned int i=0; i < dstar.numberOfDaughters(); ++i){
                dstarDaughters.push_back( &genParticles[dstar.daughterRef(i).key()] );
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
            // we have a genuine H -> D* + X, D* -> D0 pi, D0 -> K pi event
            if(res > 1) res = 1;
            break;
        }
    }
    if(res > 5) res = 0;
    return res;
}


std::vector< std::map< std::string, const reco::GenParticle* > > HToDStarMesonGenProducer::find_H_to_DStar_to_DZeroPi_to_KPiPi(
        const std::vector<reco::GenParticle>& genParticles){
    // find H -> D* + X, D* -> D0 pi, D0 -> K pi at GEN level

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

        // printouts for testing
        /*for(const reco::GenParticle& p : ccbarDaughters){
            std::cout << p.pdgId() << std::endl;
        }
        std::cout << "---" << std::endl;*/

        // loop over the decay products of the c + cbar pair
        for( const reco::GenParticle& dstar : ccbarDaughters ){
            int pdgid = dstar.pdgId();

            // check if it is a D* meson
            if(std::abs(pdgid) != 413) continue;

            // find its daughters
            std::vector<const reco::GenParticle*> dstarDaughters;
            for(unsigned int i=0; i<dstar.numberOfDaughters(); ++i){
                dstarDaughters.push_back( &genParticles[dstar.daughterRef(i).key()] );
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
                {"H", &h},
                {"DStar", &dstar},
                {"DZero", dzero},
                {"Pi1", pi},
                {"K", K},
                {"Pi2", pi2}
            };
            res.push_back(thisres);
        }
    }
    return res;
}

// define this as a plug-in
DEFINE_FWK_MODULE(HToDStarMesonGenProducer);
