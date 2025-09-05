#include "solver.h"
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <random>
#include <vector>

using namespace std;
using namespace std::chrono;
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
    cout << "Objective Value : " << state.o << endl;
    for(auto& [hid, reachableVillageList] : state.zone){
        cout << "Helicopter ID : " << hid << " -> { ";
        for(int vid : reachableVillageList){
            cout << vid << ", ";
        } 
        cout << "}" << endl;
        cout << "\tZone Others Requirement : " << state.helicopterList[hid].zone_requirement.first << endl;
        cout << "\tZone Food Requirement : " << state.helicopterList[hid].zone_requirement.second << endl;
    }
    cout << endl;
    for(HelicopterPlan& helicopterPlan : state.helicopterPlan){
        cout << "Plan for Helicopter : " << helicopterPlan.helicopter_id << endl;
        cout << "Distance Travelled by Helicopter : " << state.helicopterList[helicopterPlan.helicopter_id - 1].distanceTravelledByHeliCopter << endl;
        cout << "Number of Trips done : " << helicopterPlan.trips.size() << endl;
        for(Trip& trip: helicopterPlan.trips){
            cout << "\tDry Food Picked : " << trip.dry_food_pickup << endl;
            cout << "\tPerishable Food Picked : " << trip.perishable_food_pickup << endl;
            cout << "\tOther Supplies Picked : " << trip.other_supplies_pickup << endl;
            cout << "\tNumber of Villages visited : " << trip.drops.size() << endl;
            cout << "\tTotal Distance travelled : " << trip.distanceTravelledThisTrip << endl;
            for(Drop& drop: trip.drops){
                cout << "\t\tVillage Id : " << drop.village_id << endl;
                cout << "\t\tDry Food Dropped : " << drop.dry_food << endl;
                cout << "\t\tPerishable Food Dropped : " << drop.perishable_food << endl;
                cout << "\t\tOther Supplies Dropped : " << drop.other_supplies << endl;
            }
        }
    }
}

double objectiveFunction(State& state, ProblemData data){
    double totalValueGained = 0;
    double totalDistancetravelled = 0;
    double totalCostIncurred = 0;
    for(Village& village : state.villageList){
        totalValueGained += village.value_gained;
    }
    for(HelicopterPlan& plan : state.helicopterPlan){
        totalDistancetravelled = state.helicopterList[plan.helicopter_id - 1].distanceTravelledByHeliCopter;
        totalCostIncurred += (state.helicopterList[plan.helicopter_id - 1].fixed_cost) * plan.trips.size() + 
                                state.helicopterList[plan.helicopter_id - 1].alpha * totalDistancetravelled;
    }
    //cout << "Total Value Gained : " << totalValueGained << endl;
    //cout << "Total Trip cost : " << totalCostIncurred << endl;
    state.o = totalValueGained - totalCostIncurred;
    return totalValueGained - totalCostIncurred;
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

            //cout << "Neighbour: V" << vid<< " moved from H" << currentHeli<< " -> H" << newHeli << endl;
            //printStateInfo(neighbour);

            neighbours.push_back(neighbour);

        }
    }

    return neighbours;
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

           //cout << "Assigned common Village V" << vid<< " to Helicopter H" << chosenHeli << endl;
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

void calculateTripDistances(State& state, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    for(HelicopterPlan& plan : state.helicopterPlan){
        double totalDistanceTravelledByHelicopter = 0;
        //cout << "Distance calculation for Helicopter : " << plan.helicopter_id << endl;
        for(Trip& trip : plan.trips){
            double totalDistanceTravelledPerTrip = 0;
            bool isCity = true;
            int previous_vid;
            for(Drop& drop : trip.drops){
                if(isCity){
                    totalDistanceTravelledPerTrip += cityxvillage[plan.helicopter_id - 1][drop.village_id -1];
                    isCity = false;
                }
                else{
                    totalDistanceTravelledPerTrip += villagexvillage[previous_vid -1][drop.village_id -1];
                    
                }
                previous_vid = drop.village_id;

            }
            totalDistanceTravelledPerTrip += cityxvillage[plan.helicopter_id - 1][previous_vid -1];
            trip.distanceTravelledThisTrip = totalDistanceTravelledPerTrip;
            //cout << "\tDistance for this trip : " << totalDistanceTravelledPerTrip << endl;
            totalDistanceTravelledByHelicopter += totalDistanceTravelledPerTrip;
        }
        //cout << "Distance travelled by this Helicopter : " << totalDistanceTravelledByHelicopter << endl;
        state.helicopterList[plan.helicopter_id - 1].distanceTravelledByHeliCopter = totalDistanceTravelledByHelicopter;
    }
}

