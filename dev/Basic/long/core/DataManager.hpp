/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DataManager.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Feb 11, 2011, 1:32 PM
 */
#pragma once

#include <vector>
#include <boost/unordered_map.hpp>
#include "util/SingletonHolder.hpp"
#include "database/entity/Postcode.hpp"
#include "database/entity/PostcodeAmenities.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Singleton responsible for managing necessary data for the simulation.
         * 
         * Extensions to this class must be aware of the main purpose of it.
         * The main goal of this class is a centralized point to access to the
         * static and lookup information. For that reason this class is a 
         * Singleton and **is not thread-safe**.
         * 
         */
        class DataManager {
        public:
            virtual ~DataManager();
        
            /**
             * Loads the necessary data from datasource.
             */
            void load();
            
            const Postcode* getPostcodeById(const BigSerial postcodeId) const;
            const PostcodeAmenities* getAmenitiesById(const BigSerial postcodeId) const;
            const Postcode* getPostcodeByCode(const std::string& code) const;
            const PostcodeAmenities* getAmenitiesByCode(const std::string& code) const;
        
        private:
            template<typename T> friend class DataManagerLifeCycle;
            DataManager();
            
        public:
           
            typedef std::vector<Postcode> PostcodeList;
            typedef std::vector<PostcodeAmenities> PostcodeAmenitiesList;
            typedef boost::unordered_map<BigSerial, Postcode*> PostcodeMap;
            typedef boost::unordered_map<BigSerial, PostcodeAmenities*> PostcodeAmenitiesMap;
            typedef boost::unordered_map<std::string, Postcode*> PostcodeByCodeMap;
            typedef boost::unordered_map<std::string, PostcodeAmenities*> PostcodeAmenitiesByCodeMap;
        private:
            PostcodeList postcodes;
            PostcodeAmenitiesList amenities;
            PostcodeMap postcodesById;
            PostcodeAmenitiesMap amenitiesById;
            PostcodeByCodeMap postcodesByCode;
            PostcodeAmenitiesByCodeMap amenitiesByCode;
        };
        
        /**
         * DataManager Singleton.
         */
        template<typename T = DataManager>
        struct DataManagerLifeCycle {

            static T * create() {
                T* obj = new T();
                return obj;
            }

            static void destroy(T* obj) {
                delete obj;
            }
        };
        
        typedef SingletonHolder<DataManager, DataManagerLifeCycle> DataManagerSingleton;
    }
}