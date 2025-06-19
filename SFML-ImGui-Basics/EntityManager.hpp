#pragma once

#include "Entity.hpp"
#include "map"

using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
	EntityVec mEntities;
	EntityVec mEntitiesToAdd;
	std::map<std::string, EntityVec> mEntityMap;
	size_t mTotalEntities = 0;

	void removeDeadEntities(EntityVec& vec)
	{

	}

public:

	EntityManager() = default;

	void update()
	{
		// TODO add entities from entities to add to the vector of all entities and the vector in the map with the tag as key

		removeDeadEntities(mEntities);

		for (auto& [tag, entityVec] : mEntityMap)
		{
			removeDeadEntities(entityVec);
		}
	}

	std::shared_ptr<Entity> addEntity(const std::string& tag)
	{
		auto entity = std::shared_ptr<Entity>(new Entity(mTotalEntities++, tag));

		mEntitiesToAdd.push_back(entity);

		if (mEntityMap.find(tag) == mEntityMap.end()) { mEntityMap[tag] = EntityVec(); }
		mEntityMap[tag].push_back(entity);

		return entity;
	}

	const EntityVec& getEntities()
	{
		return mEntities;
	}

	const EntityVec& getEntities(const std::string& tag)
	{
		if (mEntityMap.find(tag) == mEntityMap.end()) { mEntityMap[tag] = EntityVec(); }
		return mEntityMap[tag];
	}

	const std::map<std::string, EntityVec>& getEntityMap()
	{
		return mEntityMap;
	}
};