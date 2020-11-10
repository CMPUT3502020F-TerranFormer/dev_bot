#ifndef BUILDINGPLACEMENTMANAGER_HPP
#define BUILDINGPLACEMENTMANAGER_HPP

#include <sc2api/sc2_api.h>
#include "sc2lib/sc2_lib.h"
#include <cstring>

/**
 * This class contains all the information on where to build buildings
 * It will eventually have a method for each building type
 * When it is complete, we can modify the BUILD Task so that it is no longer necessary (maybe????)
 * to provide a Point2D for placement, and instead call a method with the unit_type
 * which will return the correct Point2D
 */

using namespace sc2;

enum class Map { CactusValleyLE, BelShirVestigeLE, ProximaStationLE };

class BuildingPlacementManager {
public:
	BuildingPlacementManager() {}

	BuildingPlacementManager(const ObservationInterface* obs, QueryInterface* query_)
		: observation(obs), query(query_)
	{
		// initialize with starting point
		Point3D start_3d = observation->GetStartLocation();
		start_location = Point2D(start_3d.x, start_3d.y);

		// and with the map
		const char* map_name = observation->GetGameInfo().map_name.c_str();
		if (strcmp(map_name, "Cactus Valley LE (Void)") == 0) { map = Map::CactusValleyLE; }
		else if (strcmp(map_name, "Bel'Shir Vestige LE (Void)") == 0) { map = Map::BelShirVestigeLE; }
		else if (strcmp(map_name, "Proxima Station LE") == 0) { map = Map::ProximaStationLE; }
		else { 
			std::cerr << "Unrecognized Map: " << map_name << std::endl;
			map = Map::CactusValleyLE;
		}
	}

	Point2D getNextCommandCenterLocation() {
		// for now, build by clusters, in the future a more advanced build policy is needed
		switch (map) {
		case Map::CactusValleyLE: {
			// get the index of the starting base location, then increment it until a suitable location
			// has been found
			int base_index;
			if (start_location == CactusValleyLEBaseLocations[0]) { base_index = 0; }
			else if (start_location == CactusValleyLEBaseLocations[4]) { base_index = 4; }
			else if (start_location == CactusValleyLEBaseLocations[8]) { base_index = 8; }
			else { base_index = 12; }
			for (int i = 0; i < 16; ++i) {
				if (query->Placement(ABILITY_ID::BUILD_COMMANDCENTER, CactusValleyLEBaseLocations[(base_index + i) % 16]))
				{
					return CactusValleyLEBaseLocations[(base_index + i) % 16];
				}
			}
			break;
		}
		case Map::BelShirVestigeLE:{
			// get the index of the starting base location, then increment it until a suitable location
			// has been found
			int base_index;
			if (start_location == BelShirVestigeLEBaseLocations[0]) { base_index = 0; }
			else { base_index = 5; }
			for (int i = 0; i < 10; ++i) {
				if (query->Placement(ABILITY_ID::BUILD_COMMANDCENTER, BelShirVestigeLEBaseLocations[(base_index + i) % 10]))
				{
					return BelShirVestigeLEBaseLocations[(base_index + i) % 10];
				}
			}
			break; }
		case Map::ProximaStationLE: {
			int base_index;
			if (start_location == ProximaStationLEBaseLocations[0]) { base_index = 0; }
			else { base_index = 8; }
			for (int i = 0; i < 16; ++i) {
				if (query->Placement(ABILITY_ID::BUILD_COMMANDCENTER, ProximaStationLEBaseLocations[(base_index + i) % 16]))
				{
					return ProximaStationLEBaseLocations[(base_index + i) % 16];
				}
			}
			break; }
		default: std::cerr << "Invalid Map! Cannot get command center location" << std::endl;
		}
		return Point2D(0, 0);
	}

private:
	Map map;
	Point2D start_location;
	const ObservationInterface* observation;
	QueryInterface* query;

