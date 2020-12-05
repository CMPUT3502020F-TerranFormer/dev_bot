#ifndef BUILDINGPLACEMENTMANAGER_HPP
#define BUILDINGPLACEMENTMANAGER_HPP

#include <sc2api/sc2_api.h>
#include "sc2lib/sc2_lib.h"
#include <cstring>
#include "utility.hpp"
#include <cassert>
#include <algorithm>
#include <array>

/**
 * This class contains all the information on where to build buildings
 * It will eventually have a method for each building type
 * When it is complete, we can modify the BUILD Task so that it is no longer necessary
 * to provide a Point2D for placement, and instead call a method with the unit_type
 * which will return the correct Point2D <-- This has been done, but for compatibility
 * the option to pass in a point remains
 * 
 * The fixed points were obtained using the Map Editor
 */

using namespace sc2;

enum class Map { CactusValleyLE, BelShirVestigeLE, ProximaStationLE };

struct CloseToStart {
	CloseToStart(const Point2D start) : start(start) {}
	const Point2D start;
	bool operator()(const Point2D p1, const Point2D p2) const {
		return DistanceSquared2D(start, p1) < DistanceSquared2D(start, p2);
	}
};

class BuildingPlacementManager {
public:
	BuildingPlacementManager() {}

	BuildingPlacementManager(const ObservationInterface* obs, QueryInterface* query_)
		: observation(obs), query(query_)
	{
		// initialize with starting point
		Point3D start_3d = observation->GetStartLocation();
		start_location = Point2D(start_3d.x, start_3d.y);

		// and with the map -- also sort the appropriate base locations array by closeness
		// to starting point
		const char* map_name = observation->GetGameInfo().map_name.c_str();
		if (strcmp(map_name, "Cactus Valley LE (Void)") == 0) {
			map = Map::CactusValleyLE;
			std::sort(CactusValleyLEBaseLocations.begin(), CactusValleyLEBaseLocations.end(), CloseToStart(start_location));
		}
		else if (strcmp(map_name, "Bel'Shir Vestige LE (Void)") == 0) { 
			map = Map::BelShirVestigeLE;
			std::sort(BelShirVestigeLEBaseLocations.begin(), BelShirVestigeLEBaseLocations.end(), CloseToStart(start_location));
		}
		else if (strcmp(map_name, "Proxima Station LE") == 0) {
			map = Map::ProximaStationLE; 
			std::sort(ProximaStationLEBaseLocations.begin(), ProximaStationLEBaseLocations.end(), CloseToStart(start_location));
		}
		else { 
			std::cerr << "Unrecognized Map: " << map_name << std::endl;
			std::cerr << "Recognized map names are: " << std::endl
				<< "\tCactus Valley LE (Void)" << std::endl
				<< "\tBel'Shir Vestige LE (Void)" << std::endl
				<< "\tProxima Station LE" << std::endl;
			exit(1);
		}

		assert(observation->GetGameInfo().player_info.size() == 2);

		// For some reason the actual race seems to always show random, but race_requested
		// has the correct race, but I don't know if this is always the case so
		// this is not guaranteed to be correct because the race can be random -> 
		// can change it later when an enemy unit is sighted 
		for (auto& p : observation->GetGameInfo().player_info) {
			if (p.player_id != observation->GetPlayerID()) {
				if (p.race_actual == Race::Random) { enemyRace = p.race_requested; }
				else { enemyRace = p.race_actual; }
			}
		}
	}

	Point2D getNextLocation(UNIT_TYPEID unit_type, Point2D point) {
		switch (unit_type) {
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: return getNextCommandCenterLocation();
		case UNIT_TYPEID::TERRAN_SUPPLYDEPOT: return getNextSupplyDepotLocation();
		case UNIT_TYPEID::TERRAN_BARRACKS: return getNextBarracksLocation();
		case UNIT_TYPEID::TERRAN_FUSIONCORE: return getNextFusionCoreLocation();
		case UNIT_TYPEID::TERRAN_FACTORY: return getNextFactoryLocation();
		case UNIT_TYPEID::TERRAN_BUNKER: return getNextBunkerLocation(point);
		case UNIT_TYPEID::TERRAN_STARPORT: return getNextStarportLocation();
		case UNIT_TYPEID::TERRAN_ENGINEERINGBAY: return getNextEngineeringBayLocation();
		case UNIT_TYPEID::TERRAN_ARMORY: return getNextArmoryLocation();
		case UNIT_TYPEID::TERRAN_MISSILETURRET: return getNextMissileTurretLocation(point);
		default: std::cerr << "BUILDING_PLACEMENT: Unrecognized unit_type: " << (int) unit_type << std::endl;
		}
		return Point2D(0, 0);
	}

