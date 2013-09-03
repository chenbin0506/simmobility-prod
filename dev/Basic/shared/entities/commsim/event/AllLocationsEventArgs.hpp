/*
 * AllLocationsEventArgs.hpp
 *
 *  Created on: Jul 5, 2013
 *      Author: vahid
 */

#ifndef ALLLOCATIONSEVENTARGS_HPP_
#define ALLLOCATIONSEVENTARGS_HPP_

#include "event/args/EventArgs.hpp"
#include "event/EventListener.hpp"
#include<iostream>
#include "entities/commsim/serialization/Serialization.hpp"
#include "entities/commsim/broker/Broker.hpp"

namespace sim_mob {

DECLARE_CUSTOM_CALLBACK_TYPE(AllLocationsEventArgs)
class AllLocationsEventArgs: public sim_mob::event::EventArgs {
	sim_mob::AgentsMap::type  &registeredAgents;
public:
	AllLocationsEventArgs(sim_mob::AgentsMap::type &registeredAgents_);
	Json::Value ToJSON()const;
	virtual ~AllLocationsEventArgs();
};

} /* namespace sim_mob */
#endif /* ALLLOCATIONSEVENTARGS_HPP_ */
