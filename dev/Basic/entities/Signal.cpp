/* Copyright Singapore-MIT Alliance for Research and Technology */

/*
 * Signal.cpp
 *
 *  Created on: 2011-7-18
 *      Author: xrm
 */

#include "Signal.hpp"
#include <math.h>
#include "geospatial/Lane.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "util/OutputUtil.hpp"
using namespace sim_mob;

double Density[] = {1, 1, 1, 1};
double DS_all;

/* static */ std::vector<Signal const *> Signal::all_signals_;

//Private namespace
namespace {
	//parameters for calculating next cycle length
	const double  DSmax = 0.9, DSmed = 0.5, DSmin = 0.3;
	const double CLmax = 140, CLmed = 100, CLmin = 60;

	//parameters for calculating next Offset
	const double CL_low = 70, CL_up = 120;
	const double Off_low = 5, Off_up = 26;
}


const double sim_mob::Signal::SplitPlan1[] = {0.30, 0.30, 0.20, 0.20};
const double sim_mob::Signal::SplitPlan2[] = {0.20, 0.35, 0.20, 0.25};
const double sim_mob::Signal::SplitPlan3[] = {0.35, 0.35, 0.20, 0.10};
const double sim_mob::Signal::SplitPlan4[] = {0.35, 0.30, 0.10, 0.25};
const double sim_mob::Signal::SplitPlan5[] = {0.20, 0.35, 0.25, 0.20};

//Signal* sim_mob::Signal::instance_ = NULL;

sim_mob :: Signal :: Signal(unsigned int id, Node const & node): Agent(id), node_(node)
{
	initializeSignal();
        setupIndexMaps();
}


void sim_mob :: Signal :: initializeSignal()
{
	setCL(60,60,60);//default initial cycle length for SCATS
	setRL(60,60);//default initial RL for SCATS
	startSplitPlan();
	currPhase = 0;
	phaseCounter = 0;
	updateTrafficLights();

}

namespace
{
    struct Tuple
    {
        Link const * link;
        double angle;
        Crossing const * crossing;

        Tuple(Link const * link, double angle, Crossing const * crossing)
          : link(link)
          , angle(angle)
          , crossing(crossing)
        {
        }
    };

    // This functor is used as the sorting criteria in Signal::setupIndexMaps().
    struct Compare
    {
        bool operator()(Tuple const & t1, Tuple const & t2) const
        {
            return t1.angle < t2.angle;
        }
    };

    // This functor calculates the angle between a link and a reference link, which have a node
    // in common.
    struct AngleCalculator
    {
        // <node> is the node in common and <refLink> is the reference link.  Therefore <node>
        // must be one of the ends of <refLink>.
        AngleCalculator(Node const & node, Link const * refLink)
          : center_(node)
        {
            assert(refLink->getStart() == &center_ || refLink->getEnd() == &center_);
            // <refAngle_> is the angle that the <refLink> makes with the X-axis.
            refAngle_ = angle(refLink);
        }

        // Calculates the angle between <link> and <refLink>, which have <node> in common.
        // Therefore <node> must be one of the ends of <link>.
        double operator()(Link const * link) const
        {
            assert(link->getStart() == &center_ || link->getEnd() == &center_);
            return angle(link) - refAngle_;
        }

        Node const & center_;
        double refAngle_;

        // Caculates the angle that <link> with respect to the X-axis.
        double angle(Link const * link) const
        {
            Point2D point;
            if (link->getStart() == &center_)
                point = *(link->getEnd()->location);
            else
                point = *(link->getStart()->location);
            double xDiff = point.getX() - center_.location->getX();
            double yDiff = point.getY() - center_.location->getY();
            return atan2(yDiff, xDiff);
        }
    };

    // Return the Crossing object, if any, in the specified road segment.  If there are more
    // than one Crossing objects, return the one that has the least offset.
    Crossing const *
    getCrossing(RoadSegment const * road)
    {
        //Crossing const * result = 0;
        double offset = std::numeric_limits<double>::max();

        //std::map<int, RoadItem const *> const & obstacles = road->obstacles;
        //std::map<int, RoadItem const *>::const_iterator iter;

        int currOffset = static_cast<int>(offset);
        for (;;) {
        	//Get the next item, if any.
        	RoadItemAndOffsetPair res = road->nextObstacle(currOffset, true);
        	if (!res.item) {
        		break;
        	}

        	//Check if it's a Crossing.
        	if (Crossing const * crossing = dynamic_cast<Crossing const *>(res.item))
        	{
        		//Success
        		return crossing;
        	}

        	//Increment
        	currOffset += res.offset;
        }

        //Failure.
        return nullptr;
    }
}

