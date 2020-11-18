#include <iostream>
#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"

#include "TF_Bot.hpp"
#include "LadderInterface.hpp"


int main(int argc, char *argv[]) {
    RunBot(argc, argv, new TF_Bot(), sc2::Race::Terran);
    return 0;
}