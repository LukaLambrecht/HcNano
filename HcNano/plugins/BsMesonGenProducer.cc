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
    std::vector<std::string> particleNames = {"Bs", "Ds", "X", "X0", "D0", "Phi", "Pi", "KPlus", "KMinus"};
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
    // -3: at least one Bs -> Ds X -> phi pi -> K K pi anything (i.e. the decay of interest).
    // -2: at least one Bs -> Ds X -> phi pi (i.e. the decay of interest).
    // -1: at least one Bs -> D0 X
    // 1: at least one Bs -> Ds X, but excluding the above.
    // 2: at least one charmed meson/baryon
    // 3: at least one Bs, but excluding the above.
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

        // check if it is a bottom hadron
        int pdgid = p->pdgId();
        bool bMeson = (std::abs(pdgid) > 500 && std::abs(pdgid) < 600);
        bool bBaryon = (std::abs(pdgid) > 5000 && std::abs(pdgid) < 6000);
        if( !(bMeson || bBaryon) ) continue;
        if(res > 4) res = 4;

        // check if it is a Bs meson
        if(std::abs(pdgid) != 531) continue;
        const reco::GenParticle* bs = p;
        if(res > 3) res = 3;

        // find the decay products of the Bs meson
        std::vector<const reco::GenParticle*> bsDaughters;
        for(unsigned int i=0; i < bs->numberOfDaughters(); ++i){
            bsDaughters.push_back( &genParticles[bs->daughterRef(i).key()] );
        }
        
        // if there's at least some charmed meson
        if( bsDaughters.size()!=2 ) continue;
        int pdg0 = bsDaughters.at(0)->pdgId();
        int pdg1 = bsDaughters.at(1)->pdgId();
        bool cMeson0 = (std::abs(pdg0) > 400 && std::abs(pdg0) < 500);
        bool cBaryon0 = (std::abs(pdg0) > 4000 && std::abs(pdg0) < 5000);
        bool cMeson1 = (std::abs(pdg1) > 400 && std::abs(pdg1) < 500);
        bool cBaryon1 = (std::abs(pdg1) > 4000 && std::abs(pdg1) < 5000);
        if (!(cMeson0 || cBaryon0 || cMeson1 || cBaryon1)) continue;
        if(res > 2) res = 2;


        // find if they are a Ds meson and X
        const reco::GenParticle* ds;
        if( std::abs(bsDaughters.at(0)->pdgId())==431){
            ds = bsDaughters.at(0);
        } else if( std::abs(bsDaughters.at(1)->pdgId())==431){
            ds = bsDaughters.at(1);
        } else continue;
        if(res > 1) res = 1;

        // find if they are a D0 meson and X
        const reco::GenParticle* d0;
        if( std::abs(bsDaughters.at(0)->pdgId())==421){
            d0 = bsDaughters.at(0);
        } else if( std::abs(bsDaughters.at(1)->pdgId())==421){
            d0 = bsDaughters.at(1);
        } else continue;
        if(res > -1) res = -1;

        // find the decay products of the Ds meson
        std::vector<const reco::GenParticle*> dsDaughters;
        for(unsigned int i=0; i < ds->numberOfDaughters(); ++i){
            dsDaughters.push_back( &genParticles[ds->daughterRef(i).key()] );
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
        if(res > -2) res = -2;
        

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
        // we have a genuine Bs -> Ds X -> phi pi -> K K pi event
        if(res > -3) res = -3;
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

        // check if it is a Bs meson
        int pdgid = p->pdgId();
        if(std::abs(pdgid) != 531) continue;
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

        //find Ds
        if( bsDaughters.size()!=2 ) continue;
        const reco::GenParticle* ds;
        const reco::GenParticle* X;
        if( std::abs(bsDaughters.at(0)->pdgId())==431 ){
            ds = bsDaughters.at(0);
            X = bsDaughters.at(1);
        } else if( std::abs(bsDaughters.at(1)->pdgId())==431 ){
            ds = bsDaughters.at(1);
            X = bsDaughters.at(0);
        } else continue;

        //find D0
        const reco::GenParticle* d0;
        const reco::GenParticle* X0;
        if( std::abs(bsDaughters.at(0)->pdgId())==421 ){
            d0 = bsDaughters.at(0);
            X0 = bsDaughters.at(1);
        } else if( std::abs(bsDaughters.at(1)->pdgId())==421 ){
            d0 = bsDaughters.at(1);
            X0 = bsDaughters.at(0);
        } else continue;

        // find the daughters of the Ds
        std::vector<const reco::GenParticle*> dsDaughters;
        for(unsigned int i=0; i<ds->numberOfDaughters(); ++i){
            dsDaughters.push_back( &genParticles[ds->daughterRef(i).key()] );
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

        // printouts
        //std::cout << "  -> found Ds -> phi + pi" << std::endl;

        // find the daughters of the phi
        std::vector<const reco::GenParticle*> phiDaughters;
        for(unsigned int i=0; i<phi->numberOfDaughters(); ++i){
            phiDaughters.push_back( &genParticles[phi->daughterRef(i).key()] );
        }

        // printouts
        //std::cout << "phi daughters" << std::endl;
        //for( const reco::GenParticle* p: phiDaughters ){ std::cout << p->pdgId() << " "; } 
        //std::cout << std::endl;

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

        // print kinematics
        /*std::cout << "Ds kinematics:" << std::endl;
        std::cout << ds.pt() << " " << ds.eta() << " " << ds.phi() << std::endl;
        std::cout << "pion kinematics:" << std::endl;
        std::cout << pi->pt() << " " << pi->eta() << " " << pi->phi() << std::endl;
        std::cout << "phi kinematics:" << std::endl;
        std::cout << phi->pt() << " " << phi->eta() << " " << phi->phi() << std::endl;
        std::cout << "kaon1 kinematics:" << std::endl;
        std::cout << K1->pt() << " " << K1->eta() << " " << K1->phi() << std::endl;
        std::cout << "kaon2 kinematics:" << std::endl;
        std::cout << K2->pt() << " " << K2->eta() << " " << K2->phi() << std::endl;*/

        // set the particles in the output map
        std::map< std::string, const reco::GenParticle* > thisres = {
            {"Ds", ds},
            {"D0", d0},
            {"Bs", bs},
            {"X", X},
            {"X0", X0},
            {"Phi", phi},
            {"Pi", pi},
            {"KPlus", KPlus},
            {"KMinus", KMinus}
        };
        res.push_back(thisres);
    }
    return res;
}

// define this as a plug-in
DEFINE_FWK_MODULE(BsMesonGenProducer);
