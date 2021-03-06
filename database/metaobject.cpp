#include <cstdio>
#include <string>
#include "metaobject.h"
#include "interpy.h"
#include "workingmemory.h"
#include "par.h"


extern Actionary *actionary; // global actionary pointer



MetaObject::MetaObject(const char* obName, bool agent,bool obj_inst):
	instance(obj_inst),location(NULL),possessedBy(NULL)
{
	// check to see if it already exists and if so print a message
	if (actionary->isObject(std::string(obName)))
	{
		par_debug("Object already exists in MetaObject constructor %s\n",obName);
		// get the id number
		objID = actionary->getObjectID(obName);
		name = std::string(obName);
		MetaObject* rentObj=actionary->getParent(this);
		parent=rentObj;
		instance=false;
	}
	else
	{
		//If it doesn't exist in the database, then we need to add it to the actionary,
		//and if it isn't an instance, add it to the database
		objID = actionary->addObject(this, obName, agent,obj_inst);
		name = std::string(obName);
		std::string rentName = actionary->extractParentName(name);
		MetaObject* rentObj = actionary->searchByNameObj(rentName);
		if(rentObj != NULL){
			actionary->setParent(this,rentObj);
			parent=rentObj;
		}
		else
			parent=actionary->searchByIdObj(1);//This is the root, so if the instanced object doesn't have a parent,
											   //this will keep it from having funny business
	}
	//sets the default value if the object is an instance
	if(instance)
		boundingVol[0].v[0]=-1;

	// store the id in the appropriate list
	bool roomQ=actionary->isType(this,"Room");
	
	// check all agents
	if (agent && instance)
	{
		actionary->allAgents.push_back(objID);
		return;
	}
	// check type
	if(!roomQ && !instance)
	{
		actionary->allTypes.push_back(objID);
		return;
	}
	// check object instances
	if(!roomQ && instance)
	{
		actionary->allObjects.push_back(objID);
		properties=parent->getAllProperties();
		return;
	}
	// check all rooms
	if(roomQ && instance)
	{
		properties=parent->getAllProperties();
		actionary->allRooms.push_back(objID);
		return;
	}

}

///////////////////////////////////////////////////////////////////////////////
//Destroys the meta-object
///////////////////////////////////////////////////////////////////////////////
MetaObject::~MetaObject(){
	//We attach all children of the objects to the parent, all contents its container, and all possessions to its possessor
	int child_counter = 0;
	MetaObject *child=actionary->getChild(this,child_counter);
	while(child != NULL){
		child->setParent(this->parent);
		child_counter++;
		child = actionary->getChild(this, child_counter);
	}
	//Clean up the locations that are set to this
	for(std::list<MetaObject*>::iterator it =contents.begin(); it != contents.end(); it++){
		(*it)->setLocation(this->location);
	}
	//Clean up the possessions that are set to this
	for(std::list<MetaObject*>::iterator it=possessions.begin(); it != possessions.end(); it ++){
		(*it)->setPossessedBy(this->possessedBy);
	}

}
/////////////////////////////////////////
// place this object in the hierarchy
void 
MetaObject::setParent(MetaObject* aparent)
{
	parent=aparent;
	if(!instance)
		actionary->setParent(this, aparent);
}


/////////////////////////////////////////
// get the object's bounding radius
float 
MetaObject::getBoundingRadius()
{
	if(instance)
		return boundingVol[0].v[0];
	else
		return -1;
}

// set the object's bounding radius
void  
MetaObject::setBoundingRadius(float radius)
{
	if(instance)
		boundingVol[0].v[0]=radius;
}

////////////////////////////////////////////////////
//Gets a point on the bounding radius of the eight
//possible points
Vector<3>*
MetaObject::getBoundingPoint(int which){
	if(which <0 || which >8)
		return NULL;//A box has eight points

	return &boundingVol[which];
}
//////////////////////////////////////////////////
//This sets a point on the bounding box.  As long
//as the user keeps these two consistant, there 
//shouldn't be any problems
//////////////////////////////////////////////////
void
MetaObject::setBoundingPoint(Vector<3>* point, int which){
	if(which <0 || which >8 || point == NULL)
		return;//A box has eight points
	boundingVol[which].v[0]=point->v[0];
	boundingVol[which].v[1]=point->v[1];
	boundingVol[which].v[2]=point->v[2];

}

