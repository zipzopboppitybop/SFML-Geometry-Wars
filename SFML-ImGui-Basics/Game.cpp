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

		//if (temp == "Rectangle")
		//{
		//	myFileStream >> shapeName >> shapeX >> shapeY >> shapeXSpeed >> shapeYSpeed >> shapeR >> shapeG >> shapeB >> shapeWidth >> shapeHeight;
		//	Shape shape(shapeName, shapeX, shapeY, shapeXSpeed, shapeYSpeed, shapeR, shapeG, shapeB, shapeWidth, shapeHeight);
		//	shapes.push_back(shape);
		//}
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
	}
}

void Game::spawnPlayer()
{
	auto entity = mEntities.addEntity("player");

	entity->add<CTransform>(Vec2f(mWindow.getSize().x / 2, mWindow.getSize().y / 2), Vec2f(0, 0), 0.0f);

	entity->add<CShape>(mPlayerConfig.SR, mPlayerConfig.V, sf::Color(mPlayerConfig.FR, mPlayerConfig.FG, mPlayerConfig.FB), sf::Color(mPlayerConfig.OR, mPlayerConfig.OG, mPlayerConfig.OB), mPlayerConfig.OT);

	entity->add<CInput>();
}

void Game::spawnEnemy()
{
	// TODO Make enemy spawn with config and in bounds of window
	mLastEnemySpawnTime = mCurrentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
	// TODO spawn small enemies at location of destroyed enemy
	// spawn number of enemies that equal the vertices of the destroyed enemy
	// small enemy should be same color and half the size
	// small enemies are worth double points
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& target)
{
	// spawn bullet that goes towards mouse position from player position
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	// TODO use special move
}

void Game::sMovement()
{
	//TODO implement all entity movement
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
}

void Game::sEnemySpawner()
{
	// TODO implement spawning
}

void Game::sGUI()
{
	ImGui::Begin("Geometry Wars");

	ImGui::Text("Stuff Goes Here");

	ImGui::End();
}

void Game::sRender()
{
	// TODO make all entities render

	mWindow.clear();

	auto p = player();
	if (p && p->isActive())
	{
		p->get<CShape>().circle.setPosition(p->get<CTransform>().pos);
		p->get<CTransform>().angle += 1.0f;
		p->get<CShape>().circle.setRotation(sf::degrees(p->get<CTransform>().angle));
		mWindow.draw(p->get<CShape>().circle);
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
		else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->scancode == sf::Keyboard::Scancode::Space)
				player()->destroy();
		}
	}
}