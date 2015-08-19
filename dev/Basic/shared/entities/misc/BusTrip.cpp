//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "BusTrip.hpp"
#include <boost/lexical_cast.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/BusStop.hpp"
#include "logging/Log.hpp"


using namespace sim_mob;

sim_mob::BusStop_ScheduledTimes::BusStop_ScheduledTimes(DailyTime scheduled_ArrivalTime, DailyTime scheduled_DepartureTime)
: scheduled_ArrivalTime(scheduled_ArrivalTime), scheduled_DepartureTime(scheduled_DepartureTime)
{
	Scheduled_busStop = nullptr;
}

sim_mob::BusStop_RealTimes::BusStop_RealTimes(DailyTime real_ArrivalTime, DailyTime real_DepartureTime)
: real_ArrivalTime(real_ArrivalTime), real_DepartureTime(real_DepartureTime)
{
	Real_busStop = nullptr;
}

sim_mob::BusStop_RealTimes::BusStop_RealTimes(const BusStop_RealTimes& copyFrom)
: real_ArrivalTime(copyFrom.real_ArrivalTime), real_DepartureTime(copyFrom.real_DepartureTime)
, Real_busStop(copyFrom.Real_busStop)
{

}

void sim_mob::BusStop_RealTimes::setReal_BusStop(const BusStop* real_busStop)
{
	Real_busStop = const_cast<BusStop*>(real_busStop);
}

//BusStop
//BusRoute
sim_mob::BusRouteInfo::BusRouteInfo(std::string busRoute_id)
: busRouteId(busRoute_id)
{
	roadSegmentList.clear();
	busStopList.clear();
}

sim_mob::BusRouteInfo::BusRouteInfo(const BusRouteInfo& copyFrom)
: busRouteId(copyFrom.busRouteId), roadSegmentList(copyFrom.roadSegmentList)
, busStopList(copyFrom.busStopList)
{

}

void sim_mob::BusRouteInfo::addBusStop(const BusStop* aBusStop)
{
	busStopList.push_back(aBusStop);
}

void sim_mob::BusRouteInfo::addRoadSegment(const RoadSegment* aRoadSegment)
{
	roadSegmentList.push_back(aRoadSegment);
}


sim_mob::BusTrip::BusTrip(std::string entId, std::string type, unsigned int seqNumber,int requestTime,
		DailyTime start, DailyTime end, int busTripRun_sequenceNum,
		Busline* busline, int vehicle_id, std::string busRoute_id, Node* from,
		std::string fromLocType, Node* to, std::string toLocType)
: Trip(entId, type, seqNumber, requestTime, start, end, boost::lexical_cast<std::string>(busTripRun_sequenceNum),from, fromLocType, to, toLocType),
busTripRun_sequenceNum(busTripRun_sequenceNum), busline(busline), vehicle_id(vehicle_id), bus_RouteInfo(busRoute_id)
{
	   lastVisitedStop_SequenceNumber = -1;
}

void sim_mob::BusTrip::addBusStopScheduledTimes(const BusStop_ScheduledTimes& aBusStopScheduledTime)
{
	busStopScheduledTimes_vec.push_back(aBusStopScheduledTime);
}

void sim_mob::BusTrip::addBusStopRealTimes(Shared<BusStop_RealTimes>* aBusStopRealTime)
{
	busStopRealTimes_vec.push_back(aBusStopRealTime);
}

void sim_mob::BusTrip::setBusStopRealTimes(int busstopSequence_j, Shared<BusStop_RealTimes>* busStopRealTimes)
{
	//std::cout << "busStopRealTimes " << busStopRealTimes->real_ArrivalTime.getRepr_() << " " << busStopRealTimes->real_DepartureTime.getRepr_() << std::endl;
	if(!busStopRealTimes_vec.empty()) {
		busStopRealTimes_vec[busstopSequence_j] = busStopRealTimes;

	}
}

const int sim_mob::BusTrip::getBusTripStopIndex(const BusStop* stop) const {
	int index = 0;
	const std::vector<const BusStop*>& busStopVec = bus_RouteInfo.getBusStops();
	for (std::vector<const BusStop*>::const_iterator it = busStopVec.begin();
			it != busStopVec.end(); it++) {
		index++;
		if (stop == (*it)) {
			break;
		}
	}
	return index;
}