	/*CactusValleyLE Base Locations(Arranged in order), * starting location
	** (33.5, 158.5) - (66.5, 161.5) - (93.5, 156.5) - -*(158.5, 158.5)
	* -(54.5, 132.5) - -(132.5, 137.5) - (161.5, 125.5)
	* (35.5, 93.5) - (156.5, 98.5)
	* (30.5, 66.5) - (59.5, 54.5)						(137.5, 59.5)
	* *(33.5, 33.5) - (98.5, 35.5) (125.5, 30.5) * (158.5, 33.5)
	*
	*For now this will do
	*/
	//const Point2D CactusValleyLETopLeftBases[4] = { Point2D(33.5, 158.5), Point2D(66.5, 161.5), Point2D(54.5, 132.5), Point2D(93.5, 156.5) };
	//const Point2D CactusValleyLETopRightBases[4] = { Point2D(158.5, 158.5), Point2D(161.5, 125.5), Point2D(132.5, 137.5), Point2D(156.5, 98.5) };
	//const Point2D CactusValleyLEBottomLeftBases[4] = { Point2D(33.5, 33.5), Point2D(30.5, 66.5), Point2D(59.5, 54.5), Point2D(33.5, 93.5) };
	//const Point2D CactusValleyLEBottomRightBases[4] = { Point2D(158.5, 33.5), Point2D(125.5, 30.5), Point2D(137.5, 59.5), Point2D(98.5, 35.5) };
	const Point2D CactusValleyLEBaseLocations[16] =
			{ Point2D(33.5, 158.5), Point2D(66.5, 161.5), Point2D(54.5, 132.5), Point2D(93.5, 156.5), // Top Left
			Point2D(158.5, 158.5), Point2D(161.5, 125.5), Point2D(132.5, 137.5), Point2D(156.5, 98.5), // Top Right
			Point2D(33.5, 33.5), Point2D(30.5, 66.5), Point2D(59.5, 54.5), Point2D(33.5, 93.5), // Bottom Left
			Point2D(158.5, 33.5), Point2D(125.5, 30.5), Point2D(137.5, 59.5), Point2D(98.5, 35.5) }; // Bottom Right

	/* ProximaStationLE Base Location (Arranged in order), * starting location
	* (34.5, 144.5)			(93.5, 147.5)	*(137.5, 139.5) (164.5, 140.5)
	* (33.5, 98.5)		(72.5, 110.5)		(119.5, 111.5)	(149.5, 102.5)
	* (50.5, 65.5)	(80.5, 56.5)		(127.5, 57.5)		(166.5, 69.5)
	* (35.5, 27.5)	*(62.5, 28.5)		(106.5, 20.5)		(165.5, 23.5)
	*/
	const Point2D ProximaStationLEBaseLocations[16] =
	{ Point2D(137.5, 139.5), Point2D(164.5, 140.5), Point2D(119.5, 111.5), Point2D(149.5, 102.5), // Top Right
	Point2D(93.5, 147.5), Point2D(166.5, 69.5), Point2D(127.5, 57.5), Point2D(165.5, 23.5),
	Point2D(62.5, 28.5), Point2D(35.5, 27.5), Point2D(80.5, 56.5), Point2D(50.5, 65.5), // Bottom Left
	Point2D(106.5, 20.5), Point2D(33.5, 98.5), Point2D(72.5, 110.5), Point2D(34.5, 144.5) };

	/* Bel'Shir Vestige LE Base Locations (Arranged in order), *starting location 
	* *(29.5, 134.5)		(61.5. 136.5)	(98.5, 138.5)
	* (28.5, 96.5)										(120.5, 104.5)
	* (23.5, 55.5)										(115.5, 63.5)
	*				(45.5, 20.5)		(82.5, 23.5)	*(114.5, 25.5)
	*/
	const Point2D BelShirVestigeLEBaseLocations[16] =
	{ Point2D(29.5, 134.5), Point2D(61.5, 136.5), Point2D(28.5, 96.5), Point2D(98.5, 138.5), Point2D(23.5, 55.5), // Top Locations
	Point2D(114.5, 25.5), Point2D(82.5, 23.5), Point2D(115.5, 63.5), Point2D(45.5, 20.5), Point2D(120.5, 104.5) }; // Bottom Locations
};	

#endif