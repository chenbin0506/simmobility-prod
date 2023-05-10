//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <unordered_map>
#include <vector>
#include <bitset>
#include <map>
#include <stdint.h>
#include <string>
#include "behavioral/StopType.hpp"
#include "behavioral/PredayUtils.hpp"
#include "ZoneCostParams.hpp"
#include <algorithm>
#include <array>

// Type Definitions for vehicle energy struct (needs to be global)
typedef struct {
  double a = 0.0;
  double b= 0.0;
  double c = 0.0;
  double m = 0.0;
  double etaMax = 0.0;
  double etaBrake = 0.0;
  double P_idle = 0.0;
  double powertrain = 0.0;
  double batteryCapacity = 0.0;
  double batterySOC = 0.0;
  double tripTotalEnergy = 0.0;
  double internalTemp = 0.0;
  double tripGasolineGallonEquivalent = 0.0;
  int occup = 0; //jo occupancy
  int powertrainSimple = 0;
} struct0_T;

namespace sim_mob
{

/**
 * An encapsulation of a time window and its availability.
 *
 * startTime and endTime are integral numbers from 1 to 48
 * The times are considered in half hour windows.
 * There are 48 half hour windows in a day.
 *
 * x.25 represents a time value between x:00 - x:29
 * x.75 represents a time value between x:30 and x:59
 * - where x varies from 3 to 26
 *
 * The startTime must be lesser than or equal to endTime.
 *
 * 3.25 (0300 to 0329 hrs) is time 1
 * 3.75 (0330 to 0359 hrs) is time 2
 * 4.25 (0400 to 0429 hrs) is time 3
 * ... so on
 * 23.75 (1130 to 1159 hrs) is time 42
 * 24.25 (1200 to 1229 hrs) is time 43
 * ... so on
 * 26.75 (0230 to 0259 hrs) is time 48
 *
 * \author Harish Loganathan
 */

class TimeWindowAvailability
{
public:
	TimeWindowAvailability();
	TimeWindowAvailability(double startTime, double endTime, bool availability = true);

	int getAvailability() const
	{
		return availability;
	}

	void setAvailability(bool availability)
	{
		this->availability = availability;
	}

	double getEndTime() const
	{
		return endTime;
	}

	double getStartTime() const
	{
		return startTime;
	}

private:
	double startTime;
	double endTime;
	bool availability;
};

class TimeWindowsLookup
{
  public:
	// Get start and end times of time windows, with no instance-specific "availability" information
	static TimeWindowAvailability getTimeWindowAt(size_t idx);

	TimeWindowAvailability operator[](size_t i) const;

	bool areAllUnavailable() const;

	void setAllAvailable();
	void setAllUnavailable();

	void setAvailable(double startTime, double endTime);
	void setUnavailable(double startTime, double endTime);
	void setRange(double startTime, double endTime, bool val);

private:
	/**
	 * There are 48 half-hour windows in a day. Each half hour window can be the start time of a time-window, and any half-hour window
	 * after the start time can be the end time of a time-window. Therefore there are (48 * (48+1) / 2) = 1176 time windows in a day.
	 */
	static const size_t intervalsPerDay = 48;
	static const size_t numTimeWindows = intervalsPerDay * (intervalsPerDay + 1) / 2;

	typedef std::array<double, 2> TimeWindow;
	typedef std::array<TimeWindow, numTimeWindows> TimeWindows;

	// Bit array of availability, for each time window
	std::bitset<numTimeWindows> availability;

	// 2D double array of start times and end times, for each time window
	static TimeWindows timeWindows;
};

class Address
{
public:
	Address() : addressId(0), postcode(0), tazCode(0), distanceMRT(0.0), distanceBus(0.0)
	{
	}

	long getAddressId() const
	{
		return addressId;
	}

	void setAddressId(long addressId)
	{
		this->addressId = addressId;
	}

	unsigned int getPostcode() const
	{
		return postcode;
	}

	void setPostcode(unsigned int postcode)
	{
		this->postcode = postcode;
	}

