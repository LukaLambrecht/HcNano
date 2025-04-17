/*
Custom analyzer class for finding Ds mesons triplets of tracks.

The targeted decay chain is the following:
Ds -> phi pi -> K K pi
*/

// local include files
#include "PhysicsTools/HcNano/interface/DsMesonProducer.h"

// constructor //
DsMesonProducer::DsMesonProducer(const edm::ParameterSet& iConfig)
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
DsMesonProducer::~DsMesonProducer(){}

// descriptions //
void DsMesonProducer::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<std::string>("dtype", "Data type (mc or data)");
    desc.add<edm::InputTag>("packedPFCandidatesToken", edm::InputTag("packedPFCandidatesToken"));
    desc.add<edm::InputTag>("lostTracksToken", edm::InputTag("lostTracksToken"));
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void DsMesonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get all required objects from tokens
    edm::Handle<std::vector<pat::PackedCandidate>> packedPFCandidates;
    iEvent.getByToken(packedPFCandidatesToken, packedPFCandidates);
    edm::Handle<std::vector<pat::PackedCandidate>> lostTracks;
    iEvent.getByToken(lostTracksToken, lostTracks);
    MagneticField* bfield = new OAEParametrizedMagneticField("3_8T");

    // settings for gen-matching
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    std::vector< std::map< std::string, const reco::GenParticle* > > DsGenParticles;
    bool doMatching = (dtype=="mc") ? true : false;
    if( !genParticles.isValid() ) doMatching = false;
    if( doMatching ){
        DsGenParticles = DsMesonGenProducer::find_Ds_to_PhiPi_to_KKPi( *genParticles );
        if( DsGenParticles.size()==0 ) doMatching = false;
    }

    // declare output variables
    std::vector<float> DsMeson_mass;
    std::vector<float> DsMeson_pt;
    std::vector<float> DsMeson_eta;
    std::vector<float> DsMeson_phi;
    std::vector<float> DsMeson_PhiMeson_mass;
    std::vector<float> DsMeson_PhiMeson_pt;
    std::vector<float> DsMeson_PhiMeson_eta;
    std::vector<float> DsMeson_PhiMeson_phi;
    std::vector<float> DsMeson_PhiMeson_massDiff;
    std::vector<float> DsMeson_Pi_pt;
    std::vector<float> DsMeson_Pi_eta;
    std::vector<float> DsMeson_Pi_phi;
    std::vector<int> DsMeson_Pi_charge;
    std::vector<float> DsMeson_KPlus_pt;
    std::vector<float> DsMeson_KPlus_eta;
    std::vector<float> DsMeson_KPlus_phi;
    std::vector<int> DsMeson_KPlus_charge;
    std::vector<float> DsMeson_KMinus_pt;
    std::vector<float> DsMeson_KMinus_eta;
    std::vector<float> DsMeson_KMinus_phi;
    std::vector<int> DsMeson_KMinus_charge;
    std::vector<float> DsMeson_tr1tr2_deltaR;
    std::vector<float> DsMeson_tr3phi_deltaR;
    std::vector<float> DsMeson_phivtx_normchi2;
    std::vector<float> DsMeson_dsvtx_normchi2;
    std::vector<float> DsMeson_tr1tr2_sepx;
    std::vector<float> DsMeson_tr1tr2_sepy;
    std::vector<float> DsMeson_tr1tr2_sepz;
    std::vector<float> DsMeson_tr3phi_sepx;
    std::vector<float> DsMeson_tr3phi_sepy;
    std::vector<float> DsMeson_tr3phi_sepz;
    std::vector<bool> DsMeson_hasFastGenMatch;
    std::vector<bool> DsMeson_hasFastPartialGenMatch;

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
        if( reco::deltaR(tr1, tr2) > 0.27 ) continue;
	
        // candidates must have pT greater than certain value
        if(tr1.pt() < 0.6 or tr2.pt() < 0.6) continue;

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

        // make invariant mass (under the assumption of K mass for both tracks)
        ROOT::Math::PtEtaPhiMVector KPlusP4(postrack.pt(), postrack.eta(), postrack.phi(), kmass);
        ROOT::Math::PtEtaPhiMVector KMinusP4(negtrack.pt(), negtrack.eta(), negtrack.phi(), kmass);
        ROOT::Math::PtEtaPhiMVector phiP4 = KPlusP4 + KMinusP4;
        double phiInvMass = phiP4.M();

        // check if mass is close enough to phi mass
        if(std::abs(phiInvMass - phimass) > 0.07) continue;

        // fit a vertex
        std::vector<reco::TransientTrack> transpair;
        transpair.push_back(reco::TransientTrack(tr1, bfield));
        transpair.push_back(reco::TransientTrack(tr2, bfield));
        KalmanVertexFitter vtxFitter(false);
        TransientVertex phivtx = vtxFitter.vertex(transpair);
        // vertex must be valid
        if(!phivtx.isValid()) continue;
        // chi squared of fit must be small
        if(phivtx.normalisedChiSquared()>5.) continue;
        if(phivtx.normalisedChiSquared()<0.) continue;
        
        // loop over third track
	    for(unsigned k=0; k<selectedTracks.size(); k++){
            if(k==i or k==j) continue;
            const reco::Track tr3 = selectedTracks.at(k);

            // candidates must point approximately in the same direction
            if( reco::deltaR(tr3, phiP4) > 0.4 ) continue;

            // reference point of third track must be close to phi vertex
            const math::XYZPoint tr3refpoint = tr3.referencePoint();
            double trackvtxsepx = std::abs(tr3refpoint.x()-phivtx.position().x());
            double trackvtxsepy = std::abs(tr3refpoint.y()-phivtx.position().y());
            double trackvtxsepz = std::abs(tr3refpoint.z()-phivtx.position().z());
            if( trackvtxsepx>0.1 || trackvtxsepy>0.1 || trackvtxsepz>0.1 ) continue;

            // make invariant mass (under the assumption of pi mass for the third track)
            ROOT::Math::PtEtaPhiMVector piP4(tr3.pt(), tr3.eta(), tr3.phi(), pimass);
            ROOT::Math::PtEtaPhiMVector dsP4 = phiP4 + piP4;
            double dsInvMass = dsP4.M();

            // check if mass is close enough to Ds mass
            if(std::abs(dsInvMass - dsmass) > 0.1) continue;

            // do a vertex fit
            std::vector<reco::TransientTrack> transtriplet;
            transtriplet.push_back(reco::TransientTrack(tr1, bfield));
            transtriplet.push_back(reco::TransientTrack(tr2, bfield));
            transtriplet.push_back(reco::TransientTrack(tr3, bfield));
            TransientVertex dsvtx = vtxFitter.vertex(transtriplet);
            if(!dsvtx.isValid()) continue;
            if(dsvtx.normalisedChiSquared()>5.) continue;
            if(dsvtx.normalisedChiSquared()<0.) continue;

            // set properties of the Ds candidate
            DsMeson_mass.push_back( dsP4.M() );
            DsMeson_pt.push_back( dsP4.pt() );
            DsMeson_eta.push_back( dsP4.eta() );
            DsMeson_phi.push_back( dsP4.phi() );
            DsMeson_PhiMeson_mass.push_back( phiP4.M() );
            DsMeson_PhiMeson_pt.push_back( phiP4.pt() );
            DsMeson_PhiMeson_eta.push_back( phiP4.eta() );
            DsMeson_PhiMeson_phi.push_back( phiP4.phi() );
            DsMeson_PhiMeson_massDiff.push_back( dsP4.M() - phiP4.M() );
            DsMeson_Pi_pt.push_back( piP4.pt() );
            DsMeson_Pi_eta.push_back( piP4.eta() );
            DsMeson_Pi_phi.push_back( piP4.phi() );
            DsMeson_Pi_charge.push_back( tr3.charge() );
            DsMeson_KPlus_pt.push_back( KPlusP4.pt() );
            DsMeson_KPlus_eta.push_back( KPlusP4.eta() );
            DsMeson_KPlus_phi.push_back( KPlusP4.phi() );
            DsMeson_KPlus_charge.push_back( postrack.charge() );
            DsMeson_KMinus_pt.push_back( KMinusP4.pt() );
            DsMeson_KMinus_eta.push_back( KMinusP4.eta() );
            DsMeson_KMinus_phi.push_back( KMinusP4.phi() );
            DsMeson_KMinus_charge.push_back( negtrack.charge() );
            DsMeson_tr1tr2_deltaR.push_back( reco::deltaR(tr1, tr2) );
            DsMeson_tr3phi_deltaR.push_back( reco::deltaR(tr3, phiP4) );
            DsMeson_phivtx_normchi2.push_back( phivtx.normalisedChiSquared() );
            DsMeson_dsvtx_normchi2.push_back( dsvtx.normalisedChiSquared() );
            DsMeson_tr1tr2_sepx.push_back( twotracksepx );
            DsMeson_tr1tr2_sepy.push_back( twotracksepy );
            DsMeson_tr1tr2_sepz.push_back( twotracksepz );
            DsMeson_tr3phi_sepx.push_back( trackvtxsepx );
            DsMeson_tr3phi_sepy.push_back( trackvtxsepy );
            DsMeson_tr3phi_sepz.push_back( trackvtxsepz );

            // check if this candidate can be matched to gen-level
            bool hasFastGenMatch = false;
            bool hasFastPartialGenMatch = false;
            if( doMatching ){
                for( const auto& pmap : DsGenParticles){
                    double dRThreshold = 0.05;
                    if( GenTools::isGeometricTrackMatch( tr3, *pmap.at("Pi"), dRThreshold )
                        && GenTools::isGeometricTrackMatch( postrack, *pmap.at("KPlus"), dRThreshold )
                        && GenTools::isGeometricTrackMatch( negtrack, *pmap.at("KMinus"), dRThreshold ) ){
                        hasFastGenMatch = true;
                    }
                    if( GenTools::isGeometricTrackMatch( tr3, *pmap.at("Pi"), dRThreshold )
                        || GenTools::isGeometricTrackMatch( postrack, *pmap.at("KPlus"), dRThreshold )
                        || GenTools::isGeometricTrackMatch( negtrack, *pmap.at("KMinus"), dRThreshold ) ){
                        hasFastPartialGenMatch = true;
                    }
                }
            }
            DsMeson_hasFastGenMatch.push_back( hasFastGenMatch );
            DsMeson_hasFastPartialGenMatch.push_back( hasFastPartialGenMatch );

            // break loop over third track in case maximum number was reached
            if( DsMeson_mass.size() == nDsMeson_max ) break;

        } // end loop over third track
        if( DsMeson_mass.size() == nDsMeson_max ) break;
      }
      if( DsMeson_mass.size() == nDsMeson_max) break;
    } // end loop over first and second track
    delete bfield;

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(DsMeson_mass.size(), name, false);
    table->addColumn<float>("mass", DsMeson_mass, "");
    table->addColumn<float>("pt", DsMeson_pt, "");
    table->addColumn<float>("eta", DsMeson_eta, "");
    table->addColumn<float>("phi", DsMeson_phi, "");
    table->addColumn<float>("PhiMeson_mass", DsMeson_PhiMeson_mass, "");
    table->addColumn<float>("PhiMeson_pt", DsMeson_PhiMeson_pt, "");
    table->addColumn<float>("PhiMeson_eta", DsMeson_PhiMeson_eta, "");
    table->addColumn<float>("PhiMeson_phi", DsMeson_PhiMeson_phi, "");
    table->addColumn<float>("PhiMeson_massDiff", DsMeson_PhiMeson_massDiff, "");
    table->addColumn<float>("Pi_pt", DsMeson_Pi_pt, "");
    table->addColumn<float>("Pi_eta", DsMeson_Pi_eta, "");
    table->addColumn<float>("Pi_phi", DsMeson_Pi_phi, "");
    table->addColumn<int>("Pi_charge", DsMeson_Pi_charge, "");
    table->addColumn<float>("KPlus_pt", DsMeson_KPlus_pt, "");
    table->addColumn<float>("KPlus_eta", DsMeson_KPlus_eta, "");
    table->addColumn<float>("KPlus_phi", DsMeson_KPlus_phi, "");
    table->addColumn<int>("KPlus_charge", DsMeson_KPlus_charge, "");
    table->addColumn<float>("KMinus_pt", DsMeson_KMinus_pt, "");
    table->addColumn<float>("KMinus_eta", DsMeson_KMinus_eta, "");
    table->addColumn<float>("KMinus_phi", DsMeson_KMinus_phi, "");
    table->addColumn<int>("KMinus_charge", DsMeson_KMinus_charge, "");
    table->addColumn<float>("tr1tr2_deltaR", DsMeson_tr1tr2_deltaR, "");
    table->addColumn<float>("tr3phi_deltaR", DsMeson_tr3phi_deltaR, "");
    table->addColumn<float>("phivtx_normchi2", DsMeson_phivtx_normchi2, "");
    table->addColumn<float>("dsvtx_normchi2", DsMeson_dsvtx_normchi2, "");
    table->addColumn<float>("tr1tr2_sepx", DsMeson_tr1tr2_sepx, "");
    table->addColumn<float>("tr1tr2_sepy", DsMeson_tr1tr2_sepy, "");
    table->addColumn<float>("tr1tr2_sepz", DsMeson_tr1tr2_sepz, "");
    table->addColumn<float>("tr3phi_sepx", DsMeson_tr3phi_sepx, "");
    table->addColumn<float>("tr3phi_sepy", DsMeson_tr3phi_sepy, "");
    table->addColumn<float>("tr3phi_sepz", DsMeson_tr3phi_sepz, "");
    table->addColumn<bool>("hasFastGenmatch", DsMeson_hasFastGenMatch, "");
    table->addColumn<bool>("hasFastPartialGenmatch", DsMeson_hasFastPartialGenMatch, "");

    // add the table to the output
    iEvent.put(std::move(table), name);
}

// define this as a plug-in
DEFINE_FWK_MODULE(DsMesonProducer);
