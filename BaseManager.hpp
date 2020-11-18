#ifndef BASEMANAGER_HPP
#define BASEMANAGER_HPP

//
// Created by Carter Sabadash October 29
//

#include "Task.hpp"
#include "threadsafe_priority_queue.h"
#include <vector>
#include "TF_unit.hpp"
#include "utility.hpp"
#include <algorithm>
#include "BuildingPlacementManager.hpp"
#include <iostream>
#include <random>


/**
 * The purpose of this class is to manage command centers, scv's and minerals/vespene
 * Currently this will build 3 command centers and mine all resources
 * And it heals all damaged units around an undamaged command center with radius 10
 * Issue: MULE functionality is incomplete and may not work (at all?)
 * Issue: The command center transfer (when resources are depleted) has
 * to be implemented (we are currently stuck at 3 command centers)
 */

using namespace sc2;

struct Base {
	TF_unit NoUnit = TF_unit((UNIT_TYPEID)0, (Tag)-1);

	Base() {
		transfer = false;
	}
	TF_unit command;
	Point2D location;
	bool transfer;

	std::vector<TF_unit> minerals;
	std::vector<TF_unit> vespene;

	void findResources(const Units units) {
		// must be called after a command center is added, updates with the surrounding resources
		minerals.clear();
		vespene.clear();
		IsMinerals min;
		IsVespeneGeyser vesp;
		for (auto& p : units) {
			if (min(*p) && DistanceSquared2D(location, p->pos) < 225 // just add minerals close to command center
				&& p->mineral_contents > 0) {						 // must also have resources
				minerals.push_back(TF_unit(p->unit_type, p->tag));
			}
			if (vesp(*p) && DistanceSquared2D(location, p->pos) < 225
				&& p->vespene_contents > 0) {
				vespene.push_back(TF_unit(p->unit_type, p->tag));
			}
		}
	}

	bool buildNewCommand() {
		// true if more than half the resources units have been destroyed, and command is a planetary fortress
		if (command.type == UNIT_TYPEID::TERRAN_PLANETARYFORTRESS) {
			if ((vespene.size() + minerals.size()) < 5) { return true; }
		}
		return false;
	}

	/* Returns if all the resources are depleted;
	 */
	bool depleted() {
		return (minerals.empty() && vespene.empty());
	}

	/** By waiting until all resources are gone we often lose scv efficiency
	 * so wait until 1/2 resources are gone
	 * findResources must be called first to ensure that we have accurate information
	 * about the state of resources
	 */
	bool startTransfer() {
		// start transfer process (move units if planetary; else move units + command) to new location
		// implement this later
		if ((minerals.size() <= 4 || vespene.size() <= 1) && transfer == false) {
			transfer = true; // we don't want to build multiple bases in place of one
			return true;
		}
		return false;
	}
};

class BaseManager {
public:
	BaseManager(threadsafe_priority_queue<Task>* t_queue, const ObservationInterface* obs,
		std::vector<Tag>* units, BuildingPlacementManager* bpm)
		: task_queue(t_queue), observation(obs), resource_units(units), buildingPlacementManager(bpm)
	{
		Base base; // to account for if scv's are added before first command center
		Point3D start = observation->GetStartLocation();
		base.location = Point2D(start.x, start.y);
		base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
		base.command = base.NoUnit;
		active_bases.push_back(base);

		scv_count = 0;
		scv_target_count = 70;

		std::random_device r;
		rand_gen = std::mt19937(r());
	}

