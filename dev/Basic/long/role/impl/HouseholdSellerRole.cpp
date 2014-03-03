//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HouseholdSellerRole.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on May 16, 2013, 5:13 PM
 */
#include <cmath>
#include "HouseholdSellerRole.hpp"
#include "util/Statistics.hpp"
#include "util/Math.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "model/HM_Model.hpp"
#include "message/MessageBus.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/LT_Message.hpp"
#include "core/DataManager.hpp"
#include "core/AgentsLookup.hpp"

using namespace sim_mob::long_term;
using namespace sim_mob::messaging;
using std::vector;
using std::endl;
using sim_mob::Math;

namespace {
    const int TIME_ON_MARKET = 10;
    const int TIME_INTERVAL = 7;
    
    //bid_timestamp, seller_id, unit_id, price, expectation
    const std::string LOG_EXPECTATION = "%1%, %2%, %3%, %4%, %5%";
    //bid_timestamp, seller_id, bidder_id, unit_id, bid_value, status(0 - REJECTED, 1- ACCEPTED)
    const std::string LOG_BID = "%1%, %2%, %3%, %4%, %5%, %6%";

    inline void printBid(const HouseholdAgent& agent, const Bid& bid, bool accepted) {
        boost::format fmtr = boost::format(LOG_BID) % bid.getTime().ms() 
                                                    % agent.getId() 
                                                    % bid.getBidderId() 
                                                    % bid.getUnitId()
                                                    % bid.getValue()
                                                    % ((accepted) ? 1 : 0);
        AgentsLookupSingleton::getInstance().getLogger()
                .log(LoggerAgent::BIDS,fmtr.str());
        //PrintOut(fmtr.str() << endl);
    }
    
    inline void printExpectation(const timeslice& now, BigSerial unitId,
            const HouseholdAgent& agent, const ExpectationEntry& exp) {
        boost::format fmtr = boost::format(LOG_EXPECTATION) % now.ms() 
                                                            % agent.getId() 
                                                            % unitId
                                                            % exp.price
                                                            % exp.expectation;
        AgentsLookupSingleton::getInstance()
                .getLogger().log(LoggerAgent::EXPECTATIONS, fmtr.str());
        //PrintOut(fmtr.str() << endl);
    }

    /**
     * Decides over a given bid for a given expectation.
     * @param bid given by the bidder.
     * @return true if accepts the bid or false otherwise.
     */
    inline bool decide(const Bid& bid, const ExpectationEntry& entry) {
        return bid.getValue() > entry.expectation;
    }

    inline void replyBid(const HouseholdAgent& agent, const Bid& bid, 
            const BidResponse& response) {
        MessageBus::PostMessage(bid.getBidder(), LTMID_BID_RSP,
                MessageBus::MessagePtr(new BidMessage(bid, response)));
         //print bid.
         printBid(agent, bid, (response == ACCEPTED));
    }
}

HouseholdSellerRole::SellingUnitInfo::SellingUnitInfo() :
startedDay(0), interval(0), daysOnMarket(0), numExpectations(0) {
}

HouseholdSellerRole::HouseholdSellerRole(HouseholdAgent* parent)
: LT_AgentRole(parent), currentTime(0, 0), hasUnitsToSale(true), 
selling(false) {

}

HouseholdSellerRole::~HouseholdSellerRole() {
    sellingUnitsMap.clear();
}

void HouseholdSellerRole::update(timeslice now) {
    if (selling){
        if (now.ms() > currentTime.ms()) {
            // Day has changed we need to notify the last day winners.
            notifyWinnerBidders();

        }
        // Verify if is time to adjust prices for units.
        adjustNotSoldUnits();
    }
    if (hasUnitsToSale) {
        const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
        HousingMarket* market = getParent()->getMarket();
        const vector<BigSerial>& unitIds = getParent()->getUnitIds();
        //get values from parent.
        const Unit* unit = nullptr;
        DataManager& dman = DataManagerSingleton::getInstance();
        for (vector<BigSerial>::const_iterator itr = unitIds.begin();
                itr != unitIds.end(); itr++) {
            // Decides to put the house on market.
            BigSerial unitId = *itr;
            unit = dman.getUnitById(unitId);
            BigSerial tazId = dman.getUnitTazId(unitId);
            double hedonicPrice = luaModel.calculateHedonicPrice(*unit);
            calculateUnitExpectations(*unit);
            market->addEntry(HousingMarket::Entry(getParent(), unit->getId(), 
                    unit->getPostcodeId(), tazId, hedonicPrice, hedonicPrice));
            selling = true;
        }
        hasUnitsToSale = false;
    }
    //update current time.
    currentTime = now;
}

