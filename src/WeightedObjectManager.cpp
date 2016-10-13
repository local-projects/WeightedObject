//
//  WeightedObjectManager.cpp
//  BaseApp
//
//  Created by Oriol Ferrer Mesià on 04/09/14.
//
//

#include "WeightedObjectManager.h"

//////////////////////////////////////////////////////////////////////////////////

Weightable::Weightable(){
	weightGain = 2.5f;
	minWeight = 0.0f;
	maxWeight = 1.0f;
	manager = NULL;
}

void Weightable::setupWeightData(float minWeight_, float maxWeight_, float weightGain_){
	weightGain = weightGain_; minWeight = minWeight_; maxWeight = maxWeight_;
}


float Weightable::getMappedWeight(){ //remap the backend weigjht
	float ret = ofMap(weight, minWeight, maxWeight, -1.0f, 1.0f, true);
	return ret;
}

float Weightable::multiplicableWeight(){
	float res;
	float w = getMappedWeight();
	if (w > 0.0f){
		res = 1.0f + w * (weightGain - 1.0f);
	}else{
		w *= -1.0f; //remap weight to [0..1]; when zero, res =1; when 1, res = 1/mult
		res = ofMap(w, 0.0f, 1.0f, 1.0f, 1.0f / weightGain, true);
	}
	return res;
}


void Weightable::setWeightRange(float min, float max){
	minWeight = min;
	maxWeight = max;
};

int Weightable::getNumAppearances(){
	int r = 0;
	if(manager){
		r = manager->getNumAppearances(this);
	}
	return r;
}



//// THIS IS VERY RELEVANT TO ON SCREEN WEIGHT BALANCING! /////////////////////

bool WeightedPairCompare::operator()( pair< Weightable*, int> a, pair< Weightable*, int> b) const{

	Weightable* ea = a.first;
	Weightable* eb = b.first;
	float wa = ea->multiplicableWeight();
	float wb = eb->multiplicableWeight();
	float weightedCountA = a.second / (wa);
	float weightedCountB = b.second / (wb);
	if(weightedCountA == weightedCountB) return ea < eb; //if values are equal, sorty by pointer address
	else return weightedCountA < weightedCountB;
}

//////////////////////////////////////////////////////////////////////////////////


void WeightedObjectManager::setup(vector<Weightable*> allObjects){

	std::random_shuffle(allObjects.begin(), allObjects.end());

	for(int i = 0; i < allObjects.size(); i++){
		screenObjectCounter[allObjects[i]] = 0;
		shownObjects.insert(make_pair(allObjects[i], 1));
		allObjects[i]->manager = this;
	}
	//ofLogNotice("WeightedObjectManager") << "shownObjects: " << shownObjects.size();
	onScreenObjects.clear();
	allWeightedObjects = allObjects;
}


void WeightedObjectManager::addExtraObjects(vector<Weightable*> newObjects, bool resetCounters){

	if(newObjects.size() == 0) return;
	
	std::random_shuffle(newObjects.begin(), newObjects.end());

	WeightedObjectsSet newSet;
	//when adding new objects, we reset all counters!
	for(auto it : shownObjects){
		if(resetCounters){
			newSet.insert(make_pair(it.first, 1));
		}else{
			newSet.insert(make_pair(it.first, (int)ofRandom(1, 3)));
		}
	}

	for(auto & it : screenObjectCounter){
		it.second = 0;
	}

	for(int i = 0; i < newObjects.size(); i++){
		auto it = find(allWeightedObjects.begin(), allWeightedObjects.end(), newObjects[i]);
		if(it == allWeightedObjects.end()){
			screenObjectCounter[newObjects[i]] = 0;
			newSet.insert(make_pair(newObjects[i], 1));
			newObjects[i]->manager = this;
			allWeightedObjects.push_back(newObjects[i]);
		}else{
			ofLogError("WeightedObjectManager") << "Trying to add weightable object that already exists! ignoring object!";
		}
	}
	shownObjects = newSet; //overwrite the set, reset all counters

}


void WeightedObjectManager::objectEnteredScreen(Weightable* o){

	if (find(allWeightedObjects.begin(), allWeightedObjects.end(), o) != allWeightedObjects.end()){
		if(!objectIsOnScreen(o)){
			onScreenObjects.push_back(o);
		}

		map<Weightable*, int>::iterator it;
		it = screenObjectCounter.find(o);
		it->second = it->second + 1;

		offsetCounterOnSet(o, shownObjects, 1); //inc that object's screen counter
	}else{
		ofLogError("WeightedObjectManager") << "this object is not weighted";
	}
}


void WeightedObjectManager::objectExitedScreen(Weightable* o){

	if (find(allWeightedObjects.begin(), allWeightedObjects.end(), o) != allWeightedObjects.end()){
		map<Weightable*, int>::iterator it;
		it = screenObjectCounter.find(o);
		if (it != screenObjectCounter.end()){
			it->second = it->second  - 1;
			if(it->second == 0){
				vector<Weightable*>::iterator it2 = find(onScreenObjects.begin(), onScreenObjects.end(), o);
				onScreenObjects.erase(it2);
			}
			if(it->second < 0){
				it->second = 0;
				ofLogFatalError("WeightedObjectManager") << "negative object count";
			}
		}
	}else{
		ofLogError("WeightedObjectManager") << "this object is not weighted";
	}
}

bool WeightedObjectManager::objectExists(Weightable* obj){
	return (find(allWeightedObjects.begin(), allWeightedObjects.end(), obj) != allWeightedObjects.end());
}


