BaseManager* baseManager;

void resourceGameStart();

void resourceStep();

void resourceIdle(const Unit* u);

sc2::Tag getUnit(Task& t);

void buildSupplyDepot();

bool buildStructure(ABILITY_ID ability_to_build_structure, Point2D point);