	void setEnemyRace(Race race) {
		enemyRace = race;
	}


private:
	Map map;
	Point2D start_location;
	const ObservationInterface* observation;
	QueryInterface* query;
	Race enemyRace;

	int time_out = 200;
	
	std::array<sc2::Point2D, 16> CactusValleyLEBaseLocations 
		{ Point2D(33.5, 158.5), Point2D(66.5, 161.5), Point2D(54.5, 132.5), Point2D(93.5, 156.5), // Top Left
		Point2D(158.5, 158.5), Point2D(161.5, 125.5), Point2D(132.5, 137.5), Point2D(156.5, 98.5), // Top Right
		Point2D(33.5, 33.5), Point2D(30.5, 66.5), Point2D(59.5, 54.5), Point2D(33.5, 93.5), // Bottom Left
		Point2D(158.5, 33.5), Point2D(125.5, 30.5), Point2D(137.5, 59.5), Point2D(98.5, 35.5) }; // Bottom Right

	std::array<sc2::Point2D, 16> ProximaStationLEBaseLocations
		{ Point2D(137.5, 139.5), Point2D(164.5, 140.5), Point2D(119.5, 111.5), Point2D(149.5, 102.5), // Top Right
		Point2D(93.5, 147.5), Point2D(166.5, 69.5), Point2D(127.5, 57.5), Point2D(165.5, 23.5),
		Point2D(62.5, 28.5), Point2D(35.5, 27.5), Point2D(80.5, 56.5), Point2D(50.5, 65.5), // Bottom Left
		Point2D(106.5, 20.5), Point2D(33.5, 98.5), Point2D(72.5, 110.5), Point2D(34.5, 144.5) };

	std::array<Point2D, 10> BelShirVestigeLEBaseLocations
		{ Point2D(29.5, 134.5), Point2D(61.5, 136.5), Point2D(28.5, 96.5), Point2D(98.5, 138.5), Point2D(23.5, 55.5), // Top Locations
		Point2D(114.5, 25.5), Point2D(82.5, 23.5), Point2D(115.5, 63.5), Point2D(45.5, 20.5), Point2D(120.5, 104.5) }; // Bottom Locations

	Point2D getNextCommandCenterLocation() {
		// we go through the sorted list for each map and build at the closest location where
		// 1. there are enough resources, 
		// 2. There is not currently an enemy nearby (range root(200), ~14)
		// 3. It also avoids building close to an enemy base (range 40, relies on scouting info so we have a snapshot of the unit), 
		//		We can check for enemy buildings to do this
		// when checking for resources, it's usually the case that minerals are used before vespene, so we'll only check that half
		// the minerals still exist
		switch (map) {
		case Map::CactusValleyLE: {
			for (auto& loc : CactusValleyLEBaseLocations) {
				if (commandCenterPlaceable(loc)) { return loc; }
			}
			break;
		}
		case Map::BelShirVestigeLE: {
			for (auto& loc : BelShirVestigeLEBaseLocations) {
				if (commandCenterPlaceable(loc)) { return loc; }
			}
			break; 
		}
		case Map::ProximaStationLE: {
			for (auto& loc : ProximaStationLEBaseLocations) {
				if (commandCenterPlaceable(loc)) { return loc; }
			}
			break; 
		}
		default: std::cerr << "Invalid Map! Cannot get command center location" << std::endl;
		}
		std::cerr << "No Base Location Left with sufficient resources and safe from enemies" << std::endl;
		return Point2D(0, 0);
	}

	bool commandCenterPlaceable(Point2D loc) {
		if (query->Placement(ABILITY_ID::BUILD_COMMANDCENTER, loc)) {
			Units close_units = observation->GetUnits(IsClose(loc, 200));
			auto mineral_count = 0;
			for (auto& u : close_units) {
				if (u->alliance == Unit::Alliance::Enemy) { return false; }
				if (IsMinerals()(*u)) { ++mineral_count; }
			}
			Units enemies = observation->GetUnits(Unit::Alliance::Enemy, IsClose(loc, 1600));
			for (auto& u : enemies) {
				for (auto& attribute : observation->GetUnitTypeData()[u->unit_type].attributes) {
					if (attribute == Attribute::Structure) {
						return false;
					}
				}
			}
			if (mineral_count >= 4) { 
				// this is less likely for rich minerals, but as those patches have fewer minerals total
				// we'd rather build a base with more resources
				return true;
			}
		}
		return false;
	}