	int getTazCode() const
	{
		return tazCode;
	}

	void setTazCode(int tazCode)
	{
		this->tazCode = tazCode;
	}

	double getDistanceBus() const
	{
		return distanceBus;
	}

	void setDistanceBus(double distanceBus)
	{
		this->distanceBus = distanceBus;
	}

	double getDistanceMrt() const
	{
		return distanceMRT;
	}

	void setDistanceMrt(double distanceMrt)
	{
		distanceMRT = distanceMrt;
	}

private:
	long addressId;
	unsigned int postcode;
	int tazCode;
	double distanceBus; //km
	double distanceMRT; //km
};

/**
 * Simple class to store vehicle-specific information from population database.
 * \note This class is used by the mid-term behavior models.
 *
 *
 * \author Michael Choi
 */
class VehicleParams
{
public:
  enum VehicleDriveTrain
  {
    NONE, ICE, HEV, PHEV, BEV, FCV, CDBUS, HEBUS, TRAIN
  };

  struct0_T vehicleStruct;
  double previousEnergy = 0;
  double totalEnergy = 0; //jo

  VehicleParams() : vehicleId(0), drivetrain(VehicleDriveTrain::NONE), make("NONE"), model("NONE")
  {
  }
  virtual ~VehicleParams()
  {
  }
	unsigned long getVehicleId() const
	{
		return vehicleId;
	}

	void setVehicleId(const unsigned long vehicleId)
	{
		this->vehicleId = vehicleId;
	}

	const VehicleDriveTrain getDrivetrain() const
	{
		return drivetrain;
	}

	//TODO: Make this correct
	void setDrivetrain(const std::string& drivetrain)
	{
		if (drivetrain == "ICE")
			this->drivetrain = ICE;
		else if (drivetrain == "HEV")
			this->drivetrain = HEV;
		else if (drivetrain == "PHEV")
			this->drivetrain = PHEV;
		else if (drivetrain == "BEV")
			this->drivetrain = BEV;
		else if (drivetrain == "FCV")
			this->drivetrain = FCV;
		else if (drivetrain == "CDBUS")
			this->drivetrain = CDBUS;
		else if (drivetrain == "HEBUS")
			this->drivetrain = HEBUS;
		else if (drivetrain == "TRAIN")
			this->drivetrain = TRAIN;
		else // default to ICE for now
			this->drivetrain = NONE;
		this->vehicleIdString = drivetrain;
	}


	const std::string& getMake() const
	{
		return make;
	}

	void setMake(const std::string& make)
	{
		this->make = make;
	}

	const std::string& getModel() const
	{
		return model;
	}

	void setModel(const std::string& mode)
	{
		this->model = model;
	}

	struct0_T& getVehicleStruct()
	{
		return vehicleStruct;
	}

	void setVehicleStruct(struct0_T VS)
	{
		this->vehicleStruct = VS;
	}

	double getTimestepEnergy() const
	{
		return vehicleStruct.tripTotalEnergy - previousEnergy;
	}

	//jo May 7
	double getFrameEnergy() const
	{
		return totalEnergy - previousEnergy;
	}

	double getTotalEnergyConsumed() const
	{
		return totalEnergy;
	}

	void updateEnergy()
	{
		this->previousEnergy = totalEnergy;
	}
	//jo

	double getTotalEnergy() const
	{
		return vehicleStruct.tripTotalEnergy;
	}

	void updatePreviousEnergy()
	{
		this->previousEnergy = vehicleStruct.tripTotalEnergy;
	}


private:
	unsigned long vehicleId;
	std::string vehicleIdString;
	VehicleDriveTrain drivetrain;
	std::string make;
	std::string model;
};

/**
 * Simple class to store information about a person from population database.
 * \note This class is used by the mid-term behavior models.
 *
 *
 * \author Harish Loganathan
 */
class PersonParams
{
public:
	PersonParams();
	virtual ~PersonParams();

	const std::string& getHhId() const
	{
		return hhId;
	}

	void setHhId(const std::string& hhId)
	{
		this->hhId = hhId;
	}

