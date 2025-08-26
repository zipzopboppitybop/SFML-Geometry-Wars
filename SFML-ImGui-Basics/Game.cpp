#include "Game.h"

#include <iostream>

Game::Game(const std::string& config) : mText(mFont)
{
	init(config);
}

void Game::init(const std::string& config)
{
	//TODO read in config files

	// Read config file
	std::ifstream myFileStream("config.txt");
	std::string temp;

	if (!myFileStream)
	{
		std::cout << "Error: Unable to open file for reading.\n";
		return;
	}

	while (myFileStream >> temp)
	{
		if (temp == "Window")
		{
			myFileStream >> mWindowConfig.W >> mWindowConfig.H >> mWindowConfig.FL >> mWindowConfig.FS;
		}

		if (temp == "Font")
		{
			myFileStream >> mFontConfig.FF >> mFontConfig.FS >> mFontConfig.FR >> mFontConfig.FG >> mFontConfig.FB;
		}

		if (temp == "Player")
		{
			myFileStream >> mPlayerConfig.SR >> mPlayerConfig.CR >> mPlayerConfig.S >> mPlayerConfig.FR >> mPlayerConfig.FG >> mPlayerConfig.FB >> mPlayerConfig.OR >> mPlayerConfig.OG >> mPlayerConfig.OB >> mPlayerConfig.OT >> mPlayerConfig.V;
		}

		if (temp == "Enemy")
		{
			myFileStream >> mEnemyConfig.SR >> mEnemyConfig.CR >> mEnemyConfig.SMIN >> mEnemyConfig.SMAX >> mEnemyConfig.OR >> mEnemyConfig.OG >> mEnemyConfig.OB >> mEnemyConfig.OT >> mEnemyConfig.VMIN >> mEnemyConfig.VMAX >> mEnemyConfig.L >> mEnemyConfig.SI;
		}

		if (temp == "Bullet")
		{
			myFileStream >> mBulletConfig.SR >> mBulletConfig.CR >> mBulletConfig.S >> mBulletConfig.FR >> mBulletConfig.FG >> mBulletConfig.FB >> mBulletConfig.OR >> mBulletConfig.OG >> mBulletConfig.OB >> mBulletConfig.OT >> mBulletConfig.V >> mBulletConfig.L;
		}
	}


	if (!mFont.openFromFile(mFontConfig.FF))
	{
		std::cerr << "Failed to load font!\n";
	}

	mText.setFont(mFont);
	mText.setCharacterSize(mFontConfig.FS);
	mText.setFillColor(sf::Color(mFontConfig.FR, mFontConfig.FG, mFontConfig.FB) );
	mText.setString("Score: 0");

	mWindow.create(sf::VideoMode({ mWindowConfig.W, mWindowConfig.H }), "Geometry Wars!");
	mWindow.setFramerateLimit(mWindowConfig.FL);

	ImGui::SFML::Init(mWindow);

	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
	auto& players = mEntities.getEntities("player");
	if (!players.empty())
	{
		return players.front();
	}
	return nullptr;  
}

void Game::run()
{
	// TODO make it so some systems work while paused

	while (mRunning && mWindow.isOpen())
	{
		mEntities.update();

		ImGui::SFML::Update(mWindow, mDeltaClock.restart());

		sMovement();
		sUserInput();
		sEnemySpawner();
		sCollision();
		sGUI();
		sRender();

		mCurrentFrame++;

		if (player() == nullptr)
		{
			spawnPlayer();
		}
	}
}

void Game::spawnPlayer()
{
	auto entity = mEntities.addEntity("player");

	entity->add<CTransform>(Vec2f(mWindow.getSize().x / 2, mWindow.getSize().y / 2), Vec2f(0, 0), 0.0f);

	entity->add<CCollision>(mPlayerConfig.CR);

	entity->add<CShape>(mPlayerConfig.SR, mPlayerConfig.V, sf::Color(mPlayerConfig.FR, mPlayerConfig.FG, mPlayerConfig.FB), sf::Color(mPlayerConfig.OR, mPlayerConfig.OG, mPlayerConfig.OB), mPlayerConfig.OT);

	entity->add<CInput>();
}

