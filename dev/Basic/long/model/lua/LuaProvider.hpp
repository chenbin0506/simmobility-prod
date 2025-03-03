/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LuaProxy.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 9, 2013, 4:39 PM
 */
#pragma once
#include "LT_LuaModels.hpp"

namespace sim_mob {

    namespace long_term {

        class LuaProvider {
        public:
            /**
             * Gets the HousingMarket lua model.
             * 
             * Attention: you should not hold this instance.
             * This provider will give you and instance based on 
             *  current thread context.
             * 
             * @return Lua housing market model reference.
             */
            static const HM_LuaModel& getHM_Model();
            
            /**
             * Gets the developer model.
             * 
             * Attention: you should not hold this instance.
             * This provider will give you and instance based on 
             *  current thread context.
             * 
             * @return Lua developer model reference.
             */
            static const DeveloperLuaModel& getDeveloperModel();
            
            /**
             * Gets the External Events lua model.
             * 
             * Attention: you should not hold this instance.
             * This provider will give you and instance based on 
             *  current thread context.
             * 
             * @return Lua external events model reference.
             */
            static const ExternalEventsModel& getExternalEventsModel();
        };
    }
}

