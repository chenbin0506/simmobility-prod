//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PopulationSqlDao.hpp"

#include <boost/lexical_cast.hpp>
#include "conf/ConfigManager.hpp"
#include <behavioral/params/ZoneCostParams.hpp>
#include <conf/ConfigParams.hpp>
#include "DatabaseHelper.hpp"
#include "logging/Log.hpp"
#include "logging/logger.h"

//#include "medium/config/MT_Config.hpp"

using namespace sim_mob;
using namespace sim_mob::db;

namespace
{
typedef long long BigInt;
}

PopulationSqlDao::PopulationSqlDao(DB_Connection& connection) :
		SqlAbstractDao<PersonParams>(connection, "", "", "", "", "", "SELECT * FROM " +
		APPLY_SCHEMA(ConfigManager::GetInstanceRW().FullConfig().schemas.main_schema,
				ConfigManager::GetInstanceRW().FullConfig().dbStoredProcMap["individual_by_id"]))
{
}

PopulationSqlDao::~PopulationSqlDao()
{
}

void PopulationSqlDao::fromRow(Row& result, PersonParams& outObj)
{
	outObj.setPersonId(boost::lexical_cast<std::string>(result.get<BigInt>(DB_FIELD_ID)));
	outObj.setPersonTypeId(result.get<BigInt>(DB_FIELD_PERSON_TYPE_ID));
	outObj.setGenderId(result.get<BigInt>(DB_FIELD_GENDER_ID));
	outObj.setStudentTypeId(result.get<BigInt>(DB_FIELD_STUDENT_TYPE_ID));
	outObj.setVehicleOwnershipCategory(result.get<int>(DB_FIELD_VEHICLE_CATEGORY_ID));

	outObj.setAgeId(result.get<BigInt>(DB_FIELD_AGE_CATEGORY_ID));
	outObj.setIncomeIdFromIncome(result.get<double>(DB_FIELD_INCOME));
	outObj.setWorksAtHome(result.get<int>(DB_FIELD_WORK_AT_HOME));
	outObj.setCarLicense(result.get<int>(DB_FIELD_CAR_LICENSE));
	outObj.setMotorLicense(result.get<int>(DB_FIELD_MOTOR_LICENSE));
	outObj.setVanbusLicense(result.get<int>(DB_FIELD_VANBUS_LICENSE));
	outObj.setHasFixedWorkTiming(result.get<int>(DB_FIELD_WORK_TIME_FLEX));
	outObj.setHasWorkplace(result.get<int>(DB_FIELD_HAS_FIXED_WORK_PLACE));
	outObj.setIsStudent(result.get<int>(DB_FIELD_IS_STUDENT));
	outObj.setActivityAddressId(result.get<BigInt>(DB_FIELD_ACTIVITY_ADDRESS_ID));

	//household related
	outObj.setHhId(boost::lexical_cast<std::string>(result.get<BigInt>(DB_FIELD_HOUSEHOLD_ID)));
	outObj.setHomeAddressId(result.get<BigInt>(DB_FIELD_HOME_ADDRESS_ID));
	outObj.setHH_Size(result.get<int>(DB_FIELD_HH_SIZE));
	outObj.setHH_NumUnder4(result.get<int>(DB_FIELD_HH_CHILDREN_UNDER_4));
	outObj.setHH_NumUnder15(result.get<int>(DB_FIELD_HH_CHILDREN_UNDER_15));
	outObj.setHH_NumAdults(result.get<int>(DB_FIELD_HH_ADULTS));
	outObj.setHH_NumWorkers(result.get<int>(DB_FIELD_HH_WORKERS));

	VehicleParams vp;
	//vehicle related
		vp.setVehicleId(result.get<int>(DB_FIELD_V_ID));
		vp.setDrivetrain(result.get<std::string>(DB_FIELD_V_DRIVETRAIN));
		vp.setMake(result.get<std::string>(DB_FIELD_V_MAKE));
		vp.setModel(result.get<std::string>(DB_FIELD_V_MODEL));
	outObj.setVehicleParams(vp);
	//infer params
	outObj.fixUpParamsForLtPerson();
}