vector<int> fillHelicopter(ProblemData data, int zonefoodRequirements, int zoneOthersRequirements, int Wcap){
    //cout << endl;
    vector<PackageInfo> packages = data.packages;
    int nd = 0, np = 0, no = 0;

    double wd = packages[0].weight;
    double wp = packages[1].weight;
    double wo = packages[2].weight;

    double vd = packages[0].value;
    double vp = packages[1].value;

    //wcap wight total , wo weight of other
    //cout << "Wcap : " << Wcap << endl;
    // max amount of other heli can canrry
    int othersHelicopterLimit = floor(Wcap / wo);
    //cout << "Others Helicopter Limit : " << othersHelicopterLimit << endl;
    double weightRemaining = Wcap;
    //cout << "Weight Remaining : " << weightRemaining << endl;

    // Fill the zone requirements for Others Supplies
    // Giving the Other supplies first priority and deliver it first
    if(zoneOthersRequirements > 0){
        if(othersHelicopterLimit > zoneOthersRequirements){
            no = zoneOthersRequirements;
        }
        else{
            no = othersHelicopterLimit;
        }
        //cout << "No of Others to be filled : " << no << endl;
        weightRemaining -= no * wo;
    }
    //cout << "Weight Remaining after filling Others : " << weightRemaining << endl;
    // max amount of dry heli can carry
    int dryHelicopterLimit = weightRemaining / wd;
    // max amount of wet heli can carry
    //cout << "Dry Helicopter Limit : " << dryHelicopterLimit << endl;
    int perishableHelicopterLimit = weightRemaining / wp;
    //cout << "Perishable Helicopter Limit : " << perishableHelicopterLimit << endl;
    //cout << "Zone Food Requirement : " << zonefoodRequirements << endl;
    //cout << "Other Requirement : " << zoneOthersRequirements << endl;
    if(weightRemaining > 0){
        // Using equations to solve the values of nd and np
            // nd + np = R ------------- (1) here R is zoneFoodRequirements
            // nd*wd + np*wp = weightRemaining ---- (2)
            // nd*vd + nd*vp = V ------- (3)
            /*
            From (1) => np = R - nd
            Putting this value in (2)
            => nd*wd + (R - nd)*wp = weightRemaining
            => nd(wd - wp) + R*wp = weightRemaining
            => nd(wd - wp) = weightRemaining - R*wp
            => nd = (weightRemaining - R*wp) / (wd - wp)
            */
        if(zonefoodRequirements > 0){
            if(zonefoodRequirements > max(dryHelicopterLimit, perishableHelicopterLimit)){
                if(dryHelicopterLimit > perishableHelicopterLimit){
                    nd = dryHelicopterLimit;
                    np = 0;
                }
                else{
                    nd = 0;
                    np = perishableHelicopterLimit;
                }
            }
            else{
                if(zonefoodRequirements < perishableHelicopterLimit){
                    nd = 0;
                    // np = perishableHelicopterLimit; check
                    np = zonefoodRequirements;
                }
                else{
                    if(wd != wp){
                        double Wmin = zonefoodRequirements * min(wp, wd);
                        double Wmax = zonefoodRequirements * max(wp, wd);
                        // cout << "Wmin : " << Wmin << endl;
                        // cout << "Wmax : " << Wmax << endl;
                        // cout << "Weight Remaining : " << weightRemaining << endl; 
                        if(weightRemaining > Wmax){
                            weightRemaining = Wmax;
                        }
                        else if(weightRemaining < Wmin){
                            weightRemaining = Wmin;
                        }
                        nd  = ceil((weightRemaining - zonefoodRequirements * wp) / (wd - wp));
                        if(nd < 0) nd = 0;
                        if(nd > zonefoodRequirements) nd = zonefoodRequirements;

                        np = zonefoodRequirements - nd;
                    }
                    else{
                        double dd = vd / wd;
                        double dp = vp / wp;
                        if(dd > dp){
                            np = perishableHelicopterLimit;
                        }
                        else{
                            nd = dryHelicopterLimit;
                        }
                    }
                }
            }
        }
    }
    //cout << "Dry picked : " << nd << endl;
    //cout << "Perishable Picked " << np << endl;
    //cout << "Others Picked " << no << endl;
    return {nd, np, no};

}

