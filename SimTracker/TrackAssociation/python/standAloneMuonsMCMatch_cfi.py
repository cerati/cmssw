import FWCore.ParameterSet.Config as cms

standAloneMuonsMCMatch = cms.EDFilter("MCTrackMatcher",
    trackingParticles = cms.InputTag("trackingtruthprod"),
    tracks = cms.InputTag("standAloneMuons"),
    genParticles = cms.InputTag("genParticles"),
    associator = cms.string('TrackAssociatorByHits')
)


