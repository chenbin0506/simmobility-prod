/*
 * PathSetManager.hpp
 *
 *  Created on: May 6, 2013
 *      Author: Max
 */

#pragma once

#include <vector>
#include <map>
#include <string>
#include <boost/lexical_cast.hpp>
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/streetdir/KShortestPathImpl.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/PathSet/PathSetDB.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/Link.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/Person.hpp"
#include "RoadNetwork.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "util/Profiler.hpp"
#include "soci.h"
#include "soci-postgresql.h"

namespace sim_mob
{
class PathSet;
class SinglePath;
class ConfigParams;
class Loader;
class ERP_Gantry_Zone;
class ERP_Section;
class ERP_Surcharge;
class Link_travel_time;
class DatabaseLoader2;
class K_ShortestPathImpl;
class Link;
namespace batched {
class ThreadPool;
}
class PathSetDBLoader
{
public:
	PathSetDBLoader(std::string dbstr)
	:sql(soci::postgresql,dbstr) {}
	soci::session sql;
};

class PathSetParam {
private:
	PathSetParam();
	static PathSetParam *instance_;
	/// soci not thread-safe...
	boost::mutex soci_mutex;

public:
	static PathSetParam *getInstance();
	sim_mob::RoadSegment* getRoadSegmentByAimsunId(std::string id);
	// retrieve
	void getDataFromDB();
	void storeSinglePath(soci::session& sql,std::vector<sim_mob::SinglePath*>& spPool,const std::string singlePathTableName);
	void storePathSet(soci::session& sql,std::map<std::string,sim_mob::PathSet* >& psPool,const std::string pathSetTableName);
	void setTravleTimeTmpTableName(const std::string& value);
	bool createTravelTimeTmpTable();
	bool dropTravelTimeTmpTable();
	bool createTravelTimeRealtimeTable();

	double getAverageTravelTimeBySegIdStartEndTime(std::string id,sim_mob::DailyTime startTime,sim_mob::DailyTime endTime);
	double getDefaultTravelTimeBySegId(std::string id);
	double getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime);
	sim_mob::WayPoint* getWayPointBySeg(const sim_mob::RoadSegment* seg);
	sim_mob::Node* getNodeByAimsunId(std::string id);
	double getHighwayBias() { return highway_bias; }
	void initParameters();
	double bTTVOT;
	double bCommonFactor;
	double bLength;
	double bHighway;
	double bCost;
	double bSigInter;
	double bLeftTurns;
	double bWork;
	double bLeisure;
	double highway_bias;
	double minTravelTimeParam;
	double minDistanceParam;
	double minSignalParam;
	double maxHighwayParam;
	bool isUseCache;

	std::map<std::string,sim_mob::RoadSegment*> segPool; // store all segs <aimsun id ,seg>
	std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*> wpPool; // <seg , value>
	std::map<std::string,sim_mob::Node*> nodePool; // store all nodes <aimsun id ,node>

	const sim_mob::RoadNetwork* roadNetwork;
	std::vector<sim_mob::MultiNode*> multiNodesPool; //store all multi nodes in the map
	std::set<sim_mob::UniNode*> uniNodesPool; // store all uni nodes

	std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_Surcharge_pool; // <Gantry_No , value=ERP_Surcharge with same No diff time stamp>
	std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_Zone_pool; //<Gantry_no, ERP_Gantry_Zone>
	std::map<std::string,sim_mob::ERP_Section*> ERP_Section_pool;  // <aim-sun id , ERP_Section>
	//
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> > Link_default_travel_time_pool; // <segment aim-sun id ,Link_default_travel_time with diff time stamp>
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> > Link_realtime_travel_time_pool;

