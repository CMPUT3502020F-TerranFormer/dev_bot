BaseManager* baseManager;

threadsafe_priority_queue<Task> resource_queue;
std::vector<TF_unit> resource_units;

void resourceGameStart();

void resourceStep();

void resourceIdle(const Unit* u);

void buildSupplyDepot();

bool buildStructure(ABILITY_ID ability_to_build_structure, Point2D point, Tag target = -1);