	int getAgeId() const
	{
		return ageId;
	}

	void setAgeId(int ageId)
	{
		this->ageId = ageId;
	}

	int getFixedWorkLocation() const
	{
		return fixedWorkLocation;
	}

	void setFixedWorkLocation(int fixedWorkLocation)
	{
		this->fixedWorkLocation = fixedWorkLocation;
	}

	int hasFixedWorkPlace() const
	{
		return (fixedWorkLocation != 0);
	}

	int getHasFixedWorkTiming() const
	{
		return hasFixedWorkTiming;
	}

	void setHasFixedWorkTiming(int hasFixedWorkTiming)
	{
		this->hasFixedWorkTiming = hasFixedWorkTiming;
	}

	int getHomeLocation() const
	{
		return homeLocation;
	}

	void setHomeLocation(int homeLocation)
	{
		this->homeLocation = homeLocation;
	}

	int getIncomeId() const
	{
		return incomeId;
	}

	void setIncomeId(int income_id)
	{
		this->incomeId = income_id;
	}

	int getIsFemale() const
	{
		return isFemale;
	}

	void setIsFemale(int isFemale)
	{
		this->isFemale = isFemale;
	}

	int getIsUniversityStudent() const
	{
		return isUniversityStudent;
	}

	void setIsUniversityStudent(int isUniversityStudent)
	{
		this->isUniversityStudent = isUniversityStudent;
	}

	int getPersonTypeId() const
	{
		return personTypeId;
	}

	void setPersonTypeId(int personTypeId)
	{
		this->personTypeId = personTypeId;
	}

	int getWorksAtHome() const
	{
		return worksAtHome;
	}

	void setWorksAtHome(int worksAtHome)
	{
		this->worksAtHome = worksAtHome;
	}

	int getFixedSchoolLocation() const
	{
		return fixedSchoolLocation;
	}

	void setFixedSchoolLocation(int fixedSchoolLocation)
	{
		this->fixedSchoolLocation = fixedSchoolLocation;
	}

	int getStopType() const
	{
		return stopType;
	}

	void setStopType(int stopType)
	{
		this->stopType = stopType;
	}

	int isWorker() const
	{
		return (personTypeId == 1 || personTypeId == 2 || personTypeId == 3 || personTypeId == 8 || personTypeId == 9 || personTypeId == 10);
	}

	int hasDrivingLicence() const
	{
		return drivingLicence;
	}

	void setHasDrivingLicence(bool hasDrivingLicence)
	{
		this->drivingLicence = (int) hasDrivingLicence;
	}

	std::string getPersonId() const
	{
		return personId;
	}

	void setPersonId(std::string personId)
	{
		this->personId = personId;
	}

	int getHH_HasUnder15() const
	{
		return hasUnder15;
	}

	void setHH_HasUnder15(int hhUnder15)
	{
		this->hasUnder15 = (hhUnder15 > 0);
	}

	int getHH_NumUnder4() const
	{
		return hhNumUnder4;
	}

	void setHH_NumUnder4(int hhNumUnder4)
	{
		this->hhNumUnder4 = hhNumUnder4;
	}

	int getHH_OnlyAdults() const
	{
		return hhOnlyAdults;
	}

	void setHH_OnlyAdults(int hhOnlyAdults)
	{
		this->hhOnlyAdults = hhOnlyAdults;
	}

	int getHH_OnlyWorkers() const
	{
		return hhOnlyWorkers;
	}

	void setHH_OnlyWorkers(int hhOnlyWorkers)
	{
		this->hhOnlyWorkers = hhOnlyWorkers;
	}

    /*double getEduLogSum() const
	{
		return eduLogSum;
	}

	void setEduLogSum(double eduLogSum)
	{
		this->eduLogSum = eduLogSum;
	}

	double getOtherLogSum() const
	{
		return otherLogSum;
	}

	void setOtherLogSum(double otherLogSum)
	{
		this->otherLogSum = otherLogSum;
	}

	double getShopLogSum() const
	{
		return shopLogSum;
	}

	void setShopLogSum(double shopLogSum)
	{
		this->shopLogSum = shopLogSum;
	}

	double getWorkLogSum() const
	{
		return workLogSum;
	}

	void setWorkLogSum(double workLogSum)
	{
		this->workLogSum = workLogSum;
    }*/