// Populate the links_map_ and crossing_map_
//
// Currently the Signal class is designed for 4-way traffic at intersections with 4 links.
// As such, it uses arrays of size 4 for the traffic color for vehicle and pedestrian traffic
// in each link.  The order of the array follows the geospatial layout of the links -- array[n]
// and array[n+1] are for adjacent lanes.
//
// The links_map_ and crossing_map_ are intended to translate from the Link and Crossing classes
// to indexes into the abovementioned arrays in the getDriverLight() and getPedestrianLight()
// methods.
void
sim_mob::Signal::setupIndexMaps()
{
    // Currently, we assume Signals are located at Multinodes.  We need to handle the cases
    // where Signals are located at Uninodes or even at obstacle locations (the obstacle being
    // a Crossing).
    MultiNode const & multinode = dynamic_cast<MultiNode const &>(node_);
    std::set<RoadSegment*> const & roads = multinode.getRoadSegments();

    // The algorithm implemented here arbitrarily selects a link to use as a reference link. 
    // It calculates the angle between this reference link and the other links.  When the links
    // are inserted into <bag>, <Compare> will sort them accordingly to their angle.  Thus links
    // that are geospatially adjacent will also be "adjacent" inside <bag>.
    std::set<Tuple, Compare> bag;
    std::set<Link const *> links;   // used to filter out duplicate links.

    // Push the first link into <bag>, and use that link as the ref-link.  Being the ref-link,
    // this item has an angle of 0.
    std::set<RoadSegment*>::const_iterator iter = roads.begin();
    RoadSegment const * road = *iter;
    Crossing const * crossing = getCrossing(road);
    Link const * link = road->getLink();
    links.insert(link);
    bag.insert(Tuple(link, 0, crossing));
    ++iter;

    AngleCalculator angle(node_, link);
    for (; iter != roads.end(); ++iter)
    {
        road = *iter;
        crossing = getCrossing(road);
        link = road->getLink();
        if (0 == links.count(link))  // check if link is not a duplicate.
        {
            bag.insert(Tuple(link, angle(link), crossing));
            links.insert(link);
        }
    }

    // Phase 2: populate the maps.
    std::ostringstream output;
    output << "(\"Signal-location\", 0, " << getId() << ", " << this << ", {\"node\":\""
           << &node_ << "\",";
    size_t i;
    std::set<Tuple, Compare>::const_iterator iter2;
    for (i = 0, iter2 = bag.begin(); iter2 != bag.end(); ++i, ++iter2)
    {
        Tuple const & tuple = *iter2;
        links_map_.insert(std::make_pair(tuple.link, i));
        crossings_map_.insert(std::make_pair(tuple.crossing, i));

        Point2D* point;
        if (tuple.link->getStart() == &node_)
        {
            point = tuple.link->getEnd()->location;
        }
        else
        {
            point = tuple.link->getStart()->location;
        }
        if (i)
            output << ",";
        output << "\"v" << i["abcd"] << "\":\"" << tuple.link
               << "\",\"a" << "abcd"[i] << "\":\"" << 180 * (tuple.angle / M_PI)
               << "\",\"p" << i["abcd"] << "\":\"" << tuple.crossing << "\"";
    }
    output << "})" << std::endl;
    LogOut(output.str());
}

//initialize SplitPlan
void sim_mob :: Signal :: startSplitPlan()
{
	//CurrSplitPlan
	currSplitPlanID=1;
	/*for(int i = 0; i < 4; i++) {
		currSplitPlan.push_back(SplitPlan1[i]);
	}*/
	currSplitPlan.assign(SplitPlan1, SplitPlan1+4);  //This does the same thing as the for loop
	nextSplitPlan.assign(4, 0); //Initialize to the number 0, four times.

	//initialize votes for choosing SplitPlan
	vote1 = 0;
	vote2 = 0;
	vote3 = 0;
	vote4 = 0;
	vote5 = 0;
}

