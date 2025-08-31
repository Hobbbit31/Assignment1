#include "solver.h"
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <vector>

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

Solution solve(const ProblemData& problem) {
    State state;

    cout << "Starting solver..." << endl;

    Solution solution;

    int hNum = problem.helicopters.size();
    int vNum = problem.villages.size();

    cout<<hNum<<" "<<vNum<<"\n";

    auto villToVill_Dist = calculateVillagexVillage(problem);
    auto cityToVill_Dist = calculateCityxVillage(problem);

    printCityxVillage(cityToVill_Dist);
    printVillagexVillage(villToVill_Dist);
    cout<<endl;

    auto reachable = buildReachableMap(problem);
    
    // state.zone.insert(reachable.begin() , reachable.end());
    state.villageList = problem.villages;
    state.helicopterList = problem.helicopters;

    //map1.insert(map2.begin(), map2.end());
    auto common = buildCommonNode(reachable);

    set<int> commonVillages;
    for (auto& [village, helis] : common) {
        commonVillages.insert(village);
    }

    // Build new map:which contain no common village or does not contain any elemtn from comminVillages
    map<int, vector<int>> noCommon;
    
    for (auto& [heli, villages] : reachable) {
        vector<int> uniqueVillages;
        for (int v : villages) {
            if (commonVillages.find(v) == commonVillages.end()) {
                uniqueVillages.push_back(v);
            }
        }
        noCommon[heli] = uniqueVillages; // even if empty
    }
    state.zone = noCommon;

    // Print result
    // cout << "Singelton villages per helicopter:\n";
    // for (auto& [heli, villages] : noCommon) {
    //     cout << "Helicopter " << heli << " -> ";
    //     for (int v : villages) cout << v << " ";
    //     cout << endl;
    // }
    

    // cout << "------------------- Reachable Villages per Helicopter -------------------" << endl;
    // for (auto& [hid, villages] : state.zone) {
    //     cout << "Helicopter H" << hid << " can reach villages: ";
    //     if (villages.empty()) cout << "(none)";
    //     for (int vid : villages) cout << "V" << vid << " ";
    //     cout << endl;
    // }

    //  cout << "-----------------------------------------------------------------------" << endl;
    // for (auto& [vid, helis] : common) {
    //     cout << "Village " << vid << " is reachable by helicopters: ";
    //     for (int hid : helis) {
    //         cout << "H" << hid << " ";
    //     }
    //     cout << endl;
    // }

    // --- START OF PLACEHOLDER LOGIC ---
    // This is a naive example: send each helicopter on one trip to the first village.
    // This will definitely violate constraints but shows the structure.
    
    // for (const auto& helicopter : problem.helicopters) {
    //     HelicopterPlan plan;
    //     plan.helicopter_id = helicopter.id;

    //     if (!problem.villages.empty()) {
    //         Trip trip;
    //         // Pickup 1 of each package type
    //         trip.dry_food_pickup = 1;
    //         trip.perishable_food_pickup = 1;
    //         trip.other_supplies_pickup = 1;

    //         // Drop them at the first village
    //         Drop drop;
    //         drop.village_id = problem.villages[0].id;
    //         drop.dry_food = 1;
    //         drop.perishable_food = 1;
    //         drop.other_supplies = 1;

    //         trip.drops.push_back(drop);
    //         plan.trips.push_back(trip);
    //     }
    //     solution.push_back(plan);
    // }
    
    // --- END OF PLACEHOLDER LOGIC ---

    cout << "Solver finished." << endl;
    return solution;
}