void sim_mob::BusTrip::setBusRouteInfo(const std::vector<const RoadSegment*>& roadSegment_vec, const std::vector<const BusStop*>& busStop_vec)
{
	bus_RouteInfo.setRoadSegments(roadSegment_vec);
	bus_RouteInfo.setBusStops(busStop_vec);

	// addBusStopRealTimes, first time fake Times
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	for (int k = 0; k < busStop_vec.size(); k++)
	{
		Shared<BusStop_RealTimes>* pBusStopRealTimes = new Shared<BusStop_RealTimes>(config.mutexStategy(), BusStop_RealTimes());
		addBusStopRealTimes(pBusStopRealTimes);
	}

	// addBusStopScheduledTimes since only "schedule_based", "evenheadway_based" and "hybrid_based" need these
	if (config.busline_control_type() == "schedule_based" || config.busline_control_type() == "evenheadway_based"
			|| config.busline_control_type() == "hybrid_based")
	{
		std::map<int, BusStopScheduledTime> scheduledTimes = config.busScheduledTimes;
		for (std::map<int, BusStopScheduledTime>::iterator temp = scheduledTimes.begin(); temp != scheduledTimes.end(); temp++)
		{
			BusStop_ScheduledTimes busStop_ScheduledTimes(startTime + DailyTime(temp->second.offsetAT), startTime + DailyTime(temp->second.offsetDT));
			addBusStopScheduledTimes(busStop_ScheduledTimes);
		}
	}
}

sim_mob::Frequency_Busline::Frequency_Busline(DailyTime start_Time, DailyTime end_Time, int headway)
: start_Time(start_Time), end_Time(end_Time), headway(headway)
{

}

sim_mob::Busline::Busline(std::string busline_id, std::string controlType)
: busline_id(busline_id), controlType(getControlTypeFromString(controlType))
{
	controlTimePointNum0 = 15;// the number 2 in( 0->1->2->3 )
	controlTimePointNum1 = 21;
	controlTimePointNum2 = 39;
	controlTimePointNum3 = 51;
}

sim_mob::Busline::~Busline()
{
	for(size_t i = 0; i < busTrip_vec.size(); i++) {
		busTrip_vec[i].safeDeleteBusStopRealTimesVec();
	}
}

ControlTypes sim_mob::Busline::getControlTypeFromString(std::string controlType)
{
	controlType.erase(remove_if(controlType.begin(), controlType.end(), isspace),
			controlType.end());
	if (controlType == "no_control") {
		return NO_CONTROL;
	} else if (controlType == "schedule_based") {
		return SCHEDULE_BASED;
	} else if (controlType == "headway_based") {
		return HEADWAY_BASED;
	} else if (controlType == "evenheadway_based") {
		return EVENHEADWAY_BASED;
	} else if (controlType == "hybrid_based") {
		return HYBRID_BASED;
	} else {
		throw std::runtime_error("Unexpected control type.");
	}
}

void sim_mob::Busline::addBusTrip(BusTrip& aBusTrip)
{
	busTrip_vec.push_back(aBusTrip);
}

void sim_mob::Busline::addFrequencyBusline(const Frequency_Busline& aFrequencyBusline)
{
	frequency_busline.push_back(aFrequencyBusline);
}

void sim_mob::Busline::resetBusTrip_StopRealTimes(int trip_k, int busstopSequence_j, Shared<BusStop_RealTimes>* busStopRealTimes)
{
	if(!busTrip_vec.empty()) {
		//BusTrip& busTripK = busTrip_vec[trip_k];
		busTrip_vec[trip_k].setBusStopRealTimes(busstopSequence_j, busStopRealTimes);
	}
}

sim_mob::PT_Schedule::PT_Schedule()
{

}

sim_mob::PT_Schedule::~PT_Schedule()
{
	std::map<std::string, Busline*>::iterator it;
	for (it = buslineIdBuslineMap.begin(); it != buslineIdBuslineMap.end(); ++it) {
		safe_delete_item(it->second);
	}
	buslineIdBuslineMap.clear();
}

void sim_mob::PT_Schedule::registerBusLine(const std::string buslineId, Busline* aBusline)
{
	if (buslineIdBuslineMap.count(buslineId)>0) {
		throw std::runtime_error("Duplicate buslineid.");
	}

	buslineIdBuslineMap[buslineId] = aBusline;
}

void sim_mob::PT_Schedule::registerControlType(const std::string buslineId, const ControlTypes controlType)
{
	if (buslineIdControlTypeMap.count(buslineId)>0) {
		throw std::runtime_error("Duplicate buslineId.");
	}

	buslineIdControlTypeMap[buslineId] = controlType;
}

Busline* sim_mob::PT_Schedule::findBusline(const std::string& buslineId)
{
	std::map<std::string, Busline*>::const_iterator it;
	it = buslineIdBuslineMap.find(buslineId);
	if (it!=buslineIdBuslineMap.end()) {
		return it->second;
	}
	return nullptr;
}

ControlTypes sim_mob::PT_Schedule::findBuslineControlType(const std::string& buslineId)
{
	std::map<std::string, ControlTypes>::const_iterator it;
	it = buslineIdControlTypeMap.find(buslineId);
	if (it!=buslineIdControlTypeMap.end()) {
		return it->second;
	}
	return NO_CONTROL;
}