void sim_mob :: Signal ::update(frame_t frameNumber)
{

                        std::stringstream logout;
	                logout <<"(\"Signal\","<<frameNumber<<","<<getId()<<",{\"va\":\"";
	                for(int i = 0; i<3; i++) {
	                        logout<<TC_for_Driver[0][i];
	                        if (i==2) {
	                                logout <<"\",";
	                        } else {
	                                logout <<",";
	                        }
	                }
	                logout <<"\"vb\":\"";
	                for(int i = 0; i<3; i++) {
	                        logout<<TC_for_Driver[1][i];
	                        if (i==2) {
	                                logout <<"\",";
	                        } else {
	                                logout <<",";
	                        }
	                }
	                logout <<"\"vc\":\"";
	                for(int i = 0; i<3; i++) {
	                        logout<<TC_for_Driver[2][i];
	                        if (i==2) {
	                                logout <<"\",";
	                        } else {
	                                logout <<",";
	                        }
	                }
	                logout <<"\"vd\":\"";
	                for(int i = 0; i<3; i++) {
	                        logout<<TC_for_Driver[3][i];
	                        if (i==2) {
	                                logout <<"\",";
	                        } else {
	                                logout <<",";
	                        }
	                }

	                logout <<"\"pa\":\""<<TC_for_Pedestrian[0]<<"\",";
	                logout <<"\"pb\":\""<<TC_for_Pedestrian[1]<<"\",";
	                logout <<"\"pc\":\""<<TC_for_Pedestrian[2]<<"\",";
	                logout <<"\"pd\":\""<<TC_for_Pedestrian[3]<<"\"})"<<std::endl;
                        LogOut(logout.str());


	updateSignal (Density);
}

//Update Signal Light
void sim_mob :: Signal :: updateSignal (double DS[])
{
	if(phaseCounter == 0){
	//find the maximum DS
	DS_all = fmax(DS);

	//use DS_all for calculating next cycle length
	setnextCL(DS_all);

	//use next cycle length to calculate next Offset
	setnextOffset(getnextCL());

	updateOffset();
	updateprevCL();
	updatecurrCL();
	setnextSplitPlan(DS);
	updatecurrSplitPlan();
	}


	if(phaseCounter<nextCL*nextSplitPlan[0])
	{
		if(phaseCounter <= (nextCL*nextSplitPlan[0] - 3) )currPhase=0;
		else currPhase=10;
	}
	else if(phaseCounter<nextCL*(nextSplitPlan[0]+nextSplitPlan[1]))
	{
		if(phaseCounter <= (nextCL*(nextSplitPlan[0]+nextSplitPlan[1]) - 3) )currPhase=1;
		else currPhase=11;
	}
	else if(phaseCounter<nextCL*(nextSplitPlan[0]+nextSplitPlan[1]+nextSplitPlan[2]))
	{
		if(phaseCounter <= (nextCL*(nextSplitPlan[0]+nextSplitPlan[1]+nextSplitPlan[2]) - 3) )currPhase=2;
		else currPhase=12;
	}
	else if(phaseCounter <= (nextCL-3))currPhase=3;
	else currPhase=13;

	phaseCounter++;

	if(phaseCounter == floor(nextCL)){
		phaseCounter = 0;
	}else{}

	updateTrafficLights();
}

int sim_mob :: Signal :: getcurrPhase()
{
	return currPhase;
}


