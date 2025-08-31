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
    cout << endl;
    for(HelicopterPlan& helicopterPlan : state.helicopterPlan){
        cout << "Plan for Helicopter : " << helicopterPlan.helicopter_id << endl;
        cout << "Number of Trips done : " << helicopterPlan.trips.size() << endl;
        for(Trip& trip: helicopterPlan.trips){
            cout << "\tDry Food Picked : " << trip.dry_food_pickup << endl;
            cout << "\tPerishable Food Picked : " << trip.perishable_food_pickup << endl;
            cout << "\tOther Supplies Picked : " << trip.other_supplies_pickup << endl;
            for(Drop& drop: trip.drops){
                cout << "\t\tVillage Id : " << drop.village_id << endl;
                cout << "\t\tDry Food Dropped : " << drop.dry_food << endl;
                cout << "\t\tPerishable Food Dropped : " << drop.perishable_food << endl;
                cout << "\t\tOther Supplies Dropped : " << drop.other_supplies << endl;
            }
        }
    }
}

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

vector<State> generateNeighbourhood(const State& current, const map<int, set<int>>& commonVill) {
    vector<State> neighbours;

    for (auto& [vid, heliSet] : commonVill) {
        if (heliSet.size() <= 1) continue; // skip if village only has one option

        // Find current heli
        int currentHeli = -1;
        for (auto& [hid, villages] : current.zone) {
            for (int v : villages) {
                if (v == vid) {
                    currentHeli = hid;
                    break;
                }
            }
            if (currentHeli != -1) break;
        }
        if (currentHeli == -1) continue; // safety

        // For each alternative heli
        for (int newHeli : heliSet) {
            if (newHeli == currentHeli) continue;

            // Copy state
            State neighbour = current;

            // Remove vid from current heli's zone
            auto& oldList = neighbour.zone[currentHeli];
            for (auto it = oldList.begin(); it != oldList.end(); ++it) {
                if (*it == vid) {
                    oldList.erase(it);
                    break;
                }
            }

            // Add to new heli's zone
            neighbour.zone[newHeli].push_back(vid);

            cout << "Neighbour: V" << vid<< " moved from H" << currentHeli<< " -> H" << newHeli << endl;
            printStateInfo(neighbour);

            neighbours.push_back(neighbour);

        }
    }

    return neighbours;
}

void createBaseTrips(State& state, const ProblemData& data, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    /*
    This function defines 1 possible trip on the basis of distance.
    It defines a trip in order {v1, v2, v3, .... vn}{ other trips}
    if the helicopter can visit villages v1 -> v2 -> .... vn in that order
    and get back to its city without capping its dcap
    */
    
    // Choosing the Correct Trips Based on Distance travelled by the helicopter
    for(Helicopter helicopter : state.helicopterList){
        // cout << endl << "FOR Helicopter : " << helicopter.id << endl;
        int distance_travelled = 0;
        vector<Trip> trips; // to store the trips done by helicopter
        set<int> visitedVillages;
        bool iscity = true;
        // cout << "before while of loop" << endl;
        while(true){
            // cout << visitedVillages.size() << endl;
            // cout << state.zone[helicopter.id].size() << endl;
            if(visitedVillages.size() == state.zone[helicopter.id].size()){
                // cout << "BREAK THIS CURSE" << endl;
                break;
            }
            Trip trip = {0 ,0, 0}; // Initially create and empty trip
            vector<Drop> drops;
            int previous_vid;
            for(int vid : state.zone[helicopter.id]){
                // cout << "\tVillage ID trying to visit : " << vid << endl;
                Drop drop;
                int distancetonew;
                if(iscity){
                    distancetonew = distance_travelled + cityxvillage[helicopter.id - 1][vid - 1];
                    iscity = false;
                }
                else{
                    distancetonew = distance_travelled + villagexvillage[previous_vid][vid - 1];
                }
                previous_vid = vid;
                // cout << "\t\tDistance to this village : " << distancetonew << endl;
                int new_distance = distance_travelled + distancetonew;
                // cout << "\t\ttotal Distance to reach this village: " << new_distance << endl;
                // cout << "\t\tHome Distance from from this village " << new_distance + cityxvillage[helicopter.id - 1][vid - 1] << endl;
                if(new_distance < new_distance + cityxvillage[helicopter.id - 1][vid - 1] && 
                    visitedVillages.find(vid) == visitedVillages.end()){
                    distance_travelled += new_distance;
                    visitedVillages.insert(vid);
                    drop.village_id = vid;
                    drops.push_back(drop);
                }
            }
            trip.drops = drops;
            trips.push_back(trip);
        }
        // cout << "OUT OF WHILE" << endl;  
        state.helicopterPlan[helicopter.id - 1].trips = trips;
    }
    // cout << "OUT FROM EVERY LOOP" << endl;
    
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

    printReachableVillages(common);

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
    createBaseTrips(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    // cout << "BACK FROM EVALUATION" << endl;
    printStateInfo(initialstate);

    //generatinf neighbour function for intialstate

    cout<<"______________________________"<<endl;
    generateNeighbourhood(initialstate, common);

    // State intialState = intialStateCreation(problem , noCommon, common);

    
    cout << "Solver finished." << endl;
    return solution;
}