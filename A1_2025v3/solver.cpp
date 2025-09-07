#include "solver.h"
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <random>
#include <vector>

using namespace std;
using namespace std::chrono;
const double EPS = 1e-6;
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

bool checkWeightCapacity(int heli_id, int trip_no, double trip_weight, double capacity) {
    if (trip_weight > capacity + 1e-9) {
        cout << "*** WARNING: Heli " << heli_id << ", Trip " << trip_no
             << " exceeds weight capacity (" << trip_weight
             << " > " << capacity << ")." << endl;
        return false;
    }
    return true;
}

bool checkDropConsistency(int heli_id, int trip_no,int total_d, int d, int total_p, int p, int total_o, int o) {
    if (total_d > d || total_p > p || total_o > o) {
        cout << "*** WARNING: Heli " << heli_id << ", Trip " << trip_no
             << " drops more packages than picked up." << endl;
        return false;
    }
    return true;
}

bool checkTripDistance(int heli_id, int trip_no, double trip_distance, double cap) {
    if (trip_distance > cap) {
        cout << "*** WARNING: Heli " << heli_id << ", Trip " << trip_no
             << " exceeds trip distance capacity (" << trip_distance
             << " > " << cap << ")." << endl;
        return false;
    }
    return true;
}

bool checkTotalDistance(int heli_id, double total_distance, double dmax) {
    if (total_distance > dmax + 1e-9) {
        cout << "*** WARNING: Heli " << heli_id
             << " exceeds DMax (" << total_distance
             << " > " << dmax << ")." << endl;
        return false;
    }
    return true;
}

bool isValidState(State& state, const ProblemData& data){
    // Check if this State does not Exceed Dmax
    double wd = data.packages[0].weight, wp = data.packages[1].weight, wo = data.packages[2].weight;
    bool weightCons = false, dropCons = false, tripCons = false, totaldistanceCons = false;
    bool flag = false;
    for(HelicopterPlan plan : state.helicopterPlan){
        if(flag){
            break;
        }
        int trip_number = 1;
        Helicopter helicopter = state.helicopterList[plan.helicopter_id - 1];
        for(Trip trip : plan.trips){
            double tripWeight = wd*trip.dry_food_pickup + wp*trip.perishable_food_pickup + wo*trip.other_supplies_pickup;
            int dryDropped = 0, perishableFoodDropped = 0, othersDropped = 0;
            // CHECK FOR WEIGHT CAP OF HELI
            for(Drop drop : trip.drops){
                dryDropped += drop.dry_food;
                perishableFoodDropped += drop.perishable_food;
                othersDropped += drop.other_supplies;
            }
            weightCons = checkWeightCapacity(helicopter.id, trip_number, tripWeight, helicopter.weight_capacity);
            dropCons = checkDropConsistency(helicopter.id, trip_number, 
                dryDropped, trip.dry_food_pickup,
                perishableFoodDropped, trip.perishable_food_pickup,
                othersDropped, trip.other_supplies_pickup);
            tripCons = checkTripDistance(helicopter.id, trip_number, trip.distanceTravelledThisTrip, helicopter.distance_capacity);
            if(!weightCons || !dropCons || !tripCons){
                //cout << "W " << weightCons << " D " << dropCons << " T " << tripCons << endl;
                flag = true;
                break;
            }

        }
        totaldistanceCons = checkTotalDistance(helicopter.id, helicopter.distanceTravelledByHeliCopter, data.d_max);
        if(!totaldistanceCons){
            //cout << "something there wrong";
            flag = true;
            break;
        }
    }
    if(flag){
        return false;
    }
    //cout << "THIS IS A VALID STATE " << endl;
    return true;
}

