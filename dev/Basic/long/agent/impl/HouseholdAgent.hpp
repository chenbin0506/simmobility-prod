//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdAgent.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on May 16, 2013, 6:36 PM
 */
#pragma once
#include <vector>
#include "core/HousingMarket.hpp"
#include "agent/LT_Agent.hpp"
#include "database/entity/Household.hpp"


namespace sim_mob {

    namespace long_term {
        
        class HM_Model;
        class HouseholdBidderRole;
        class HouseholdSellerRole;
        /**
         * Represents an Long-Term household agent.
         * An household agent has the following capabilities:
         * - Sell units.
         * - Bid units. 
         */
        class HouseholdAgent :  public LT_Agent {
        public:
            HouseholdAgent(HM_Model* model, Household* hh, 
                    HousingMarket* market);
            virtual ~HouseholdAgent();
            
            //not-thread safe
            void addUnitId (const BigSerial& unitId);
            void removeUnitId (const BigSerial& unitId);
            const std::vector<BigSerial>& getUnitIds() const;
            HM_Model* getModel() const;
            HousingMarket* getMarket() const;
            const Household* getHousehold() const;
            
        protected:
            /**
             * Inherited from LT_Agent.
             */
            bool onFrameInit(timeslice now);
            sim_mob::Entity::UpdateStatus onFrameTick(timeslice now);
            void onFrameOutput(timeslice now);
            
            /**
             * Inherited from Entity. 
             */
            void onWorkerEnter();
            void onWorkerExit();
            virtual void HandleMessage(messaging::Message::MessageType type, 
                    const messaging::Message& message);
        private:
            /**
             * Method called to find some unit and bid it.
             * @param now current time.
             * @return true if some bid was sent, false otherwise.
             */
            bool bidUnit(timeslice now);
            
            /**
             * Events callbacks.
             */
            virtual void OnEvent(event::EventId eventId, 
                    event::EventPublisher* sender, const event::EventArgs& args);
            virtual void OnEvent(event::EventId eventId, event::Context ctxId, 
                    event::EventPublisher* sender, const event::EventArgs& args);
        private:
            typedef std::vector<BigSerial> UnitIdList;
      
        private:
            HM_Model* model;
            HousingMarket* market;
            Household* household;
            UnitIdList unitIds;
            HouseholdBidderRole* bidder;
            HouseholdSellerRole* seller;
        };
    }
}

