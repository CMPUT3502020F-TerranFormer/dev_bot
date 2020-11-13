threadsafe_priority_queue<Task> scout_queue;
std::vector<TF_unit> scout_units;

void scoutGameStart();

void scoutStep();

void scoutIdle(const Unit* u);

