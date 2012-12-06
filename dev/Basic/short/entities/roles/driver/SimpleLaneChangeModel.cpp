/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <limits>

#include "entities/models/LaneChangeModel.hpp"
#include "geospatial/Lane.hpp"

using std::numeric_limits;
using namespace sim_mob;



double sim_mob::SimpleLaneChangeModel::executeLaneChanging(DriverUpdateParams& p, double totalLinkDistance, double vehLen, LANE_CHANGE_SIDE currLaneChangeDir)
{
  return 1; //TODO

}
