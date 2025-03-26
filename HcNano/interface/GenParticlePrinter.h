/*
Simple analyzer to print some gen particle info.
For testing and debugging, not meant to be used in production.
*/

#ifndef GEN_PARTICLE_PRINTER_H
#define GEN_PARTICLE_PRINTER_H

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
#include "FWCore/Utilities/interface/InputTag.h"

// dataformats include files
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"

// local include files
#include "PhysicsTools/HcNano/interface/GenTools.h"


class GenParticlePrinter : public edm::stream::EDProducer<> {
  private:

    // attributes and variables
    const std::string name;

    // template member functions
    void produce(edm::Event&, const edm::EventSetup&) override;

    // tokens
    edm::EDGetTokenT<reco::GenParticleCollection> genParticlesToken;

  public:
    // constructor, destructor, and other meta-functions
    GenParticlePrinter(const edm::ParameterSet&);
    ~GenParticlePrinter() override;
    static void fillDescriptions(edm::ConfigurationDescriptions&);
};

#endif
