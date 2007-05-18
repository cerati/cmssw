//Framework Headers
#include "FWCore/ParameterSet/interface/ParameterSet.h"

//TrackingTools Headers
#include "TrackingTools/PatternTools/interface/MediumProperties.h"

// Famos Headers
#include "FastSimulation/Event/interface/FSimEvent.h"
#include "FastSimulation/Event/interface/FSimTrack.h"
#include "FastSimulation/ParticlePropagator/interface/ParticlePropagator.h"
#include "FastSimulation/TrackerSetup/interface/TrackerLayer.h"
#include "FastSimulation/MaterialEffects/interface/MaterialEffects.h"

#include "FastSimulation/MaterialEffects/interface/PairProductionSimulator.h"
#include "FastSimulation/MaterialEffects/interface/MultipleScatteringSimulator.h"
#include "FastSimulation/MaterialEffects/interface/BremsstrahlungSimulator.h"
#include "FastSimulation/MaterialEffects/interface/EnergyLossSimulator.h"
#include "FastSimulation/MaterialEffects/interface/NuclearInteractionSimulator.h"

#include <list>
#include <utility>
#include <iostream>
#include <vector>
#include <string>

MaterialEffects::MaterialEffects(const edm::ParameterSet& matEff,
				 const RandomEngine* engine)
  : PairProduction(0), Bremsstrahlung(0), 
    MultipleScattering(0), EnergyLoss(0), 
    NuclearInteraction(0),
    pTmin(999.), random(engine)
{
  // Set the minimal photon energy for a Brem from e+/-

  bool doPairProduction     = matEff.getParameter<bool>("PairProduction");
  bool doBremsstrahlung     = matEff.getParameter<bool>("Bremsstrahlung");
  bool doEnergyLoss         = matEff.getParameter<bool>("EnergyLoss");
  bool doMultipleScattering = matEff.getParameter<bool>("MultipleScattering");
  bool doNuclearInteraction = matEff.getParameter<bool>("NuclearInteraction");
  
  // Set the minimal pT before giving up the dE/dx treatment

  if ( doPairProduction ) { 

    double photonEnergy = matEff.getParameter<double>("photonEnergy");
    PairProduction = new PairProductionSimulator(photonEnergy,
						 random);

  }

  if ( doBremsstrahlung ) { 

    double bremEnergy = matEff.getParameter<double>("bremEnergy");
    double bremEnergyFraction = matEff.getParameter<double>("bremEnergyFraction");
    Bremsstrahlung = new BremsstrahlungSimulator(bremEnergy,
						 bremEnergyFraction,
						 random);

  }

  if ( doEnergyLoss ) { 

    pTmin = matEff.getParameter<double>("pTmin");
    EnergyLoss = new EnergyLossSimulator(random);

  }

  if ( doMultipleScattering ) { 

    MultipleScattering = new MultipleScatteringSimulator(random);

  }

  if ( doNuclearInteraction ) { 
    std::vector<std::string> listOfFiles 
      = matEff.getUntrackedParameter<std::vector<std::string> >("fileNames");
    std::vector<double> pionEnergies 
      = matEff.getUntrackedParameter<std::vector<double> >("pionEnergies");
    std::vector<double> ratioRatio 
      = matEff.getUntrackedParameter<std::vector<double> >("ratioRatio");
    double pionEnergy 
      = matEff.getParameter<double>("pionEnergy");
    double lengthRatio 
      = matEff.getParameter<double>("lengthRatio");
    std::string inputFile 
      = matEff.getUntrackedParameter<std::string>("inputFile");
    // Construction
    NuclearInteraction = 
      new NuclearInteractionSimulator(listOfFiles,
				      pionEnergies,
				      pionEnergy,
				      lengthRatio,
				      ratioRatio,
				      inputFile,
				      random);
  }

}


MaterialEffects::~MaterialEffects() {

  if ( PairProduction ) delete PairProduction;
  if ( Bremsstrahlung ) delete Bremsstrahlung;
  if ( EnergyLoss ) delete EnergyLoss;
  if ( MultipleScattering ) delete MultipleScattering;
  if ( NuclearInteraction ) delete NuclearInteraction;

}

