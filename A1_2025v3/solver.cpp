#include "solver.h"
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <random>


using namespace std;
// You can add any helper functions or classes you need here.

/**
 * @brief The main function to implement your search/optimization algorithm.
 * * This is a placeholder implementation. It creates a simple, likely invalid,
 * plan to demonstrate how to build the Solution object. 
 * * TODO: REPLACE THIS ENTIRE FUNCTION WITH YOUR ALGORITHM.
 * 
 * 
 */

void createRandomInitialState(State& state, const ProblemData& data,const map<int, vector<int>> singletonVillageList, const map<int , set<int>> commonVill){
    /*
    Changed the name from IntitalStateCreationg to createRandomInitialState
    instead of using this funtion to create an initial state. we will provide it with an already created state and 
    it will randomize it in some manner.
    changed name from helicopterTripList to singletonVillageList
    */ 
    random_device rd;
    mt19937 gen(rd());
    for (auto& [vid, heliSet] : commonVill) {
        if (!heliSet.empty()) {
            vector<int> options(heliSet.begin(), heliSet.end());

            // Pick one helicopter randomly
            uniform_int_distribution<> pickDist(0, (int)options.size() - 1);
            int chosenHeli = options[pickDist(gen)];

            // Assign vilageid to chosen Helicopter and push back in the vector
            state.zone[chosenHeli].push_back(vid);

           cout << "Assigned common Village V" << vid<< " to Helicopter H" << chosenHeli << endl;
        }
    }


    //intialise empty helicopter plans
    for(auto& [hid , vid] : singletonVillageList ){
        HelicopterPlan heli;
        heli.helicopter_id = hid;

        state.helicopterPlan.push_back(heli);        

    }



    // cout << "================== Helicopter Plans ==================" << endl;
    // for (const auto& plan : intialState.helicopterPlan) {
    //     cout << "Helicopter H" << plan.helicopter_id 
    //          << " has " << plan.trips.size() << " trips." << endl;

    //     for (int t = 0; t < plan.trips.size(); t++) {
    //         const auto& trip = plan.trips[t];
    //         cout << "  Trip " << t+1 << ": Pickup [Dry=" << trip.dry_food_pickup
    //              << ", Perishable=" << trip.perishable_food_pickup
    //              << ", Other=" << trip.other_supplies_pickup << "]" << endl;

    //         for (const auto& drop : trip.drops) {
    //             cout << "    Drop at Village V" << drop.village_id 
    //                  << ": Dry=" << drop.dry_food
    //                  << ", Perishable=" << drop.perishable_food
    //                  << ", Other=" << drop.other_supplies << endl;
    //         }
    //     }
    // }
}

void evaluateState(State& state, const ProblemData& data, vector<vector<int>> cityxvilalge, vector<vector<int>> villagexvillage){
    // Still work in progress.
    for(Helicopter helicopter : state.helicopterList){
        cout << "For Helicopter " << helicopter.id << "->";
        for(int vid : state.zone[helicopter.id]){
            cout << " " << vid << ", ";
        }
        cout << endl;
    }
}

 map<int, vector<int>> buildReachableMap(const ProblemData& problem) {
    map<int, vector<int>> reachableMap;

    for (const auto& heli : problem.helicopters) {
        vector<int> reachableVillages;
        int cityIndex = heli.home_city_id - 1;  // city where heli is based

        for (int j = 0; j < problem.villages.size(); j++) {
            int dx = problem.cities[cityIndex].x - problem.villages[j].coords.x;
            int dy = problem.cities[cityIndex].y - problem.villages[j].coords.y;
            double dist = sqrt(dx * dx + dy * dy);

            // round-trip check
            if (2 * dist <= heli.distance_capacity) {
                reachableVillages.push_back(problem.villages[j].id);
            }
        }
        reachableMap[heli.id] = reachableVillages;
    }

    return reachableMap;
}

vector<vector<int>> calculateVillagexVillage(const ProblemData& problem) {
    int num_villages = problem.villages.size();
    vector<vector<int>> dist(num_villages, vector<int>(num_villages, 0));

    for (int i = 0; i < num_villages; ++i) {
        for (int j = 0; j < num_villages; ++j) {
            if (i != j) {
                int dx = problem.villages[i].coords.x - problem.villages[j].coords.x;
                int dy = problem.villages[i].coords.y - problem.villages[j].coords.y;
                dist[i][j] = static_cast<int>(round(sqrt(dx * dx + dy * dy)));
            }
        }
    }
    return dist;
}

vector<vector<int>> calculateCityxVillage(const ProblemData& problem) {
    int num_cities = problem.cities.size();
    int num_villages = problem.villages.size();
    vector<vector<int>> dist(num_cities, vector<int>(num_villages, 0));

    for (int i = 0; i < num_cities; ++i) {
        for (int j = 0; j < num_villages; ++j) {
            int dx = problem.cities[i].x - problem.villages[j].coords.x;
            int dy = problem.cities[i].y - problem.villages[j].coords.y;
            dist[i][j] = static_cast<int>(round(sqrt(dx * dx + dy * dy)));
        }
    }
    return dist;
}