//use SCATS to determine next cyecle length
void sim_mob :: Signal :: setnextCL (double DS)
{
	//parameters in SCATS
	double RL0;
	//double diff_CL,diff_CL0;
	double w1 = 0.45, w2 = 0.33, w3 = 0.22;


	//calculate RL0
	if (DS <= DSmed) {
		RL0 = CLmin + (DS - DSmin)*(CLmed - CLmin)/(DSmed - DSmin);
	} else { //if (DS>DSmed)
		RL0=CLmed + (DS - DSmed)*(CLmax - CLmed)/(DSmax - DSmed);
	}
	//else {}


	int sign;
	double diff_CL;
	if(RL0-currCL >= 0) {
		diff_CL = RL0 - currCL; sign = 1;
	} else {
		diff_CL = currCL - RL0; sign = -1;
	}



	//modify the diff_CL0
	double diff_CL0;
	if (diff_CL <= 4) {
		diff_CL0 = diff_CL;
	} else if (diff_CL > 4 && diff_CL <= 8) {
		diff_CL0 = 0.5*diff_CL + 2;
	} else {
		diff_CL0 = 0.25*diff_CL + 4;
	}

	double RL1 = currCL + sign*diff_CL0;

	//RL is partly determined by its previous values
	double RL = w1*RL1 + w2*prevRL1 + w3*prevRL2;

	//update previous RL
	prevRL2 = prevRL1;
	prevRL1 = RL1;


	sign = (RL >= currCL) ? 1 : -1;  //This is equivalent.
	/*if(RL >= currCL) {
		sign = 1;
	} else {
		sign = -1;
	}*/

	//set the maximum change as 6s
	if (abs(RL - currCL) <= 6) {
		nextCL = RL;
	} else {
		nextCL = currCL + sign*6;
	}

	//when the maximum changes in last two cycle are both larger than 6s, the value can be set as 9s
	if ( ((nextCL - currCL) >= 6 && (currCL - prevCL) >= 6) || ((nextCL - currCL) <= -6 && (currCL - prevCL) <= -6) ) {
		if (abs(RL-currCL) <= 9) {
			nextCL = RL;
		} else {
			nextCL = currCL + sign*9;
		}
	}
}


void sim_mob :: Signal :: updateprevCL() {
	prevCL=currCL;
}

void sim_mob :: Signal :: updatecurrCL() {
	currCL=nextCL;
}

void sim_mob :: Signal :: updateprevRL1 (double RL1){
	prevRL1=RL1;
}

void sim_mob :: Signal :: updateprevRL2 (double RL2){
	prevRL2=RL2;
}