void Game::spawnEnemy()
{
	// TODO Make enemy spawn with config and in bounds of window
	std::uniform_real_distribution<float> distX(0.0f, static_cast<float>(mWindow.getSize().x));
	std::uniform_real_distribution<float> distY(0.0f, static_cast<float>(mWindow.getSize().y));
	std::uniform_real_distribution<float> distSpeed(mEnemyConfig.SMIN, mEnemyConfig.SMAX);
	std::uniform_real_distribution<float> distVertices(mEnemyConfig.VMIN, mEnemyConfig.VMAX);
	std::uniform_real_distribution<float> distR(0.0f, 255.0f); 
	std::uniform_real_distribution<float> distG(0.0f, 255.0f);
	std::uniform_real_distribution<float> distB(0.0f, 255.0f);

	auto entity = mEntities.addEntity("enemy");

	float randomX = distX(rng);
	float randomY = distY(rng);
	float randomSpeedX = distSpeed(rng);
	float randomSpeedY = distSpeed(rng);
	float randomVertices = distVertices(rng);
	float randomR = distR(rng);
	float randomG = distG(rng);
	float randomB = distB(rng);


	entity->add<CTransform>(Vec2f(randomX, randomY), Vec2f(randomSpeedX, randomSpeedY), 0.0f);

	entity->add<CCollision>(mEnemyConfig.CR);

	entity->add<CShape>(mEnemyConfig.SR, randomVertices, sf::Color(randomR, randomG, randomB), sf::Color(mEnemyConfig.OR, mEnemyConfig.OG, mEnemyConfig.OB), mEnemyConfig.OT);

	mLastEnemySpawnTime = mCurrentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
	// TODO spawn small enemies at location of destroyed enemy
	// spawn number of enemies that equal the vertices of the destroyed enemy
	// small enemy should be same color and half the size
	// small enemies are worth double points
	std::cout << "My father was destroyed!" << std::endl;
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& target)
{
	// spawn bullet that goes towards mouse position from player position
	auto bullet = mEntities.addEntity("bullet"); 

	bullet->add<CTransform>(Vec2f(entity->get<CTransform>().pos.x, entity->get<CTransform>().pos.y), Vec2f(5, 0), 0.0f);

	bullet->add<CCollision>(mBulletConfig.CR);

	bullet->add<CShape>(mBulletConfig.SR, mBulletConfig.V, sf::Color(mBulletConfig.FR, mBulletConfig.FG, mBulletConfig.FB), sf::Color(mBulletConfig.OR, mBulletConfig.OG, mBulletConfig.OB), mBulletConfig.OT);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	// TODO use special move
}

void Game::sMovement()
{
	//TODO implement all entity movement
	if (!player())
	{
		return;
	}

	auto& playerInput = player()->get<CInput>();
	auto& playerMovement = player()->get<CTransform>();
	auto& playerCollisionRadius = player()->get<CCollision>().radius;
	auto windowSize = mWindow.getSize();

	playerMovement.velocity.x = 0;
	playerMovement.velocity.y = 0;

	if (playerInput.right == true && playerMovement.pos.x + playerCollisionRadius < windowSize.x)
	{
		playerMovement.velocity.x = mPlayerConfig.S;
	}

	if (playerInput.left == true && playerMovement.pos.x - playerCollisionRadius > 0)
	{
		playerMovement.velocity.x = -mPlayerConfig.S;
	}

	if (playerInput.up == true && playerMovement.pos.y - playerCollisionRadius > 0)
	{
		playerMovement.velocity.y = -mPlayerConfig.S;
	}

	if (playerInput.down == true && playerMovement.pos.y + playerCollisionRadius < windowSize.y)
	{
		playerMovement.velocity.y = mPlayerConfig.S;
	}
	

	for (auto& entity : mEntities.getEntities())
	{
		if (entity->isActive())
		{
			auto& transform = entity->get<CTransform>();
			transform.pos.x += transform.velocity.x;
			transform.pos.y += transform.velocity.y;
		}
	}
}

