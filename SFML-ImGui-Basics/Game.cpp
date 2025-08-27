#include "Game.h"

#include <iostream>

Game::Game(const std::string& config) : mText(mFont)
{
	init(config);
}

void Game::init(const std::string& config)
{
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

void Game::setPaused(bool paused)
{
	mPaused = paused;
}

void Game::run()
{
	while (mRunning && mWindow.isOpen())
	{
		mEntities.update();

		ImGui::SFML::Update(mWindow, mDeltaClock.restart());

		if (!mPaused)
		{
			if (mMovement)
			{
				sMovement();
			}
			if (mLifespan)
			{
				sLifespan();
			}
			if (mCollision)
			{
				sCollision();
			}
			if (mSpawning)
			{
				sEnemySpawner();
			}
			mCurrentFrame++;
		}

		sGUI();
		sRender();
		sUserInput();

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
	int randomVertices = distVertices(rng);
	float randomR = distR(rng);
	float randomG = distG(rng);
	float randomB = distB(rng);


	entity->add<CTransform>(Vec2f(randomX, randomY), Vec2f(randomSpeedX, randomSpeedY), 0.0f);

	entity->add<CCollision>(mEnemyConfig.CR);

	entity->add<CShape>(mEnemyConfig.SR, randomVertices, sf::Color(randomR, randomG, randomB), sf::Color(mEnemyConfig.OR, mEnemyConfig.OG, mEnemyConfig.OB), mEnemyConfig.OT);

	entity->add<CScore>(randomVertices * 100);

	mLastEnemySpawnTime = mCurrentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
	auto entityShape = entity->get<CShape>().circle;

	
	for (size_t i = 0; i < entityShape.getPointCount(); i++)
	{
		auto smallEnemy = mEntities.addEntity("enemy");
		auto entityPos = entity->get<CTransform>().pos;
		float angle = (360.0f / entityShape.getPointCount()) * i;
		float rad = angle * (3.14159265358979323846 / 180.0f);
		Vec2f dir(std::cos(rad), std::sin(rad));
		Vec2f velocity = dir * 2.0f;

		smallEnemy->add<CTransform>(entityPos, velocity, angle);

		smallEnemy->add<CCollision>(mEnemyConfig.CR/ 2);

		smallEnemy->add<CShape>(mEnemyConfig.SR / 2, entityShape.getPointCount(), sf::Color(entityShape.getFillColor()), sf::Color(entityShape.getOutlineColor()), entityShape.getOutlineThickness());

		smallEnemy->add<CLifespan>(mEnemyConfig.L);

		smallEnemy->add<CScore>(entityShape.getPointCount() * 200);
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2f& target)
{
	auto bullet = mEntities.addEntity("bullet"); 
	Vec2f entityTransform(entity->get<CTransform>().pos.x, entity->get<CTransform>().pos.y);

	float angle = target.angle(entityTransform);
	Vec2 normalize = target.normalize(entityTransform);

	bullet->add<CTransform>(Vec2f(entity->get<CTransform>().pos.x, entity->get<CTransform>().pos.y), Vec2f(mBulletConfig.S * normalize.x, mBulletConfig.S * normalize.y), angle);

	bullet->add<CCollision>(mBulletConfig.CR);

	bullet->add<CShape>(mBulletConfig.SR, mBulletConfig.V, sf::Color(mBulletConfig.FR, mBulletConfig.FG, mBulletConfig.FB), sf::Color(mBulletConfig.OR, mBulletConfig.OG, mBulletConfig.OB), mBulletConfig.OT);

	bullet->add<CLifespan>(mBulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
	// TODO use special move
}

void Game::sMovement()
{
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
	for (auto& entity : mEntities.getEntities())
	{
		if (entity->has<CLifespan>())
		{
			auto& entityLifespan = entity->get<CLifespan>();

			entityLifespan.remaining--;

			float alpha = static_cast<float>(entityLifespan.remaining) / entityLifespan.lifespan;

			sf::Color color = entity->get<CShape>().circle.getFillColor();
			color.a = 255 * alpha;
			entity->get<CShape>().circle.setFillColor(color);
			entity->get<CShape>().circle.setOutlineColor(color);

			if (entityLifespan.remaining <= 0)
			{
				entity->destroy();
			}
		}
	}
}

void Game::sCollision()
{
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

				if (enemy->get<CCollision>().radius == mEnemyConfig.CR)
				{
					spawnSmallEnemies(enemy);
				}

				mScore += enemy->get<CScore>().score;

				std::ostringstream oss;
				oss << "Score: " << mScore;
				mText.setString(oss.str());

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
			if (entity->get<CCollision>().radius == mEnemyConfig.CR)
			{
				spawnSmallEnemies(entity);
			}

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
	if (mShow_imgui)
	{
		ImGui::Begin("Geometry Wars");

		if (ImGui::BeginTabBar("MyTabBar"))
		{
			if (ImGui::BeginTabItem("Systems"))
			{
				ImGui::Checkbox("Movement", &mMovement);
				ImGui::Checkbox("Lifespan", &mLifespan);
				ImGui::Checkbox("Collision", &mCollision);
				ImGui::Checkbox("Spawning", &mSpawning);
				ImGui::SliderInt("Spawn", &mEnemyConfig.SI, 2, 200);
				if(ImGui::Button("Manual Spawn"))
				{
					spawnEnemy();
				}
				ImGui::Checkbox("GUI", &mShow_imgui);
				ImGui::Checkbox("Rendering", &mRendering);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Entities"))
			{
				if (ImGui::CollapsingHeader("Entities"))
				{
					auto bullets = mEntities.getEntities("bullet");
					auto enemies = mEntities.getEntities("enemy");
					ImGui::Indent();
					if (ImGui::TreeNode("Bullets"))
					{
						for (auto& bullet : bullets)
						{
							auto bulletColor = bullet->get<CShape>().circle.getFillColor();
							auto bulletPos = bullet->get<CTransform>().pos;
							int id = bullet->id();
							const std::string& tag = bullet->tag();
							float x = bulletPos.x, y = bulletPos.y;
							ImGui::PushID(id);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(bulletColor.r, bulletColor.g, bulletColor.b, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(bulletColor.r, bulletColor.g, bulletColor.b, 0.8f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(bulletColor.r, bulletColor.g, bulletColor.b, 0.6f));
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));

							if (ImGui::Button("D", ImVec2(36, 36)))
							{
								bullet->destroy();
							}

							ImGui::PopStyleColor(4);
							ImGui::SameLine();
							ImGui::Text("%d %s (%.0f, %.0f)", id, tag.c_str(), x, y);
							ImGui::PopID();
						}

						ImGui::TreePop();
					}
					if (ImGui::TreeNode("Enemies"))
					{
						for (auto& enemy : enemies)
						{
							if (enemy->get<CCollision>().radius == mEnemyConfig.CR)
							{
								auto enemyColor = enemy->get<CShape>().circle.getFillColor();
								auto enemyPos = enemy->get<CTransform>().pos;
								int id = enemy->id();
								const std::string& tag = enemy->tag();
								float x = enemyPos.x, y = enemyPos.y;
								ImGui::PushID(id);
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(enemyColor.r / 255.0f, enemyColor.g / 255.0f, enemyColor.b / 255.0f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(enemyColor.r / 255.0f, enemyColor.g / 255.0f, enemyColor.b / 255.0f, 0.8f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(enemyColor.r / 255.0f, enemyColor.g / 255.0f, enemyColor.b / 255.0f, 0.6f));
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

								if (ImGui::Button("D", ImVec2(36, 36)))
								{
									enemy->destroy();
								}

								ImGui::PopStyleColor(4);
								ImGui::SameLine();
								ImGui::Text("%d %s (%.0f, %.0f)", id, tag.c_str(), x, y);
								ImGui::PopID();
							}
						}

						ImGui::TreePop();
					}
					if (ImGui::TreeNode("Player"))
					{
						if (player())
						{
							auto playerColor = player()->get<CShape>().circle.getOutlineColor();
							auto playerPos = player()->get<CTransform>().pos;
							int id = player()->id();
							const std::string& tag = player()->tag();
							float x = playerPos.x, y = playerPos.y;

							ImGui::PushID(id);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(playerColor.r / 255.0f, playerColor.g / 255.0f, playerColor.b / 255.0f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(playerColor.r / 255.0f, playerColor.g / 255.0f, playerColor.b / 255.0f, 0.8f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(playerColor.r / 255.0f, playerColor.g / 255.0f, playerColor.b / 255.0f, 0.6f));
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

							if (ImGui::Button("D", ImVec2(36, 36)))
							{
								player()->destroy();
							}

							ImGui::PopStyleColor(4);
							ImGui::SameLine();
							ImGui::Text("%d %s (%.0f, %.0f)", id, tag.c_str(), x, y);
							ImGui::PopID();
						}

						ImGui::TreePop();
					}
					if (ImGui::TreeNode("Small Enemies"))
					{
						for (auto& enemy : enemies)
						{
							if (enemy->get<CCollision>().radius != mEnemyConfig.CR)
							{
								auto enemyColor = enemy->get<CShape>().circle.getFillColor();
								auto enemyPos = enemy->get<CTransform>().pos;
								int id = enemy->id();
								const std::string& tag = enemy->tag();
								float x = enemyPos.x, y = enemyPos.y;
								ImGui::PushID(id);
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(enemyColor.r / 255.0f, enemyColor.g / 255.0f, enemyColor.b / 255.0f, 1.0f));
								ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(enemyColor.r / 255.0f, enemyColor.g / 255.0f, enemyColor.b / 255.0f, 0.8f));
								ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(enemyColor.r / 255.0f, enemyColor.g / 255.0f, enemyColor.b / 255.0f, 0.6f));
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

								if (ImGui::Button("D", ImVec2(36, 36)))
								{
									enemy->destroy();
								}

								ImGui::PopStyleColor(4);
								ImGui::SameLine();
								ImGui::Text("%d %s (%.0f, %.0f)", id, tag.c_str(), x, y);
								ImGui::PopID();
							}
						}

						ImGui::TreePop();
					}
					ImGui::Unindent();
				}
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}
}

void Game::sRender()
{
	mWindow.clear();

	if (mRendering)
	{
		for (auto& entity : mEntities.getEntities())
		{
			auto& transform = entity->get<CTransform>();
			auto& shape = entity->get<CShape>();

			shape.circle.setPosition(transform.pos);
			transform.angle += 1.0f;
			shape.circle.setRotation(sf::degrees(transform.angle));

			mWindow.draw(shape.circle);
		}

		mWindow.draw(mText);
	}

	ImGui::SFML::Render(mWindow);

	mWindow.display();
}

void Game::sUserInput()
{
	while (const std::optional event = mWindow.pollEvent())
	{
		ImGui::SFML::ProcessEvent(mWindow,*event);

		if (event->is<sf::Event::Closed>())
		{
			mWindow.close();
		}

		if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)
		{
			continue;
		}
			
		if (!player())
		{
			return;
		}

		auto& playerInput = player()->get<CInput>();

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			switch (keyPressed->scancode)
			{
			case sf::Keyboard::Scancode::D:
				playerInput.right = true;
				break;
			case sf::Keyboard::Scancode::A:
				playerInput.left = true;
				break;
			case sf::Keyboard::Scancode::W:
				playerInput.up = true;
				break;
			case sf::Keyboard::Scancode::S:
				playerInput.down = true;
				break;
			case sf::Keyboard::Scancode::Space:
				setPaused(!mPaused);
				break;
			case sf::Keyboard::Scancode::Grave:
				mShow_imgui = !mShow_imgui;
				break;
			default:
				break;
			}
		}
		else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
		{
			switch (keyReleased->scancode)
			{
			case sf::Keyboard::Scancode::D:
				playerInput.right = false;
				break;
			case sf::Keyboard::Scancode::A:
				playerInput.left = false;
				break;
			case sf::Keyboard::Scancode::W:
				playerInput.up = false;
				break;
			case sf::Keyboard::Scancode::S:
				playerInput.down = false;
				break;
			default:
				break;
			}
		}
		else if (auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouseClick->button == sf::Mouse::Button::Left)
			{
				spawnBullet(player(), { static_cast<float>(mouseClick->position.x), static_cast<float>(mouseClick->position.y) });
			}
		}
	}
}