//use DS to choose SplitPlan for next cycle
void sim_mob :: Signal :: setnextSplitPlan (double DS[])
{
	double proDS[4];// projected DS
	double maxproDS[6];// max projected DS of each SplitPlan
	//int i;

	//Calculate max proDS of SplitPlan1
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan1[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan1[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan1[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan1[3];
	maxproDS[1]=fmax(proDS);

	//Calculate max proDS of SplitPlan2
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan2[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan2[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan2[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan2[3];
	maxproDS[2]=fmax(proDS);

	//Calculate max proDS of SplitPlan3
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan3[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan3[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan3[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan3[3];
	maxproDS[3]=fmax(proDS);

	//Calculate max proDS of SplitPlan4
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan4[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan4[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan4[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan4[3];
	maxproDS[4]=fmax(proDS);

	//Calculate max proDS of SplitPlan5
	proDS[0] = DS[0] * currSplitPlan[0] / SplitPlan5[0];
	proDS[1] = DS[1] * currSplitPlan[1] / SplitPlan5[1];
	proDS[2] = DS[2] * currSplitPlan[2] / SplitPlan5[2];
	proDS[3] = DS[3] * currSplitPlan[3] / SplitPlan5[3];
	maxproDS[5]=fmax(proDS);

	//find the minimum value among the max projected DS, vote for its ID;
	vote1=fmin_ID(maxproDS);

	//next SplitPlan is determined by votes in last 5 cycles
	nextSplitPlanID=calvote(vote1,vote2,vote3,vote4,vote5);

	//update votes;
	vote5=vote4;vote4=vote3;vote3=vote2;vote2=vote1;

	//Get a reference to the SplitPlan array
	const double* SplitPlan = nullptr;

	//Retrieve the pointer
	switch(nextSplitPlanID)
	{
		case 1:
			SplitPlan = SplitPlan1;
			break;
		case 2:
			SplitPlan = SplitPlan2;
			break;
		case 3:
			SplitPlan = SplitPlan3;
			break;
		case 4:
			SplitPlan = SplitPlan4;
			break;
		case 5:
			SplitPlan = SplitPlan5;
			break;
		default:
			assert(false);
	}

	//Set the next split plan
	nextSplitPlan[0] = SplitPlan[0];
	nextSplitPlan[1] = SplitPlan[1];
	nextSplitPlan[2] = SplitPlan[2];
	nextSplitPlan[3] = SplitPlan[3];
}



void sim_mob :: Signal :: updatecurrSplitPlan() {
	currSplitPlanID = nextSplitPlanID;
	for(int i = 0; i < 4; i++) {
		currSplitPlan[i] = nextSplitPlan[i];
	}
}


//use next cycle length to calculate next Offset
void sim_mob :: Signal :: setnextOffset(double nextCL)
{
	if(nextCL <= CL_low) {
		nextOffset = Off_low;
	} else if(nextCL > CL_low && nextCL <= CL_up) {
		nextOffset = Off_low + (nextCL - CL_low)*(Off_up - Off_low)/(CL_up - CL_low);
	} else {
		nextOffset = Off_up;
	}
}


void sim_mob :: Signal :: updateOffset(){
	currOffset=nextOffset;
}


//NOTE: Helper arrays for setting TC_* data (in an anonymous namespace).
namespace {
//Pedestrian template
const int TC_for_PedestrianTemplate[][4] = {
	{1,3,1,3},   //Case  0
	{1,3,1,3},   //Case 10
	{1,1,1,1},   //Case  1
	{1,1,1,1},   //Case 11
	{3,1,3,1},   //Case  2
	{3,1,3,1},   //Case 12
	{1,1,1,1},   //Case  3
	{1,1,1,1},   //Case 13
};

//Driver template
const int TC_for_DriverTemplate[][4][3] = {
	{{3,3,1}, {1,1,1}, {3,3,1}, {1,1,1}},   //Case  0
	{{2,2,1}, {1,1,1}, {2,2,1}, {1,1,1}},   //Case 10
	{{1,1,3}, {1,1,1}, {1,1,3}, {1,1,1}},   //Case  1
	{{1,1,2}, {1,1,1}, {1,1,2}, {1,1,1}},   //Case 11
	{{1,1,1}, {3,3,1}, {1,1,1}, {3,3,1}},   //Case  2
	{{1,1,1}, {2,2,1}, {1,1,1}, {2,2,1}},   //Case 12
	{{1,1,1}, {1,1,3}, {1,1,1}, {1,1,3}},   //Case  3
	{{1,1,1}, {1,1,2}, {1,1,1}, {1,1,2}},   //Case 13
};

} //End anon namespace



//updata traffic lights information in a way that can be easily
//recognized by driver and pedestrian
void sim_mob :: Signal :: updateTrafficLights(){
	//Get a relative ID into the TS arrays.
	size_t relID = 0;
	switch(currPhase) {
		case 0:
			relID=0; break;
		case 10:
			relID=1; break;
		case 1:
			relID=2; break;
		case 11:
			relID=3; break;
		case 2:
			relID=4; break;
		case 12:
			relID=5; break;
		case 3:
			relID=6; break;
		case 13:
			relID=7; break;
		default:
			assert(false);

	}


	//Update
	for (size_t i=0; i<4; i++) {
		TC_for_Driver[i] = TC_for_DriverTemplate[relID][i];
	}
	TC_for_Pedestrian = TC_for_PedestrianTemplate[relID];

}

namespace
{
    sim_mob::Signal::TrafficColor
    convertToTrafficColor(int i)
    {
        switch (i)
        {
        case 1:
            return sim_mob::Signal::Red;
        case 2:
            return sim_mob::Signal::Amber;
        case 3:
            return sim_mob::Signal::Green;
        default:
            return sim_mob::Signal::Red;
        }
    }
}

namespace
{
    std::string
    mismatchError(char const * const func_name, Signal const & signal, RoadSegment const & road)
    {
        std::ostringstream stream;
        stream << func_name << ": mismatch in Signal and Lane; Details as follows" << std::endl;
        stream << "    Signal is located at (" << *signal.getNode().location << std::endl; 
        stream << "    Lane is part of RoadSegment going from " << *road.getStart()->location << " to "
               << *road.getEnd()->location << std::endl;
        return stream.str();
    }
}

sim_mob::Signal::VehicleTrafficColors
sim_mob::Signal::getDriverLight(Lane const & lane)
const
{
    RoadSegment const * road = lane.getRoadSegment();
    Link const * link = road->getLink();
    std::map<Link const *, size_t>::const_iterator iter = links_map_.find(link);
    if (iter == links_map_.end())
    {
        throw mismatchError("Signal::getDriverLight(lane)", *this, *road);
    }

    size_t index = iter->second;
    const int* threeIntegers = TC_for_Driver[index];
    TrafficColor left = convertToTrafficColor(threeIntegers[0]);
    TrafficColor forward = convertToTrafficColor(threeIntegers[1]);
    TrafficColor right = convertToTrafficColor(threeIntegers[2]);
    return VehicleTrafficColors(left, forward, right);
}

sim_mob::Signal::TrafficColor
sim_mob::Signal::getDriverLight(Lane const & fromLane, Lane const & toLane)
const
{
    RoadSegment const * fromRoad = fromLane.getRoadSegment();
    Link const * fromLink = fromRoad->getLink();
    std::map<Link const *, size_t>::const_iterator iter = links_map_.find(fromLink);
    if (iter == links_map_.end())
    {
        throw mismatchError("Signal::getDriverLight(fromLane, toLane)", *this, *fromRoad);
    }
    size_t fromIndex = iter->second;

    RoadSegment const * toRoad = toLane.getRoadSegment();
    Link const * toLink = toRoad->getLink();
    iter = links_map_.find(toLink);
    if (iter == links_map_.end())
    {
        throw mismatchError("Signal::getDriverLight(fromLane, toLane)", *this, *toRoad);
    }
    size_t toIndex = iter->second;

    // When links_map was populated in setupIndexMaps(), the links were numbered in anti-clockwise
    // direction.  The following switches are based on this fact.
    VehicleTrafficColors colors = getDriverLight(fromLane);
    if (fromIndex > toIndex)
    {
        int diff = fromIndex - toIndex;
        switch (diff)
        {
        case 0: return Red; // U-turn is not supported currently.
        case 1: return colors.left;
        case 2: return colors.forward;
        case 3: return colors.right;
        default: return Red;
        }
    }
    else
    {
        int diff = toIndex - fromIndex;
        switch (diff)
        {
        case 0: return Red; // U-turn is not supported currently.
        case 1: return colors.right;
        case 2: return colors.forward;
        case 3: return colors.left;
        default: return Red;
        }
    }

    return Red;
}

sim_mob::Signal::TrafficColor
sim_mob::Signal::getPedestrianLight(Crossing const & crossing)
const
{
    std::map<Crossing const *, size_t>::const_iterator iter = crossings_map_.find(&crossing);
    if (iter == crossings_map_.end())
    {
        std::ostringstream stream;
        stream << "Signal::getPedestrianLight: Mismatch in Signal and Crossing; Details as follows" << std::endl;
        stream << "    Signal is located at " << *node_.location << std::endl; 
        stream << "    Crossing near-line is " << crossing.nearLine.first << " to "
               << crossing.nearLine.second << std::endl;
        stream << "    Crossing far-line is " << crossing.farLine.first << " to "
               << crossing.farLine.second << std::endl;
        throw stream.str();
    }
    size_t index = iter->second;
    return convertToTrafficColor(TC_for_Pedestrian[index]);
}


//find the max projected DS in each SplitPlan
double sim_mob :: Signal :: fmax(const double proDS[])
{
	double max = proDS[0];
	for(int i = 1; i < 4; i++)
	{
		if(proDS[i] > max) {
			max = proDS[i];
		}
		//else{}
	}
	return max;
}


//find the minimum among the max projected DS
int sim_mob :: Signal :: fmin_ID(const double maxproDS[])
{
	int min=1;
	for (int i = 2; i <= 5; i++)
	{
		if(maxproDS[i] < maxproDS[min]) {
			min = i;
		}
		//else{}
	}
	return min;
}

//determine next SplitPlan according to the votes in last 5 cycle
int sim_mob :: Signal :: calvote(unsigned int vote1,unsigned int vote2, unsigned int vote3, unsigned int vote4, unsigned int vote5)
{
	assert(vote1<6);
	assert(vote2<6);
	assert(vote3<6);
	assert(vote4<6);
	assert(vote5<6);

	int vote_num[6] = {0,0,0,0,0,0};
	int ID = 1;

	vote_num[vote1]++;
	vote_num[vote2]++;
	vote_num[vote3]++;
	vote_num[vote4]++;
	vote_num[vote5]++;
	for(int i = 1; i <= 5; i++) {
		if(vote_num[i] > vote_num[ID]) {
			ID=i;
		}
		//else{}
	}
	return ID;
}
