//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "message/MessageBus.hpp"
#include "message/MessageHandler.hpp"
#include "message/MT_Message.hpp"

namespace sim_mob
{
namespace medium
{

using namespace messaging;

/**
 * simple struct to hold waiting time information for a person
 */
struct PersonWaitingTime
{
	/** id of person who submitted this waiting time record*/
	unsigned int personId;
	/** stop number of bus stop where the person is/was waiting*/
	std::string busStopNo;
	/** hh:mi:ss format time at which this waiting time information was collected*/
	std::string currentTime;
	/** waiting time in seconds*/
	double waitingTime;
	/** bus line for which the person waited*/
	std::string busLine;
	/** number of times this person was denied boarding before he got a chance to board a bus */
	unsigned int deniedBoardingCount;

	/**
	 * constructs a string of comma separated values to be printed in output files
	 * @returns printable csv string
	 */
	std::string getCSV() const;
};

/**
 * Message to transfer person waiting time at bus stop
 */
class PersonWaitingTimeMessage: public messaging::Message
{
public:
	PersonWaitingTimeMessage(const PersonWaitingTime& personWaitingInfo) : personWaitingTime(personWaitingInfo)	{}
	virtual ~PersonWaitingTimeMessage() {}
	PersonWaitingTime personWaitingTime;
};

/**
 * simple struct to store arrival time at bus stop and pertinent info for buses
 */
struct BusArrivalTime
{
	/** bus stop number*/
	std::string busStopNo;
	/** bus line*/
	std::string busLine;
	/** trip number for the bus line*/
	std::string tripId;
	/** bus stop sequence number*/
	unsigned int sequenceNo;
	/** arrival time at bus stop in hh:mi:ss format*/
	std::string arrivalTime;
	/** dwell time at bus stop	 */
	std::string dwellTime;
	/** dwell time at bus stop in seconds */
	double dwellTimeSecs;
	/** percentage occupancy of bus when it arrives at this stop*/
	double pctOccupancy; //percentage

	/**
	 * constructs a string of comma separated values to be printed in output files
	 * @returns printable csv string
	 */
	std::string getCSV() const;
};

/**
 * Message to transfer bus arrival time at bus stop
 */
class BusArrivalTimeMessage : public messaging::Message
{
public:
	BusArrivalTimeMessage(const BusArrivalTime& busArrivalInfo) : busArrivalInfo(busArrivalInfo) {}
	virtual ~BusArrivalTimeMessage() {}
	BusArrivalTime busArrivalInfo;
};

/**
 * struct to store travel time of persons taking public transit
 */
struct PersonTravelTime
{
	/** person id*/
	unsigned int personId;
	/** start location of trip*/
	std::string tripStartPoint;
	/** end location of trip*/
	std::string tripEndPoint;
	/** start location of subtrip*/
	std::string subStartPoint;
	/** end location of subtrip*/
	std::string subEndPoint;
	/** start location type - busstop/node/mrtstop etc. */
	std::string subStartType;
	/** end location type - busstop/node/mrtstop etc. */
	std::string subEndType;
	/** mode of travel*/
	std::string mode;
	/** pt line id */
	std::string service;
	/** arrival time at end point*/
	std::string arrivalTime;
	/** travel time for this sub trip in seconds */
	double travelTime;

	/**
	 * constructs a string of comma separated values to be printed in output files
	 * @returns printable csv string
	 */
	std::string getCSV() const;
};

/**
 * Message for person travel time
 */
class PersonTravelTimeMessage: public messaging::Message
{
public:
	PersonTravelTimeMessage(const PersonTravelTime& personTravelTime) : personTravelTime(personTravelTime) {}
	virtual ~PersonTravelTimeMessage() {}

	PersonTravelTime personTravelTime;
};

/**
 * struct to store number of persons waiting at a stop
 */
struct WaitingCount
{
	/** time in hh:mi:ss format when this information is collected*/
	std::string currTime;
	/** bus stop number*/
	std::string busStopNo;
	/** count of persons waiting at stop*/
	unsigned int count;

	/**
	 * constructs a string of comma separated values to be printed in output files
	 * @returns printable csv string
	 */
	std::string getCSV() const;
};

/**
 * waiting count message
 */
class WaitingCountMessage: public messaging::Message
{
public:
	WaitingCountMessage(const WaitingCount& cnt) : waitingCnt(cnt) {}
	virtual ~WaitingCountMessage() {}
	WaitingCount waitingCnt;
};

/**
 *  Class to store PT stop related stats for a particular interval
 *  This class is intended to be used for both bus stops and MRT stops
 */
class StopStats
{
public:
	StopStats() : needsInitialization(true), serviceLine(std::string()), stopCode(std::string()), interval(0), waitingTime(0), waitingCount(0), dwellTime(0), numArrivals(0)
	{}

	/** initialization flag */
	bool needsInitialization;
	/** PT service line id (bus line id) */
	std::string serviceLine;
	/** PT stop code (bus stop code) */
	std::string stopCode;
	/** index of time interval */
	unsigned int interval;
	/** average waiting time for stop with code stopCode, for line identified by serviceLine for this time interval*/
	double waitingTime;
	/** number of persons who contributed to this waiting time */
	double waitingCount;
	/** average dwell time at stop with code stopCode, for line identified by serviceLine for this time interval*/
	double dwellTime;
	/** number of PT vehicle (bus or train) arrivals for stop with code stopCode and for line identified by serviceLine for this time interval*/
	double numArrivals;

	/**
	 * constructs a string of comma separated values to be printed in output files
	 * @returns printable csv string
	 */
	std::string getCSV() const;
};

/**
 * class to load, track and store PT stop related statistics
 */
class StopStatsManager
{
private:
	/**
	 * map of interval => [stopCode => [serviceLine => StopStats] ] collected in current simulation
	 */
	std::map<unsigned int, std::map<std::string, std::map<std::string, StopStats> > > stopStatsMap;

	/**
	 * map of interval => [stopCode => [serviceLine => StopStats] ] loaded from previous simulations
	 */
	std::map<unsigned int, std::map<std::string, std::map<std::string, StopStats> > > historicalStopStatsMap;

	/** width of an interval in seconds */
	unsigned int intervalWidth;

	unsigned int getTimeInSecs(const std::string& time) const;

public:
	StopStatsManager();

	void addStopStats(const BusArrivalTime& busArrival);

	void addStopStats(const PersonWaitingTime& personWaiting);

	void exportStopStats();
};

/**
 * Statistics collector for PT entities.
 * \author Zhang Huai Peng
 * \author Harish Loganathan
 */
class PT_Statistics : public messaging::MessageHandler
{
public:
	virtual ~PT_Statistics();

	/**
	 * fetches singleton instance
	 */
	static PT_Statistics* getInstance();

	/**
	 * deletes singleton instance
	 */
	static void resetInstance();

	/**
	 * collect and store the data received through messages
	 */
	virtual void HandleMessage(Message::MessageType type, const Message& message);

	/**
	 * store collected statistics to output files
	 */
	void storeStatistics();

private:
	PT_Statistics();

	/**store stop to stop journey time. bus stop No. is key */
	std::vector<BusArrivalTime> busJourneyTimes;

	/**store for waiting time at bus stop*/
	std::map<std::string, PersonWaitingTime> personWaitingTimes;

	/**store for number of persons waiting at each bus stop */
	std::vector<WaitingCount> waitingCounts;

	/**travel time store*/
	std::vector<PersonTravelTime> personTravelTimes;

	/**PT stop statistics manager*/
	StopStatsManager stopStatsMgr;

	static PT_Statistics* instance;
};

}
}