    double getActivityLogsum(StopType activityType) const
    {
        return activityLogsums.at(activityType);
    }

    void setActivityLogsum(StopType activityType, double logsum)
    {
        activityLogsums[activityType] = logsum;
    }

	int getStudentTypeId() const
	{
		return studentTypeId;
	}

	void setStudentTypeId(int studentTypeId)
	{
		this->studentTypeId = studentTypeId;
	}

	double getHouseholdFactor() const
	{
		return householdFactor;
	}

	void setHouseholdFactor(double householdFactor)
	{
		this->householdFactor = householdFactor;
	}

	int getMissingIncome() const
	{
		return missingIncome;
	}

	void setMissingIncome(int missingIncome)
	{
		this->missingIncome = missingIncome;
	}

	double getDpsLogsum() const
	{
		return dpsLogsum;
	}

	void setDpsLogsum(double dpsLogsum)
	{
		this->dpsLogsum = dpsLogsum;
	}

	double getDptLogsum() const
	{
		return dptLogsum;
	}

	void setDptLogsum(double dptLogsum)
	{
		this->dptLogsum = dptLogsum;
	}

	double getDpbLogsum() const
	{
		return dpbLogsum;
	}

	void setDpbLogsum(double dpbLogsum)
	{
		this->dpbLogsum = dpbLogsum;
	}

	bool getCarLicense() const
	{
		return carLicense;
	}

	void setCarLicense(bool carLicense)
	{
		this->carLicense = carLicense;
	}

	int getHhSize() const
	{
		return hhSize;
	}

	void setHH_Size(int hhSize)
	{
		this->hhSize = hhSize;
	}

	bool getMotorLicense() const
	{
		return motorLicense;
	}

	void setMotorLicense(bool motorLicence)
	{
		this->motorLicense = motorLicence;
	}

	bool getVanbusLicense() const
	{
		return vanbusLicense;
	}

	void setVanbusLicense(bool vanbusLicense)
	{
		this->vanbusLicense = vanbusLicense;
	}

	int getGenderId() const
	{
		return genderId;
	}

	void setGenderId(int genderId)
	{
		this->genderId = genderId;
	}

	long getHomeAddressId() const
	{
		return homeAddressId;
	}

	void setHomeAddressId(long homeAddressId)
	{
		this->homeAddressId = homeAddressId;
	}

	long getActivityAddressId() const
	{
		return activityAddressId;
	}

	void setActivityAddressId(long activityAddressId)
	{
		this->activityAddressId = activityAddressId;
	}

	int getHH_NumAdults() const
	{
		return hhNumAdults;
	}

	void setHH_NumAdults(int hhNumAdults)
	{
		this->hhNumAdults = hhNumAdults;
	}

	int getHH_NumUnder15() const
	{
		return hhNumUnder15;
	}

	void setHH_NumUnder15(int hhNumUnder15)
	{
		this->hhNumUnder15 = hhNumUnder15;
	}

	int getHH_NumWorkers() const
	{
		return hhNumWorkers;
	}

	void setHH_NumWorkers(int hhNumWorkers)
	{
		this->hhNumWorkers = hhNumWorkers;
	}

	bool hasWorkplace() const
	{
		return fixedWorkplace;
	}

	void setHasWorkplace(bool hasFixedWorkplace)
	{
		this->fixedWorkplace = hasFixedWorkplace;
	}

	int isStudent() const
	{
		return student;
	}

	void setIsStudent(bool isStudent)
	{
		this->student = isStudent;
	}

	double getTravelProbability() const
	{
		return travelProbability;
	}

	void setTravelProbability(double travelProbability)
	{
		this->travelProbability = travelProbability;
	}

	double getTripsExpected() const
	{
		return tripsExpected;
	}