Trip generateTripsViaSupplies(State& state, const ProblemData& data, int helicopterId, vector<int> zoneVillages){
    //cout << endl << "----------------Checking for trips for : " << helicopterId << "-------------------------" << endl;
    //First Calculate the requiremenets for this trip.
    int tripOtherRequirements = 0, tripFoodrequirement = 0;
    int i = 0;
    double vd = data.packages[0].value, vp = data.packages[1].value, vo = data.packages[2].value;
    for(int vid : zoneVillages){
        //cout << "vid : " << vid << endl;
        tripOtherRequirements += state.villageList[vid - 1].other_supplies_requirement;
        tripFoodrequirement += state.villageList[vid - 1].food_requirement;
    }
    //cout << "Trip other requirement : " << tripOtherRequirements << endl;
    //cout << "Trip Food Requirement : " << tripFoodrequirement << endl;

    Trip trip;
    vector<Drop> drops;
    // fill the helicopter according to this trip.
    vector<int> helicopter = fillHelicopter(data, tripFoodrequirement, tripOtherRequirements, state.helicopterList[helicopterId - 1].weight_capacity);
    int nd = helicopter[0], np = helicopter[1], no = helicopter[2];
    trip.dry_food_pickup = nd;
    trip.perishable_food_pickup = np;
    trip.other_supplies_pickup = no;
    while(i < zoneVillages.size()){
        //First check if this village is already completed or not
        if(state.villageList[zoneVillages[i] - 1].other_supplies_requirement != 0 
            || state.villageList[zoneVillages[i] - 1].food_requirement != 0){
            //cout << "Village we are checking : " << zoneVillages[i] << endl;
            Drop drop;
            int dryFoodDropped = 0, perishableFoodDropped = 0, othersDropped = 0;
            // Give nd, np, no according to the village requirement and how
            // much we have in our helicopter
            if(state.villageList[zoneVillages[i] - 1].other_supplies_requirement > no){
                othersDropped = no;
                state.villageList[zoneVillages[i] - 1].other_supplies_requirement -= no;
                no = 0;
            }
            else{
                othersDropped = state.villageList[zoneVillages[i] - 1].other_supplies_requirement;
                no -= state.villageList[zoneVillages[i] - 1].other_supplies_requirement;
                state.villageList[zoneVillages[i] - 1].other_supplies_requirement = 0;
            }
            if(state.villageList[zoneVillages[i] - 1].food_requirement >= np){
                perishableFoodDropped = np;
                state.villageList[zoneVillages[i] - 1].food_requirement -= np;
                np = 0;
            }
            else{
                perishableFoodDropped = state.villageList[zoneVillages[i] - 1].food_requirement;
                np -= state.villageList[zoneVillages[i] - 1].food_requirement;
                state.villageList[zoneVillages[i] - 1].food_requirement = 0;
            }
            if(state.villageList[zoneVillages[i] - 1].food_requirement > nd){
                dryFoodDropped = nd;
                state.villageList[zoneVillages[i] - 1].food_requirement -= nd;
                nd = 0;
            }
            else{
                dryFoodDropped = state.villageList[zoneVillages[i] - 1].food_requirement;
                nd -= state.villageList[zoneVillages[i] - 1].food_requirement;
                state.villageList[zoneVillages[i] - 1].food_requirement = 0;
            }
            //Add this village to our drop
            drop.village_id = zoneVillages[i];
            drop.dry_food = dryFoodDropped;
            drop.perishable_food = perishableFoodDropped;
            drop.other_supplies = othersDropped;
            drops.push_back(drop);
            state.villageList[zoneVillages[i] - 1].value_gained += dryFoodDropped*vd + perishableFoodDropped*vp + othersDropped*vo;
            //Cant make any more trip just return.
            if(nd == 0 && np == 0 && no == 0){
                break;
            }
        }
        i++;
    }
    trip.drops = drops;
    // cout << "Printing for those whose drops are remaining : " << endl;
    // for(int vid : zoneVillages){
    //     cout << "\tfood remaining : " << state.villageList[vid - 1].food_requirement << endl;
    //     cout << "\tOthers Remaining : " << state.villageList[vid - 1].other_supplies_requirement << endl;
    // }
    // cout << endl;
    return trip;
}

