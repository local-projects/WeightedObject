```
#include "WeightedObjectManager.h"

//This will handle object occurrence with weights, giving you weighted appearances of objects.

//you have an array of objects
vector<Weightable*> allObjects;

//you need to setup their weight range and actual weight

float minObjWeight = 0.0f;
float maxObjWeight = 1.0f;
float weightGain = 10.0f; //how many more times objects with 100% more gain will show up

for(int i = 0; i < allObjects.size(); i++){
	allObjects[i]->setupWeightData(minObjWeight, maxObjWeight, weightGain);
	allObjects[i]->setWeight( random(0,1) );
}

//then you create a manager for these objects

wom = new WeightedObjectManager();
wom->setup(allObjects);


// and now you can ask for a new object
//regardless if it's already on screen
	Weightable* nextObject = wom->getNextObject(true)

//or only from the ones that aren't on screen now
	Weightable* nextObject = wom->getNextObject(false)

//or a truly random object
Weightable* nextObject = wom->getTrulyRandomObject()


//You must keep the manager informed when objects appear and disappear from screen

void objectEnteredScreen(Weightable*);
void objectExitedScreen(Weightable*);


```