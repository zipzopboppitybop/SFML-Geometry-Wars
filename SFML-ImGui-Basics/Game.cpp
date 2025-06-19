#include "Game.h"

#include <iostream>

Game::Game(const std::string& config) : mText(mFont)
{
	init(config);
}

void Game::init(const std::string& config)
{
	//TODO read in config files

	if (!mFont.openFromFile("fonts/Roboto-Regular.ttf"))
	{
		std::cerr << "Failed to load font!\n";
	}

	mText.setFont(mFont);
	mText.setCharacterSize(24);
	mText.setFillColor(sf::Color::White);
	mText.setString("Score: 0");

	mWindow.create(sf::VideoMode({ 1280, 720 }), "Geometry Wars!");
	mWindow.setFramerateLimit(60);

	ImGui::SFML::Init(mWindow);

	ImGui::GetStyle().ScaleAllSizes(2.0f);
	ImGui::GetIO().FontGlobalScale = 2.0f;

	spawnPlayer();
}

std::shared_ptr<Entity> Game::player()
{
	auto& players = mEntities.getEntities("player");
	return players.front();
}

void Game::run()
{
	// TODO make it so some systems work while paused

	while (mRunning)
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

	entity->add<CTransform>(Vec2f(200.0f, 200.0f), Vec2f(1.0f, 1.0f), 0.0f);

	entity->add<CShape>(32.0f, 8, sf::Color(10,10,10), sf::Color(255, 0, 0), 4.0f);

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
	auto& transform = player()->get<CTransform>();
	transform.pos.x += transform.velocity.x;
	transform.pos.y += transform.velocity.y;
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

	player()->get<CShape>().circle.setPosition(player()->get<CTransform>().pos);

	player()->get<CTransform>().angle += 1.0f;

	player()->get<CShape>().circle.setRotation(sf::degrees(player()->get<CTransform>().angle));

	mWindow.draw(player()->get<CShape>().circle);

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
			if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
				mWindow.close();
		}
	}
}