void PopulationSqlDao::toRow(PersonParams& data, Parameters& outParams, bool update)
{
}

void PopulationSqlDao::getOneById(long long id, PersonParams& outParam)
{
	db::Parameters params;
	db::Parameter idParam(id);
	params.push_back(idParam);
	getById(params, outParam);
}

void PopulationSqlDao::getAllIds(std::vector<long>& outList)
{
	if (isConnected())
    {
		ConfigParams& fullConfig = ConfigManager::GetInstanceRW().FullConfig();
		Statement query(connection.getSession<soci::session>());
		const std::string MAIN_SCHEMA = fullConfig.schemas.main_schema;
		//LogInfo("MAIN_SCHEMA: %s", MAIN_SCHEMA.c_str());
		const std::string TABLE_NAME = fullConfig.dbStoredProcMap["get_individual_id_list"];
		//LogInfo("TABLE_NAME: %s", TABLE_NAME.c_str());
		const std::string DB_SP_GET_INDIVIDUAL_IDS = APPLY_SCHEMA(MAIN_SCHEMA,TABLE_NAME);
		//LogInfo("DB_SP_GET_INDIVIDUAL_IDS: %s", DB_SP_GET_INDIVIDUAL_IDS.c_str());
		const std::string DB_GET_ALL_PERSON_IDS = "SELECT * FROM " + DB_SP_GET_INDIVIDUAL_IDS;
		//LogInfo("DB_GET_ALL_PERSON_IDS: %s", DB_GET_ALL_PERSON_IDS.c_str());
		prepareStatement(DB_GET_ALL_PERSON_IDS, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			outList.push_back((*it).get<BigInt>(DB_FIELD_ID));
		}
		Print() << "Person Ids loaded from LT database: " << outList.size() << std::endl;
	}
}

void PopulationSqlDao::getAddresses()
{
	if (isConnected())
	{
		PersonParams::clearAddressLookup();
		PersonParams::clearZoneAddresses();
		Statement query(connection.getSession<soci::session>());
		ConfigParams& fullConfig = ConfigManager::GetInstanceRW().FullConfig();
		const std::string MAIN_SCHEMA = fullConfig.schemas.main_schema;
		const std::string TABLE_NAME = fullConfig.dbStoredProcMap["addresses"];
		const std::string DB_SP_GET_ADDRESSES = APPLY_SCHEMA(MAIN_SCHEMA, TABLE_NAME);
		const std::string DB_GET_ADDRESSES = "SELECT * FROM " + DB_SP_GET_ADDRESSES;
		prepareStatement(DB_GET_ADDRESSES, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			long addressId = (*it).get<BigInt>(DB_FIELD_ADDRESS_ID);
			sim_mob::Address address;
			address.setAddressId(addressId);
			address.setPostcode((*it).get<int>(DB_FIELD_POSTCODE));
			address.setTazCode((*it).get<int>(DB_FIELD_TAZ_CODE));
			address.setDistanceMrt((*it).get<double>(DB_FIELD_DISTANCE_MRT));
			address.setDistanceBus((*it).get<double>(DB_FIELD_DISTANCE_BUS));
			PersonParams::setAddressLookup(address);
			PersonParams::setZoneNodeAddressesMap(address);
		}
	}
	else
	{
		std::cout << "Zone lookup Not connected!! " << std::endl;
	}
}