	/* This checks around a given point */
	Point2D getPointPlacement(ABILITY_ID aid_to_place, Point2D base_point, float multiplier = 1) {
		Point2D point(0, 0);
		for (auto i = 0; i < time_out; ++i) {
			point = Point2D(base_point.x + (GetRandomScalar() * multiplier), base_point.y + (GetRandomScalar() * multiplier));
			if (query->Placement(aid_to_place, point)) {
				return point;
			}
		}
		std::cerr << "Time out getting building location, ABILITY_ID: " << (int) aid_to_place 
			<< " around Point (" << base_point.x << ", " << base_point.y << ")" << std::endl;
		return Point2D(0, 0);
	}

	/* This checks around all the command centers 
	* Try to form a 3x3 grid of buildings with the command center in the center
	* All locations must be >4 units from minerals, and >5 units (horizontallly) from the next location
	* (4 units vertically, so units can move between them) this lets all add-ons be built as long as they aren't
	*	against a wall (so check for placement 1 unit to the right before it's considered valid)
	* They will form a (semi) ring around the command center. 2+ Rings can exists, but we will first try to 
	* get a complete ring around each command center of the previous size
	*/
	Point2D getPlacement(ABILITY_ID aid_to_place) {
		// for now, get a random point with radius 15 around a command center
		Point2D point(0, 0);
		Units command_centers = observation->GetUnits(Unit::Alliance::Self, IsCommandCenter());

		auto ring = 0; // ring 0 is 1x1 (the command center)
		while (++ring < 6) {
			for (auto& c : command_centers) {
				auto minerals = observation->GetUnits(Unit::Alliance::Neutral,
					[c](const Unit& u) { return IsMinerals()(u) && IsClose(c->pos, 100)(u); });
				for (auto x = -ring; x <= ring; ++x) { // to get x coord
					point = c->pos;
					point.x += (x * 5.0f);
					for (auto y = -ring; y <= ring; ++y) { // y coord
						point.y = c->pos.y;
						point.y += (y * 4.0f);
						if (x == 0) { // so units can move between command center & building
							if (y < 0) { point.y -= 1; }
							else if (y > 0) { point.y += 1; }
						}
						if (query->Placement(aid_to_place, point)) { 
							// check that an add-on can be built, and that
							// it doesn't get in the way of scvs mining minerals
							Point2D right = point;
							++right.x;
							if (query->Placement(aid_to_place, right)
								&& !PointNearUnits(minerals, 4.0f)(point)) {
								return point;
							}
						}
					}
				}
			}
			++ring;
		}
		std::cerr << "Error getting Ring building location, ABILITY_ID: " << (int) aid_to_place << std::endl;
		return Point2D(0, 0);
	}

	Point2D getNextSupplyDepotLocation() {
		// get a location that is by minerals, but away from the command center
		// (so at least 6 distance from it), but within 4 from a mineral
		Point2D point(0, 0);
		Units command_centers = observation->GetUnits(Unit::Alliance::Self, IsCommandCenter());
		Units minerals = observation->GetUnits(Unit::Alliance::Neutral,
			[command_centers](const Unit& u) 
			{ return IsNearUnits(command_centers, 10.0f)(u) && IsMinerals()(u); });

		for (auto i = 0; i < time_out; ++i) {
			for (auto& m : minerals) {
				point = Point2D(m->pos.x + (GetRandomScalar() * 4.0f), m->pos.y + (GetRandomScalar() * 4.0f));
				for (auto& c : command_centers) {
					if (DistanceSquared2D(c->pos, point) > 40
						&& DistanceSquared2D(c->pos, m->pos) < 100
						&& query->Placement(ABILITY_ID::BUILD_SUPPLYDEPOT, point))
					{
						return point;
					}
				}
			}
		}
		return Point2D(0, 0);
	}

	Point2D getNextBarracksLocation() {
		// we'll just build it near a command center for now
		// which is the same as the supply depots
		return getPlacement(ABILITY_ID::BUILD_BARRACKS);
	}

	Point2D getNextFusionCoreLocation() {
		return getPlacement(ABILITY_ID::BUILD_FUSIONCORE);
	}

	Point2D getNextFactoryLocation() {
		return getPlacement(ABILITY_ID::BUILD_FACTORY);
	}

	Point2D getNextBunkerLocation(Point2D pos) {
		return getPointPlacement(ABILITY_ID::BUILD_BUNKER, pos, 3.0f);
	}

	Point2D getNextStarportLocation() {
		return getPlacement(ABILITY_ID::BUILD_STARPORT);
	}

	Point2D getNextEngineeringBayLocation() {
		return getPlacement(ABILITY_ID::BUILD_ENGINEERINGBAY);
	}

	Point2D getNextArmoryLocation() {
		return getPlacement(ABILITY_ID::BUILD_ARMORY);
	}

	Point2D getNextMissileTurretLocation(Point2D pos) {
		return getPointPlacement(ABILITY_ID::BUILD_MISSILETURRET, pos, 4.0f);
	}
};	

#endif