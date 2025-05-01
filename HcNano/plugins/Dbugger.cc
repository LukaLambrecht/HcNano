/*
Custom analyzer class for debugging (temp)
*/

// local include files
#include "PhysicsTools/HcNano/interface/Dbugger.h"

// constructor //
Dbugger::Dbugger(const edm::ParameterSet& iConfig)
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
Dbugger::~Dbugger(){}

// descriptions //
void Dbugger::fillDescriptions(edm::ConfigurationDescriptions &descriptions){
    edm::ParameterSetDescription desc;
    desc.add<std::string>("name", "Name for output table");
    desc.add<std::string>("dtype", "Data type (mc or data)");
    desc.add<edm::InputTag>("packedPFCandidatesToken", edm::InputTag("packedPFCandidatesToken"));
    desc.add<edm::InputTag>("lostTracksToken", edm::InputTag("lostTracksToken"));
    desc.add<edm::InputTag>("genParticlesToken", edm::InputTag("genParticlesToken"));
    descriptions.addWithDefaultLabel(desc);
}

// produce (main method) //
void Dbugger::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

    // get all required objects from tokens
    edm::Handle<std::vector<pat::PackedCandidate>> packedPFCandidates;
    iEvent.getByToken(packedPFCandidatesToken, packedPFCandidates);
    edm::Handle<std::vector<pat::PackedCandidate>> lostTracks;
    iEvent.getByToken(lostTracksToken, lostTracks);
    MagneticField* bfield = new OAEParametrizedMagneticField("3_8T");

    // settings for gen-matching
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    iEvent.getByToken(genParticlesToken, genParticles);
    bool doMatching = false;

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

    // initializations
    int osCounterBeforeSelections = 0;
    int ssCounterBeforeSelections = 0;
    int osCounter1 = 0;
    int ssCounter1 = 0;
    int osCounter2 = 0;
    int ssCounter2 = 0;
    int osCounter3 = 0;
    int ssCounter3 = 0;
    int osCounter4 = 0;
    int ssCounter4 = 0;
    int osCounter5 = 0;
    int ssCounter5 = 0;
    int osCounter6 = 0;
    int ssCounter6 = 0;
    int osCounter7 = 0;
    int ssCounter7 = 0;
    int osCounterAfterSelections = 0;
    int ssCounterAfterSelections = 0;

    // loop over pairs of tracks
    for(unsigned i=0; i<selectedTracks.size(); i++){
      for(unsigned j=i+1; j<selectedTracks.size(); j++){
        const reco::Track tr1 = selectedTracks.at(i);
        const reco::Track tr2 = selectedTracks.at(j);

        if(tr1.charge() * tr2.charge() > 0) ssCounterBeforeSelections++;
        else osCounterBeforeSelections++;

        // candidates must point approximately in the same direction
        if( reco::deltaR(tr1, tr2) > 0.27 ) continue;

        // candidates must have pT greater than certain value
        if(tr1.pt() < 0.6 or tr2.pt() < 0.6) continue;

        if(tr1.charge() * tr2.charge() > 0) ssCounter1++;
        else osCounter1++;

        // reference points of both tracks must be close together
        const math::XYZPoint tr1refpoint = tr1.referencePoint();
        const math::XYZPoint tr2refpoint = tr2.referencePoint();
        double twotracksepx = std::abs(tr1refpoint.x()-tr2refpoint.x());
        double twotracksepy = std::abs(tr1refpoint.y()-tr2refpoint.y());
        double twotracksepz = std::abs(tr1refpoint.z()-tr2refpoint.z());
        if( twotracksepx>0.1 || twotracksepy>0.1 || twotracksepz>0.1 ) continue;

        if(tr1.charge() * tr2.charge() > 0) ssCounter2++;
        else osCounter2++;

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

        if(tr1.charge() * tr2.charge() > 0) ssCounter3++;
        else osCounter3++;

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

        if(tr1.charge() * tr2.charge() > 0) ssCounter4++;
        else osCounter4++;

        // loop over third track
        for(unsigned k=0; k<selectedTracks.size(); k++){
            if(k==i or k==j) continue;
            const reco::Track tr3 = selectedTracks.at(k);

            // candidates must point approximately in the same direction
            if( reco::deltaR(tr3, phiP4) > 0.4 ) continue;

            if(tr1.charge() * tr2.charge() > 0) ssCounter5++;
            else osCounter5++;

            // reference point of third track must be close to phi vertex
            const math::XYZPoint tr3refpoint = tr3.referencePoint();
            double trackvtxsepx = std::abs(tr3refpoint.x()-phivtx.position().x());
            double trackvtxsepy = std::abs(tr3refpoint.y()-phivtx.position().y());
            double trackvtxsepz = std::abs(tr3refpoint.z()-phivtx.position().z());
            if( trackvtxsepx>0.1 || trackvtxsepy>0.1 || trackvtxsepz>0.1 ) continue;

            if(tr1.charge() * tr2.charge() > 0) ssCounter6++;
            else osCounter6++;

            // make invariant mass (under the assumption of pi mass for the third track)
            ROOT::Math::PtEtaPhiMVector piP4(tr3.pt(), tr3.eta(), tr3.phi(), pimass);
            ROOT::Math::PtEtaPhiMVector dsP4 = phiP4 + piP4;
            double dsInvMass = dsP4.M();

            // check if mass is close enough to Ds mass
            if(std::abs(dsInvMass - dsmass) > 0.1) continue;

            if(tr1.charge() * tr2.charge() > 0) ssCounter7++;
            else osCounter7++;

            // do a vertex fit
            std::vector<reco::TransientTrack> transtriplet;
            transtriplet.push_back(reco::TransientTrack(tr1, bfield));
            transtriplet.push_back(reco::TransientTrack(tr2, bfield));
            transtriplet.push_back(reco::TransientTrack(tr3, bfield));
            TransientVertex dsvtx = vtxFitter.vertex(transtriplet);
            if(!dsvtx.isValid()) continue;
            if(dsvtx.normalisedChiSquared()>5.) continue;
            if(dsvtx.normalisedChiSquared()<0.) continue;
 
            if(tr1.charge() * tr2.charge() > 0) ssCounterAfterSelections++;
            else osCounterAfterSelections++;
        }
      }
    } // end loop over first and second track
    delete bfield;

    // make the table
    auto table = std::make_unique<nanoaod::FlatTable>(1, name, true);
    table->addColumnValue<int>("osCounterBeforeSelections", osCounterBeforeSelections, "");
    table->addColumnValue<int>("ssCounterBeforeSelections", ssCounterBeforeSelections, "");
    table->addColumnValue<int>("osCounter1", osCounter1, "");
    table->addColumnValue<int>("ssCounter1", ssCounter1, "");
    table->addColumnValue<int>("osCounter2", osCounter2, "");
    table->addColumnValue<int>("ssCounter2", ssCounter2, "");
    table->addColumnValue<int>("osCounter3", osCounter3, "");
    table->addColumnValue<int>("ssCounter3", ssCounter3, "");
    table->addColumnValue<int>("osCounter4", osCounter4, "");
    table->addColumnValue<int>("ssCounter4", ssCounter4, "");
    table->addColumnValue<int>("osCounter5", osCounter5, "");
    table->addColumnValue<int>("ssCounter5", ssCounter5, "");
    table->addColumnValue<int>("osCounter6", osCounter6, "");
    table->addColumnValue<int>("ssCounter6", ssCounter6, "");
    table->addColumnValue<int>("osCounter7", osCounter7, "");
    table->addColumnValue<int>("ssCounter7", ssCounter7, "");
    table->addColumnValue<int>("osCounterAfterSelections", osCounterAfterSelections, "");
    table->addColumnValue<int>("ssCounterAfterSelections", ssCounterAfterSelections, "");
    
    // add the table to the output
    iEvent.put(std::move(table), name);
}

// define this as a plug-in
DEFINE_FWK_MODULE(Dbugger);
