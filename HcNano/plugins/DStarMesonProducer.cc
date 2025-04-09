/*
Custom analyzer class for finding D* mesons from triplets of tracks.

The targeted decay chain is the following:
D* -> D0 pi -> K pi pi
*/

// local include files
#include "PhysicsTools/HcNano/interface/DStarMesonProducer.h"

// constructor //
DStarMesonProducer::DStarMesonProducer(const edm::ParameterSet& iConfig)
  : name(iConfig.getParameter<std::string>("name")),
    dtype(iConfig.getParameter<std::string>("dtype")),
    packedPFCandidatesToken(consumes<std::vector<pat::PackedCandidate>>(
        iConfig.getParameter<edm::InputTag>("packedPFCandidatesToken"))),
    lostTracksToken(consumes<std::vector<pat::PackedCandidate>>(
        iConfig.getParameter<edm::InputTag>("lostTracksToken"))),
    genParticlesToken(consumes<std::vector<reco::GenParticle>>(
        iConfig.getParameter<edm::InputTag>("genParticlesToken"))){
    // declare tables to be produced
    produces<nanoaod::FlatTable>(name);
}

// destructor //
DStarMesonProducer::~DStarMesonProducer(){}

// descriptions //
void DStarMesonProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<std::string>("dtype", "Data type (mc or data)");
    desc.add<edm::InputTag>("packedPFCandidatesToken", edm::InputTag("packedPFCandidatesToken"));
    desc.add<edm::InputTag>("lostTracksToken", edm::InputTag("lostTracksToken"));
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void DStarMesonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get all required objects from tokens
    edm::Handle<std::vector<pat::PackedCandidate>> packedPFCandidates;
    iEvent.getByToken(packedPFCandidatesToken, packedPFCandidates);
    edm::Handle<std::vector<pat::PackedCandidate>> lostTracks;
    iEvent.getByToken(lostTracksToken, lostTracks);
    MagneticField* bfield = new OAEParametrizedMagneticField("3_8T");

    // settings for gen-matching
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    std::vector< std::map< std::string, const reco::GenParticle* > > DStarGenParticles;
    bool doMatching = (dtype=="mc") ? true : false;
    if( !genParticles.isValid() ) doMatching = false;
    if( doMatching ){
        DStarGenParticles = DStarMesonGenProducer::find_DStar_to_DZeroPi_to_KPiPi( *genParticles );
        if( DStarGenParticles.size()==0 ) doMatching = false;
    }

    // declare output variables
    std::vector<float> DStarMeson_mass;
    std::vector<float> DStarMeson_pt;
    std::vector<float> DStarMeson_eta;
    std::vector<float> DStarMeson_phi;
    std::vector<float> DStarMeson_DZeroMeson_mass;
    std::vector<float> DStarMeson_DZeroMeson_pt;
    std::vector<float> DStarMeson_DZeroMeson_eta;
    std::vector<float> DStarMeson_DZeroMeson_phi;
    std::vector<float> DStarMeson_DZeroMeson_massDiff;
    std::vector<float> DStarMeson_Pi1_pt;
    std::vector<float> DStarMeson_Pi1_eta;
    std::vector<float> DStarMeson_Pi1_phi;
    std::vector<float> DStarMeson_K_pt;
    std::vector<float> DStarMeson_K_eta;
    std::vector<float> DStarMeson_K_phi;
    std::vector<float> DStarMeson_Pi2_pt;
    std::vector<float> DStarMeson_Pi2_eta;
    std::vector<float> DStarMeson_Pi2_phi;
    std::vector<float> DStarMeson_tr1tr2_deltaR;
    std::vector<float> DStarMeson_tr3d0_deltaR;
    std::vector<float> DStarMeson_d0vtx_normchi2;
    std::vector<float> DStarMeson_dstarvtx_normchi2;
    std::vector<float> DStarMeson_tr1tr2_sepx;
    std::vector<float> DStarMeson_tr1tr2_sepy;
    std::vector<float> DStarMeson_tr1tr2_sepz;
    std::vector<float> DStarMeson_tr3d0_sepx;
    std::vector<float> DStarMeson_tr3d0_sepy;
    std::vector<float> DStarMeson_tr3d0_sepz;
    std::vector<bool> DStarMeson_hasFastGenMatch;
    std::vector<bool> DStarMeson_hasFastPartialGenMatch;

    // merge packed candidate tracks and lost tracks
    std::vector<reco::Track> allTracks;
    for(const pat::PackedCandidate& pc: *packedPFCandidates){
        if(pc.hasTrackDetails()){
            reco::Track track = *pc.bestTrack();
            allTracks.push_back(track);
        }
    }

    for(const pat::PackedCandidate& pc: *lostTracks){
        if(pc.hasTrackDetails()){
            reco::Track track = *pc.bestTrack();
            allTracks.push_back(track);
        }
    }

    // preselect tracks
    std::vector<reco::Track> selectedTracks;
    for(const reco::Track& track: allTracks){
        if(!track.quality(reco::TrackBase::qualityByName("highPurity"))) continue;
        if(track.pt() < 0.3) continue;
	    selectedTracks.push_back(track);
    }

    // loop over pairs of tracks
    for(unsigned i=0; i<selectedTracks.size(); i++){
      for(unsigned j=i+1; j<selectedTracks.size(); j++){
        const reco::Track tr1 = selectedTracks.at(i);
        const reco::Track tr2 = selectedTracks.at(j);

        // candidates must have opposite charge
        if(tr1.charge() * tr2.charge() > 0) continue;

        // candidates must point approximately in the same direction
        if( reco::deltaR(tr1, tr2) > 0.4 ) continue;

        // reference points of both tracks must be close together
        const math::XYZPoint tr1refpoint = tr1.referencePoint();
        const math::XYZPoint tr2refpoint = tr2.referencePoint();
        double twotracksepx = std::abs(tr1refpoint.x()-tr2refpoint.x());
        double twotracksepy = std::abs(tr1refpoint.y()-tr2refpoint.y());
        double twotracksepz = std::abs(tr1refpoint.z()-tr2refpoint.z());
        if( twotracksepx>0.1 || twotracksepy>0.1 || twotracksepz>0.1 ) continue;
       
        // find which track is positive and which is negative
        reco::Track postrack;
        reco::Track negtrack;
        if(tr1.charge()>0. and tr2.charge()<0){
            postrack = tr1;
            negtrack = tr2;
        } else if(tr1.charge()<0. and tr2.charge()>0){
	        postrack = tr2;
	        negtrack = tr1;
        } else continue; // should not normally happen but just for safety

        // make invariant mass
        // (note: although the D0 meson decays preferentially to K- pi+ rather than K+ pi-,
        //  still both possibilities must be considered since the original particle could
        //  be an anti-D0, which decays preferentially to K+ pi-)
        ROOT::Math::PtEtaPhiMVector piPlusP4(postrack.pt(), postrack.eta(), postrack.phi(), pimass);
        ROOT::Math::PtEtaPhiMVector KMinusP4(negtrack.pt(), negtrack.eta(), negtrack.phi(), kmass);
        ROOT::Math::PtEtaPhiMVector KPlusP4(postrack.pt(), postrack.eta(), postrack.phi(), kmass);
        ROOT::Math::PtEtaPhiMVector piMinusP4(negtrack.pt(), negtrack.eta(), negtrack.phi(), pimass);
        ROOT::Math::PtEtaPhiMVector dzeroP4 = piPlusP4 + KMinusP4;
        ROOT::Math::PtEtaPhiMVector dzerobarP4 = piMinusP4 + KPlusP4;
        double dzeroInvMass = dzeroP4.M();
        double dzerobarInvMass = dzerobarP4.M();
        
        // invariant mass must be close to resonance mass
        ROOT::Math::PtEtaPhiMVector pi2P4(0, 0, 0, 0);
        ROOT::Math::PtEtaPhiMVector KP4(0, 0, 0, 0);
        if( (std::abs(dzeroInvMass - dzeromass) < 0.035)
            && (std::abs(dzeroInvMass - dzeromass) < std::abs(dzerobarInvMass - dzeromass)) ){
            pi2P4 = piPlusP4;
            KP4 = KMinusP4;
        } else if( (std::abs(dzerobarInvMass - dzeromass) < 0.035) 
                   && (std::abs(dzerobarInvMass - dzeromass) < std::abs(dzeroInvMass - dzeromass)) ){
            pi2P4 = piMinusP4;
            KP4 = KPlusP4;
            dzeroP4 = dzerobarP4;
            dzeroInvMass = dzerobarInvMass;
        } else continue;

        // fit a vertex
        std::vector<reco::TransientTrack> transpair;
        transpair.push_back(reco::TransientTrack(tr1, bfield));
        transpair.push_back(reco::TransientTrack(tr2, bfield));
        KalmanVertexFitter vtxFitter(false);
        TransientVertex dzerovtx = vtxFitter.vertex(transpair);
        // vertex must be valid
        if(!dzerovtx.isValid()) continue;
        // chi squared of fit must be small
        if(dzerovtx.normalisedChiSquared()>5.) continue;
        if(dzerovtx.normalisedChiSquared()<0.) continue;
        
        // loop over third track
	    for(unsigned k=0; k<selectedTracks.size(); k++){
            if(k==i or k==j) continue;
            const reco::Track tr3 = selectedTracks.at(k);

            // candidates must point approximately in the same direction
            if( reco::deltaR(tr3, dzeroP4) > 0.1 ) continue;

            // candidates must have pT greater certain value
            if( tr3.pt() < 0.5 ) continue;

            // reference point of third track must be close to phi vertex
            const math::XYZPoint tr3refpoint = tr3.referencePoint();
            double trackvtxsepx = std::abs(tr3refpoint.x()-dzerovtx.position().x());
            double trackvtxsepy = std::abs(tr3refpoint.y()-dzerovtx.position().y());
            double trackvtxsepz = std::abs(tr3refpoint.z()-dzerovtx.position().z());
            if( trackvtxsepx>0.1 || trackvtxsepy>0.1 || trackvtxsepz>0.1 ) continue;

            // make invariant mass (under the assumption of pi mass for the third track)
            ROOT::Math::PtEtaPhiMVector pi1P4(tr3.pt(), tr3.eta(), tr3.phi(), pimass);
            ROOT::Math::PtEtaPhiMVector dstarP4 = dzeroP4 + pi1P4;
            double dstarInvMass = dstarP4.M();

            // check if mass is close enough to D* mass
            if(std::abs(dstarInvMass - dstarmass) > 0.1) continue;

            // do a vertex fit
            std::vector<reco::TransientTrack> transtriplet;
            transtriplet.push_back(reco::TransientTrack(tr1, bfield));
            transtriplet.push_back(reco::TransientTrack(tr2, bfield));
            transtriplet.push_back(reco::TransientTrack(tr3, bfield));
            TransientVertex dstarvtx = vtxFitter.vertex(transtriplet);
            if(!dstarvtx.isValid()) continue;
            if(dstarvtx.normalisedChiSquared()>5.) continue;
            if(dstarvtx.normalisedChiSquared()<0.) continue;

            // set properties of the D* candidate
            DStarMeson_mass.push_back( dstarP4.M() );
            DStarMeson_pt.push_back( dstarP4.pt() );
            DStarMeson_eta.push_back( dstarP4.eta() );
            DStarMeson_phi.push_back( dstarP4.phi() );
            DStarMeson_DZeroMeson_mass.push_back( dzeroP4.M() );
            DStarMeson_DZeroMeson_pt.push_back( dzeroP4.pt() );
            DStarMeson_DZeroMeson_eta.push_back( dzeroP4.eta() );
            DStarMeson_DZeroMeson_phi.push_back( dzeroP4.phi() );
            DStarMeson_DZeroMeson_massDiff.push_back( dstarP4.M() - dzeroP4.M() );
            DStarMeson_Pi1_pt.push_back( pi1P4.pt() );
            DStarMeson_Pi1_eta.push_back( pi1P4.eta() );
            DStarMeson_Pi1_phi.push_back( pi1P4.phi() );
            DStarMeson_K_pt.push_back( KP4.pt() );
            DStarMeson_K_eta.push_back( KP4.eta() );
            DStarMeson_K_phi.push_back( KP4.phi() );
            DStarMeson_Pi2_pt.push_back( pi2P4.pt() );
            DStarMeson_Pi2_eta.push_back( pi2P4.eta() );
            DStarMeson_Pi2_phi.push_back( pi2P4.phi() );
            DStarMeson_tr1tr2_deltaR.push_back( reco::deltaR(tr1, tr2) );
            DStarMeson_tr3d0_deltaR.push_back( reco::deltaR(tr3, dzeroP4) );
            DStarMeson_d0vtx_normchi2.push_back( dzerovtx.normalisedChiSquared() );
            DStarMeson_dstarvtx_normchi2.push_back( dstarvtx.normalisedChiSquared() );
            DStarMeson_tr1tr2_sepx.push_back( twotracksepx );
            DStarMeson_tr1tr2_sepy.push_back( twotracksepy );
            DStarMeson_tr1tr2_sepz.push_back( twotracksepz );
            DStarMeson_tr3d0_sepx.push_back( trackvtxsepx );
            DStarMeson_tr3d0_sepy.push_back( trackvtxsepy );
            DStarMeson_tr3d0_sepz.push_back( trackvtxsepz );            

            // check if this candidate can be matched to gen-level
            bool hasFastGenMatch = false;
            bool hasFastPartialGenMatch = false;
            if( doMatching ){
                for( const auto& pmap : DStarGenParticles){
                    double dRThreshold = 0.05;
                    if( GenTools::isGeometricTrackMatch( tr3, *pmap.at("Pi1"), dRThreshold )
                        && ( (GenTools::isGeometricTrackMatch( postrack, *pmap.at("K"), dRThreshold )
                              && GenTools::isGeometricTrackMatch( negtrack, *pmap.at("Pi2"), dRThreshold ) )
                             || (GenTools::isGeometricTrackMatch( postrack, *pmap.at("Pi2"), dRThreshold )
                              && GenTools::isGeometricTrackMatch( negtrack, *pmap.at("K"), dRThreshold ) ) ) ){
                        hasFastGenMatch = true;
                    }
                    if( GenTools::isGeometricTrackMatch( tr3, *pmap.at("Pi1"), dRThreshold )
                         || GenTools::isGeometricTrackMatch( postrack, *pmap.at("K"), dRThreshold )
                         || GenTools::isGeometricTrackMatch( negtrack, *pmap.at("Pi2"), dRThreshold )
                         || GenTools::isGeometricTrackMatch( postrack, *pmap.at("Pi2"), dRThreshold )
                         || GenTools::isGeometricTrackMatch( negtrack, *pmap.at("K"), dRThreshold ) ){
                        hasFastPartialGenMatch = true;
                    }
                }
            }
            DStarMeson_hasFastGenMatch.push_back( hasFastGenMatch );
            DStarMeson_hasFastPartialGenMatch.push_back( hasFastPartialGenMatch );

            // break loop over third track in case maximum number was reached
            if( DStarMeson_mass.size() == nDStarMeson_max ) break;

        } // end loop over third track
        if( DStarMeson_mass.size() == nDStarMeson_max ) break;
      }
      if( DStarMeson_mass.size()  == nDStarMeson_max) break;
    } // end loop over first and second track
    delete bfield;

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(DStarMeson_mass.size(), name, false);
    table->addColumn<float>("mass", DStarMeson_mass, "");
    table->addColumn<float>("pt", DStarMeson_pt, "");
    table->addColumn<float>("eta", DStarMeson_eta, "");
    table->addColumn<float>("phi", DStarMeson_phi, "");
    table->addColumn<float>("DZeroMeson_mass", DStarMeson_DZeroMeson_mass, "");
    table->addColumn<float>("DZeroMeson_pt", DStarMeson_DZeroMeson_pt, "");
    table->addColumn<float>("DZeroMeson_eta", DStarMeson_DZeroMeson_eta, "");
    table->addColumn<float>("DZeroMeson_phi", DStarMeson_DZeroMeson_phi, "");
    table->addColumn<float>("DZeroMeson_massDiff", DStarMeson_DZeroMeson_massDiff, "");
    table->addColumn<float>("Pi1_pt", DStarMeson_Pi1_pt, "");
    table->addColumn<float>("Pi1_eta", DStarMeson_Pi1_eta, "");
    table->addColumn<float>("Pi1_phi", DStarMeson_Pi1_phi, "");
    table->addColumn<float>("K_pt", DStarMeson_K_pt, "");
    table->addColumn<float>("K_eta", DStarMeson_K_eta, "");
    table->addColumn<float>("K_phi", DStarMeson_K_phi, "");
    table->addColumn<float>("Pi2_pt", DStarMeson_Pi2_pt, "");
    table->addColumn<float>("Pi2_eta", DStarMeson_Pi2_eta, "");
    table->addColumn<float>("Pi2_phi", DStarMeson_Pi2_phi, "");
    table->addColumn<float>("tr1tr2_deltaR", DStarMeson_tr1tr2_deltaR, "");
    table->addColumn<float>("tr3d0_deltaR", DStarMeson_tr3d0_deltaR, "");
    table->addColumn<float>("d0vtx_normchi2", DStarMeson_d0vtx_normchi2, "");
    table->addColumn<float>("dstarvtx_normchi2", DStarMeson_dstarvtx_normchi2, "");
    table->addColumn<float>("tr1tr2_sepx", DStarMeson_tr1tr2_sepx, "");
    table->addColumn<float>("tr1tr2_sepy", DStarMeson_tr1tr2_sepy, "");
    table->addColumn<float>("tr1tr2_sepz", DStarMeson_tr1tr2_sepz, "");
    table->addColumn<float>("tr3d0_sepx", DStarMeson_tr3d0_sepx, "");
    table->addColumn<float>("tr3d0_sepy", DStarMeson_tr3d0_sepy, "");
    table->addColumn<float>("tr3d0_sepz", DStarMeson_tr3d0_sepz, "");
    table->addColumn<bool>("hasFastGenmatch", DStarMeson_hasFastGenMatch, "");
    table->addColumn<bool>("hasFastPartialGenmatch", DStarMeson_hasFastPartialGenMatch, "");

    // add the table to the output
    iEvent.put(std::move(table), name);
}

// define this as a plug-in
DEFINE_FWK_MODULE(DStarMesonProducer);
