import FWCore.ParameterSet.Config as cms

from Configuration.ProcessModifiers.trackingMkFitCommon_cff import *
from Configuration.ProcessModifiers.trackingMkFitInitialStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitLowPtQuadStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitHighPtTripletStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitLowPtTripletStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitDetachedQuadStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitDetachedTripletStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitMixedTripletStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitPixelLessStep_cff import *
from Configuration.ProcessModifiers.trackingMkFitTobTecStep_cff import *

# Use mkFit in all iterations
trackingMkFit = cms.ModifierChain(
    trackingMkFitCommon,
    trackingMkFitInitialStep,
    trackingMkFitLowPtQuadStep,
    trackingMkFitHighPtTripletStep,
    trackingMkFitLowPtTripletStep,
    trackingMkFitDetachedQuadStep,
    trackingMkFitDetachedTripletStep,
    trackingMkFitMixedTripletStep,
    trackingMkFitPixelLessStep,
    trackingMkFitTobTecStep,
)