vector<Trip> generateTripsViaDistance(State& state, const ProblemData& data, int helicopterId, vector<int> zoneVillages, double distanceCap, double dmax, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    int j = 0;
    bool newTrip = true;
    int totalDistanceTravelledPerTrip = 0;
    int distanceRequiredToReachBase;
    vector<Trip> trips;
    while(j < zoneVillages.size()){
        // Check if the village is completed or not
        if(state.villageList[zoneVillages[j] - 1].other_supplies_requirement != 0
            || state.villageList[zoneVillages[j] - 1].food_requirement != 0){
                // Distance from base to next village
                if(newTrip){
                    //cout << "New TRIP distance from base : " << helicopterId << " To : " << zoneVillages[j] << endl;
                    totalDistanceTravelledPerTrip += cityxvillage[helicopterId -1][zoneVillages[j] - 1];
                    newTrip = false;
                }
                // Distance from previous village to current village
                else{
                    //cout << "TRIP distance from village : " << zoneVillages[j] << " To : " << zoneVillages[j] << endl;
                    totalDistanceTravelledPerTrip += villagexvillage[zoneVillages[j - 1] - 1][zoneVillages[j] - 1];
                }
                //cout << "total distance travelled till now : " << totalDistanceTravelledPerTrip << endl;
                distanceRequiredToReachBase = totalDistanceTravelledPerTrip + cityxvillage[helicopterId - 1][zoneVillages[j] - 1];
                //cout << "Distance Required to reach base : " << distanceRequiredToReachBase << endl;
                if(distanceRequiredToReachBase <= distanceCap){
                    j++;
                }
                else{
                    //cout << "Calling Trip 1" << endl;
                    Trip trip = generateTripsViaSupplies(state, data, helicopterId, vector<int>(zoneVillages.begin(), zoneVillages.begin() + j));
                    trips.push_back(trip);
                    j = 0;
                    totalDistanceTravelledPerTrip = 0;
                    distanceRequiredToReachBase = 0;
                    newTrip = true;
                }
                if(j == zoneVillages.size()){
                    //cout << "Calling Trip 2" << endl;
                    Trip trip = generateTripsViaSupplies(state, data, helicopterId, vector<int>(zoneVillages.begin(), zoneVillages.begin() + j));
                    trips.push_back(trip);
                    j = 0;
                    totalDistanceTravelledPerTrip = 0;
                    distanceRequiredToReachBase = 0;
                    newTrip = true;
                }
        }
        else{
            j++;
        }
    }
    return trips;
}

void generateTrips2(State& state, const ProblemData& data, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    for(Helicopter helicopter : state.helicopterList){
        vector<Trip> tripForThisHelicopter = generateTripsViaDistance(state, data, helicopter.id, state.zone[helicopter.id], helicopter.distance_capacity, helicopter.weight_capacity, cityxvillage, villagexvillage);
        state.helicopterPlan[helicopter.id - 1].trips = tripForThisHelicopter;
    }
}

