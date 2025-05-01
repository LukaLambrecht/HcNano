/*
Custom analyzer class for debugging (temp)
*/

#ifndef Dbugger_H
#define Dbugger_H

// system include files
#include <memory>
#include <unordered_map>

// root classes
#include <Math/Vector4D.h>

// general include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// vertex fitter include files
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/ParametrizedEngine/src/OAEParametrizedMagneticField.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"

// data format include files
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/PatCandidates/interface/PATObject.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidateFwd.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"

// nanoaod include files
#include "DataFormats/NanoAOD/interface/FlatTable.h"

// local include files
#include "PhysicsTools/HcNano/interface/GenTools.h"
#include "PhysicsTools/HcNano/interface/DsMesonGenProducer.h"


class Dbugger : public edm::stream::EDProducer<> {
  private:

    // constants
    static constexpr double pimass = 0.13957;
    static constexpr double pimass2 = pimass*pimass;
    static constexpr double kmass = 0.493677;
    static constexpr double kmass2 = kmass*kmass;
    static constexpr double phimass = 1.019461;
    static constexpr double dsmass = 1.96847;

    // attributes and variables
    const std::string name;
    const std::string dtype;

    // template member functions
    void produce(edm::Event&, const edm::EventSetup&) override;

    // helper functions

    // tokens
    edm::EDGetTokenT<std::vector<pat::PackedCandidate>> packedPFCandidatesToken;
    edm::EDGetTokenT<std::vector<pat::PackedCandidate>> lostTracksToken;
    edm::EDGetTokenT<std::vector<reco::GenParticle>> genParticlesToken;

  public:
    // constructor, destructor, and other meta-functions
    explicit Dbugger(const edm::ParameterSet&);
    ~Dbugger() override;
    static void fillDescriptions(edm::ConfigurationDescriptions&);
};

#endif