void 
MetaObject::setObjectName(std::string newName)
{
	name = newName;
	if(!instance)
		actionary->setObjectName(this, newName);
}
///////////////////////////////////////////////////////////////////////////////
//This uses the all agents vector to determine if the object in question
//is an agent
///////////////////////////////////////////////////////////////////////////////
bool 
MetaObject::isAgent()
{
	return actionary->isAgent(this);
}
///////////////////////////////////////////////////////////////////////////////
//This uses the all rooms vector to determine if the object in question
//is an agent
///////////////////////////////////////////////////////////////////////////////
bool 
MetaObject::isRoom()
{
	return actionary->isRoom(this);
}


Vector<3>*
MetaObject::getPosition()
{	
	if(instance)
		return &position;
	else
		return NULL;
}

void
MetaObject::setPosition(Vector<3>* vector)
{
	//Note: position should only be for instanced object
	position.v[0]=vector->v[0];
	position.v[1]=vector->v[1];
	position.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getVelocity()
{
	if(instance)
		return &velocity;
	else
		return NULL;
}

void
MetaObject::setVelocity(Vector<3>* vector)
{
   	velocity.v[0]=vector->v[0];
	velocity.v[1]=vector->v[1];
	velocity.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getAcceleration()
{
   	if(instance)
		return &acceleration;
	else
		return NULL;
}

void
MetaObject::setAcceleration(Vector<3>* vector)
{
	acceleration.v[0]=vector->v[0];
	acceleration.v[1]=vector->v[1];
	acceleration.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getOrientation()
{
	if(instance)
		return &orientation;
	else
		return NULL;
}

void
MetaObject::setOrientation(Vector<3>* vector)
{
	
	orientation.v[0]=vector->v[0];
	orientation.v[1]=vector->v[1];
	orientation.v[2]=vector->v[2];
}

Vector<3>*
MetaObject::getCoordinateSystem()
{
   	if(instance)
		return &coordinateSystem;
	else
		return NULL;
}

void
MetaObject::setCoordinateSystem(Vector<3>* vector){
	coordinateSystem.v[0]=vector->v[0];
	coordinateSystem.v[1]=vector->v[1];
	coordinateSystem.v[2]=vector->v[2];
}

void
MetaObject::addSite( int siteType,
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ)
{
	actionary->addGraspSite(this,  siteType, sitePosX, sitePosY, sitePosZ,
		siteOrientX, siteOrientY, siteOrientZ);
}

void
MetaObject::updateSite(int siteType,     // use -999 where values should not be altered
		float sitePosX, float sitePosY, float sitePosZ,
		float siteOrientX, float siteOrientY, float siteOrientZ)
{
	actionary->updateGraspSite(this,siteType, sitePosX, sitePosY, sitePosZ,
		siteOrientX, siteOrientY, siteOrientZ);
}

void
MetaObject::removeSite(int siteType){
	actionary->removeGraspSite(this,siteType);
}

MetaObject*
MetaObject::searchSites(char* siteName){ // return the site id or -1 if not found
	int site_type=actionary->getSiteType(siteName);
	return actionary->searchGraspSites(this, site_type);
}

MetaObject*
MetaObject::searchSites(int site_type){

	return actionary->searchGraspSites(this,site_type);
}

std::string
MetaObject::getSiteName(int siteType){
	return actionary->getGraspSiteName(siteType);
}

float
MetaObject::getSitePos(int siteType, int which){
	return actionary->getGraspSitePos(this, siteType, which);
}
Vector<3>
MetaObject::getSitePos(int siteType){
	return actionary->getGraspSitePos(this,siteType);
}
float
MetaObject::getSiteOrient(int siteType, int which){
	return actionary->getGraspSiteOrient(this, siteType, which);
}
Vector<3>
MetaObject::getSiteOrient(int siteType){
	return actionary->getGraspSiteOrient(this, siteType);
}


void 
MetaObject::setPossessedBy(MetaObject* obj)
{
	//We want to get out of the loop if we have already set the location
	if (obj == NULL || possessedBy == obj)
		return;
  if(possessedBy != obj){
	  //This will automatically remove this from the other's possession
	if(possessedBy != NULL)
		obj->removeFromPossessions(this);

	possessedBy=obj;
	possessedBy->addPossession(this);//We need to make sure these two are the same
	}
}
MetaObject* 
MetaObject::getPossessedBy()
{
	return possessedBy;
}

void		  
MetaObject::removeFromPossessions(MetaObject* obj)
{
	if(obj == NULL)
		return;
	possessions.remove(obj);
}

void 
MetaObject::addPossession(MetaObject* obj)
{
	if(!this->searchPossession(obj))
		possessions.push_back(obj);
}

void 
MetaObject::deletePossessions()
{
	possessions.clear();
}
bool
MetaObject::searchPossession(MetaObject *obj){
	if(obj == NULL)
		return false;
	for(std::list<MetaObject*>::iterator it=possessions.begin(); it != possessions.end(); it++){
		if(obj == (*it))
			return true;
	}
	return false;
}
//Quick overload for easy authoring
bool 
MetaObject::searchPossession(std::string objName)
{
	MetaObject *obj=actionary->searchByNameObj(objName);
	return this->searchPossession(obj);
}
MetaObject*
MetaObject::getPossessionOfType(MetaObject *obj){
	if(obj == NULL)
		return NULL;

	for(std::list<MetaObject*>::iterator it=possessions.begin(); it != possessions.end(); it++){
		if(actionary->isType((*it),obj))
			return (*it);
	}
	return NULL;
}
MetaObject* 
MetaObject::getPossessionOfType(std::string typeName)
{
	MetaObject* obj=actionary->searchByNameObj(typeName);
	return this->getPossessionOfType(obj);
}


/*Location functions*/
MetaObject*
MetaObject::getLocation()
{
   return location;
}
///////////////////////////////////////////////////////////////////////////////
//Keeps searching to location until it finds a room.
///////////////////////////////////////////////////////////////////////////////
MetaObject*
MetaObject::getRoomLocation(){
	if(location->isRoom())
		return location;
	//Before recursing, we need to make sure we have something to recurse to
	if(location->getLocation() == NULL)
		return NULL;

	return location->getRoomLocation();
	
}
///////////////////////////////////////////////////////////////////////////////
//Set's the location, which can be understood as the contents of the object
///////////////////////////////////////////////////////////////////////////////
void
MetaObject::setLocation(MetaObject* loc)
{
	//We want to get out of the loop if we have already set the location
	if (loc == NULL || location == loc)
		return;
  if(location != loc){
	  //This will automatically remove this from the other contents
	if(location != NULL)
		location->removeFromContents(this);

	location=loc;
	location->addContents(this);//We need to make sure these two are the same
  }
}
///////////////////////////////////////////////////////////////////////////////
//Overloaded to allow easier authoring
///////////////////////////////////////////////////////////////////////////////
void		
MetaObject::setLocation(std::string objName)
{
	MetaObject *loc=actionary->searchByNameObj(objName);
	setLocation(loc);
}


void
MetaObject::addContents(MetaObject* obj)
{	
	if(obj == NULL)
		return;
	//First, we should make sure the object isn't in the contents
	//This will probably slow it down a bit
	if(searchContents(obj))
		return;
	contents.push_back(obj);
	//We should also update the location of the object to be 
	//within the contents of the object
	obj->setLocation(this);
}
void 
MetaObject::removeFromContents(MetaObject* obj)
{
	if(obj == NULL)
		return;
	contents.remove(obj);
}


void
MetaObject::deleteContents()
{
	contents.clear();  
}

bool
MetaObject::searchContents(MetaObject* obj)
{
	if(obj == NULL)
		return false;
	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end(); it++)
		if((*it)==obj)
			return true;
		
	/*We also wish to recursively find objects that could be within
	other objects*/
	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end(); it++)
		if((*it)->searchContents(obj))
			return true;
	
	return false;

}
///////////////////////////////////////////////////////////////////////////////
//Overloaded operator for ease of authoring
///////////////////////////////////////////////////////////////////////////////
bool
MetaObject::searchContents(std::string obj_name){
	MetaObject* obj=actionary->searchByNameObj(obj_name);
	return searchContents(obj);
}
///////////////////////////////////////////////////////////////////////////////
//Search contents for types attempts to find an object within another object's
//contents.  the not_agents flag filter's out agents, so one agent won't search
//another agent's contents for an item.  This prevents stealing.
///////////////////////////////////////////////////////////////////////////////
MetaObject* 
MetaObject::searchContentsForType(MetaObject *type, bool not_agents)
{
	if(type == NULL)
		return NULL;

	MetaObject *found=NULL;
	/*We're going to recurse like we did last time*/
	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end() && (found == NULL); it++)
		if(actionary->isType((*it),type))
			found=(*it);

	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end() && (found == NULL); it++)
		if(not_agents){
			if(!(*it)->isAgent())
				found=(*it)->searchContentsForType(type,not_agents);
		}
		else
			found=(*it)->searchContentsForType(type,not_agents);
			
	return found; 
}
///////////////////////////////////////////////////////////////////////////////
//This is the overloaded search that can act using a string
//////////////////////////////////////////////////////////////////////////////
MetaObject*
MetaObject::searchContentsForType(std::string type_name,bool not_agents){
	MetaObject *type=actionary->searchByNameObj(type_name);
	return this->searchContentsForType(type,not_agents);
}
///////////////////////////////////////////////////////////////////////////////
//Changes the contents position to match a given position.  This will almost
//always be the object's position
///////////////////////////////////////////////////////////////////////////////
void
MetaObject::updateContentsPosition(Vector<3>* new_position){
	if(new_position == NULL)
		return;  //Why wouldn't they send a position

	if(contents.size() == 0)
		return; //Need some way of stopping the recursion

	for(std::list<MetaObject*>::iterator it=contents.begin(); it != contents.end(); it++){
		//Here, we should check for the object's contents, and then essentially the contents
		//of the objects within the contents 
		(*it)->setPosition(new_position);
		(*it)->updateContentsPosition(new_position);
	}
}
// property
int
MetaObject::setProperty(std::string prop_type,std::string prop_name){
	parProperty* prop=actionary->getPropertyType(prop_type);
	if(prop != NULL)
		return this->setProperty(prop_type,prop->getPropertyValueByName(prop_name));

		return -1;
}

