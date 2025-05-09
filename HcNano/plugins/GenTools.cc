/*
Tools for working with gen-level particles and decay chains
*/

#include "PhysicsTools/HcNano/interface/GenTools.h"

const reco::GenParticle* GenTools::getFirstMother(
        const reco::GenParticle& gen,
        const std::vector<reco::GenParticle>& genParticles){
    if(gen.numberOfMothers() == 0) return nullptr;
    return &genParticles[gen.motherRef(0).key()];
}

const int GenTools::getFirstMotherIndex(
        const reco::GenParticle& gen,
        const std::vector<reco::GenParticle>& genParticles){
    if(gen.numberOfMothers() == 0) return -1;
    return gen.motherRef(0).key();
}

const reco::GenParticle* GenTools::getMother(
        const reco::GenParticle& gen,
        const std::vector<reco::GenParticle>& genParticles){
    const reco::GenParticle* mom = getFirstMother(gen, genParticles);
    if(!mom) return nullptr;
    else if(mom->pdgId() == gen.pdgId()) return getMother(*mom, genParticles);
    else return mom;
}

int GenTools::getMotherPdgId(
        const reco::GenParticle& gen,
        const std::vector<reco::GenParticle>& genParticles){
    const reco::GenParticle* mom = getMother(gen, genParticles);
    if(!mom) return 0;
    return mom->pdgId();
}

std::vector<reco::GenParticle> GenTools::getQuarkDaughters(
        const reco::GenParticle& quark,
        const std::vector<reco::GenParticle>& genParticles){
    std::vector<reco::GenParticle> res;
    int quarkPdgId = quark.pdgId();
    
    // loop over daughters
    for(unsigned int i=0; i < quark.numberOfDaughters(); ++i){
        const reco::GenParticle daughter = genParticles[quark.daughterRef(i).key()];
        int daughterPdgId = daughter.pdgId();
        
        // if original quark is not last copy,
        // only look for the next copy and go recursively
        // (skipping all potential other daughters)
        if( !quark.isLastCopy() ){
            if( daughterPdgId != quarkPdgId ) continue;
            for(const reco::GenParticle& d : getQuarkDaughters(daughter, genParticles)){
                res.push_back(d);
            }
        }

        // if original quark is last copy, add all daughters
        // note: sometimes it seems there are still copies making it to this stage,
        // so need to add an extra check to skip them explicitly.
        else{
            bool duplicate = false;
            for(const reco::GenParticle& check : res){
                if( daughter.pdgId()==check.pdgId()
                    && GenTools::isGeometricGenParticleMatch(daughter, check, 0.05) ){
                    duplicate = true;
                }
            }
            if( !duplicate ) res.push_back(daughter);
        }
    }
    return res;
}

std::vector<reco::GenParticle> GenTools::getQuarkPairDaughters(
        const reco::GenParticle& quark1,
        const reco::GenParticle& quark2,
        const std::vector<reco::GenParticle>& genParticles){
    // get daughters of a quark-antiquark pair.
    // the issue is that sometimes the daughters of both quarks
    // are stored for each of both quarks (and sometimes not),
    // so need to remove duplicates.

    std::vector<reco::GenParticle> res;
    for(const reco::GenParticle& d : getQuarkDaughters(quark1, genParticles)){
        res.push_back(d);
    }
    for(const reco::GenParticle& d : getQuarkDaughters(quark2, genParticles)){
        bool duplicate = false;
        for(const reco::GenParticle& check : res){
            if( d.pdgId()==check.pdgId() && GenTools::isGeometricGenParticleMatch(d, check, 0.05) ){
                duplicate = true;
            }
        }
        if( !duplicate ) res.push_back(d);
    }
    return res;
}

bool GenTools::considerForMatching(
        const reco::Candidate& reco,
        const reco::GenParticle& gen,
        const bool differentId){
    if(abs(gen.pdgId()) != 22 or !differentId){
      if(abs(reco.pdgId()) != abs(gen.pdgId())) return false;
    }
    if(abs(reco.pdgId()) == 15 && abs(gen.pdgId()) == 15) return gen.status() == 2 && gen.isLastCopy();
    return gen.status() == 1;
}

