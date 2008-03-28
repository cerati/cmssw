import FWCore.ParameterSet.Config as cms

from MagneticField.Engine.volumeBasedMagneticField_cfi import *
from Geometry.CMSCommonData.cmsIdealGeometryXML_cfi import *
from Geometry.TrackerGeometryBuilder.trackerGeometry_cfi import *
from Geometry.TrackerNumberingBuilder.trackerNumberingGeometry_cfi import *
from RecoTracker.GeometryESProducer.TrackerRecoGeometryESProducer_cfi import *
from RecoLocalTracker.SiStripRecHitConverter.StripCPEfromTrackAngle_cfi import *
from RecoLocalTracker.SiStripRecHitConverter.SiStripRecHitMatcher_cfi import *
from RecoLocalTracker.SiPixelRecHits.PixelCPEParmError_cfi import *
from RecoTracker.TransientTrackingRecHit.TransientTrackingRecHitBuilder_cfi import *
from RecoTracker.MeasurementDet.MeasurementTrackerESProducer_cfi import *
from TrackingTools.MaterialEffects.MaterialPropagator_cfi import *
import copy
from RecoTracker.TransientTrackingRecHit.TransientTrackingRecHitBuilder_cfi import *
pixelTTRHBuilderWithoutAngle = copy.deepcopy(ttrhbwr)
from RecoTracker.TkSeedingLayers.TTRHBuilderWithoutAngle4PixelTriplets_cfi import *
from RecoTracker.TkSeedingLayers.PixelLayerTriplets_cfi import *
from RecoPixelVertexing.PixelTrackFitting.PixelTracks_cfi import *
from RecoTracker.TkSeedGenerator.SeedGeneratorFromProtoTracksEDProducer_cfi import *
pixelTTRHBuilderWithoutAngle.StripCPE = 'Fake'
pixelTTRHBuilderWithoutAngle.ComponentName = 'PixelTTRHBuilderWithoutAngle'

