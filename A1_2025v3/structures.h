#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <set>
#include <map>
#include <vector>
#include <cmath> // For sqrt and pow
using namespace std;

// --- GEOMETRIC & ENTITY STRUCTURES ---

struct Point {
    double x, y;
};

// --- UTILITY FUNCTIONS ---

/**
 * @brief Calculates the Euclidean distance between two points.
 */
inline double distance(const Point& p1, const Point& p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}


// --- PROBLEM & SOLUTION STRUCTURES (remaining definitions are the same) ---

struct PackageInfo {
    double weight, value;
};

struct Village {
    int id;
    Point coords;
    int population;
    int food_requirement;   // food requirement for that particular village
    int other_supplies_requirement; //Others requirement for that particular village
    double value_gained = 0;    // value gained by that village
};

struct Helicopter {
    int id;
    int home_city_id;
    double weight_capacity;
    double distance_capacity;
    double fixed_cost; // F
    double alpha;
    pair<int, int> zone_requirement = {0,0};
    double distanceTravelledByHeliCopter = 0;
};

struct ProblemData {
    double time_limit_minutes;
    double d_max;
    vector<PackageInfo> packages;
    vector<Point> cities;
    vector<Village> villages;
    vector<Helicopter> helicopters;
};

struct Drop {
    int village_id;
    int dry_food;
    int perishable_food;
    int other_supplies;
};

struct Trip {
    int dry_food_pickup;
    int perishable_food_pickup;
    int other_supplies_pickup;
    int tripOtherRequirment = 0;
    int tripFoodRequirement = 0;
    bool done = false;
    double distanceTravelledThisTrip = 0;
    vector<Drop> drops;
};

struct HelicopterPlan {
    int helicopter_id;
    vector<Trip> trips;
};


// Adding a constrcutor to State as to directly add the vilalgelist and helicopterlist to the state
// Added some other vairables to keep track of all the other dynamic variable related to the code.
struct State{
    map< int, vector<int>> zone;
    vector<Village> villageList;
    vector<Helicopter> helicopterList;
    vector<HelicopterPlan> helicopterPlan;
    double f;  // Total Value gained by every village combined
    double c;  // Total cost incurred by every helicopter combined 
    double o;  // f - c

    State(map<int, vector<int>> zones, const ProblemData& problem, const vector<HelicopterPlan> helicopter_plan) {
        zone = zones;
        helicopterList = problem.helicopters;
        helicopterPlan = helicopter_plan;
        villageList = problem.villages;

        // Initialize the requirements and values of all the villages.
        for(auto& village : villageList) {
            village.value_gained = 0;
            village.food_requirement = 9 * village.population; //each person needs 9 units of food
            village.other_supplies_requirement = village.population; // each person needs 1 unit of
        }
        
    }
};


using Solution = vector<HelicopterPlan>;

#endif // STRUCTURES_H