	void setTripsExpected(double tripsExpected)
	{
		this->tripsExpected = tripsExpected;
	}

	int getVehicleOwnershipCategory() const
	{
		return (static_cast<int>(vehicleOwnershipCategory));
	}

	VehicleOwnershipOption getVehicleOwnershipOption() const
	{
		return vehicleOwnershipCategory;
	}

	const VehicleParams getConstVehicleParams() const
	{
		return vehicleParams;
	}

	VehicleParams& getVehicleParams()
	{
		return vehicleParams;
	}

	void setVehicleParams(VehicleParams& vehicleParams) 
	{
		this->vehicleParams = vehicleParams;
	}

	void setVehicleOwnershipCategory(int vehicleOwnershipCategory);

	static double* getIncomeCategoryLowerLimits()
	{
		return incomeCategoryLowerLimits;
	}

	static const std::map<long, sim_mob::Address>& getAddressLookup()
	{
		return addressLookup;
	}

    static void setAddressLookup(const sim_mob::Address& address);

    static void removeInvalidAddress();

    static void clearAddressLookup();

	static const std::map<unsigned int, unsigned int>& getPostcodeNodeMap()
	{
		return postCodeToNodeMapping;
	}

    static void setPostCodeNodeMap(const sim_mob::Address& address,const ZoneNodeParams& nodeId);

    static void clearPostCodeNodeMap();

	static const std::map<int, std::vector<long> >& getZoneAddresses()
	{
		return zoneAddresses;
	}
    static void setZoneNodeAddressesMap(const sim_mob::Address& address);

    static void clearZoneAddresses();

	void setAllTimeWindowsAvailable();

	/**
	 * get the availability for a time window for tour
	 *
	 * @param timeWnd time window index to check availability
	 * @param mode of activity for which time window is being predicted
	 *
	 * @return 0 if time window is not available; 1 if available
	 *
	 * NOTE: This function is invoked from the Lua layer. The return type is int in order to be compatible with Lua.
	 *       Lua does not support boolean types.
	 */
	int getTimeWindowAvailability(size_t timeWnd, int mode) const;

	/**
	 * overload function to set availability of times in timeWnd to 0
	 *
	 * @param startTime start time
	 * @param endTime end time
	 */
	void blockTime(double startTime, double endTime);

	/**
	 * prints the fields of this object
	 */
	std::string print();

	/**
	 * sets income ID by looking up income on a pre loaded map of income ranges.
	 * handles incomeId value mismatch between preday and long-term formats. See implementation for details.
	 *
	 * @param income the income value
	 */
	void setIncomeIdFromIncome(double income);

	/**
	 * maps vehicleCategoryID from LT db to preday understandable format
	 *
	 * @param vehicleCategoryId LT vechicle category id
	 */
	void setVehicleOwnershipFromCategoryId(int vehicleCategoryId);

	/**
	 * infers params used in preday system of models from params obtained from LT population
	 */
	void fixUpParamsForLtPerson();

	/**
	 * looks up TAZ code for a given address ID from LT population data
	 *
	 * @param addressId input address id
	 *
	 * @return TAZ code for addressId
	 */
	int getTAZCodeForAddressId(long addressId) const;

	/**
	 * looks up postcode for a given address ID from LT population data
	 *
	 * @param addressId input address id
	 *
	 * @return postcode for addressId
	 */
	unsigned int getSimMobNodeForAddressId(long addressId) const;

	/**
	 * returns the list of address ids in a zone
	 * @param zoneCode input zone code
	 * @return list of address ids in a zone
	 */
	const std::vector<long>& getAddressIdsInZone(int zoneCode) const;