const reco::GenParticle* GenTools::geometricMatch(
        const reco::Candidate& reco,
        const std::vector<reco::GenParticle>& genParticles,
        const bool differentId){
    reco::GenParticle const* match = nullptr;
    TLorentzVector recoV(reco.px(), reco.py(), reco.pz(), reco.energy());
    double minDeltaR = 99999.;
    for(auto genIt = genParticles.cbegin(); genIt != genParticles.cend(); ++genIt){
        if(considerForMatching(reco, *genIt, differentId) ){
            TLorentzVector genV(genIt->px(), genIt->py(), genIt->pz(), genIt->energy());
            double deltaR = recoV.DeltaR(genV);
            if(deltaR < minDeltaR){
                minDeltaR = deltaR;
                match = &*genIt;
            }
        }
    }
    if(minDeltaR > 0.2){
      if(!differentId) match = geometricMatch(reco, genParticles, true);
      else             return nullptr;
    }
    return match;
}

const reco::GenParticle* GenTools::geometricTrackMatch(
        const reco::Track& tr,
        const std::vector<reco::GenParticle>& genParticles,
        int abspdgid,
        double deltaRThreshold ){
    // find the best geometric match with given pdg id for a track
    const reco::GenParticle* match = nullptr;
    ROOT::Math::PtEtaPhiMVector trvec( tr.pt(), tr.eta(), tr.phi(), 0.0 );
    double minDeltaR = 99999.;
    //double matchEta = 0.; // only used for debugging
    //double matchPhi = 0.; // only used for debugging
    for(auto genIt = genParticles.cbegin(); genIt!=genParticles.cend(); ++genIt){
        if( std::abs(genIt->pdgId())!=abspdgid ) continue;
        ROOT::Math::PtEtaPhiMVector gpvec( genIt->pt(), genIt->eta(), genIt->phi(), 0.0 );
        double deltaR = ROOT::Math::VectorUtil::DeltaR( trvec, gpvec );
        if(deltaR < minDeltaR){
            minDeltaR = deltaR;
            //matchEta = genIt->eta();
            //matchPhi = genIt->phi();
            match = &*genIt;
        }
    }
    if(minDeltaR > deltaRThreshold) return nullptr;
    // printouts for testing
    /*std::cout << "results from geometricTrackMatch:" << std::endl;
    std::cout << "match eta: " << matchEta << std::endl;
    std::cout << "match phi: " << matchPhi << std::endl;*/
    return match;
}

const bool GenTools::isGeometricTrackMatch(
        const reco::Track& tr,
        const reco::GenParticle& gp,
        double deltaRThreshold ){
    // decide whether a given track matches a given gen particle
    ROOT::Math::PtEtaPhiMVector trvec( tr.pt(), tr.eta(), tr.phi(), 0.0 );
    ROOT::Math::PtEtaPhiMVector gpvec( gp.pt(), gp.eta(), gp.phi(), 0.0 );
    double deltaR = ROOT::Math::VectorUtil::DeltaR( trvec, gpvec );
    if(deltaR < deltaRThreshold) return true;
    else return false;
}

const bool GenTools::isGeometricGenParticleMatch(
        const reco::GenParticle& gp1,
        const reco::GenParticle& gp2,
        double deltaRThreshold ){
    // decide whether a given gen particle matches another gen particle
    ROOT::Math::PtEtaPhiMVector gp1vec( gp1.pt(), gp1.eta(), gp1.phi(), 0.0 );
    ROOT::Math::PtEtaPhiMVector gp2vec( gp2.pt(), gp2.eta(), gp2.phi(), 0.0 );
    double deltaR = ROOT::Math::VectorUtil::DeltaR( gp1vec, gp2vec );
    if(deltaR < deltaRThreshold) return true;
    else return false;
}
