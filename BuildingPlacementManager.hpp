#ifndef BUILDINGPLACEMENTMANAGER_HPP
#define BUILDINGPLACEMENTMANAGER_HPP

#include <sc2api/sc2_api.h>
#include "sc2lib/sc2_lib.h"
#include <cstring>

/**
 * This class contains all the information on where to build buildings
 * It will eventually have a method for each building type
 * When it is complete, we can modify the BUILD Task so that it is no longer necessary
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

private:
	Map map;
	Point2D start_location;
	const ObservationInterface* observation;
	QueryInterface* query;
};

#endif