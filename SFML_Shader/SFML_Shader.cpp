#include "pch.h"
// SFML libraries
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
using namespace sf;
float rateX = 843.0f / 1920.0f;
float rateY = 843.0f / 1080.0f;
int ts = 54; //tile size;
int totalScore = 0;
int highestScore = 0;

struct piece
{
	int x, y, col, row, kind, match, alpha;
	piece() { match = 0; alpha = 255; }
} grid[10][10];

void swap(piece p1, piece p2)
{
	std::swap(p1.col, p2.col);
	std::swap(p1.row, p2.row);

	grid[p1.row][p1.col] = p1;
	grid[p2.row][p2.col] = p2;
}


int main()
{
	// _____________________
	// ::: Create window :::

	// Create a window with the same pixel depth as the desktop
	sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
	ts = rateX * desktopMode.width / 8;
	float offsetX = desktopMode.width*(1 - rateX) / 2;
	float offsetY = desktopMode.height*(1 - rateY) / 2;
	float textOffsetX = desktopMode.width*rateX + offsetX*1.5;
	float testOffsetY = desktopMode.height/2;
	Vector2i offset(offsetX, offsetY);
	//std::cout << "rateX" << ts << " " << desktopMode.width << " " << offsetX << " " << offsetY;


	//RenderWindow window(VideoMode(740, 480), "SFML Bejeweled");
	sf::RenderWindow window(sf::VideoMode(desktopMode.width,
		desktopMode.height,
		desktopMode.bitsPerPixel),
		"SFML Bejeweled"
		, sf::Style::Fullscreen);
	window.setFramerateLimit(60);

	// Enable vertical sync. (vsync)
	//window.setVerticalSyncEnabled(true);


	Texture t1, t2;
	t1.loadFromFile("images/wallpaper.png");
	t2.loadFromFile("images/game_sprite.png");

	Sprite background(t1), gems(t2);

	for (int i = 1; i <= 8; i++)
		for (int j = 1; j <= 8; j++)
		{
			grid[i][j].kind = rand() % 4;
			grid[i][j].col = j;
			grid[i][j].row = i;
			grid[i][j].x = j * ts;
			grid[i][j].y = i * ts;
		}

	int x0, y0, x, y; int click = 0; Vector2i pos;
	bool isSwap = false, isMoving = false;

	// ____________________
	// ::: Load texture :::

	// Create texture from PNG file
	sf::Texture texture;
	if (!texture.loadFromFile("sfml_icon_small.png"))
	{
		std::cerr << "Error while loading texture" << std::endl;
		return -1;
	}
	// Enable the smooth filter. The texture appears smoother so that pixels are less noticeable.
	texture.setSmooth(true);



	// _______________________________________
	// ::: Create sprite and apply texture :::

	// Create the sprite and apply the texture
	sf::Sprite sprite;
	sprite.setTexture(texture);
	sf::FloatRect spriteSize = sprite.getGlobalBounds();
	// Set origin in the middle of the sprite
	sprite.setOrigin(spriteSize.width / 2., spriteSize.height / 2.);



	// _______________
	// ::: Shaders :::

	// Check if shaders are available
	if (!sf::Shader::isAvailable())
	{
		std::cerr << "Shader are not available" << std::endl;
		return -1;
	}

	// Load shaders
	sf::Shader shader;
	if (!shader.loadFromFile("shader_mouse_fire.frag", sf::Shader::Fragment))
	{
		std::cerr << "Error while shaders" << std::endl;
		return -1;
	}
	// ___________________________
	// ::: Load distortion map :::

	sf::Texture distortionMap;

	if (!distortionMap.loadFromFile("distortion_map.png"))
	{
		sf::err() << "Error while loading distortion map" << std::endl;
		return -1;
	}

	// It is important to set repeated to true to enable scrolling upwards
	distortionMap.setRepeated(true);
	// Setting smooth to true lets us use small maps even on larger images
	distortionMap.setSmooth(true);

	// Set texture to the shader
	shader.setParameter("currentTexture", sf::Shader::CurrentTexture);
	shader.setParameter("distortionMapTexture", distortionMap);

	// Shader parameters
	float distortionFactor = .05f;
	float riseFactor = .2f;


	// _________________
	// ::: Main loop :::

	sf::Clock timer;
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// Close the window if a key is pressed or if requested
			if (event.type == sf::Event::Closed) window.close();

			if (event.type == Event::KeyPressed)
				if (event.key.code == Keyboard::Escape)
				{
					window.close();
				}
			if (event.type == Event::MouseButtonPressed)
				if (event.key.code == Mouse::Left)
				{
					if (!isSwap && !isMoving)
						click++;
					pos = Mouse::getPosition(window) - offset;
				}

			// If a key is pressed
			//if (event.type == sf::Event::KeyPressed)
			//{
			//	switch (event.key.code)
			//	{
			//		// If escape is pressed, close the application
			//	case  sf::Keyboard::Escape: window.close(); break;

			//		// Process the up, down, left and right keys to modify parameters
			//	case sf::Keyboard::Up:     distortionFactor *= 2.f;    break;
			//	case sf::Keyboard::Down:    distortionFactor /= 2.f;    break;
			//	case sf::Keyboard::Left:    riseFactor *= 2.f;          break;
			//	case sf::Keyboard::Right:   riseFactor /= 2.f;          break;
			//	default: break;
			//	}
			//}
		}

		if (click == 1) {
			x0 = pos.x / ts + 1;
			y0 = pos.y / ts + 1;
		}
		if (click == 2) {
			x = pos.x / ts + 1;
			y = pos.y / ts + 1;
			if (abs(x - x0) + abs(y - y0) == 1) {
				swap(grid[y0][x0], grid[y][x]);
				isSwap = 1;
				click = 0;
			}
			else {
				click = 1;
			}
		}

		//Match finding
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++)
			{
				if (grid[i][j].kind == grid[i + 1][j].kind)
					if (grid[i][j].kind == grid[i - 1][j].kind)
						for (int n = -1; n <= 1; n++)
							grid[i + n][j].match++;

				if (grid[i][j].kind == grid[i][j + 1].kind)
					if (grid[i][j].kind == grid[i][j - 1].kind)
						for (int n = -1; n <= 1; n++)
							grid[i][j + n].match++;
			}

		//Moving animation
		isMoving = false;
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++)
			{
				piece &p = grid[i][j];
				int dx, dy;
				for (int n = 0; n < 16; n++) // 4 - speed
				{
					dx = p.x - p.col * ts;
					dy = p.y - p.row * ts;
					if (dx)
						p.x -= dx / abs(dx);
					if (dy)
						p.y -= dy / abs(dy);
				}
				if (dx || dy)
					isMoving = 1;
			}

		//Deleting amimation
		if (!isMoving)
			for (int i = 1; i <= 8; i++)
				for (int j = 1; j <= 8; j++)
					if (grid[i][j].match)
						if (grid[i][j].alpha > 10)
						{
							grid[i][j].alpha -= 10;
							isMoving = true;
						}

		//Get score
		int score = 0;
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++)
				score += grid[i][j].match;
		if (score<highestScore) {
			totalScore += highestScore;
			highestScore = 0;
		}
		else if (score > highestScore) {
			highestScore = score;
		}
		Text scoreText;
		Font font;
		if (!font.loadFromFile("TTMeiHeiJ.ttf")) {
			std::cerr << ".Error while loading font" << std::endl;
			return -1;
		}
		//if(score)
		//std::cout << score << std::endl;
		scoreText.setFont(font);
		if (!score) {
			scoreText.setString(std::to_string(totalScore));
		}
		else {
			scoreText.setString(std::to_string(totalScore + score));
		}
		//scoreText.setString("Hello");
		scoreText.setCharacterSize(100); // in pixels, not points!
		scoreText.setFillColor(Color::White);
		scoreText.setOrigin(scoreText.getLocalBounds().width / 2., scoreText.getLocalBounds().height / 2.);
		scoreText.setPosition(textOffsetX,testOffsetY);

		//Second swap if no match
		if (isSwap && !isMoving)
		{
			if (!score)
				swap(grid[y0][x0], grid[y][x]);
			isSwap = 0;
		}

		//Update grid
		if (!isMoving)
		{
			for (int i = 8; i > 0; i--)
				for (int j = 1; j <= 8; j++)
					if (grid[i][j].match)
						for (int n = i; n > 0; n--)
							if (!grid[n][j].match)
							{
								swap(grid[n][j], grid[i][j]);
								break;
							};

			for (int j = 1; j <= 8; j++)
				for (int i = 8, n = 0; i > 0; i--)
					if (grid[i][j].match)
					{
						grid[i][j].kind = rand() % 4;
						grid[i][j].y = -ts * n++;
						grid[i][j].match = 0;
						grid[i][j].alpha = 255;
					}
		}


		// Set shader parameters
		shader.setParameter("time", timer.getElapsedTime().asSeconds());
		shader.setParameter("distortionFactor", distortionFactor);
		shader.setParameter("riseFactor", riseFactor);


		// Clear the window and apply grey background
		window.clear(sf::Color(127, 127, 127));

		// Draw the sprite and apply shader
		sprite.setPosition(window.getSize().x / 2., window.getSize().y / 2.);
		sprite.setScale(2, 2);
		//window.draw(sprite, &shader);
		window.draw(background); //Draw background
		for (int i = 1; i <= 8; i++)	//Draw gems
			for (int j = 1; j <= 8; j++)
			{
				piece p = grid[i][j];
				float rate = ts / 150.0f;
				gems.setTextureRect(IntRect(p.kind * 150, 0, 150, 150));
				gems.setScale(Vector2f(rate, rate));
				gems.setColor(Color(255, 255, 255, p.alpha));
				gems.setPosition(p.x, p.y);
				gems.move(offset.x - ts, offset.y - ts);
				//std::cout << " " << p.kind;
				window.draw(gems, &shader);
			}
		window.draw(scoreText);


		// Update display and wait for vsync
		window.display();
	}
	return 0;
}