	/**
	 * This handles the actions that will be made during a step
	 * It's easier (and more clear) to handle stuff like building refineries
	 * in here instead of when a unit is added/idle/deleted
	 * This automatically repairs all units close to the command center (10 units)
	 */
	void step() {
		// Stuff that is particular to each base, such as refineries, handling excess scv's
		// we must check that build progress is complete when looking for units that can be repaired
		Units scvs = observation->GetUnits(Unit::Alliance::Self, IsSCV());
		for (auto& base : active_bases) {
			// smaller range than 15, should still include refineries and immediate units
			Units units = observation->GetUnits(Unit::Alliance::Self, IsClose(base.location, 100)); 
			const Unit* command = observation->GetUnit(base.command.tag);
			if (command == nullptr) { return; }

			// do not build refineries while the command center is damaged -> wait until the command
			// center is repaired, then it is probably safe
			if (command->health < command->health_max
				&& command->build_progress >= 1) {
				task_queue->push(Task(REPAIR, RESOURCE_AGENT, 6, command->tag, ABILITY_ID::EFFECT_REPAIR, 6));
			}
			else {
				// if they are already built, this won't do anything; but it is simpler
				// to just try than to check if they are not built

				// must be finished so we are guaranteed to have vision of the geysers
				if (command->build_progress >= 1) {
					buildRefineries(command);
				}

				// repair all units that are close to the command center
				// if it's not being attacked, it is probably safe for the scv
				// I'm unsure if this works for units inside a bunker/medi-vac
				for (auto& unit : units) {
					if (unit->health < unit->health_max
						&& unit->build_progress >= 1) {
						task_queue->push(Task(REPAIR, RESOURCE_AGENT, 5, unit->tag, ABILITY_ID::EFFECT_REPAIR, 1));
					}
				}
			}

			// we also deal with bases that have too many scvs mining (1 scv / base / step)
			if (command->assigned_harvesters > command->ideal_harvesters) {
				for (auto& unit : units) {
					if (unit->unit_type == UNIT_TYPEID::TERRAN_SCV) {
						assignSCV(unit);
						break;
					}
				}
			}

			// we must also make sure the base still has resources, if not move it to isolated_bases
			// or start the transfer process --> implement later
			base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
			if (command->build_progress == 1) {
				// when a command center is just built not all resources are in vision, so check the # ideal harvesters is not max
				if (base.startTransfer() && command->ideal_harvesters <= 8) {
					Point2D baseLocation = buildingPlacementManager->getNextCommandCenterLocation();
					if (baseLocation != Point2D(0, 0)) {
						task_queue->push(Task(BUILD, RESOURCE_AGENT, 6,
							UNIT_TYPEID::TERRAN_COMMANDCENTER, ABILITY_ID::BUILD_COMMANDCENTER, baseLocation));
					}
				}
				else if (base.depleted()) {
					isolated_bases.push_back(base.command);
					for (auto& it = active_bases.cbegin(); it != active_bases.cend(); ++it) {
						if (base.command.tag == it->command.tag) {
							active_bases.erase(it);
							return;
						}
					}
				}
			}
		}

		// Isolated bases have a subset of the actions of active_bases
		// they still repair nearby units to make it easy for the other agents,
		// but it is preferred to send them to active bases
		for (auto& base : isolated_bases) {
			// smaller range than 15, should still include refineries and immediate units
			const Unit* command = observation->GetUnit(base.tag);
			if (command == nullptr) { return; }
			Units units = observation->GetUnits(Unit::Alliance::Self, IsClose(command->pos, 100));

			// do not build refineries while the command center is damaged -> wait until the command
			// center is repaired, then it is probably safe
			if (command->health < command->health_max
				&& command->build_progress >= 1) {
				task_queue->push(Task(REPAIR, RESOURCE_AGENT, 6, command->tag, ABILITY_ID::EFFECT_REPAIR, 6));
			}
			else {
				// repair all units that are close to the command center
				// if it's not being attacked, it is probably safe for the scv
				// I'm unsure if this works for units inside a bunker/medi-vac
				for (auto& unit : units) {
					if (unit->health < unit->health_max
						&& unit->build_progress >= 1) {
						task_queue->push(Task(REPAIR, RESOURCE_AGENT, 4, unit->tag, ABILITY_ID::EFFECT_REPAIR, 1));
					}
				}
			}
			// if it turns out that when there are no resources left scvs
			// continue working at a command center, then reassign all scvs
		}

		// Build a new command center if necessary, right now, we use preset conditions
		// remember that a unit is added when it starts being build
		// build command centers at 20/40 scv's, or when a planetary fortress is running out of resources
		// ISSUE: Sometimes builds another command center when it shouldn't be
		int num_command_centers = active_bases.size() + isolated_bases.size();
		bool build = false;

		// to stay alive as long as possible
		if (num_command_centers == 0) { build = true; }
		else if (active_bases.size() < 2 && scv_count >= 20) { build = true; }
		else if (active_bases.size() < 3 && scv_count >= 40) { build = true; }
		else if (active_bases.size() < 4 && scv_count == scv_target_count) { build = true; }

		// then check if we have a planetary fortress that is running out of resources
		// build a new command center in advance so there is less idle time
		// ISSUE: Implement base transfers with LIFT OFF
		for (auto& base : active_bases) {
			if (base.buildNewCommand()) {
				build = true;
				break;
			}
		}

		if (build) {
			Point2D baseLocation = buildingPlacementManager->getNextCommandCenterLocation();
			if (baseLocation != Point2D(0, 0)) {
				task_queue->push(Task(BUILD, RESOURCE_AGENT, 6,
					UNIT_TYPEID::TERRAN_COMMANDCENTER, ABILITY_ID::BUILD_COMMANDCENTER, baseLocation));
			}
		}

		// we must also deal with vespene refineries that have excess workers
		Units refineries = observation->GetUnits(Unit::Alliance::Self, IsVespeneRefinery());
		for (auto& r : refineries) {
			if (r->assigned_harvesters > r->ideal_harvesters) {
				for (auto& s : scvs) {
					for (auto& order : s->orders) {
						if (order.ability_id == ABILITY_ID::HARVEST_GATHER
							&& order.target_unit_tag == r->tag) {
							assignSCV(s);
							return;
						}
					}
				}
			}
		}
	}