vector<Trip> generateTrips( int helicopterId, double distanceCap, vector<int> zoneVillages, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    /*
    This function is used to traverse through the given set of zone villages which are reachable by helicopterID
    and create the trips based on the distance capacity of the helicopter
    */
    // cout << endl << "FOR Helicopter : " << helicopterId << endl;
    // cout << "Distance Cap for this helicopter : " << distanceCap << endl;
    int distance_travelled = 0;
    vector<Trip> trips; // to store the trips done by helicopter
    set<int> visitedVillages;
    while(true){
        // cout << endl<< "STARTING WHILE LOOP " << endl;
        bool iscity = true;
        if(visitedVillages.size() == zoneVillages.size()){
            // cout << "BREAK THIS CURSE" << endl;
            break;
        }
        Trip trip = {0 ,0, 0}; // Initially create and empty trip
        vector<Drop> drops;
        int previous_vid;
        for(int vid : zoneVillages){
            if(visitedVillages.find(vid) == visitedVillages.end()){
                Drop drop;
                int distancetonew = 0;
                if(iscity){
                    // cout << "\tVillage ID trying to visit : " << vid << " FROM CITY : " << helicopterId << endl;
                    distancetonew = cityxvillage[helicopterId - 1][vid - 1];
                    iscity = false;
                }
                else{
                    // cout << "\tVillage ID trying to visit : " << vid << " FROM Village : " << previous_vid << endl;
                    //cout << "\t\tPREVIOUS : " << previous_vid << endl;
                    //cout << "\t\tCURRENT : " << vid << endl;
                    //cout << "\t\tVillage previous to current Distance : " << villagexvillage[previous_vid - 1][vid - 1] << endl;
                    distancetonew = villagexvillage[previous_vid - 1][vid - 1];
                }
                previous_vid = vid;
                // cout << "\t\tDistance from - to : " << distancetonew << endl;
                int new_distance = distance_travelled + distancetonew;
                // cout << "\t\ttotal Distance to reach this village: " << new_distance << endl;
                // cout << "\t\tHome Distance from from this village " << new_distance + cityxvillage[helicopterId - 1][vid - 1] << endl;
                if(new_distance + cityxvillage[helicopterId - 1][vid - 1] <= distanceCap){
                    if(visitedVillages.find(vid) == visitedVillages.end()){
                        // cout << "\t\tInserting village " << vid << " To visited villages " << endl;
                        distance_travelled = new_distance;
                        visitedVillages.insert(vid);
                        drop.village_id = vid;
                        drops.push_back(drop);
                    }
                }
                else{
                    // cout << "OH NO THIS DISTANCE IS GREATER THAN DCAP. CANT GO HERE MAKE A NEW TRIP" << endl;
                }
            }
        }
        //cout << "Visited Villages => " << visitedVillages.size() << endl;
        trip.drops = drops;
        trips.push_back(trip);
    }
    return trips;
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
        /*
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
        */
        // cout << "OUT OF WHILE" << endl;  
        state.helicopterPlan[helicopter.id - 1].trips = generateTrips(helicopter.id,helicopter.distance_capacity, state.zone[helicopter.id], cityxvillage, villagexvillage);
    }
    // cout << "OUT FROM EVERY LOOP" << endl;
}

