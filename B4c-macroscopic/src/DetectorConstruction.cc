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
//
/// \file DetectorConstruction.cc
/// \brief Implementation of the B4c::DetectorConstruction class

#include "DetectorConstruction.hh"
#include "CalorimeterSD.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"

#include "G4SDManager.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Box.hh"


namespace B4c
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal
G4GlobalMagFieldMessenger* DetectorConstruction::fMagFieldMessenger = nullptr;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Define materials
  DefineMaterials();

  // Define volumes
  return DefineVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{
  // Define materials using NIST Manager
  auto nistManager = G4NistManager::Instance();
  nistManager->FindOrBuildMaterial("G4_WATER");
  nistManager->FindOrBuildMaterial("G4_LITHIUM_FLUORIDE");
  nistManager->FindOrBuildMaterial("G4_AIR");

  // Print materials
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::DefineVolumes()
{
  // Get materials
  auto water = G4Material::GetMaterial("G4_WATER");
  auto LiF = G4Material::GetMaterial("G4_LITHIUM_FLUORIDE");
  auto air = G4Material::GetMaterial("G4_AIR");

  /////////////// Print state of materials ///////////////
  // Helper function to convert G4State to string
  auto GetStateString = [](G4State state) {
    switch (state) {
      case kStateSolid: return "Solid";
      case kStateLiquid: return "Liquid";
      case kStateGas: return "Gas";
      default: return "Undefined";
    }
  };

  G4cout << "State of LiF: " << GetStateString(LiF->GetState()) << G4endl;
  G4cout << "State of water: " << GetStateString(water->GetState()) << G4endl;
  G4cout << "State of air: " << GetStateString(air->GetState()) << G4endl;


  /////////////// Volume settings ///////////////
  G4double worldRadius = 1 * cm;
  G4double worldHeight = 10 * cm;
  G4double gap = 1 * cm;

  G4double phanRadius = worldRadius;
  G4double phanHeight = worldHeight - gap;
  G4double phan_z = gap/2;

  // Be aware that placement moves with mother volume
  G4double SD_Radius = worldRadius;
  G4double SD_Height = 1 * um;
  [[maybe_unused]] const G4double e = -phanHeight/2 + SD_Height/2; // entrance
  [[maybe_unused]] const G4double tbp = -phanHeight/2 + 77180 * um; // theoretical bragg peak
  G4double SD_z = e;

  auto worldMaterial = air;
  auto phanMaterial = water;
  auto SDMaterial = water;

  // In the following:
  //  - S: solid --> representing the geometric shape
  //  - PV: physical volume --> representing the placed instance
  //  - LV: logical volume


  //
  // World
  //
  auto worldS
    = new G4Tubs("World",		// its name
                 0,			// its inner radius
		 worldRadius,		// its radius
		 worldHeight/2,		// its half height
		 0.*deg,		// its start angle
		 360.*deg);		// is total angle

  auto worldLV
    = new G4LogicalVolume(
                 worldS,           // its solid
                 worldMaterial,    // its material
                 "World");         // its name

  auto worldPV
    = new G4PVPlacement(
                 0,                // no rotation
                 G4ThreeVector(),  // at (0,0,0)
                 worldLV,          // its logical volume
                 "World",          // its name
                 0,                // its mother  volume
                 false,            // no boolean operation
                 0,                // copy number
                 fCheckOverlaps);  // checking overlaps



  //
  // Phantom
  //
  auto phanS
    = new G4Tubs("Phantom",		// its name
                 0,			// its inner radius
                 phanRadius,		// its radius
                 phanHeight/2,		// its half height
                 0.*deg,		// its start angle
                 360.*deg);		// is total angle

  auto phanLV
    = new G4LogicalVolume(
                 phanS,			// its solid
                 phanMaterial,		// its material
                 "Phantom");		// its name


  new G4PVPlacement(
                 0,					// no rotation
                 G4ThreeVector(0, 0, phan_z),		// its location
                 phanLV,				// its logical volume
                 "Phantom",				// its name
                 worldLV,				// its mother  volume
                 false,					// no boolean operation
                 0,					// copy number
                 fCheckOverlaps);			// checking overlaps


  //
  // Sensitive Detector
  //

  auto SensitiveDetectorS
	= new G4Tubs("SensitiveDetector",		// its name
                 0,             			// its inner radius
                 SD_Radius,          			// its radius
                 SD_Height/2,         			// its half height
                 0.*deg,                		// its start angle
                 360.*deg);             		// is total angle

  auto SensitiveDetectorLV
	= new G4LogicalVolume(
			SensitiveDetectorS,	// its solid
			SDMaterial,		// its material
			"SensitiveDetector");	// its name



  new G4PVPlacement(
		0, 								// its rotation
		G4ThreeVector(0, 0, SD_z),					// its placement
		SensitiveDetectorLV,						// its logical volume
		"SensitiveDetector",						// its name
		phanLV,								// its mother volume
		false,								// no boolean operation
		0,								// copy number
		fCheckOverlaps);						// checking overlaps


  //
  // Always return the physical World
  //
  return worldPV;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
  // G4SDManager::GetSDMpointer()->SetVerboseLevel(1);

  //
  // Sensitive detectors
  //

  auto SensitiveDetector = new CalorimeterSD(					// create new sensitive detector
				"SensitiveDetector",				// its name
				"SensitiveDetectorHitsCollection",		// name of hit collection --> where recorded interactions are stored
				1);						// no. of cells (layers)
  G4SDManager::GetSDMpointer()->AddNewDetector(SensitiveDetector); 		// register the SD in Geant4's SD manager
  SetSensitiveDetector("SensitiveDetector", SensitiveDetector);			// assign sensitive detector to the logical volume

  //
  // Magnetic field
  //
  // Create global magnetic field messenger.
  // Uniform magnetic field is then created automatically if
  // the field value is not zero.
  G4ThreeVector fieldValue;
  fMagFieldMessenger = new G4GlobalMagFieldMessenger(fieldValue);
  fMagFieldMessenger->SetVerboseLevel(1);

  // Register the field messenger for deleting
  G4AutoDelete::Register(fMagFieldMessenger);

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
