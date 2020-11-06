#ifndef ACTION_HPP
#define ACTION_HPP

#include "sc2api/sc2_api.h"

/**
 * This is a class to hold the data for the ActionInterface, specifically UnitCommand
 * because the UnitCommand methods are not available unless the class inherits from
 * Agent, and we can't do that with 4 separate agents
 * 
 * For simplicity we will have a queue of Actions, and depening on CommandType, a 
 * different version of UnitCommand will be called. See details below
 */
using namespace sc2;
enum CommandType {SELF, POINT, TARGET, M_SELF, M_POINT, M_TARGET};

struct Action {

	/** UnitCommand(const Unit *unit, AbilityID ability, bool queued_command=false)
	 * @param type: SELF
	 * @param unit: The unit to command
	 * @param aid: The ability id to use
	 * @param queue_command: Whether to queue the command to the unit or not
	 */
	Action(enum CommandType type, const Unit* unit, AbilityID aid, bool queued_command = false)
		: type(type), unit(unit), ability_id(aid), queued_move(queued_command) {}

	/** UnitCommand(const Unit *unit, AbilityID ability, const Point2D point, bool queued_command=false)
	 * @param type: POINT
	 * @param unit: The unit to command
	 * @param aid: The ability id to use
	 * @param point: The point to move to/target
	 * @param queue_command: Whether to queue the command to the unit or not
	 */
	Action(enum CommandType type, const Unit* unit, AbilityID aid, const Point2D point, bool queued_command = false)
		: type(type), unit(unit), ability_id(aid), point(point), queued_move(queued_command) {}

	/** UnitCommand(const Unit *unit, AbilityID ability, const Unit* target, bool queued_command=false)
	 * @param type: POINT
	 * @param unit: The unit to command
	 * @param aid: The ability id to use
	 * @param target: The target unit
	 * @param queue_command: Whether to queue the command to the unit or not
	 */
	Action(enum CommandType type, const Unit* unit, AbilityID aid, const Unit* target, bool queued_command = false)
		: type(type), unit(unit), ability_id(aid), target(target), queued_move(queued_command) {}

	/** UnitCommand (const Units &units, AbilityID ability, bool queued_move=false)
	 * @param type: M_SELF
	 * @param units: The units to command
	 * @param aid: The ability id to use
	 * @param queue_command: Whether to queue the command to the unit or not
	 */
	Action(enum CommandType type, const Units unit, AbilityID aid, bool queued_command = false)
		: type(type), units(units), ability_id(aid), queued_move(queued_command) {}

	/**UnitCommand (const Units &units, AbilityID ability, const Point2D &point, bool queued_command=false)
	 * @param type: M_POINT
	 * @param units: The units to command
	 * @param aid: The ability id to use
	 * @param point: The point to move to/target
	 * @param queue_command: Whether to queue the command to the unit or not
	 */
	Action(enum CommandType type, const Units unit, AbilityID aid, const Point2D point, bool queued_command = false)
		: type(type), units(units), ability_id(aid), point(point), queued_move(queued_command) {}

	/** UnitCommand (const Units &units, AbilityID ability, const Unit* target, bool queued_move=false)
	 * @param type: M_TARGET
	 * @param units: The units to command
	 * @param aid: The ability id to use
	 * @param target: The target unit
	 * @param queue_command: Whether to queue the command to the unit or not
	 */
	Action(enum CommandType type, const Units unit, AbilityID aid, const Unit* target, bool queued_command = false)
		: type(type), units(units), ability_id(aid), target(target), queued_move(queued_command) {}

	enum CommandType type;
	const Units units;
	const Unit* unit;
	AbilityID ability_id;
	const Point2D point;
	const Unit* target;
	bool queued_move;
};

#endif