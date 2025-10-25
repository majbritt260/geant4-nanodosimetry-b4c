//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// #### Collective hits inside calorimeter --> gets single hit information from CalorHit.cc ###
//
/// \file CalorimeterSD.cc
/// \brief Implementation of the B4c::CalorimeterSD class

#include "CalorimeterSD.hh"
#include "G4HCofThisEvent.hh" // Handles hit colelctiosn for an event
#include "G4Step.hh" // Stores information about a particle step
#include "G4ThreeVector.hh" // Defines 3D vectors
#include "G4SDManager.hh" // Manages all sensitive detectors
#include "G4ios.hh" // Provides input/output functionalities
#include "G4VProcess.hh"

namespace B4c
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CalorimeterSD::CalorimeterSD(const G4String& name,		// name of sensitive detector
                             const G4String& hitsCollectionName,// name for storing hit data
                             G4int nofCells) 			// no. of cells/layers
 : G4VSensitiveDetector(name), fNofCells(nofCells) 		// registers the hits collection name
{
  collectionName.insert(hitsCollectionName);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CalorimeterSD::~CalorimeterSD()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CalorimeterSD::Initialize(G4HCofThisEvent* hce) // called at beginning of each event to set up the hit collection
{
  // Create hits collection to store hits
  fHitsCollection
    = new CalorHitsCollection(SensitiveDetectorName, collectionName[0]);

  // Add this collection in hce
  auto hcID
    = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection( hcID, fHitsCollection );

  // Create hits
  // fNofCells for cells + one more for total sums
  for (G4int i=0; i<fNofCells+1; i++ ) {
    fHitsCollection->insert(new CalorHit());
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


// Funciton for handling energy depositon and step tracking in the SD
G4bool CalorimeterSD::ProcessHits(G4Step* step,
                                     G4TouchableHistory*)
{
  // Energy deposit
  auto edep = step->GetTotalEnergyDeposit();

  // Step length --> only charged particles have meaningful step-length data
  G4double stepLength = 0.;
  if ( step->GetTrack()->GetDefinition()->GetPDGCharge() != 0. ) {
    stepLength = step->GetStepLength();
  }


////////////////// IONIZATION COUNTER /////////////////
G4int nIon = 0;

if (step->GetPreStepPoint()->GetPhysicalVolume()->GetName() == "SensitiveDetector") {
    const auto secondaries = step->GetSecondaryInCurrentStep();
    for (const auto& sec : *secondaries) {
        if (sec->GetCreatorProcess() &&
            sec->GetCreatorProcess()->GetProcessName().find("Ioni") != std::string::npos &&
            sec->GetDefinition()->GetParticleName() == "e-") {
            nIon++;
        }
    }
}



/*G4int nIon = 0;
if (step->GetPreStepPoint()->GetPhysicalVolume()->GetName() == "SensitiveDetector") {
    if (step->GetTotalEnergyDeposit() != 0) {
      G4String process_name = step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();

      auto pos = process_name.find("Ioni"); 	// looking for "Ioni" substring in process_name
      if (pos != std::string::npos) {  		// found!

 const auto secondaries = step->GetSecondaryInCurrentStep();
  for (const auto& sec : *secondaries) {
      if (sec->GetDefinition()->GetParticleName() == "e-"){

        nIon++;} 				// increase cluster size
    }
  }

}
}
*/
/*

////////////////// CHECK FOR MULTIPLE SPECIFIC PROCESSES ////////////////
G4int nIon = 0;
const auto secondaries = step->GetSecondaryInCurrentStep();
for (const auto& sec : *secondaries) {
    const auto* creator = sec->GetCreatorProcess();
    if (creator) {
        const G4String& pname = creator->GetProcessName();
        const bool eIoni = pname == "e-_G4DNAIonisation";
        const bool hIoni = pname == "proton_G4DNAIonisation";
        if (eIoni || hIoni) {
            nIon++;
        }
    }
}
*/

/*
///////////////// CHECK FOR SINGULAR SPECIFIC PROCESS //////////////////
  // Ionization yield
  // Count ionizations (based on secondary electron production)
  G4int nIon = 0;
  const auto secondaries = step->GetSecondaryInCurrentStep();
  for (const auto& sec : *secondaries) {
     if (sec->GetCreatorProcess()->GetProcessName() == "e-_G4DNAIonisation") {nIon++;}
  }

*/
// auto:         work with copy
// auto&:        work with original
// const:        prevents modifications

/*
///////////////// CHECK FOR ALL IONIZATION PROCESSES //////////////////
  G4int nIon = 0;
  const auto secondaries = step->GetSecondaryInCurrentStep();			// Get list of secondary particles in current step
  for (const auto& sec : *secondaries) {					// Loop over  secondaries (by reference, read-only)
      if (sec->GetDefinition()->GetParticleName() == "e-") {			// Check if secondary is electron
          const G4VProcess* creator = sec->GetCreatorProcess();			// Get process that created electron
          if (creator) {							// Not all particles have creator process, i.e. primary electrons should not be counted
              const auto& pname = creator->GetProcessName();

    G4cout << "[Secondary] "
           << "Created by process: " << creator->GetProcessName()
           << " | Particle: " << sec->GetDefinition()->GetParticleName()
           << " | Position: " << sec->GetPosition()
           << G4endl;

			// Get creator process name
              if (pname.find("Ioni") != std::string::npos) {			// .find() returns npos if "Ioni" is not found in pname
                 //const auto* volume = sec->GetTouchableHandle()->GetVolume();   // Make sure the ionization happened in the SD --> unnecessary, we only count inside the SD to begin with???
    		 //if (volume && volume->GetName() == "SensitiveDetector") {
        	   nIon++;


              }
          }
      }
  }
*/
  // Ignore steps with no energy loss and no movement --> avoids unnecessary calculations
  if ( edep==0. && stepLength == 0. ) return false;

  // Get calorimeter cell when the hit occured
  auto touchable = (step->GetPreStepPoint()->GetTouchable());
  auto layerNumber = touchable->GetReplicaNumber(1);

  // Get hit accounting data for this cell
  auto hit = (*fHitsCollection)[layerNumber];
  if ( ! hit ) { // Check if the hit object for this specific calorimeter layer exists
    G4ExceptionDescription msg;
    msg << "Cannot access hit " << layerNumber;
    G4Exception("CalorimeterSD::ProcessHits()",
      "MyCode0004", FatalException, msg);
  }

  // Get hit for total accounting --> accesses last element in the fHitsCollections which stores total accumulated data for ALL layers
  auto hitTotal
    = (*fHitsCollection)[fHitsCollection->entries()-1];

  // Record energy deposition and step length into the hit objects
  hit->Add(edep, stepLength, nIon);
  hitTotal->Add(edep, stepLength, nIon);

  return true; // Indicate that a valid hit was recorded
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CalorimeterSD::EndOfEvent(G4HCofThisEvent*)
{
  if ( verboseLevel>0 /*1*/ ) {
     auto nofHits = fHitsCollection->entries();
     G4cout
       << G4endl
       << "-------->Hits Collection: in this event there are " << nofHits
       << " hits in the tracker chambers: " << G4endl;
     for ( std::size_t i=0; i<nofHits; ++i ) (*fHitsCollection)[i]->Print();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