void PopulationSqlDao::getIncomeCategories(double incomeLowerLimits[])
{
	if (isConnected())
	{
		Statement query(connection.getSession<soci::session>());
		ConfigParams& fullConfig = ConfigManager::GetInstanceRW().FullConfig();
		const std::string MAIN_SCHEMA = fullConfig.schemas.main_schema;
		const std::string TABLE_NAME = fullConfig.dbTableNamesMap["income_cat"];
		const std::string DB_TABLE_INCOME_CATEGORIES = APPLY_SCHEMA(MAIN_SCHEMA,TABLE_NAME );
		const std::string DB_GET_INCOME_CATEGORIES = "SELECT * FROM " + DB_TABLE_INCOME_CATEGORIES;
		prepareStatement(DB_GET_INCOME_CATEGORIES, db::EMPTY_PARAMS, query);
		ResultSet rs(query);

		double lowLimit = 0;
		incomeLowerLimits[0] = 0;
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			lowLimit = (*it).get<double>(DB_FIELD_INCOME_CATEGORY_LOWER_LIMIT);
			if (lowLimit > 0)
			{
				incomeLowerLimits[(*it).get<BigInt>(DB_FIELD_ID)] = lowLimit;
			}
		}
	}
}

void PopulationSqlDao::getVehicleCategories(std::map<int, std::bitset<6> >& vehicleCategories)
{
	if (isConnected())
	{
		vehicleCategories.clear();
		Statement query(connection.getSession<soci::session>());
		ConfigParams& fullConfig = ConfigManager::GetInstanceRW().FullConfig();
		const std::string MAIN_SCHEMA = fullConfig.schemas.main_schema;
		const std::string TABLE_NAME = fullConfig.dbTableNamesMap["vehicle_ownership_status"];
		const std::string DB_TABLE_VEHICLE_OWNERSHIP_STATUS = APPLY_SCHEMA(MAIN_SCHEMA, TABLE_NAME);
		const std::string DB_GET_VEHICLE_OWNERSHIP_STATUS = "SELECT * FROM " + DB_TABLE_VEHICLE_OWNERSHIP_STATUS;
		prepareStatement(DB_GET_VEHICLE_OWNERSHIP_STATUS, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			std::bitset<6> vehOwnershipBits;
			std::string categoryDescription = (*it).get<std::string>(DB_FIELD_VEHICLE_CATEGORY_NAME);
			if (categoryDescription.find(SEARCH_STRING_NO_VEHICLE) != std::string::npos)
			{
				vehOwnershipBits[0] = 1;
			}
			if (categoryDescription.find(SEARCH_STRING_MULT_MOTORCYCLE_ONLY) != std::string::npos)
			{
				vehOwnershipBits[1] = 1;
			}
			if (categoryDescription.find(SEARCH_STRING_ONE_CAR_OFF_PEAK_W_WO_MC) != std::string::npos)
			{
				vehOwnershipBits[2] = 1;
			}
			if (categoryDescription.find(SEARCH_STRING_ONE_NORMAL_CAR) != std::string::npos)
			{
				vehOwnershipBits[3] = 1;
			}
			if(categoryDescription.find(SEARCH_STRING_ONE_CAR_PLUS_MULT_MC) != std::string::npos)
			{
				vehOwnershipBits[4] = 1;
			}
			if(categoryDescription.find(SEARCH_STRING_MULT_CAR_W_WO_MC) != std::string::npos)
			{
				vehOwnershipBits[5] = 1;
			}

			vehicleCategories[(*it).get<BigInt>(DB_FIELD_ID)] = vehOwnershipBits;
		}
	}
}
std::string var_value()
{

    const std::unordered_map<StopType, ActivityTypeConfig>& activityTypeConfig = ConfigManager::GetInstance().FullConfig().getActivityTypeConfigMap();
    int num_activity= activityTypeConfig.size();
    std::string column_name =" VALUES ( :v1, ";  // for first column person_id
    for(int i=1;i<=num_activity;i++)
    {
        column_name += ":v" + std::to_string(i+1) + ", ";
    }
    column_name += ":v" + std::to_string( num_activity + 2 ) + ", :v" + std::to_string( num_activity + 3 ) + ", :v" + std::to_string( num_activity + 4 ) + ") ";

    return column_name;
}
SimmobSqlDao::SimmobSqlDao(db::DB_Connection& connection, const std::string& tableName, const std::vector<std::string>& activityLogsumColumns) :
		SqlAbstractDao<PersonParams>(
				connection,
				tableName,
				("INSERT INTO " + tableName + var_value()), //" VALUES (:v1, :v2, :v3, :v4, :v5, :v6, :v7, :v8)"), //insert
				"", //update
				("TRUNCATE " + tableName), //delete
				"", //get all
				"SELECT "
                    + getLogsumColumnsStr(activityLogsumColumns)
					+ " FROM " + tableName + " where person_id = :_id" //get by id
                ), activityLogsumColumns(activityLogsumColumns)
{
}