void Game::sLifespan()
{
	//TODO implement lifespan for all entities
	// if entity has lifespand reduce it by one
	// if no lifespan destroy it
}

void Game::sCollision()
{
	// Implement all collision
	if (!player())
	{
		return;
	}

	auto windowSize = mWindow.getSize();
	auto playerTransform = player()->get<CTransform>().pos;
	auto playerCollsionRadius = player()->get<CCollision>().radius;

	for (auto& bullet : mEntities.getEntities("bullet"))
	{
		for (auto& enemy : mEntities.getEntities("enemy"))
		{
			float dist = bullet->get<CTransform>().pos.dist(enemy->get<CTransform>().pos);

			if (dist < bullet->get<CCollision>().radius + enemy->get<CCollision>().radius)
			{
				bullet->destroy();
				spawnSmallEnemies(enemy);
				enemy->destroy();
			}
		}
	}

	for (auto& entity : mEntities.getEntities("enemy"))
	{
		auto& transform = entity->get<CTransform>();
		auto collisionRadius = entity->get<CCollision>().radius;

		if (transform.pos.x - collisionRadius < 0 || transform.pos.x + collisionRadius > windowSize.x)
		{
			transform.velocity.x *= -1;
		}

		if (transform.pos.y - collisionRadius < 0 || transform.pos.y + collisionRadius > windowSize.y)
		{
			transform.velocity.y *= -1;
		}

		float dist = playerTransform.dist(transform.pos);

		if (dist < collisionRadius + playerCollsionRadius)
		{
			entity->destroy();
			player()->destroy();
		}
	}
}

void Game::sEnemySpawner()
{
	if (mCurrentFrame - mLastEnemySpawnTime >= mEnemyConfig.SI)
	{
		spawnEnemy();
	}
}

void Game::sGUI()
{
	ImGui::Begin("Geometry Wars");

	ImGui::Text("Stuff Goes Here");

	ImGui::End();
}

void Game::sRender()
{
	mWindow.clear();

	for (auto& entity : mEntities.getEntities())
	{
		if (!entity->isActive()) continue;

		auto& transform = entity->get<CTransform>();
		auto& shape = entity->get<CShape>();

		shape.circle.setPosition(transform.pos);
		transform.angle += 1.0f;
		shape.circle.setRotation(sf::degrees(transform.angle));

		mWindow.draw(shape.circle);
	}

	ImGui::SFML::Render(mWindow);

	mWindow.display();
}

void Game::sUserInput()
{
	while (const std::optional event = mWindow.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			mWindow.close();
		}

		if (!player())
		{
			return;
		}
		else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->scancode == sf::Keyboard::Scancode::D)
			{
				player()->get<CInput>().right = true;
			}

			if (keyPressed->scancode == sf::Keyboard::Scancode::A)
			{
				player()->get<CInput>().left = true;
			}

			if (keyPressed->scancode == sf::Keyboard::Scancode::W)
			{
				player()->get<CInput>().up = true;
			}

			if (keyPressed->scancode == sf::Keyboard::Scancode::S)
			{
				player()->get<CInput>().down = true;
			}

			if (keyPressed->scancode == sf::Keyboard::Scancode::Space)
			{
				Vec2f mousePos(sf::Mouse::getPosition(mWindow).x, sf::Mouse::getPosition(mWindow).y);
				spawnBullet(player(), mousePos);
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			if (keyReleased->scancode == sf::Keyboard::Scancode::D)
			{
				player()->get<CInput>().right = false;
			}

			if (keyReleased->scancode == sf::Keyboard::Scancode::A)
			{
				player()->get<CInput>().left = false;
			}

			if (keyReleased->scancode == sf::Keyboard::Scancode::W)
			{
				player()->get<CInput>().up = false;
			}

			if (keyReleased->scancode == sf::Keyboard::Scancode::S)
			{
				player()->get<CInput>().down = false;
			}
		}
	}
}