void MaterialEffects::interact(FSimEvent& mySimEvent, 
			       const TrackerLayer& layer,
			       ParticlePropagator& myTrack,
			       unsigned itrack) {

  MaterialEffectsSimulator::RHEP_const_iter DaughterIter;
  double radlen;
  theEnergyLoss = 0;
  theNormalVector = normalVector(layer,myTrack);
  radlen = radLengths(layer,myTrack);

//-------------------
//  Photon Conversion
//-------------------

  if ( PairProduction && myTrack.pid()==22 ) {
    
    //
    PairProduction->updateState(myTrack,radlen);

    if ( PairProduction->nDaughters() ) {	
      //add a vertex to the mother particle
      int ivertex = mySimEvent.addSimVertex(myTrack.vertex(),itrack);
      
      // This was a photon that converted
      for ( DaughterIter = PairProduction->beginDaughters();
	    DaughterIter != PairProduction->endDaughters(); 
	    ++DaughterIter) {

	mySimEvent.addSimTrack(*DaughterIter, ivertex);

      }
      // The photon converted. Return.
      return;
    }
  }

  if ( myTrack.pid() == 22 ) return;

//------------------------
//   Nuclear interactions
//------------------------ 

  if ( NuclearInteraction && abs(myTrack.pid()) > 100 
                          && abs(myTrack.pid()) < 1000000) { 

    // A few fudge factors ...
    double factor = 1.;

    if ( !layer.sensitive() ) { 
      if ( layer.layerNumber() == 107 ) { 
	double eta = myTrack.vertex().Eta();
	factor = eta > 2.2 ? 1.0 +(eta-2.2)*3.0 : 1.0;
      }	else if ( layer.layerNumber() == 113 ) { 
	double zed = fabs(myTrack.Z());
	factor = zed > 116. ? 0.6 : 1.4;
      } else if ( layer.layerNumber() == 115 ) {
	factor = 0.0;
      }
    }
    
    // Simulate a nuclear interaction
    NuclearInteraction->updateState(myTrack,radlen*factor);

    if ( NuclearInteraction->nDaughters() ) { 

      //add a end vertex to the mother particle
      int ivertex = mySimEvent.addSimVertex(myTrack.vertex(),itrack);
      
      // This was a hadron that interacted inelastically
      for ( DaughterIter = NuclearInteraction->beginDaughters();
	    DaughterIter != NuclearInteraction->endDaughters(); 
	    ++DaughterIter) {

	mySimEvent.addSimTrack(*DaughterIter, ivertex);

      }
      // The hadron is destroyed. Return.
      return;
    }
    
  }

  if ( myTrack.charge() == 0 ) return;

  if ( !Bremsstrahlung && !EnergyLoss && !MultipleScattering ) return;

//----------------
//  Bremsstrahlung
//----------------

  if ( Bremsstrahlung && abs(myTrack.pid())==11 ) {
        
    Bremsstrahlung->updateState(myTrack,radlen);

    if ( Bremsstrahlung->nDaughters() ) {
      
      // Add a vertex, but do not attach it to the electron, because it 
      // continues its way...
      int ivertex = mySimEvent.addSimVertex(myTrack.vertex(),itrack);

      for ( DaughterIter = Bremsstrahlung->beginDaughters();
	    DaughterIter != Bremsstrahlung->endDaughters(); 
	    ++DaughterIter) {
	mySimEvent.addSimTrack(*DaughterIter, ivertex);
      }
      
    }
    
  } 


////--------------
////  Energy loss 
///---------------

  if ( EnergyLoss )
  {
    theEnergyLoss = myTrack.E();
    EnergyLoss->updateState(myTrack,radlen);
    theEnergyLoss -= myTrack.E();
  }
  

////----------------------
////  Multiple scattering
///-----------------------

  if ( MultipleScattering && myTrack.Pt() > pTmin ) {
    //    MultipleScattering->setNormalVector(normalVector(layer,myTrack));
    MultipleScattering->setNormalVector(theNormalVector);
    MultipleScattering->updateState(myTrack,radlen);
  }
    
}

double
MaterialEffects::radLengths(const TrackerLayer& layer,
			    ParticlePropagator& myTrack) {

  // Thickness of layer
  theThickness = layer.surface().mediumProperties()->radLen();

  GlobalVector P(myTrack.Px(),myTrack.Py(),myTrack.Pz());
  
  // Effective length of track inside layer (considering crossing angle)
  //  double radlen = theThickness / fabs(P.dot(theNormalVector)/(P.mag()*theNormalVector.mag()));
  double radlen = theThickness / fabs(P.dot(theNormalVector)) * P.mag();

  // This is disgusting. It should be in the geometry description, by there
  // is no way to define a cylinder with a hole in the middle...
  double rad = myTrack.R();
  double zed = fabs(myTrack.Z());

  double factor = 1;

  if ( rad > 16. && zed < 299. ) {

    // Simulate cables out the TEC's
    if ( zed > 122. && layer.sensitive() ) { 

      if ( zed < 165. ) { 
	if ( rad < 24. ) factor = 3.0;
      } else {
	if ( rad < 32.5 )  factor = 3.0;
	else if ( (zed > 220. && rad < 45.0) || 
		  (zed > 250. && rad < 54.) ) factor = 0.3;
      }
    }

    // Less material on all sensitive layers of the Silicon Tracker
    else if ( zed < 20. && layer.sensitive() ) { 
      if ( rad > 55. ) factor = 0.50;
      else if ( zed < 10 ) factor = 0.77;
    }
    // Much less cables outside the Si Tracker barrel
    else if ( rad > 118. && zed < 250. ) { 
      if ( zed < 116 ) factor = 0.225 * .75 ;
      else factor = .75;
    }
    // No cable whatsoever in the Pixel Barrel.
    else if ( rad < 18. && zed < 26. ) factor = 0.08;
  }

  theThickness *= factor;

  return radlen * factor;

}

GlobalVector
MaterialEffects::normalVector(const TrackerLayer& layer,
			      ParticlePropagator& myTrack ) const {
  return layer.forward() ?  
    layer.disk()->normalVector() :
    GlobalVector(myTrack.X(),myTrack.Y(),0.)/myTrack.R();
}

void 
MaterialEffects::save() { 

  // Save current nuclear interactions in the event libraries.
  if ( NuclearInteraction ) NuclearInteraction->save();

}