double objectiveFunction(State& state, ProblemData data){
    // Calculate and return the Objective Function
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

void calculateTripDistances(State& state, vector<vector<double>> cityxvillage, vector<vector<double>> villagexvillage){
    /*
    This Function is Defined to calculate Trip Distances covered by each helicopter and the 
    Total distance travelled by each helicopter and save them in the state itself.
    */

    // cout << endl;
    // cout << "------------------------------------------------------HELICOPTER PLAN -----------------------------" << endl;
    // for(HelicopterPlan plan : state.helicopterPlan){
    //     cout << "Helicopter Id : " << plan.helicopter_id << endl;
    //     cout << "Home ID : " << state.helicopterList[plan.helicopter_id - 1].home_city_id << endl;
    // }
    // cout << endl;
    for(HelicopterPlan& plan : state.helicopterPlan){
        double totalDistanceTravelledByHelicopter = 0;
        //cout << "Distance calculation for Helicopter : " << plan.helicopter_id << ", With Home City : " << state.helicopterList[plan.helicopter_id - 1].home_city_id << endl;
        //cout << "Number of Trips done : " << plan.trips.size() << endl;
        for(Trip& trip : plan.trips){
            double totalDistanceTravelledPerTrip = 0;
            bool isCity = true;
            int previous_vid = plan.helicopter_id;
            for(Drop& drop : trip.drops){
                if(isCity){
                    //cout << "\tDistance from City : " << state.helicopterList[plan.helicopter_id - 1].home_city_id << " To Village : " << drop.village_id << " : " << cityxvillage[state.helicopterList[plan.helicopter_id - 1].home_city_id - 1][drop.village_id -1] << endl;
                    totalDistanceTravelledPerTrip += cityxvillage[state.helicopterList[plan.helicopter_id - 1].home_city_id - 1][drop.village_id -1];
                    isCity = false;
                }
                else{
                    //cout << "\tDistance from village : " << previous_vid << " To Village : " << drop.village_id << " : " << villagexvillage[previous_vid -1][drop.village_id -1] << endl;
                    totalDistanceTravelledPerTrip += villagexvillage[previous_vid -1][drop.village_id -1];
                    
                }
                previous_vid = drop.village_id;

            }
            totalDistanceTravelledPerTrip += cityxvillage[state.helicopterList[plan.helicopter_id - 1].home_city_id - 1][previous_vid -1];
            totalDistanceTravelledPerTrip += trip.distanceTravelledThisTrip;
            trip.distanceTravelledThisTrip = totalDistanceTravelledPerTrip;
            //cout << "\tDistance for this trip : " << totalDistanceTravelledPerTrip << endl;
            totalDistanceTravelledByHelicopter += totalDistanceTravelledPerTrip;
        }
        //cout << "Distance travelled by this Helicopter : " << totalDistanceTravelledByHelicopter << endl;
        state.helicopterList[plan.helicopter_id - 1].distanceTravelledByHeliCopter = totalDistanceTravelledByHelicopter;
        //cout << endl;
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

pair<Trip,bool> generateTripsViaSupplies(State& state, const ProblemData& data, Helicopter& helicopter, vector<int> zoneVillages, int limiterIndex, double dmax, vector<vector<double>> cityxvillage, vector<vector<double>> villagexvillage){
    /*
    Breaks trips on the basis of How much supplies the helicopter can complete in its one trip
    */
   
    //cout << endl << "----------------Checking for trips for : " << helicopter.id << "-------------------------" << endl;
    //First Calculate the requiremenets for this trip.
    int tripOtherRequirements = 0, tripFoodrequirement = 0;
    int i = 0;
    double vd = data.packages[0].value, vp = data.packages[1].value, vo = data.packages[2].value;
    for(int vid = 0; vid < limiterIndex ; vid++){
        //cout << "vid : " << vid << endl;
        tripOtherRequirements += state.villageList[zoneVillages[vid] - 1].other_supplies_requirement;
        tripFoodrequirement += state.villageList[zoneVillages[vid] - 1].food_requirement;
    }
    //cout << "Trip other requirement : " << tripOtherRequirements << endl;
    //cout << "Trip Food Requirement : " << tripFoodrequirement << endl;

    Trip trip;
    vector<Drop> drops;
    // fill the helicopter according to this trip.
    vector<int> helicopterWeights = fillHelicopter(data, tripFoodrequirement, tripOtherRequirements, helicopter.weight_capacity);
    int nd = helicopterWeights[0], np = helicopterWeights[1], no = helicopterWeights[2];
    double tripDistance = 0, distanceToReachBase = 0, distanceToNextStop = 0;
    bool isCity = true;

    trip.dry_food_pickup = nd;
    trip.perishable_food_pickup = np;
    trip.other_supplies_pickup = no;
    bool dmax_capped = false;
    while(i < limiterIndex){
        double distance = 0;
        //First check if this village is already completed or not
        if(state.villageList[zoneVillages[i] - 1].other_supplies_requirement != 0 
            || state.villageList[zoneVillages[i] - 1].food_requirement != 0){
            //cout << "Village we are checking : " << zoneVillages[i] << endl;
            if(isCity){
                distanceToNextStop = cityxvillage[helicopter.home_city_id - 1][zoneVillages[i] - 1];
                isCity = false;
                //cout << "Next Stop is Village : " << zoneVillages[i] << " From city : " << helicopter.home_city_id << endl;
            }
            else{
                distanceToNextStop += villagexvillage[zoneVillages[i - 1] - 1][zoneVillages[i] - 1];
                //cout << "Next Stop is Village : " << zoneVillages[i] << " From Village : " << zoneVillages[i - 1] << endl;
            }
            if(state.helicopterList[helicopter.id - 1].distanceTravelledByHeliCopter + distanceToNextStop + cityxvillage[helicopter.home_city_id - 1][zoneVillages[i] - 1] > dmax){
                //cout << "WILL HAVE TO CUT THIS TRIP SORT WE ARE OUT OF FLYING CAPACITY " << endl;

                //ANY VILLAGE WONT BE ABLE TO VISIT BY THIS HELICOPTER
                for(int vid : zoneVillages){
                    state.villageList[vid - 1].other_supplies_requirement = 0;
                    state.villageList[vid - 1].food_requirement = 0;
                }
                dmax_capped = true;
                break;
            }

            distanceToReachBase = distanceToNextStop + cityxvillage[helicopter.home_city_id - 1][zoneVillages[i] - 1];
            //cout << "\tDistance Between stops : " << distanceToNextStop << endl;
            //cout << "\tDistance Required to Reach home : " << distanceToReachBase << endl;
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
            //cout << "HERE 3.2" << endl;
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
            //cout << "HERE 3.3" << endl;
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
            //cout << "HERE 3.4" << endl;
            //Add this village to our drop
            //cout << "HERE 4.1" << endl;
            drop.village_id = zoneVillages[i];
            //cout << "HERE 4.2" << endl;
            drop.dry_food = dryFoodDropped;
            drop.perishable_food = perishableFoodDropped;
            drop.other_supplies = othersDropped;
            drops.push_back(drop);
            //cout << "HERE 4.3" << endl;
            state.villageList[zoneVillages[i] - 1].value_gained += dryFoodDropped*vd + perishableFoodDropped*vp + othersDropped*vo;
            //Cant make any more trip just return.
            //cout << "HERE 4.4" << endl;
            if(nd == 0 && np == 0 && no == 0){
                //cout << "HERE 5.1" << endl;
                break;
            }
        }
        i++;
    }
    trip.distanceTravelledThisTrip = distanceToReachBase;
    state.helicopterList[helicopter.id - 1].distanceTravelledByHeliCopter += distanceToReachBase;
    //helicopter.distanceTravelledByHeliCopter += distanceToReachBase;
    trip.drops = drops;
    // cout << "Printing for those whose drops are remaining : " << endl;
    // for(int vid : zoneVillages){
    //     cout << "\tfood remaining : " << state.villageList[vid - 1].food_requirement << endl;
    //     cout << "\tOthers Remaining : " << state.villageList[vid - 1].other_supplies_requirement << endl;
    // }
    // cout << endl;
    return {trip, dmax_capped};
}

vector<Trip> generateTripsViaDistance(State& state, const ProblemData& data, Helicopter& helicopter, vector<int> zoneVillages, double distanceCap, double dmax, vector<vector<double>> cityxvillage, vector<vector<double>> villagexvillage){
    /*
    Returns the trips that can be complete by the helicopter based on its distance and then 
    uses the helper fucntion generateTripsViaSupplies to break it down even further and 
    return the actual trips of helicopeter
    */

    int j = 0;
    bool newTrip = true;
    double totalDistanceTravelledPerTrip = 0;
    double distanceRequiredToReachBase;
    vector<Trip> trips;
    while(j < zoneVillages.size()){
        // Check if the village is completed or not
        if(state.villageList[zoneVillages[j] - 1].other_supplies_requirement != 0
            || state.villageList[zoneVillages[j] - 1].food_requirement != 0){
                // Distance from base to next village
                if(newTrip){
                    //cout << "New TRIP distance from base : " << helicopter.home_city_id << " To : " << zoneVillages[j] << endl;
                    totalDistanceTravelledPerTrip += cityxvillage[helicopter.home_city_id -1][zoneVillages[j] - 1];
                    newTrip = false;
                }
                // Distance from previous village to current village
                else{
                    //cout << "TRIP distance from village : " << zoneVillages[j - 1] << " To : " << zoneVillages[j] << endl;
                    totalDistanceTravelledPerTrip += villagexvillage[zoneVillages[j - 1] - 1][zoneVillages[j] - 1];
                }
                //cout << "total distance travelled till now : " << totalDistanceTravelledPerTrip << endl;
                distanceRequiredToReachBase = totalDistanceTravelledPerTrip + cityxvillage[helicopter.home_city_id - 1][zoneVillages[j] - 1];
                //cout << "Distance Required to reach base : " << distanceRequiredToReachBase << endl;
                //cout << "Distance Cap : " << distanceCap << endl;
                if(distanceRequiredToReachBase <= distanceCap + EPS){
                    j++;
                }
                else{
                    //cout << "Calling Trip 1" << endl;
                    pair<Trip ,bool> result = generateTripsViaSupplies(state, data, helicopter, vector<int>(zoneVillages.begin(), zoneVillages.begin() + j), j, data.d_max, cityxvillage, villagexvillage);
                    //cout << "7.1" << endl;
                    trips.push_back(result.first);
                    if(result.second){
                        break;
                    }
                    j = 0;
                    totalDistanceTravelledPerTrip = 0;
                    distanceRequiredToReachBase = 0;
                    newTrip = true;
                }
                if(j == zoneVillages.size()){
                    //cout << "Calling Trip 2" << endl;
                    //cout << "8.1" << endl;
                    pair<Trip ,bool> result = generateTripsViaSupplies(state, data, helicopter, vector<int>(zoneVillages.begin(), zoneVillages.begin() + j),  j, data.d_max, cityxvillage, villagexvillage);
                    //cout << "8.2" << endl;
                    trips.push_back(result.first);
                    if(result.second){
                        break;
                    }
                    //cout << "8.4" << endl;
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

void generateTrips(State& state, const ProblemData& data, vector<vector<double>> cityxvillage, vector<vector<double>> villagexvillage){
    for(Helicopter helicopter : state.helicopterList){
        //cout << "Generating Trip for Helicopter : " << helicopter.id << endl;
        vector<Trip> tripForThisHelicopter = generateTripsViaDistance(state, data, helicopter, state.zone[helicopter.id], helicopter.distance_capacity, helicopter.weight_capacity, cityxvillage, villagexvillage);
        state.helicopterPlan[helicopter.id - 1].trips = tripForThisHelicopter;
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
            if (2 * dist < heli.distance_capacity + EPS) {
                reachableVillages.push_back(problem.villages[j].id);
            }
        }
        reachableMap[heli.id] = reachableVillages;
    }

    return reachableMap;
}

vector<vector<double>> calculateVillagexVillage(const ProblemData& problem) {
    int num_villages = problem.villages.size();
    vector<vector<double>> dist(num_villages, vector<double>(num_villages, 0));

    for (int i = 0; i < num_villages; ++i) {
        for (int j = 0; j < num_villages; ++j) {
            if (i != j) {
                double dx = problem.villages[i].coords.x - problem.villages[j].coords.x;
                double dy = problem.villages[i].coords.y - problem.villages[j].coords.y;
                dist[i][j] = sqrt(dx * dx + dy * dy);
            }
        }
    }
    return dist;
}

vector<vector<double>> calculateCityxVillage(const ProblemData& problem) {
    double num_cities = problem.cities.size();
    double num_villages = problem.villages.size();
    vector<vector<double>> dist(num_cities, vector<double>(num_villages, 0));

    for (int i = 0; i < num_cities; ++i) {
        for (int j = 0; j < num_villages; ++j) {
            double dx = problem.cities[i].x - problem.villages[j].coords.x;
            double dy = problem.cities[i].y - problem.villages[j].coords.y;
            dist[i][j] = sqrt(dx * dx + dy * dy);
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

map<int, vector<int>> buildSingletonVillages(const map<int, vector<int>>& reachable, const map<int, set<int>>& common) 
{
    set<int> commonVillages;
    for (auto& [village, helis] : common) {
        commonVillages.insert(village);
    }

    map<int, vector<int>> singleTonVillageList;
    for (auto& [heli, villages] : reachable) {
        vector<int> uniqueVillages;
        for (int v : villages) {
            if (commonVillages.find(v) == commonVillages.end()) {
                uniqueVillages.push_back(v);
            }
        }
        singleTonVillageList[heli] = uniqueVillages; // can be empty
    }

    return singleTonVillageList;
}

void printVillagexVillage(const vector<vector<double>>& dist) {
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

void printCityxVillage(const vector<vector<double>>& dist) {
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

void removeMultipleHelicopter(ProblemData& data){
    vector<int> indexesOfDucplicate = {};
    set<int> alreadyhave;
    for(Helicopter helicopter : data.helicopters){
        if(alreadyhave.find(helicopter.id) == alreadyhave.end()){
            alreadyhave.insert(helicopter.id);
        }
        else{
            indexesOfDucplicate.push_back(helicopter.id);
        }
    }
    for(int i = 0; i < indexesOfDucplicate.size(); i++){
        data.helicopters.erase(data.helicopters.begin() + indexesOfDucplicate[i] - i);
    }
}

Solution solve(const ProblemData& problem) {
    // State state;

    cout << "Starting solver..." << endl;
    auto start = high_resolution_clock::now();
    Solution solution;
    int TIME_LIMIT = problem.time_limit_minutes * 60 - 2;

    
    int hNum = problem.helicopters.size();
    int vNum = problem.villages.size();

    // cout<<hNum<<" "<<vNum<<"\n";

    auto villToVill_Dist = calculateVillagexVillage(problem);
    auto cityToVill_Dist = calculateCityxVillage(problem);

    //printCityxVillage(cityToVill_Dist);
    //printVillagexVillage(villToVill_Dist);
    cout<<endl;

    auto reachable = buildReachableMap(problem);
    
    // state.zone.insert(reachable.begin() , reachable.end());
    // state.villageList = problem.villages;
    // state.helicopterList = problem.helicopters;

    //map1.insert(map2.begin(), map2.end());
    auto common = buildCommonNode(reachable);
    cout << endl;
    set<int> commonVillages;
    for (auto& [village, helis] : common) {
        commonVillages.insert(village);
    }

    // Changing no common to singleTonVillageList
    // Build new map:which contain no common village or does not contain any elemtn from comminVillages
    map<int, vector<int>> singleTonVillageList = buildSingletonVillages(reachable, common);
    
    //state.zone = noCommon;

    // Print result
    // Print common villages
    // cout << "Common villages (reachable by >1 heli):\n";
    // for (auto& [vid, helis] : common) {
    //     cout << "Village " << vid << " -> ";
    //     for (int h : helis) cout << h << " ";
    //     cout << endl;
    // }

    // // Print singleton villages
    // cout << "\nSingleton villages per helicopter:\n";
    // for (auto& [hid, villages] : singleTonVillageList) {
    //     cout << "Helicopter " << hid << " -> ";
    //     for (int v : villages) cout << v << " ";
    //     cout << endl;
    // }
    int numbersOfStateExplored = 0;

    State initialstate(singleTonVillageList, problem, solution);
    createRandomInitialState(initialstate, problem, singleTonVillageList, common);
    State current = initialstate;
    //cout << endl;
    generateTrips(initialstate, problem, cityToVill_Dist, villToVill_Dist);
    //cout << "RETURNED " << endl;
    //calculateTripDistances(initialstate, cityToVill_Dist, villToVill_Dist);
    double maxObjectiveValue = objectiveFunction(initialstate, problem);
    //cout << isValidState(initialstate, problem);
    // if(isValidState(initialstate, problem)){
    //     break;
    // }
    numbersOfStateExplored += 1;
    
    //printStateInfo(current);
    //cout << "After Generating Trips" << endl;
    //cout << endl;
    //cout << "Objective Value of Initital : " << maxObjectiveValue << endl;
    //cout << endl;
    //printStateInfo(initialstate);
    // cout << endl;
    // for(Helicopter helicopter : initialstate.helicopterList){
    //     cout << "Helicopter ID : " << helicopter.id << endl;
    //     cout << "Helicopter City ID : " << helicopter.home_city_id << endl; 
    // }
    // cout << endl;
    // for(HelicopterPlan plan : initialstate.helicopterPlan){
    //     cout << "Helicopter Id : " << plan.helicopter_id << endl;
    //     cout << "Home ID : " << initialstate.helicopterList[plan.helicopter_id - 1].home_city_id << endl;
    // }
    // cout << endl;
    // vector<State> neighbours = generateNeighbourhood(stateClone, common);
    // vector<State> neighboursClone = neighbours;

    State champion = initialstate;
    bool timeLimitExceeded = false; 
    // This loop is used for Random jump
    while(true){
        //cout << "Checking" << endl;
        auto now = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(now - start);
        // if(duration.count() % 60 < 5){
        //     cout << "TIME : " << duration.count() % 5 << endl;
        // }
        if(duration.count() >= TIME_LIMIT){
            //if we are above time limit we break out
            break;
        }
        double maxObjectiveValueOfNeighbours = INT_MIN;
        int indexOfMaxNeighbour = -1;
        vector<State> neighbours = generateNeighbourhood(current, common);
        vector<State> neighboursclone = neighbours;
        //cout << "HOW MANY NEIGHBORS : " << neighbours.size() << endl;
        for(int i = 0; i < neighbours.size(); i++){
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<seconds>(now - start);
            if(duration.count() % 60 < 5){
                cout << "Time Spent : " << duration.count() << endl; 
            }
            if(duration.count() >= TIME_LIMIT){
                //if we are above time limit we break out
                timeLimitExceeded = true;
                break;
            }
            //cout << "Time Spent : " << duration.count() << endl;
            //cout << "Checking for Neighbor : " << i + 1 << endl;
            generateTrips(neighboursclone[i], problem, cityToVill_Dist, villToVill_Dist);
            double neighbourObjectiveValue = objectiveFunction(neighboursclone[i], problem);
            numbersOfStateExplored += 1;
            if(maxObjectiveValueOfNeighbours < neighbourObjectiveValue){
                maxObjectiveValueOfNeighbours = neighbourObjectiveValue;
                indexOfMaxNeighbour = i;
            }
        }
        if(indexOfMaxNeighbour != -1 && 
            isValidState(neighboursclone[indexOfMaxNeighbour], problem) && 
                maxObjectiveValueOfNeighbours > champion.o){
            champion = neighboursclone[indexOfMaxNeighbour];
        }
        if(maxObjectiveValueOfNeighbours > current.o && indexOfMaxNeighbour != -1){
            current = neighbours[indexOfMaxNeighbour];
        }
        else{
            State initialstate(singleTonVillageList, problem, solution);
            createRandomInitialState(initialstate, problem, singleTonVillageList, common);
            current = initialstate;
        }
        if(timeLimitExceeded){
            break;
        }
    }
    //printStateInfo(champion);
    //isValidState(champion, problem);
    cout << "Solver finished." << endl;
    // cout << "Number of State Explored : " << numbersOfStateExplored << endl;
    //Check if our state is valid 
    // if not then return dummy answer : this will be only possible if no helicopter can complete its trip without capping over Dmax
    //else return the champion node.
    if(!isValidState(champion, problem)) return solution;
    return champion.helicopterPlan;
}