/*
Tools for working with gen-level particles and decay chains
*/

#ifndef GenTools_H
#define GenTools_H

#include <set>

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "TLorentzVector.h"
#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>

namespace GenTools{

    // find mother particle
    const reco::GenParticle* getFirstMother(const reco::GenParticle&, const std::vector<reco::GenParticle>&);
    const int getFirstMotherIndex(const reco::GenParticle&, const std::vector<reco::GenParticle>&);
    const reco::GenParticle* getMother(const reco::GenParticle&, const std::vector<reco::GenParticle>&);
    int getMotherPdgId(const reco::GenParticle&, const std::vector<reco::GenParticle>&);

    // geometric matching
    const reco::GenParticle* geometricMatch(
        const reco::Candidate& reco,
        const std::vector<reco::GenParticle>& genParticles,
        const bool differentId=false);
    bool considerForMatching(
        const reco::Candidate& reco,
        const reco::GenParticle& gen,
        const bool differentId);
    const reco::GenParticle* geometricTrackMatch(
        const reco::Track&,
        const std::vector<reco::GenParticle>&, int, double);
    const bool isGeometricTrackMatch(
        const reco::Track&,
        const reco::GenParticle&, double);
    const bool isGeometricGenParticleMatch(
        const reco::GenParticle&,
        const reco::GenParticle&, double);
}

#endif