Weightable* WeightedObjectManager::getNextObject(bool itsOkIfItsOnScreen){

	WeightedObjectsSet::iterator it;
	Weightable* myResult = NULL;

	//this set we walk is ordered from less often shown entries across the day to more shown
	for(it = shownObjects.begin(); it != shownObjects.end(); it++){

		Weightable* obj = it->first;
		bool onScreen = objectIsOnScreen(obj);

		if (!onScreen || itsOkIfItsOnScreen){
			pair< Weightable*, int> p = *it;
			Weightable* candidate = p.first;
			myResult = candidate;
			break;
		}
	}
	if (myResult == NULL){
		int index = floor(ofRandom(0, allWeightedObjects.size()));

		if(!allWeightedObjects.size()){
			ofLogWarning("WeightedObjectManager") << "no allWeightedObjects" ;
			myResult = NULL;
        }else{
			myResult = allWeightedObjects[index];
			ofLogWarning("WeightedObjectManager") << "not enough different objects to show on screen, showing random object" ;
        }
	}
	return myResult;
}


Weightable* WeightedObjectManager::getTrulyRandomObject(){
    Weightable* myResult = NULL;
    int index = floor(ofRandom(0, allWeightedObjects.size()));
    myResult = allWeightedObjects[index];
    
    return myResult;
}


bool WeightedObjectManager::objectIsOnScreen(Weightable* e){
	return ( find(onScreenObjects.begin(), onScreenObjects.end(), e) != onScreenObjects.end() );
}


int WeightedObjectManager::getNumAppearances(Weightable* o){
	WeightedObjectsSet::iterator it;
	it = find_if( shownObjects.begin(), shownObjects.end(), WeightedPairCompare(o) );
	bool existsInSet = (it != shownObjects.end());

	if (existsInSet){ //update the set, remove item, inc counter, add again
		pair< Weightable*, int> p = *it;
		return p.second;
	}
	return 0;
}


void WeightedObjectManager::drawDebug(int x, int y, int limit){

	y += 10;
	string out[4]; //3 beign the text lines

	out[0] = "ID: "; //entry ID
	out[1] = " c: "; //counter
	out[2] = " w: "; //content weight
	out[3] = "wc: "; //counter with weights

	WeightedObjectsSet::iterator it = shownObjects.begin();
	WeightedObjectsSet * mySet = (WeightedObjectsSet*) &shownObjects;

	//walk all
	int totalC = 0;
	if(limit == 0) limit = shownObjects.size();

	int c = 0;
	for(it = mySet->begin(); it != mySet->end(); it++){

		Weightable* obj = it->first;

		if(obj){

			int counter = it->second;

			//add unique ID
			out[0] += "[" + obj->getWeightID() + "] " ;

			//add on screen count
			char aux[22];
			sprintf(aux, "%03d", counter );
			totalC += counter;
			out[1] += "[" + string(aux) + "] " ;

			//add on screen weight
			float www = 100 * obj->getMappedWeight();
			int ww = ofClamp(int(www), -99, 99);
			sprintf(aux, "%+03d", ww); //trim float to 3 chars
			out[2] += "[" + string(aux) + "] " ;

			//add
			sprintf(aux, "%03d", (int)(counter / obj->multiplicableWeight()) );
			out[3] += "[" + string(aux) + "] " ;
		}
		c++;
		if(c > limit) break;
	}

	int DEBUG_LINE_HEIGHT = 16;
	ofColor debugBgColor = ofColor::black;
	ofDrawBitmapStringHighlight("## OnScreen Total Counters + weights ##############",
								x, y - DEBUG_LINE_HEIGHT , debugBgColor, ofColor::white);

	ofDrawBitmapStringHighlight(out[0], x, y, debugBgColor, ofColor::yellow);
	ofDrawBitmapStringHighlight(out[1], x, y + DEBUG_LINE_HEIGHT * 1, debugBgColor, ofColor::yellow);
	ofDrawBitmapStringHighlight(out[2], x, y + DEBUG_LINE_HEIGHT * 2, debugBgColor, ofColor::yellow);
	ofDrawBitmapStringHighlight(out[3], x, y + DEBUG_LINE_HEIGHT * 3, debugBgColor, ofColor::yellow);

	ofDrawBitmapStringHighlight("## OnScreen Objects ##############",
								x, y +  DEBUG_LINE_HEIGHT * 4.5, debugBgColor, ofColor::white);

	string screenObjs;
	for(int i = 0; i < onScreenObjects.size(); i++){
		screenObjs += "[" + onScreenObjects[i]->getWeightID() + "] ";
	}
	ofDrawBitmapStringHighlight(screenObjs, x, y + DEBUG_LINE_HEIGHT * 5.5, debugBgColor, ofColor::green);

}




void WeightedObjectManager::offsetCounterOnSet( Weightable* o, WeightedObjectsSet & set, int offset ){

	WeightedObjectsSet::iterator it;
	it = find_if( set.begin(), set.end(), WeightedPairCompare(o) );
	bool existsInSet = (it != set.end());

	if (existsInSet){ //update the set, remove item, inc counter, add again
		pair< Weightable*, int> p = *it;
		set.erase(it);
		p.second = p.second + offset;
		set.insert(p);
	}
}

void WeightedObjectManager::resetCounterOnSet( Weightable* o, WeightedObjectsSet & set, int value ){

	WeightedObjectsSet::iterator it;
	it = find_if( set.begin(), set.end(), WeightedPairCompare(o) );
	bool existsInSet = (it != set.end());

	if (existsInSet){ //update the set, remove item, reset value, add again
		pair< Weightable*, int> p = *it;
		set.erase(it);
		p.second = value;
		set.insert(p);
	}
}