	void addUnit(const Unit* u) {
		// Add MULES?? -> should still work because they are included in unitIdle()
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: {
			++scv_count;
			assignSCV(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (active_bases.size() == 1) { // check for initial case where scv's were added before command center
				if (active_bases.front().command.tag == -1) {
					active_bases.front().command = TF_unit(u->unit_type, u->tag);
				}
				else {
					Base base = Base();
					base.command = TF_unit(u->unit_type, u->tag);
					base.location = u->pos;
					base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
					active_bases.push_back(base);
				}
			}
			else {
				Base base = Base();
				base.command = TF_unit(u->unit_type, u->tag);
				base.location = u->pos;
				base.findResources(observation->GetUnits(Unit::Alliance::Neutral));
				active_bases.push_back(base);
			}
		}
		// not yet sure how upgrades are handled -> maybe have to add ORBITAL_COMMAND, etc
		}
	}

	/**
	 * Takes appropriate actions for the type of deleted unit
	 * This does not modify resource_units; that's for the resource agent
	 */
	void deleteUnit(const Unit* u) {
		// if it's a COMMAND CENTER also have to reassign all scvs
		if (u->unit_type.ToType() == UNIT_TYPEID::TERRAN_SCV) { --scv_count; }
		IsCommandCenter f;
		if (f(*u)) { // it is necessary to remove the base so scvs will be properly assigned
					 // building command centers is handled in step()
			for (auto it = active_bases.cbegin(); it != active_bases.cend(); ++it) {
				if (it->command.tag == u->tag) {
					active_bases.erase(it);
					return;
				}
			}
		}
	}

	/**
	 * Get's an scv that is idle or harvesting; preferring close scvs
	 * If there are no close scvs idle or harvesting (<15 away from point)
	 * Then a search is performed for idle or harvesting scvs disregarding distance
	 * This does not prefer idle scv's over harvesting; idle scvs will just take over harvesting
	 * This may return the same scv if called multiple times during the same step
	 * (a random available scv is selected to minimize this chance)
	 * It is recommended to call this method without a point when you know there is likely only 1 scv
	 * nearby and you want to build multiple buildings
	 */
	TF_unit getSCV(Point2D point = Point2D(0, 0)) {
		// get's an scv that is idle or harvesting; should check the closest scv
		// does not show a preference for idle scvs over harvesting; idle scvs will just take over harvesting
		// If this is called multiple times during the same step
		// the same scv can be returned each time -> randomness when choosing scvs

		Units scvs = observation->GetUnits(Unit::Alliance::Self, IsSCV());
		if (scvs.empty()) { return Base().NoUnit; } // we have no scv's
		Units possible_scvs;

		// first get all harvesting/idle (or close, if ia point is given) scv's
		// check that they are also in resource_units
		for (auto& scv : scvs) {
			if (std::find(std::begin(*resource_units), std::end(*resource_units), scv->tag) == std::end(*resource_units)) {
				continue;
			}
			if (scv->orders.empty()) {
				if (point != Point2D(0, 0)) {
					if (DistanceSquared2D(scv->pos, point) < 225) { // 15**2
						possible_scvs.push_back(scv);
					}
				}
				else {
					possible_scvs.push_back(scv);
				}
			}
			else if (scv->orders.front().ability_id == ABILITY_ID::SMART
				|| scv->orders.front().ability_id == ABILITY_ID::HARVEST_GATHER) {
				if (point != Point2D(0, 0)) {
					if (DistanceSquared2D(scv->pos, point) < 225) { // 15**2
						possible_scvs.push_back(scv);
					}
				}
				else {
					possible_scvs.push_back(scv);
				}
			}
		}

		// then search from all harvesting scvs if none are available (if no point is specified
		// this has already been done)
		if (possible_scvs.empty() && point != Point2D(0, 0)) {
			return getSCV();
		}

		// if there are no scvs that can immediately be reassigned
		// just get a random scv
		if (possible_scvs.empty()) { return TF_unit(scvs.front()->unit_type, scvs.front()->tag); }

		// otherwise return a random scv
		std::uniform_int_distribution<> distrib(0, possible_scvs.size() - 1);
		int index = distrib(rand_gen);
		return TF_unit(scvs.data()[index]->unit_type, scvs.data()[index]->tag);
	}