void dropSupplies(State& state, const ProblemData& data, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    for(HelicopterPlan& helicopterPlan : state.helicopterPlan){
        cout << "Droppings of Helicopter : " << helicopterPlan.helicopter_id << endl;
        while(true){
            vector<int> remainingVillages = {}; 
            for(Trip& trip: helicopterPlan.trips){
                if(!trip.done){
                    int nd, np, no;
                    vector<int> result = fillHelicopter(data, trip.tripFoodRequirement, trip.tripOtherRequirment, state.helicopterList[helicopterPlan.helicopter_id - 1].weight_capacity);
                    nd = result[0], np = result[1], no = result[2]; 
                    cout << "\tTrip Other Requirement : " << trip.tripOtherRequirment << endl;
                    cout << "\tTrip Food Requirement : " << trip.tripFoodRequirement << endl;
                    cout << "\t nd : " << nd << ", np : " << np << ", no : " << no << endl;
                    for(Drop& drop: trip.drops){
                        cout << "\t\tVillage Id : " << drop.village_id << endl;
                        int villageOtherRequirement = state.villageList[drop.village_id - 1].other_supplies_requirement;
                        int villageFoodRequirement = state.villageList[drop.village_id - 1].food_requirement;
                        cout << "\t\t\tBEFORE DROPPING " << endl;
                        cout << "\t\t\tvillage other requirements : " << villageOtherRequirement << endl;
                        cout << "\t\t\tvillage food requirements : " << villageFoodRequirement << endl;
                        cout << "\t\t\tnd : " << nd << ", np : " << np << ", no : " << no << endl;
                        if(no >= villageOtherRequirement){
                            no -= villageOtherRequirement;
                            villageOtherRequirement = 0;
                        }
                        else{
                            villageFoodRequirement -= no;
                            no = 0;
                        }
                        if(np >= villageFoodRequirement){
                            np -= villageFoodRequirement;
                            villageFoodRequirement = 0;
                        }
                        else{
                            villageFoodRequirement -= np;
                            np = 0;
                        }
                        if(nd >= villageFoodRequirement){
                            nd -= villageFoodRequirement;
                            villageFoodRequirement = 0;
                        }
                        else{
                            villageFoodRequirement -= nd;
                            nd = 0;
                        }
                        state.villageList[drop.village_id - 1].other_supplies_requirement = villageOtherRequirement;
                        state.villageList[drop.village_id -1].food_requirement = villageFoodRequirement;
                        cout << "\t\t\tAFTER DROPPING " << endl;
                        cout << "\t\t\tvillage other requirements : " << villageOtherRequirement << endl;
                        cout << "\t\t\tvillage food requirements : " << villageFoodRequirement << endl;
                        cout << "\t\t\tnd : " << nd << ", np : " << np << ", no : " << no << endl;

                        if(nd == 0 && np == 0 && no == 0){
                            if(villageFoodRequirement > 0 || villageOtherRequirement > 0){
                                cout << "!!!!!!!!!!!!SOME VILLAGES LEFT WILL HAVE TO MAKE NEW TRIP" << endl;
                                remainingVillages.push_back(drop.village_id);
                            }
                            else{
                                cout << "THIS VILLAGE IS DONE LETS GO TO NEXT STOP" << endl;
                            }
                        }
                        
                    }
                    trip.done = true;
                }
            }

            /*
            check if any village is remaining for this helicopter.
            if : then add it to remaining villages and apply this loop again until all villages are satisfied.
            else : you are done.
            */
            if(remainingVillages.size() != 0){
                cout << endl << "These villages are remaining : " << endl;
                for(int vid : remainingVillages){
                    cout << "\t" << vid << endl;
                }
                vector<Trip> newTrips = generateTrips(helicopterPlan.helicopter_id, state.helicopterList[helicopterPlan.helicopter_id - 1].distance_capacity, remainingVillages, cityxvillage, villagexvillage);
                for(Trip& trip : newTrips){
                    helicopterPlan.trips.push_back(trip);
                }
            }
            else{
                break;
            }
        }
    }
}