void HouseholdSellerRole::HandleMessage(Message::MessageType type,
        const Message& message) {
    switch (type) {
        case LTMID_BID:// Bid received 
        {
            const BidMessage& msg = MSG_CAST(BidMessage, message);
            BigSerial unitId = msg.getBid().getUnitId();
            bool decision = false;
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry)) {
                //verify if is the bid satisfies the asking price.
                decision = decide(msg.getBid(), entry);
                if (decision) {
                    //get the maximum bid of the day
                    Bids::iterator bidItr = maxBidsOfDay.find(unitId);
                    Bid* maxBidOfDay = nullptr;
                    if (bidItr != maxBidsOfDay.end()) {
                        maxBidOfDay = &(bidItr->second);
                    }

                    if (!maxBidOfDay) {
                        maxBidsOfDay.insert(std::make_pair(unitId, msg.getBid()));
                    } else if (maxBidOfDay->getValue() < msg.getBid().getValue()) {
                        // bid is higher than the current one of the day.
                        // it is necessary to notify the old max bidder
                        // that his bid was not accepted.
                        //reply to sender.
                        replyBid(*getParent(), *maxBidOfDay, BETTER_OFFER);
                        maxBidsOfDay.erase(unitId);
                        //update the new bid and bidder.
                        maxBidsOfDay.insert(std::make_pair(unitId, msg.getBid()));
                    } else {
                        replyBid(*getParent(), msg.getBid(), BETTER_OFFER);
                    }
                } else {
                    replyBid(*getParent(), msg.getBid(), NOT_ACCEPTED);
                }
            } else {
                // Sellers is not the owner of the unit or unit is not available.
                replyBid(*getParent(), msg.getBid(), NOT_AVAILABLE);
            }
            Statistics::increment(Statistics::N_BIDS);
            break;
        }
        default:break;
    }
}

void HouseholdSellerRole::adjustNotSoldUnits() {
    HousingMarket* market = getParent()->getMarket();
    const IdVector& unitIds = getParent()->getUnitIds();
    const Unit* unit = nullptr;
    const HousingMarket::Entry* unitEntry = nullptr;
    DataManager& dman = DataManagerSingleton::getInstance();
    for (IdVector::const_iterator itr = unitIds.begin(); itr != unitIds.end(); 
            itr++) {
        BigSerial unitId = *itr;
        unitEntry = market->getEntryById(unitId);
        unit = dman.getUnitById(unitId);
        if (unitEntry && unit){
            ExpectationEntry entry;
            if (getCurrentExpectation(unitId, entry) 
                && entry.price != unitEntry->getAskingPrice()) {
                HousingMarket::Entry updatedEntry(*unitEntry);
                updatedEntry.setAskingPrice(entry.price);
                market->updateEntry(updatedEntry);
            }
        }
    }
}

void HouseholdSellerRole::notifyWinnerBidders() {
    HousingMarket* market = getParent()->getMarket();
    for (Bids::iterator itr = maxBidsOfDay.begin(); itr != maxBidsOfDay.end();
            itr++) {
        Bid& maxBidOfDay = itr->second;
        replyBid(*getParent(), maxBidOfDay, ACCEPTED);
        market->removeEntry(maxBidOfDay.getUnitId());
        getParent()->removeUnitId(maxBidOfDay.getUnitId());
        sellingUnitsMap.erase(maxBidOfDay.getUnitId());
    }
    // notify winners.
    maxBidsOfDay.clear();
}

void HouseholdSellerRole::calculateUnitExpectations(const Unit& unit) {
    const HM_LuaModel& luaModel = LuaProvider::getHM_Model();
    SellingUnitInfo info;
    info.startedDay = currentTime.ms();
    info.interval = TIME_INTERVAL;
    info.daysOnMarket = TIME_ON_MARKET;
    info.numExpectations = (info.interval == 0) ? 0 : ceil(abs(info.daysOnMarket/info.interval));
    luaModel.calulateUnitExpectations(unit, info.numExpectations, info.expectations);
    sellingUnitsMap.erase(unit.getId());
    sellingUnitsMap.insert(std::make_pair(unit.getId(), info));
    for (int i = 0; i < info.numExpectations; i++) {
        printExpectation(currentTime, unit.getId(), *getParent(), info.expectations[i]);
    }
}

bool HouseholdSellerRole::getCurrentExpectation(const BigSerial& unitId,
        ExpectationEntry& outEntry) {
    UnitsInfoMap::iterator it = sellingUnitsMap.find(unitId);
    if (it != sellingUnitsMap.end()) {
        SellingUnitInfo& info = it->second;
        unsigned int index = floor(abs(info.startedDay - currentTime.ms()) / info.interval);
        if (index < info.expectations.size()) {
            ExpectationEntry& expectation = info.expectations[index];
            outEntry.expectation = expectation.expectation;
            outEntry.price = expectation.price;
            return true;
        }
    }
    return false;
}