SimmobSqlDao::~SimmobSqlDao()
{
}

void SimmobSqlDao::fromRow(db::Row& result, PersonParams& outObj)
{
    StopType activityType = 1;
    for (const auto& column : activityLogsumColumns)
    {
        outObj.setActivityLogsum(activityType, result.get<double>(column));
        ++activityType;
    }
    outObj.setDptLogsum(result.get<double>(DB_FIELD_DPT_LOGSUM));
	outObj.setDpsLogsum(result.get<double>(DB_FIELD_DPS_LOGSUM));
	outObj.setDpbLogsum(result.get<double>(DB_FIELD_DPB_LOGSUM));
}

void SimmobSqlDao::toRow(PersonParams& data, db::Parameters& outParams, bool update)
{
    outParams.push_back(data.getPersonId());
    for (int activityType = 1; activityType <= activityLogsumColumns.size(); ++activityType)
    {
        outParams.push_back(data.getActivityLogsum(activityType));
    }
	outParams.push_back(data.getDptLogsum());
    outParams.push_back(data.getDpsLogsum());
    outParams.push_back(data.getDpbLogsum());
}

std::string SimmobSqlDao::getLogsumColumnsStr(const std::vector<std::string>& actvtylogsumClmns)
{
    std::string columnStr = "";
    for (const auto& column : actvtylogsumClmns)
    {
        columnStr += (column + ",");
    }
    columnStr += DB_FIELD_DPT_LOGSUM + "," + DB_FIELD_DPS_LOGSUM +  "," + DB_FIELD_DPB_LOGSUM;

    return columnStr;
}

void SimmobSqlDao::getLogsumById(long long id, PersonParams& outObj)
{
	db::Parameters params;
	params.push_back(id);
	getById(params, outObj);
}

void SimmobSqlDao::getPostcodeNodeMap()
{
	if (isConnected())
	{
		PersonParams::clearPostCodeNodeMap();
		ConfigParams& fullConfig = ConfigManager::GetInstanceRW().FullConfig();
		const std::string PUBLIC_SCHEMA = fullConfig.schemas.public_schema;
		const std::string TABLE_NAME = fullConfig.dbStoredProcMap["postcode_node_map"];
		const std::string DB_SP_GET_POSTCODE_NODE_MAP = APPLY_SCHEMA(PUBLIC_SCHEMA, TABLE_NAME);
		const std::string DB_GET_POSTCODE_NODE_MAP = "SELECT * FROM " + DB_SP_GET_POSTCODE_NODE_MAP;
		Statement query(connection.getSession<soci::session>());
		prepareStatement(DB_GET_POSTCODE_NODE_MAP, db::EMPTY_PARAMS, query);
		ResultSet rs(query);
		for (ResultSet::const_iterator it = rs.begin(); it != rs.end(); ++it)
		{
			sim_mob::Address address;
			address.setPostcode((*it).get<int>(DB_FIELD_POSTCODE));
			ZoneNodeParams nodeId;
			nodeId.setNodeId((*it).get<BigInt>(DB_FIELD_NODE_ID));
			PersonParams::setPostCodeNodeMap(address,nodeId);
		}
	}
}