map<int, set<int>> buildCommonNode(const map<int, vector<int>>& reachableMap) {
    map<int, set<int>> villageToHelis; // village_id -> set of heli_ids

    for (auto& [hid, villages] : reachableMap) {
        for (int vid : villages) {
            villageToHelis[vid].insert(hid);
        }
    }

    // Filter only those villages that are reachable by more than 1 heli
    map<int, set<int>> common;
    for (auto& [vid, helis] : villageToHelis) {
        if (helis.size() > 1) {
            common[vid] = helis;
        }
    }

    return common;
}

void printVillagexVillage(const vector<vector<int>>& dist) {
    cout << "-----------------------Village to Village Distances:-----------------------------------" << endl;
    for (const auto& row : dist) {
        cout << "V" << &row - &dist[0] + 1 << ": ";
        for (const auto& d : row) {
            cout << "V" << &d - &row[0] + 1 << "=";
            cout << d << " ";
        }
        cout << endl;
    }
}

void printCityxVillage(const vector<vector<int>>& dist) {
    cout << "---------------------------City to Village Distances:---------------------------------" << endl;
    for (const auto& row : dist) {
        cout << "C" << &row - &dist[0] + 1 << ": ";
        for (const auto& d : row) {
            cout << "V" << &d - &row[0] + 1 << "=";
            cout << d << " ";
        }
        cout << endl;
    }
}

void printReachableCity(State& state){
    cout << "------------------- Reachable Villages per Helicopter -------------------" << endl;
    for (auto& [hid, villages] : state.zone) {
        cout << "Helicopter H" << hid << " can reach villages: ";
        if (villages.empty()) cout << "(none)";
        for (int vid : villages) cout << "V" << vid << " ";
        cout << endl;
    }
}

void printReachableVillages(auto& common){
    cout << "-----------------------------------------------------------------------" << endl;
    for (auto& [vid, helis] : common) {
        cout << "Village " << vid << " is reachable by helicopters: ";
        for (int hid : helis) {
            cout << "H" << hid << " ";
        }
        cout << endl;
    }
}

void printStateInfo(State& state){
    //This function can be used to print the information regarding any states
    cout << "---------------------------State info---------------------------" << endl;
    for(auto& [hid, reachableVillageList] : state.zone){
        cout << "Helicopter ID : " << hid << " -> { ";
        for(int vid : reachableVillageList){
            cout << vid << ", ";
        } 
        cout << "}" << endl;
    }
}

Solution solve(const ProblemData& problem) {
    // State state;

    cout << "Starting solver..." << endl;

    Solution solution;

    int hNum = problem.helicopters.size();
    int vNum = problem.villages.size();

    // cout<<hNum<<" "<<vNum<<"\n";

    auto villToVill_Dist = calculateVillagexVillage(problem);
    auto cityToVill_Dist = calculateCityxVillage(problem);

    printCityxVillage(cityToVill_Dist);
    printVillagexVillage(villToVill_Dist);
    cout<<endl;

    auto reachable = buildReachableMap(problem);
    
    // state.zone.insert(reachable.begin() , reachable.end());
    // state.villageList = problem.villages;
    // state.helicopterList = problem.helicopters;

    //map1.insert(map2.begin(), map2.end());
    auto common = buildCommonNode(reachable);

    set<int> commonVillages;
    for (auto& [village, helis] : common) {
        commonVillages.insert(village);
    }

    // Changing no common to singleTonVillageList
    // Build new map:which contain no common village or does not contain any elemtn from comminVillages
    map<int, vector<int>> singleTonVillageList;
    
    for (auto& [heli, villages] : reachable) {
        vector<int> uniqueVillages;
        for (int v : villages) {
            if (commonVillages.find(v) == commonVillages.end()) {
                uniqueVillages.push_back(v);
            }
        }
        singleTonVillageList[heli] = uniqueVillages; // even if empty
    }
    //state.zone = noCommon;

    // Print result
    // cout << "Singelton villages per helicopter:\n";
    // for (auto& [heli, villages] : singleTonVillageList) {
    //     cout << "Helicopter " << heli << " -> ";
    //     for (int v : villages) cout << v << " ";
    //     cout << endl;
    // }

    State initialstate(singleTonVillageList, problem, solution);
    createRandomInitialState(initialstate, problem, singleTonVillageList, common);
    evaluateState(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    // printStateInfo(initialstate);
    // State intialState = intialStateCreation(problem , noCommon, common);

    
    cout << "Solver finished." << endl;
    return solution;
}