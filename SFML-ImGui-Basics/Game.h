#pragma once

#include "Entity.hpp"
#include "EntityManager.hpp"

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game
{
	sf::RenderWindow mWindow;
	EntityManager mEntities;
	sf::Font mFont;
	sf::Text mText;
	PlayerConfig mPlayerConfig;
	EnemyConfig mEnemyConfig;
	BulletConfig mBulletConfig;
	sf::Clock mDeltaClock;
	int mScore = 0;
	int mCurrentFrame = 0;
	int mLastEnemySpawnTime = 0;
	bool mPaused = false;
	bool mRunning = true;

	void init(const std::string& config);
	void setPaused(bool paused);

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sGUI();
	void sEnemySpawner();
	void sCollision();


	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity);
	
	std::shared_ptr<Entity> player();

public:
	Game(const std::string& config);

	void run();
};