	/**
	 * Handles idle units
	 * **Command centers ignore the default priority limitations
	 * and produce scvs with priority 8
	 */
	void unitIdle(const Unit* u) {
		// make scv's if not enough; (do orbital_scan if requested)
		switch (u->unit_type.ToType()) {
		case UNIT_TYPEID::TERRAN_SCV: {
			assignSCV(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_MULE: {
			assignMULE(u);
			break;
		}
		case UNIT_TYPEID::TERRAN_PLANETARYFORTRESS: {
			if (scv_count <= scv_target_count) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_PLANETARYFORTRESS, u->tag));
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
			if (scv_count <= scv_target_count) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 6, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_COMMANDCENTER, u->tag));
			}
			break;
		}
		case UNIT_TYPEID::TERRAN_ORBITALCOMMAND: {
			if (scv_count <= scv_target_count) {
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::TRAIN_SCV, UNIT_TYPEID::TERRAN_SCV, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u->tag));
			}
			else if (u->energy >= 50){ // MULE energy cost
				task_queue->push(Task(TRAIN, RESOURCE_AGENT, 5, ABILITY_ID::EFFECT_CALLDOWNMULE, UNIT_TYPEID::TERRAN_MULE, UNIT_TYPEID::TERRAN_ORBITALCOMMAND, u->tag));
			}
			break;
		}
		}
	}

	/**
	 * Depending on how many command centers we have, we want to have so much extra supply
	 * available; try to keep 3/4/6 supply ahead (1/2/3+ command centers)
	 */
	int getSupplyFloat() {
		int command_centers = isolated_bases.size() + active_bases.size();
		if (command_centers <= 1) { return 3; }
		else if (command_centers == 2) { return 4; }
		else { return 6; }
	}

private:
	threadsafe_priority_queue<Task>* task_queue;
	const ObservationInterface* observation;
	BuildingPlacementManager* buildingPlacementManager;
	std::vector<TF_unit> isolated_bases; // pretty much empty bases except for (planetary fortress)
	std::vector<Base> active_bases; // should have 3 bases -> potentially 4-6 when transferring to new location
	
	int scv_count; // total active scv count
	int scv_target_count; // aim for 70?

	std::vector<Tag>* resource_units;

	std::mt19937 rand_gen; //Standard mersenne_twister_engine seeded with rd()

	void assignSCV(const Unit* u) {
		for (auto& p : active_bases) { // saturate bases with scv's
			if (p.command.tag == -1) { return; }
			const Unit* base = observation->GetUnit(p.command.tag);
			if (base->assigned_harvesters < base->ideal_harvesters) {
				task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::HARVEST_GATHER, p.minerals.back().tag));
				return;
			}
			// if minerals are saturated, check vespene
			Units vespene = observation->GetUnits(IsVespeneRefinery());
			for (auto& v : vespene) {
				if (DistanceSquared2D(base->pos, v->pos) <= 225) { // 15**2
					if (v->assigned_harvesters < v->ideal_harvesters) {
						task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::HARVEST_GATHER, v->tag));
						return;
					}
				}
			}
		}
		// if all are saturated, do nothing -> change this later??
	}

	void assignMULE(const Unit* u) {
		// similar to assign SCV -> get the nearest base with minerals
		// and make it harvest them
		Base closest_base;
		int distance2 = 1000000; // 1000**2, larger than maps
		for (auto& base : active_bases) {
			if (DistanceSquared2D(u->pos, base.location) < distance2
				&& base.minerals.size() != 0) {
				closest_base = base;
				distance2 = DistanceSquared2D(u->pos, base.location);
			}
		}
		// make sure it doesn't fail if there are no bases with minerals
		if (closest_base.minerals.size() == 0) { return; }
		task_queue->push(Task(HARVEST, 11, u->tag, ABILITY_ID::HARVEST_GATHER, closest_base.minerals.front().tag));
	}

	/**
	 * Adds Refineries for the command center to the task queue
	 * Must be called when the geysers are guaranteed to be in vision
	 * don't add to queue when refineries already exist
	 */
	void buildRefineries(const Unit* command) {
		if (command->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER) {
			if (observation->GetUnits(Unit::Alliance::Self, IsVespeneRefinery()).size() >= 2) { return; }
			Units vespene = observation->GetUnits(IsVespeneGeyser());
			for (auto& p : vespene) {
				if (DistanceSquared2D(command->pos, p->pos) < 225) {
					task_queue->push(Task(BUILD, RESOURCE_AGENT, 5, UNIT_TYPEID::TERRAN_REFINERY,
						ABILITY_ID::BUILD_REFINERY, p->tag));
				}
			}
		}
	}


};
#endif