void applyTrips(State& state, const ProblemData& data, vector<vector<int>> cityxvillage, vector<vector<int>> villagexvillage){
    vector<vector<pair<int, int>>> helicopterTripsRequirement;
    for(HelicopterPlan& helicopterPlan : state.helicopterPlan){
        vector<pair<int, int>> totalZoneRequirment;
        int zoneOtherRequirement = 0, zoneFoodRequirement = 0;
        cout << "Plan for Helicopter : " << helicopterPlan.helicopter_id << endl;
        cout << "Number of Trips done : " << helicopterPlan.trips.size() << endl;
        for(Trip& trip: helicopterPlan.trips){
            int thisTripFoodRequirement = 0;
            int thisTripOtherRequirement = 0;
            for(Drop& drop: trip.drops){
                thisTripFoodRequirement += state.villageList[drop.village_id - 1].food_requirement;
                thisTripOtherRequirement += state.villageList[drop.village_id - 1].other_supplies_requirement;
            }
            trip.tripOtherRequirment = thisTripOtherRequirement;
            trip.tripFoodRequirement = thisTripFoodRequirement;
            zoneOtherRequirement += thisTripOtherRequirement;
            zoneFoodRequirement += thisTripFoodRequirement;
            totalZoneRequirment.push_back({thisTripOtherRequirement, thisTripFoodRequirement});
        }
        state.helicopterList[helicopterPlan.helicopter_id].zone_requirement = {zoneOtherRequirement, zoneFoodRequirement};
        helicopterTripsRequirement.push_back(totalZoneRequirment);
    }
    // cout << endl << "PRINTING THIS " << endl;
    // for(int heli_index = 0; heli_index < state.helicopterList.size(); heli_index++){
    //     cout << "For Helicopter : " << heli_index + 1 << endl;
    //     for(int trips = 0; trips < helicopterTripsRequirement[heli_index].size(); trips++){
    //         cout << "\tFor trip : " << trips + 1 << endl;
    //         cout << "\t\tOther requirement : " << helicopterTripsRequirement[heli_index][trips].first << endl;
    //         cout << "\t\tFood requirement : " << helicopterTripsRequirement[heli_index][trips].second << endl;
    //     }
    // }

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
    auto start = high_resolution_clock::now();
    Solution solution;
    int TIME_LIMIT = 58;
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
    cout << endl;
    //printReachableVillages(common);
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
    // cout << endl;
    // cout << "Singelton villages per helicopter:\n";
    // for (auto& [heli, villages] : singleTonVillageList) {
    //     cout << "Helicopter " << heli << " -> ";
    //     for (int v : villages) cout << v << " ";
    //     cout << endl;
    // }


    State initialstate(singleTonVillageList, problem, solution);
    createRandomInitialState(initialstate, problem, singleTonVillageList, common);

    State current = initialstate;
    // vector<State> neighbours = generateNeighbourhood(stateClone, common);
    // vector<State> neighboursClone = neighbours;

    generateTrips2(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    calculateTripDistances(initialstate, cityToVill_Dist, villToVill_Dist);
    double maxObjectiveValue = objectiveFunction(initialstate, problem);

    cout << "Objective Value of Initital : " << maxObjectiveValue << endl;

    State champion = initialstate;  
    while(true){
        auto now = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(now - start);
        if(duration.count() >= TIME_LIMIT){
            break;
        }
        double maxObjectiveValueOfNeighbours = INT_MIN;
        int indexOfMaxNeighbour = -1;
        vector<State> neighbours = generateNeighbourhood(current, common);
        vector<State> neighboursclone = neighbours;
        for(int i = 0; i < neighbours.size(); i++){
            generateTrips2(neighboursclone[i], problem, cityToVill_Dist, villToVill_Dist);
            calculateTripDistances(neighboursclone[i], cityToVill_Dist, villToVill_Dist);
            double neighbourObjectiveValue = objectiveFunction(neighboursclone[i], problem);
            if(maxObjectiveValueOfNeighbours < neighbourObjectiveValue){
                maxObjectiveValueOfNeighbours = neighbourObjectiveValue;
                indexOfMaxNeighbour = i;
            }
        }
        if(maxObjectiveValueOfNeighbours > champion.o){
            champion = neighboursclone[indexOfMaxNeighbour];
        }
        if(maxObjectiveValueOfNeighbours > current.o){
            current = neighbours[indexOfMaxNeighbour];
        }
        else{
            State initialstate(singleTonVillageList, problem, solution);
            createRandomInitialState(initialstate, problem, singleTonVillageList, common);
            current = initialstate;
        }
    }
    
    //printStateInfo(initialstate);
    //createBaseTrips(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    //applyTrips(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    //dropSupplies(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    //cout << "BACK FROM EVALUATION" << endl;
    //printStateInfo(initialstate);
    //vector<Trip> trips = generateTripsViaDistance(initialstate, problem, 1, {1, 2, 3, 4, 5, 6}, 130, 500, cityToVill_Dist, villToVill_Dist);
    //initialstate.helicopterPlan[0].trips = trips;
    printStateInfo(champion);
    cout << "Solver finished." << endl;
    return champion.helicopterPlan;
}