	std::unordered_map<StopType, double> getActivityLogsums() const;
	/**
	* getter function for timeWindowLookup
	*/
	TimeWindowsLookup getTimeWindowLookup()
	{
		return this->timeWindowsLookup;
	}

private:
	std::string personId;
	std::string hhId;
	int personTypeId;
	int ageId;
	int isUniversityStudent;
	int studentTypeId;
	int genderId;
	int isFemale;
	int incomeId;
	int missingIncome;
	int worksAtHome;
	VehicleOwnershipOption vehicleOwnershipCategory;
	VehicleParams vehicleParams;
	int hasFixedWorkTiming;
	int homeLocation;
	long homeAddressId;
	int fixedWorkLocation;
	int fixedSchoolLocation;
	long activityAddressId;
	int stopType;
	int drivingLicence;
	bool carLicense;
	bool motorLicense;
	bool vanbusLicense;
	bool fixedWorkplace;
	bool student;

	//household related
	int hhSize;
	int hhNumAdults;
	int hhNumWorkers;
	int hhNumUnder4;
	int hhNumUnder15;
	int hhOnlyAdults;
	int hhOnlyWorkers;
	int hasUnder15;
	double householdFactor;

	//logsums
	double workLogSum;
	double eduLogSum;
	double shopLogSum;
	double otherLogSum;

    std::unordered_map<StopType, double> activityLogsums;

	double dptLogsum;
	double dpsLogsum;
	double dpbLogsum;

	double travelProbability;
	double tripsExpected;

	/**
	 * Time windows availability for the person.
	 */
	TimeWindowsLookup timeWindowsLookup;

	/**
	 * income category lookup containing lower limits of each category.
	 * income category id for a specific income is the index of the greatest element lower than the income in this array
	 * index 0 corresponds no income.
	 */
	static double incomeCategoryLowerLimits[12];

	/**
	 * address to taz map
	 */
	static std::map<long, sim_mob::Address> addressLookup;

	/**
	 * postcode to simmobility node mapping
	 */
	static std::map<unsigned int, unsigned int> postCodeToNodeMapping;

	/**
	 * zone to number of addresses in zone map
	 */
	static std::map<int, std::vector<long> > zoneAddresses;
};

class ZoneAddressParams
{
public:
	ZoneAddressParams(const std::map<long, sim_mob::Address>& addressLkp, const std::vector<long>& znAddresses);
	virtual ~ZoneAddressParams();

	/**
	 * gets number of addresses in a zone
	 */
	int getNumAddresses() const;

	/**
	 * gets distance to mrt for given address
	 */
	double getDistanceMRT(int addressIdx) const;

	/**
	 * gets distance to bus stop for given address
	 */
	double getDistanceBus(int addressIdx) const;

	/**
	 * fetches address for an index
	 */
	long getAddressId(int addressIdx) const;

private:
	/**
	 * address to taz map
	 */
	const std::map<long, sim_mob::Address>& addressLookup;

	/**
	 * zone to number of addresses in zone map
	 */
	const std::vector<long>& zoneAddresses;

	/** number of addresses in zone*/
	int numAddresses;
};

/**
 * Class for storing person parameters related to usual work location model
 *
 * \author Harish Loganathan
 */
class UsualWorkParams
{
public:
	int getFirstOfMultiple() const
	{
		return firstOfMultiple;
	}

	void setFirstOfMultiple(int firstOfMultiple)
	{
		this->firstOfMultiple = firstOfMultiple;
	}

	int getSubsequentOfMultiple() const
	{
		return subsequentOfMultiple;
	}

	void setSubsequentOfMultiple(int subsequentOfMultiple)
	{
		this->subsequentOfMultiple = subsequentOfMultiple;
	}

	double getWalkDistanceAm() const
	{
		return walkDistanceAM;
	}

	void setWalkDistanceAm(double walkDistanceAm)
	{
		walkDistanceAM = walkDistanceAm;
	}

	double getWalkDistancePm() const
	{
		return walkDistancePM;
	}

	void setWalkDistancePm(double walkDistancePm)
	{
		walkDistancePM = walkDistancePm;
	}

	double getZoneEmployment() const
	{
		return zoneEmployment;
	}

	void setZoneEmployment(double zoneEmployment)
	{
		this->zoneEmployment = zoneEmployment;
	}

private:
	int firstOfMultiple;
	int subsequentOfMultiple;
	double walkDistanceAM;
	double walkDistancePM;
	double zoneEmployment;
};
} // end namespace sim_mob

