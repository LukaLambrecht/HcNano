/*
Custom analyzer class for investigating gen-level D* meson decays.
*/

#ifndef DStarMesonGenProducer_H
#define DStarMesonGenProducer_H

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

// specific include files
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"

// nanoaod include files
#include "DataFormats/NanoAOD/interface/FlatTable.h"

// local include files
#include "PhysicsTools/HcNano/interface/GenTools.h"


class DStarMesonGenProducer : public edm::stream::EDProducer<> {
  private:

    // attributes and variables
    const std::string name;

    // template member functions
    void produce(edm::Event&, const edm::EventSetup&) override;

    // tokens
    edm::EDGetTokenT<reco::GenParticleCollection> genParticlesToken;

  public:
    // constructor, destructor, and other meta-functions
    explicit DStarMesonGenProducer(const edm::ParameterSet&);
    ~DStarMesonGenProducer() override;
    static void fillDescriptions(edm::ConfigurationDescriptions&);

    // helper functions
    static int find_DStar_decay_type(const std::vector<reco::GenParticle>&);
    static std::vector< std::map< std::string, const reco::GenParticle* > > find_DStar_to_DZeroPi_to_KPiPi(
      const std::vector<reco::GenParticle>&, const bool);
};

#endif
