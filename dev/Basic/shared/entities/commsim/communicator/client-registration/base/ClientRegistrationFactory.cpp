//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ClientRegistrationFactory.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "ClientRegistrationFactory.hpp"
//#include "entities/commsim/communicator/client-registration/base/ClinetRegistrationHandler.hpp"

#include "entities/commsim/communicator/client-registration/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/communicator/client-registration/derived/ns3/NS3ClientRegistration.hpp"

#include <boost/assign/list_of.hpp>

namespace sim_mob {

ClientRegistrationFactory::ClientRegistrationFactory() {
	//NOTE: I am doing this manually for now; c++11 is throwing a fit.
	ClientTypeMap.clear();
	ClientTypeMap["ANDROID_EMULATOR"] = ConfigParams::ANDROID_EMULATOR;
	ClientTypeMap["NS3_SIMULATOR"] = ConfigParams::NS3_SIMULATOR;
	//ClientTypeMap = boost::assign::map_list_of("ANDROID_EMULATOR", ConfigParams::ANDROID_EMULATOR)("ConfigParams::NS3_SIMULATOR", ConfigParams::NS3_SIMULATOR);
	// TODO Auto-generated constructor stub

}

//gets a handler either from a cache or by creating a new one
boost::shared_ptr<sim_mob::ClientRegistrationHandler> ClientRegistrationFactory::getHandler(ConfigParams::ClientType type)
{
	boost::shared_ptr<sim_mob::ClientRegistrationHandler> handler;
	//if handler is already registered && the registered handler is not null
	std::map<ConfigParams::ClientType, boost::shared_ptr<sim_mob::ClientRegistrationHandler> >::iterator it = ClientRegistrationHandlerMap.find(type);
	if(it != ClientRegistrationHandlerMap.end())
	{
		//get the handler ...
		handler = (*it).second;
	}
	else
	{
		//else, create a cache entry ...
		bool typeFound = true;
		switch(type)
		{
		case ConfigParams::ANDROID_EMULATOR:
			handler.reset(new sim_mob::AndroidClientRegistration());
			break;
		case ConfigParams::NS3_SIMULATOR:
			handler.reset(new sim_mob::NS3ClientRegistration);
			break;
		default:
			typeFound = false;
		}
		//register this baby
		if(typeFound)
		{
			ClientRegistrationHandlerMap[type] = handler;
		}
	}

	return handler;
}

ClientRegistrationFactory::~ClientRegistrationFactory() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