	std::string pathset_traveltime_realtime_table_name; // table store travel time ,used to calculate pathset size
	std::string pathset_traveltime_tmp_table_name; // table store travel time generated by runtime, use for next simulation
public:
	//unused
	bool insertTravelTimeCSV2TmpTable(std::ofstream& csvFile,std::string& csvFileName);
	bool copyTravelTimeDataFromTmp2RealtimeTable(std::ofstream& csvFile,std::string& csvFileName);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////
class ERP_Gantry_Zone
{
public:
	ERP_Gantry_Zone() {}
	ERP_Gantry_Zone(ERP_Gantry_Zone &src):Gantry_no(src.Gantry_no),Zone_Id(src.Zone_Id) {}
	std::string Gantry_no;
	std::string Zone_Id;
};
class ERP_Section
{
public:
	ERP_Section() {}
	ERP_Section(ERP_Section &src);
	int section_id;
	int ERP_Gantry_No;
	std::string ERP_Gantry_No_str;
	OpaqueProperty<int> originalSectionDB_ID;  // seg aim-sun id ,rs->originalDB_ID.setProps("aimsun-id", currSec->id);
};
class ERP_Surcharge
{
public:
	ERP_Surcharge() {}
	ERP_Surcharge(ERP_Surcharge& src):Gantry_No(src.Gantry_No),Start_Time(src.Start_Time),End_Time(src.End_Time),Rate(src.Rate),
			Vehicle_Type_Id(src.Vehicle_Type_Id),Vehicle_Type_Desc(src.Vehicle_Type_Desc),Day(src.Day),
			start_time_dt(sim_mob::DailyTime(src.Start_Time)),end_time_dt(sim_mob::DailyTime(src.End_Time)){}
	std::string Gantry_No;
	std::string Start_Time;
	std::string End_Time;
	sim_mob::DailyTime start_time_dt;
	sim_mob::DailyTime end_time_dt;
	double Rate;
	int Vehicle_Type_Id;
	std::string Vehicle_Type_Desc;
	std::string Day;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Link_travel_time
{
public:
	Link_travel_time() {};
	Link_travel_time(Link_travel_time& src);
	int link_id;
	std::string start_time;
	std::string end_time;
	sim_mob::DailyTime start_time_dt;
	sim_mob::DailyTime end_time_dt;
	double travel_time;
	OpaqueProperty<int> originalSectionDB_ID;

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// length of a path with segments in meter
inline double generateSinglePathLengthPT(std::vector<WayPoint*>& wp)
{
	double res=0;
	for(int i=0;i<wp.size();++i)
	{
		WayPoint* w = wp[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			res += seg->length;
		}
	}
	return res/100.0; //meter
}
enum TRIP_PURPOSE
{
	work = 1,
	leisure = 2
};

template<typename T>
std::string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

///a cache structure. used in pathsetmanager
struct personOD{
	const sim_mob::Person* per;
	const sim_mob::SubTrip *subtrip;//find the OD in here.
	personOD(const sim_mob::Person* per,const sim_mob::SubTrip *subtrip):
		per(per),subtrip(subtrip)
	{}
};

/// Roadsegment-Person-O-D
typedef std::multimap<const sim_mob::RoadSegment*, personOD > RPOD ;// Roadsegment-Person-O-D  :)

class PathSetManager {
	///option to wait for each group of pathset generation to complete before starting another group
	///for all 'link eliminations' and 'random perturbation'
	bool serialPathSetGroup;
public:
	static PathSetManager* getInstance()
	{
//		static PathSetManager instance;
//		return instance;
		if(!instance_)
		{
			instance_ = new PathSetManager();
		}
		return instance_;
	}

	static sim_mob::Profiler profiler;
	sim_mob::batched::ThreadPool *threadpool_;
public:
	bool generateAllPathSetWithTripChain2();
	sim_mob::SinglePath * generateSinglePathByFromToNodes(const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,const sim_mob::RoadSegment* exclude_seg=NULL);
	sim_mob::SinglePath *  generateSinglePathByFromToNodes3(
			   const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
			   std::map<std::string,SinglePath*>& wp_spPool,
			   const std::set<const sim_mob::RoadSegment*> & exclude_seg=std::set<const sim_mob::RoadSegment*>());

	sim_mob::SinglePath* generateShortestTravelTimePath(const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
			   std::map<std::string,SinglePath*>& wp_spPool,
			   sim_mob::TimeRange tr = sim_mob::MorningPeak,
			   const sim_mob::RoadSegment* exclude_seg=NULL,
			   int random_graph_idx=0);
	PathSet *generatePathSetByFromToNodes(const sim_mob::Node *fromNode,
			const sim_mob::Node *toNode,
			const sim_mob::SubTrip* st,
			bool isUseCache=true);
	bool LoadSinglePathDBwithId(
			std::string& pathset_id,
			std::vector<sim_mob::SinglePath*>& spPool);
	std::map<std::string,sim_mob::PathSet*> generatePathSetByTripChainItemPool(std::vector<sim_mob::TripChainItem*> &tci);
	void  generatePathSetByTrip(std::map<std::string,sim_mob::PathSet*> &subTripId_pathSet,sim_mob::Trip *trip);
	bool isUseCacheMode() { return isUseCache; }
	double getUtilityBySinglePath(sim_mob::SinglePath* sp);
	std::vector<WayPoint> generateBestPathChoice2(const sim_mob::SubTrip* st);
	std::vector<WayPoint> generateBestPathChoiceMT(const sim_mob::SubTrip* st, Profiler & personProfiler, const std::set<const sim_mob::RoadSegment*> & exclude_seg=std::set<const sim_mob::RoadSegment*>(), bool isUseCache = true, bool isUseDB = true);
	std::vector<WayPoint> generateBestPathChoiceMT(const sim_mob::Person * per, const sim_mob::SubTrip* st, const std::set<const sim_mob::RoadSegment*> & exclude_seg=std::set<const sim_mob::RoadSegment*>(), bool isUseCache = true, bool isUseDB = true);
	bool generateAllPathChoicesMT(PathSet* ps, Profiler & personProfiler, const std::set<const sim_mob::RoadSegment*> & exclude_seg=std::set<const sim_mob::RoadSegment*>());
	void generateTravelTimeSinglePathes(const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
			   std::map<std::string,SinglePath*>& wp_spPool,
			   sim_mob::PathSet& ps_);
	void generatePathesByLinkElimination(std::vector<WayPoint*>& path,
			std::map<std::string,SinglePath*>& wp_spPool,
			sim_mob::PathSet& ps_,
			const sim_mob::Node* fromNode,
			const sim_mob::Node* toNode);
	void generatePathesByTravelTimeLinkElimination(std::vector<WayPoint*>& path,
				std::map<std::string,SinglePath*>& wp_spPool,
				sim_mob::PathSet& ps_,
				const sim_mob::Node* fromNode,
				const sim_mob::Node* toNode,
				sim_mob::TimeRange tr);
	bool getBestPathChoiceFromPathSet(sim_mob::PathSet& ps);
	void initParameters();
	// get
	std::vector<WayPoint> getPathByPerson(sim_mob::Person* per); // person has person id and current subtrip id
	sim_mob::PathSet* getPathSetByFromToNodeAimsunId(std::string id);
	bool getSinglePathById(std::string &id,sim_mob::SinglePath** s);
	std::map<const sim_mob::RoadSegment*,SinglePath*> getseg_pathSetNull() { return seg_pathSetNull; }
	double getTravelTime(sim_mob::SinglePath *sp);
	double getTravelTimeBySegId(std::string id,sim_mob::DailyTime startTime);
	// store
	void storePersonIdPathSets(std::string personId,std::map<std::string,sim_mob::PathSet*> subTripId_pathSet);
	unsigned long size();
	std::string& getTravleTimeTmpTableName() { return pathset_traveltime_tmp_table_name; }
	std::string& getTravleTimeRealtimeTableName() { return pathset_traveltime_realtime_table_name; }
	void setCSVFileName();
	bool insertTravelTime2TmpTable(sim_mob::Link_travel_time& data);
	bool copyTravelTimeDataFromTmp2RealtimeTable();
	void init();
	void clearPools();

	void setScenarioName(std::string& name){ scenarioName = name; }
	void insertFromTo_BestPath_Pool(std::string& id ,std::vector<WayPoint>& value);
	bool getCachedBestPath(std::string id, std::vector<WayPoint> & value);
	void cacheODbySegment(const sim_mob::Person*,const SubTrip *,std::vector<WayPoint> &);
	const std::pair<RPOD::const_iterator,RPOD::const_iterator > getODbySegment(const sim_mob::RoadSegment* segment) const;
	PathSetDBLoader *psDbLoader;
	soci::session *sql;

	PathSetManager();
	~PathSetManager();

private:
	static PathSetManager *instance_;
	StreetDirectory* stdir;
	const sim_mob::RoadNetwork* roadNetwork;
	std::vector<sim_mob::MultiNode*> multiNodesPool; //store all multi nodes in the map
	std::set<sim_mob::UniNode*> uniNodesPool; // store all uni nodes

	std::map<std::string,sim_mob::PathSet* > pathSetPool; // store all pathset , key = from node aimsun id + to node aimsun id
														  // value = pathset
	std::map<std::string,sim_mob::RoadSegment*> segPool; // store all segs ,key= aimsun id ,value = seg
	std::map<const sim_mob::RoadSegment*,sim_mob::WayPoint*> wpPool; // key=seg , value=wp point
	std::map<std::string,sim_mob::Node*> nodePool; // store all nodes ,key= aimsun id ,value = node
	std::map<std::string,SinglePath*> waypoint_singlepathPool; // key is waypoints' Segment1AimsunId_Segment2AimsunId_Segment2AimsunId....
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 // value is SinglePath
	std::map<const sim_mob::RoadSegment*,SinglePath*> seg_pathSetNull;
	// use sbutrip id as key, b/c each trip as least as one subtrip
	std::map<std::string, std::map<std::string,sim_mob::PathSet*> > personPathSetPool;    //map<personID,map<subTripId,pathset> >
	//
	std::string scenarioName;
	//
	std::map<std::string ,std::vector<WayPoint> > fromto_bestPath;
	std::map<std::string ,std::vector<WayPoint> >::iterator fromto_bestPath_it;

	///a cache to help answer this question: a given road segment is within which path(s)
	RPOD pathSegments;


	std::string csvFileName;
	std::ofstream csvFile;
	//
	std::map<std::string,std::vector<sim_mob::ERP_Surcharge*> > ERP_Surcharge_pool; // key=Gantry_No , value=ERP_Surcharge with same No diff time stamp
	std::map<std::string,sim_mob::ERP_Gantry_Zone*> ERP_Gantry_Zone_pool; //key=Gantry_no, value = ERP_Gantry_Zone
	std::map<std::string,sim_mob::ERP_Section*> ERP_Section_pool;  // key=aim-sun id , value = ERP_Section

	std::map<std::string,std::vector<sim_mob::Link_travel_time*> > Link_default_travel_time_pool; // key= segment aim-sun id value=Link_default_travel_time with diff time stamp
	std::map<std::string,std::vector<sim_mob::Link_travel_time*> > Link_realtime_travel_time_pool;

	sim_mob::K_ShortestPathImpl *kshortestImpl;

	double bTTVOT;
	double bCommonFactor;
	double bLength;
	double bHighway;
	double bCost;
	double bSigInter;
	double bLeftTurns;
	double bWork;
	double bLeisure;
	double highway_bias;
	double minTravelTimeParam;
	double minDistanceParam;
	double minSignalParam;
	double maxHighwayParam;

	bool isUseCache;
	std::string pathset_traveltime_realtime_table_name; // table store travel time ,used to calculate pathset size
	std::string pathset_traveltime_tmp_table_name; // table store travel time generated by runtime, use for next simulation

	PathSetParam *pathSetParam;
	/*unused*/
public:
	double getTravelCost(sim_mob::SinglePath *sp);
	sim_mob::SinglePath* getSinglePath(std::string id);
	bool getWayPointPath2(std::vector<WayPoint> &wp,sim_mob::SinglePath** s);
	int generateSinglePathByFromToNodes_(
			   const sim_mob::Node *fromNode,
			   const sim_mob::Node *toNode,
			   std::map<std::string,SinglePath*>& wp_spPool,
			   const sim_mob::RoadSegment* exclude_seg);

	bool generateSinglePathByFromToNodes2(
			const sim_mob::Node *fromNode,
			const sim_mob::Node *toNode,
			sim_mob::SinglePath& sp,
			const sim_mob::RoadSegment* exclude_seg);
	void generatePaths2Node(const sim_mob::Node *toNode);
	sim_mob::PathSet* getPathSetByPersonIdAndSubTripId(std::string personId,std::string subTripId);
	std::vector<WayPoint> generateBestPathChoice(sim_mob::Person* per,
			sim_mob::PathSet* ps,bool isReGenerate=false);
	const sim_mob::Node* getToNodefromTripChainItems(std::vector<sim_mob::TripChainItem*> &tci);
	const sim_mob::Node* getFromNodefromTripChainItems(std::vector<sim_mob::TripChainItem*> &tci);
	sim_mob::PathSet* generatePathSetBySubTrip(const sim_mob::SubTrip* st);
	void storePath(sim_mob::SinglePath* singlePath);
	void generateAllPathSetWithTripChainPool(std::map<std::string, std::vector<sim_mob::TripChainItem*> > *tripChainPool);
	bool generateAllPathSetWithTripChain();
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SinglePath
{
public:
	SinglePath() : purpose(work),utility(0.0),pathsize(0.0),travel_cost(0.0),
	signal_number(0.0),right_turn_number(0.0),length(0.0),travle_time(0.0),highWayDistance(0.0),
	isMinTravelTime(0),isMinDistance(0),isMinSignal(0),isMinRightTurn(0),isMaxHighWayUsage(0){}
	SinglePath(SinglePath &source);
	void init(std::vector<WayPoint>& wpPools);
	void clear();
	std::vector<WayPoint*> shortestWayPointpath;
	std::map<const RoadSegment*,WayPoint*> shortestSegPath;
	PathSet *pathSet; // parent
	const sim_mob::RoadSegment* excludeSeg; // can be null
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;

	double highWayDistance;
	int isMinTravelTime;
	int isMinDistance;
	int isMinSignal;
	int isMinRightTurn;
	int isMaxHighWayUsage;

	bool isNeedSave2DB;
	std::string id;   //id: seg1id_seg2id_seg3id
	std::string pathset_id;
	double utility;
	double pathsize;
	double travel_cost;
	int signal_number;
	int right_turn_number;
	std::string scenario;
	double length;
	double travle_time;
	sim_mob::TRIP_PURPOSE purpose;
	/*unused*/

	SinglePath(SinglePath *source);
	SinglePath(SinglePath *source,const sim_mob::RoadSegment* seg);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PathSet
{
public:
	PathSet():has_path(0) {};
	PathSet(const sim_mob::Node *fn,const sim_mob::Node *tn) : fromNode(fn),toNode(tn),logsum(0),has_path(0) {}
	PathSet(const PathSet &ps);
	~PathSet();
	bool isInit;
	bool hasBestChoice;
	std::vector<WayPoint*> bestWayPointpathP;  //best choice
	const sim_mob::Node *fromNode;
	const sim_mob::Node *toNode;
	std::string personId; //person id
	std::string tripId; // trip item id
	SinglePath* oriPath;  // shortest path with all segments
	std::vector<sim_mob::SinglePath*> pathChoices;
	std::map<std::string,sim_mob::SinglePath*> SinglePathPool;
	bool isNeedSave2DB;
	double logsum;
	const sim_mob::SubTrip* subTrip; // pathset use info of subtrip to generate all things
	std::string id;
	std::string from_node_id;
	std::string to_node_id;
	std::string singlepath_id;
	std::string excludedPaths;
	std::string scenario;
	int has_path;
public:
	PathSetManager *psMgr;

	/*unused*/

	PathSet(PathSet *ps);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline double getTravelCost2(sim_mob::SinglePath *sp);

inline std::string makeWaypointsetString(std::vector<WayPoint>& wp);

std::string getNumberFromAimsunId(std::string &aimsunid);

std::vector<WayPoint*> convertWaypoint2Point(std::vector<WayPoint> wp);

std::vector<WayPoint> convertWaypointP2Wp(std::vector<WayPoint*> wp);

inline double generateSinglePathLength(std::vector<WayPoint*>& wp) // unit is meter
{
	double res=0;
	for(int i=0;i<wp.size();++i)
	{
		WayPoint* w = wp[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			res += seg->length;
		}
	}
	return res/100.0; //meter
}

inline void generatePathSizeForPathSet2(sim_mob::PathSet *ps,bool isUseCache=true);
inline std::map<const RoadSegment*,WayPoint*> generateSegPathByWaypointPathP(std::vector<WayPoint*>& wp);
inline size_t getLaneIndex2(const Lane* l);

inline void calculateRightTurnNumberAndSignalNumberByWaypoints(sim_mob::SinglePath *sp);

inline double calculateHighWayDistance(sim_mob::SinglePath *sp)
{
	double res=0;
	if(!sp) return 0.0;
	for(int i=0;i<sp->shortestWayPointpath.size();++i)
	{
		sim_mob::WayPoint* w = sp->shortestWayPointpath[i];
		if (w->type_ == WayPoint::ROAD_SEGMENT) {
			const sim_mob::RoadSegment* seg = w->roadSegment_;
			if(seg->maxSpeed >= 60)
			{
				res += seg->length;
			}
		}
	}
	return res/100.0; //meter
}

inline float gen_random_float(float min, float max)
{
    boost::mt19937 rng;
    boost::uniform_real<float> u(min, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(rng, u);
    return gen();
}

inline void myReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != std::string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}
//unused
inline int calculateRightTurnNumberByWaypoints(std::map<const RoadSegment*,WayPoint>& segWp);
inline std::map<const RoadSegment*,WayPoint> generateSegPathByWaypointPath(std::vector<WayPoint>& wp);
}//namespace
