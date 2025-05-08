/*
Custom analyzer class for finding D* mesons from triplets of tracks.

The targeted decay chain is the following:
D* -> D0 pi -> K pi pi
*/

// local include files
#include "PhysicsTools/HcNano/interface/HToDStarMesonProducer.h"

// constructor //
HToDStarMesonProducer::HToDStarMesonProducer(const edm::ParameterSet& iConfig)
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
HToDStarMesonProducer::~HToDStarMesonProducer(){}

// descriptions //
void HToDStarMesonProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<std::string>("dtype", "Data type (mc or data)");
    desc.add<edm::InputTag>("packedPFCandidatesToken", edm::InputTag("packedPFCandidatesToken"));
    desc.add<edm::InputTag>("lostTracksToken", edm::InputTag("lostTracksToken"));
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void HToDStarMesonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get all required objects from tokens
    edm::Handle<std::vector<pat::PackedCandidate>> packedPFCandidates;
    iEvent.getByToken(packedPFCandidatesToken, packedPFCandidates);
    edm::Handle<std::vector<pat::PackedCandidate>> lostTracks;
    iEvent.getByToken(lostTracksToken, lostTracks);
    MagneticField* bfield = new OAEParametrizedMagneticField("3_8T");

    // settings for gen-matching
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    std::vector< std::map< std::string, const reco::GenParticle* > > HToDStarGenParticles;
    bool doMatching = (dtype=="mc") ? true : false;
    if( !genParticles.isValid() ) doMatching = false;
    if( doMatching ){
        HToDStarGenParticles = HToDStarMesonGenProducer::find_H_to_DStar_to_DZeroPi_to_KPiPi( *genParticles );
        if( HToDStarGenParticles.size()==0 ) doMatching = false;
    }

    // declare output variables
    std::vector<float> HToDStarMeson_mass;
    std::vector<float> HToDStarMeson_pt;
    std::vector<float> HToDStarMeson_eta;
    std::vector<float> HToDStarMeson_phi;
    std::vector<float> HToDStarMeson_DZeroMeson_mass;
    std::vector<float> HToDStarMeson_DZeroMeson_pt;
    std::vector<float> HToDStarMeson_DZeroMeson_eta;
    std::vector<float> HToDStarMeson_DZeroMeson_phi;
    std::vector<float> HToDStarMeson_DZeroMeson_massDiff;
    std::vector<float> HToDStarMeson_Pi1_pt;
    std::vector<float> HToDStarMeson_Pi1_eta;
    std::vector<float> HToDStarMeson_Pi1_phi;
    std::vector<int> HToDStarMeson_Pi1_charge;
    std::vector<float> HToDStarMeson_K_pt;
    std::vector<float> HToDStarMeson_K_eta;
    std::vector<float> HToDStarMeson_K_phi;
    std::vector<int> HToDStarMeson_K_charge;
    std::vector<float> HToDStarMeson_Pi2_pt;
    std::vector<float> HToDStarMeson_Pi2_eta;
    std::vector<float> HToDStarMeson_Pi2_phi;
    std::vector<int> HToDStarMeson_Pi2_charge;
    std::vector<float> HToDStarMeson_tr1tr2_deltaR;
    std::vector<float> HToDStarMeson_tr3d0_deltaR;
    std::vector<float> HToDStarMeson_d0vtx_normchi2;
    std::vector<float> HToDStarMeson_dstarvtx_normchi2;
    std::vector<float> HToDStarMeson_tr1tr2_sepx;
    std::vector<float> HToDStarMeson_tr1tr2_sepy;
    std::vector<float> HToDStarMeson_tr1tr2_sepz;
    std::vector<float> HToDStarMeson_tr3d0_sepx;
    std::vector<float> HToDStarMeson_tr3d0_sepy;
    std::vector<float> HToDStarMeson_tr3d0_sepz;
    std::vector<bool> HToDStarMeson_hasFastGenMatch;
    std::vector<bool> HToDStarMeson_hasFastPartialGenMatch;
    std::vector<bool> HToDStarMeson_hasFastAllOriginGenMatch;

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
        // note: now disabled for study to check if candidates with same charge
        //       can be used for background estimation.
        //if(tr1.charge() * tr2.charge() > 0) continue;

        // candidates must point approximately in the same direction
        if( reco::deltaR(tr1, tr2) > 0.4 ) continue;

        // reference points of both tracks must be close together
        const math::XYZPoint tr1refpoint = tr1.referencePoint();
        const math::XYZPoint tr2refpoint = tr2.referencePoint();
        double twotracksepx = std::abs(tr1refpoint.x()-tr2refpoint.x());
        double twotracksepy = std::abs(tr1refpoint.y()-tr2refpoint.y());
        double twotracksepz = std::abs(tr1refpoint.z()-tr2refpoint.z());
        if( twotracksepx>0.02 || twotracksepy>0.02 || twotracksepz>0.05 ) continue;
       
        // find which track is positive and which is negative
        reco::Track postrack;
        reco::Track negtrack;
        if(tr1.charge()>0. and tr2.charge()<0){
            postrack = tr1;
            negtrack = tr2;
        } else if(tr1.charge()<0. and tr2.charge()>0){
	        postrack = tr2;
	        negtrack = tr1;
        } else {
            // if both tracks have the same charge
            // (e.g. in combinatorial background),
            // assign them randomly.
            if( rand() % 2 == 0 ){
                postrack = tr1;
                negtrack = tr2;
            } else {
                postrack = tr2;
                negtrack = tr1;
            }
        }

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
        reco::Track pi2Track;
        reco::Track KTrack;
        if( (std::abs(dzeroInvMass - dzeromass) < 0.035)
            && (std::abs(dzeroInvMass - dzeromass) < std::abs(dzerobarInvMass - dzeromass)) ){
            pi2P4 = piPlusP4;
            KP4 = KMinusP4;
            pi2Track = postrack;
            KTrack = negtrack;
        } else if( (std::abs(dzerobarInvMass - dzeromass) < 0.035) 
                   && (std::abs(dzerobarInvMass - dzeromass) < std::abs(dzeroInvMass - dzeromass)) ){
            pi2P4 = piMinusP4;
            KP4 = KPlusP4;
            pi2Track = negtrack;
            KTrack = postrack;
            dzeroP4 = dzerobarP4;
            dzeroInvMass = dzerobarInvMass;
        } else continue;

        // K candidate must have a given minimum pt
        if( KP4.pt() < 1. ) continue;

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

            // pi candidate must have a given minimum pt
            if( tr3.pt() < 0.5 ) continue;

            // candidates must point approximately in the same direction
            if( reco::deltaR(tr3, dzeroP4) > 0.1 ) continue;

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
            HToDStarMeson_mass.push_back( dstarP4.M() );
            HToDStarMeson_pt.push_back( dstarP4.pt() );
            HToDStarMeson_eta.push_back( dstarP4.eta() );
            HToDStarMeson_phi.push_back( dstarP4.phi() );
            HToDStarMeson_DZeroMeson_mass.push_back( dzeroP4.M() );
            HToDStarMeson_DZeroMeson_pt.push_back( dzeroP4.pt() );
            HToDStarMeson_DZeroMeson_eta.push_back( dzeroP4.eta() );
            HToDStarMeson_DZeroMeson_phi.push_back( dzeroP4.phi() );
            HToDStarMeson_DZeroMeson_massDiff.push_back( dstarP4.M() - dzeroP4.M() );
            HToDStarMeson_Pi1_pt.push_back( pi1P4.pt() );
            HToDStarMeson_Pi1_eta.push_back( pi1P4.eta() );
            HToDStarMeson_Pi1_phi.push_back( pi1P4.phi() );
            HToDStarMeson_Pi1_charge.push_back( tr3.charge() );
            HToDStarMeson_K_pt.push_back( KP4.pt() );
            HToDStarMeson_K_eta.push_back( KP4.eta() );
            HToDStarMeson_K_phi.push_back( KP4.phi() );
            HToDStarMeson_K_charge.push_back( KTrack.charge() );
            HToDStarMeson_Pi2_pt.push_back( pi2P4.pt() );
            HToDStarMeson_Pi2_eta.push_back( pi2P4.eta() );
            HToDStarMeson_Pi2_phi.push_back( pi2P4.phi() );
            HToDStarMeson_Pi2_charge.push_back( pi2Track.charge() );
            HToDStarMeson_tr1tr2_deltaR.push_back( reco::deltaR(tr1, tr2) );
            HToDStarMeson_tr3d0_deltaR.push_back( reco::deltaR(tr3, dzeroP4) );
            HToDStarMeson_d0vtx_normchi2.push_back( dzerovtx.normalisedChiSquared() );
            HToDStarMeson_dstarvtx_normchi2.push_back( dstarvtx.normalisedChiSquared() );
            HToDStarMeson_tr1tr2_sepx.push_back( twotracksepx );
            HToDStarMeson_tr1tr2_sepy.push_back( twotracksepy );
            HToDStarMeson_tr1tr2_sepz.push_back( twotracksepz );
            HToDStarMeson_tr3d0_sepx.push_back( trackvtxsepx );
            HToDStarMeson_tr3d0_sepy.push_back( trackvtxsepy );
            HToDStarMeson_tr3d0_sepz.push_back( trackvtxsepz );            

            // check if this candidate can be matched to gen-level
            bool hasFastGenMatch = false;
            bool hasFastPartialGenMatch = false;
            if( doMatching ){
                for( const auto& pmap : HToDStarGenParticles){
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
            HToDStarMeson_hasFastGenMatch.push_back( hasFastGenMatch );
            HToDStarMeson_hasFastPartialGenMatch.push_back( hasFastPartialGenMatch );

            // break loop over third track in case maximum number was reached
            if( HToDStarMeson_mass.size() == nHToDStarMeson_max ) break;

        } // end loop over third track
        if( HToDStarMeson_mass.size() == nHToDStarMeson_max ) break;
      }
      if( HToDStarMeson_mass.size()  == nHToDStarMeson_max) break;
    } // end loop over first and second track
    delete bfield;

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(HToDStarMeson_mass.size(), name, false);
    table->addColumn<float>("mass", HToDStarMeson_mass, "");
    table->addColumn<float>("pt", HToDStarMeson_pt, "");
    table->addColumn<float>("eta", HToDStarMeson_eta, "");
    table->addColumn<float>("phi", HToDStarMeson_phi, "");
    table->addColumn<float>("DZeroMeson_mass", HToDStarMeson_DZeroMeson_mass, "");
    table->addColumn<float>("DZeroMeson_pt", HToDStarMeson_DZeroMeson_pt, "");
    table->addColumn<float>("DZeroMeson_eta", HToDStarMeson_DZeroMeson_eta, "");
    table->addColumn<float>("DZeroMeson_phi", HToDStarMeson_DZeroMeson_phi, "");
    table->addColumn<float>("DZeroMeson_massDiff", HToDStarMeson_DZeroMeson_massDiff, "");
    table->addColumn<float>("Pi1_pt", HToDStarMeson_Pi1_pt, "");
    table->addColumn<float>("Pi1_eta", HToDStarMeson_Pi1_eta, "");
    table->addColumn<float>("Pi1_phi", HToDStarMeson_Pi1_phi, "");
    table->addColumn<int>("Pi1_charge", HToDStarMeson_Pi1_charge, "");
    table->addColumn<float>("K_pt", HToDStarMeson_K_pt, "");
    table->addColumn<float>("K_eta", HToDStarMeson_K_eta, "");
    table->addColumn<float>("K_phi", HToDStarMeson_K_phi, "");
    table->addColumn<int>("K_charge", HToDStarMeson_K_charge, "");
    table->addColumn<float>("Pi2_pt", HToDStarMeson_Pi2_pt, "");
    table->addColumn<float>("Pi2_eta", HToDStarMeson_Pi2_eta, "");
    table->addColumn<float>("Pi2_phi", HToDStarMeson_Pi2_phi, "");
    table->addColumn<int>("Pi2_charge", HToDStarMeson_Pi2_charge, "");
    table->addColumn<float>("tr1tr2_deltaR", HToDStarMeson_tr1tr2_deltaR, "");
    table->addColumn<float>("tr3d0_deltaR", HToDStarMeson_tr3d0_deltaR, "");
    table->addColumn<float>("d0vtx_normchi2", HToDStarMeson_d0vtx_normchi2, "");
    table->addColumn<float>("dstarvtx_normchi2", HToDStarMeson_dstarvtx_normchi2, "");
    table->addColumn<float>("tr1tr2_sepx", HToDStarMeson_tr1tr2_sepx, "");
    table->addColumn<float>("tr1tr2_sepy", HToDStarMeson_tr1tr2_sepy, "");
    table->addColumn<float>("tr1tr2_sepz", HToDStarMeson_tr1tr2_sepz, "");
    table->addColumn<float>("tr3d0_sepx", HToDStarMeson_tr3d0_sepx, "");
    table->addColumn<float>("tr3d0_sepy", HToDStarMeson_tr3d0_sepy, "");
    table->addColumn<float>("tr3d0_sepz", HToDStarMeson_tr3d0_sepz, "");
    table->addColumn<bool>("hasFastGenmatch", HToDStarMeson_hasFastGenMatch, "");
    table->addColumn<bool>("hasFastPartialGenmatch", HToDStarMeson_hasFastPartialGenMatch, "");

    // add the table to the output
    iEvent.put(std::move(table), name);
}

// define this as a plug-in
DEFINE_FWK_MODULE(HToDStarMesonProducer);