int
MetaObject::setProperty(std::string prop_type,int prop_value){
	parProperty* prop=actionary->getPropertyType(prop_type);
	if(prop != NULL && prop_value >-1){
		properties[prop]=prop_value;
		if(!instance){
			return actionary->setProperty(this,prop_type,prop_value);
		}
		return 1;
	}
	else
		return -1;
}

void
MetaObject::removeProperty(std::string prop_type){
	parProperty *prop=actionary->getPropertyType(prop_type);
	if(prop != NULL){
		std::map<parProperty*,int>::iterator it=properties.find(prop);
		if(it != properties.end())
			properties.erase(it);
		if(!instance)
			actionary->removeProperty(this,prop_type);
	}
}


std::string
MetaObject::getPropertyName(std::string prop_type){
	return actionary->getPropertyNameByValue(prop_type,getPropertyValue(prop_type));
}

int
MetaObject::getPropertyValue(std::string prop_type){
	parProperty *prop=actionary->getPropertyType(prop_type);
	if(prop != NULL){
		std::map<parProperty*,int>::const_iterator it=properties.find(prop);
		if(it != properties.end())
			return (*it).second;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
//This basically calls the actionary's check for affordance
//and returns true if the object can be used in that position
///////////////////////////////////////////////////////////////////////////////
bool
MetaObject::searchAffordance(MetaAction *act, int which){
	if(act == NULL || which <0 || which > act->getNumObjects())
		return false;

	int all_pos=actionary->searchAffordance(act,this);
		if(all_pos==which)
			return true;

	//Breaks the recursion
	if(this->parent == NULL)
		return false;

	return this->parent->searchAffordance(act,which);
}

MetaAction*
MetaObject::searchAffordance(int position, int which){
	return actionary->searchAffordance(this,position,which);
}