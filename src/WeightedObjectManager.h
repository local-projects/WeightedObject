//
//  WeightedObjectManager.h
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 04/09/14.
//
//

#ifndef __BaseApp__Weights__
#define __BaseApp__Weights__


#include "ofMain.h"
#include "CH_Constants.h"


float multiplicableWeight(float weight);

class WeightedObjectManager;

//// Your object MUST subclass this! ///////////
class Weightable{

	friend class WeightedObjectManager;
	friend class WeightedPairCompare;

public:

	Weightable();

	//TODO should this be static? does it make sense for objects to have different weight gains / range?
	void setupWeightData(float minWeight_, float maxWeight_, float weightGain_);

	void setWeight(float w){weight = w;}
	float getWeight(){return weight;}
	virtual string getWeightID() = 0; //must implement this! must return a 3-char string! for debug purposes


	int getNumAppearances();

	//should be > 1.0!
	//if one object has a min weight, and another one has max weight
	//how many times more should one show up compared to the other one?
	void setWeightGain(float g){ weightGain = g; }

	//set it up at least once! otherwise defaults to weights ranging [0.0 .. 1.0]
	void setWeightRange(float min, float max);

protected:

	float weight;

private:

	float getMappedWeight();
	float multiplicableWeight();

	float minWeight;
	float maxWeight;
	float weightGain; 	//so the heighest weight will be X times as visible as the lowest;
						//if set to 1.0, weights are useless.
						//this is kind of a gamma/contrast curve for weights!

	WeightedObjectManager * manager; //who is managing me?
};


//// SORTING HELPERS //////////////////////////////////

class WeightedPairCompare{
public:

	WeightedPairCompare(){_e = NULL;}
	WeightedPairCompare(Weightable* const& e){_e = e;}

	bool operator()( pair <Weightable*, int> a, pair< Weightable*, int> b) const;
	bool operator () (pair<Weightable*, int> const& p){
		return (p.first == _e);
    }
private:
	Weightable* _e;
};

typedef set< pair<Weightable*, int>, WeightedPairCompare> WeightedObjectsSet; /*simplify defintion*/


//// MAIN CLASS //////////////////////////////////


class WeightedObjectManager{

public:

	//set up with the list of all the objects in the app
	void setup(vector<Weightable*> allObjects);

	//in case we get new objects on the fly / liveUpdate
	void addExtraObjects(vector<Weightable*> newObjects);

	//let me know what's going on with the objects
	void objectEnteredScreen(Weightable*);
	void objectExitedScreen(Weightable*);
	int getNumAppearances(Weightable*);

	bool objectExists(Weightable*);

	Weightable* getTrulyRandomObject();

	//ask me what object should come on screen next
	Weightable* getNextObject(bool itsOkIfItsOnScreen = false);
	//retrieve an obj according to their weight;
	//this makes sure your objects show time is balanced as their weight defines.

	void drawDebug(int x, int y, int limit = 0);

	int getNumObjects(){ return allWeightedObjects.size();}


private:

	vector<Weightable*>		onScreenObjects; 		//quick access to objects on screen right now
	map<Weightable*, int>	screenObjectCounter; 	//counters for objects on screen now;
	WeightedObjectsSet		shownObjects;			//count objects that have been on screen over time

	vector<Weightable*>		allWeightedObjects;

	bool objectIsOnScreen(Weightable*);

	void offsetCounterOnSet( Weightable* o, WeightedObjectsSet & set, int offset);
	void resetCounterOnSet( Weightable* o, WeightedObjectsSet & set, int value);

};


#endif /* defined(__BaseApp__Weights__) */
