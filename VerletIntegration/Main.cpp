#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <thread>
#include <algorithm>

#define W 800
#define H 800
#define PI 3.14159265359f
#define radius 4.f

class VerletIntegration {

private:

	struct Point {
		float x;
		float y;
		float oldX;
		float oldY;
		bool isPinned;
	};

	struct Stick {
		Point* p0;
		Point* p1;
		float length;
	};

	std::vector<Point*> points;
	std::vector<Stick*> sticks;
	sf::CircleShape* circlePoint = new sf::CircleShape(radius);



	std::vector<Stick*> vecS;
	int* times = new int(0);

	float bounce = 0.9f
		, gravity = .5f
		, friction = .999f;


	float distanceNode(Point p1, Point p2) {
		return sqrtf((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
	};
	float distanceNode(Point* p1, Point* p2) {
		return sqrtf((p2->x - p1->x) * (p2->x - p1->x) + (p2->y - p1->y) * (p2->y - p1->y));
	};
	float distance(float dx, float dy) {
		return sqrtf(dx * dx + dy * dy);
	};


public:
	Point* selectedp;
	void sortDistance(std::vector<Point*>& points, Point* p) {
		std::sort(points.begin(), points.end(),
			[&](Point* _l, Point* _r) {
				return (distanceNode(p, _l) < distanceNode(p, _r));
			});
	}
	void initPoints(float posX, float posY) {

		Point* p = new Point();
		Stick* s = new Stick();

		p->x = posX;
		p->y = posY;
		p->oldX = posX;
		p->oldY = posY;
		p->isPinned = true;
		points.push_back(p);

		if (points.size() > 1) {

			sortDistance(points, p);

			s->p0 = points[0];
			s->p1 = points[1];
			//s->length = distanceNode(s->p0, s->p1);
			s->length = 20;
			sticks.push_back(s);
			if (points.size() > 2 && distanceNode(points[0], points[2]) <= s->length + 50)
			{
				Stick* s = new Stick();
				s->p0 = points[0];
				s->p1 = points[2];
				s->length = 20;
				//s->length = distanceNode(s->p0, s->p1);
				sticks.push_back(s);
			}
		}
		std::cout << "sticks.size() " << sticks.size() << std::endl;
	}
	void updatePoints() {
		if (points.size() == 0) {
			sticks.clear();
			sticks.shrink_to_fit();
		}
		for (int i = 0; i < points.size(); i++)
		{
			if (points[i]) {
				Point* p = points[i];

				if (!p->isPinned) {
					float vx = (p->x - p->oldX) * friction;
					float vy = (p->y - p->oldY) * friction;

					p->oldX = p->x;
					p->oldY = p->y;
					p->x += vx;
					p->y += vy;
					p->y += gravity;
				}
			}
		}
	}
	void constrainPoints() {
		for (int i = 0; i < points.size(); i++) {
			if (points[i]) {
				Point* p = points[i];
				if (!p->isPinned) {
					float vx = (p->x - p->oldX) * friction;
					float vy = (p->y - p->oldY) * friction;

					if (p->x >= W - 50 - radius) {
						p->x = W - 50 - radius;
						p->oldX = p->x + vx * bounce;

					}
					else if (p->x <= 0 + 50 + radius) {
						p->x = 0 + 50 + radius;
						p->oldX = p->x + vx * bounce;
					}
					if (p->y >= H - 50 - radius) {
						p->y = H - 50 - radius;
						p->oldY = p->y + vy * bounce;

					}
					else if (p->y <= 0 + 50 + radius) {
						p->y = 0 + 50 + radius;
						p->oldY = p->y + vy * bounce;
					}
				}
			}
		}
	}
	void renderPoints(sf::RenderWindow* window) {
		for (int i = 0; i < points.size(); i++)
		{
			if (points[i]) {
				Point* p = points[i];
				if (p->isPinned) {
					circlePoint->setFillColor(sf::Color::Red);
				}
				else {
					circlePoint->setFillColor(sf::Color::Green);
				}
				circlePoint->setPosition(p->x, p->y);
				window->draw(*circlePoint);
			}
		}
	};
	void updateSticks() {

		for (int i = 0; i < sticks.size(); i++)
		{
			Stick* s = sticks[i];

			float dx = s->p0->x - s->p1->x;
			float dy = s->p0->y - s->p1->y;
			float distance = VerletIntegration::distance(dx, dy);
			float difference = s->length - distance;
			float percent = difference / distance / 2;
			float offsetX = dx * percent;
			float offsetY = dy * percent;

			if (!s->p0->isPinned) {
				s->p0->x += offsetX;
				s->p0->y += offsetY;
			}
			if (!s->p1->isPinned) {
				s->p1->x -= offsetX;
				s->p1->y -= offsetY;
			}

		}
	}
	void renderSticks(sf::RenderWindow* window) {
		for (int i = 0; i < sticks.size(); i++) {

			Stick* s = sticks[i];
			sf::Vertex line[] = {
				sf::Vertex(sf::Vector2f(s->p0->x + radius, s->p0->y + radius)),
				sf::Vertex(sf::Vector2f(s->p1->x + radius, s->p1->y + radius)) };

			window->draw(line, 2, sf::Lines);
		}
	}
	void pinClick(float posX, float posY, bool pin = false) {
		Point* p = selectedp;
		sortDistance(points, p);
		if (!pin) {
			points[0]->isPinned = !points[0]->isPinned;
		}
		else {
			points[0]->isPinned = pin;
		}
	}
	Point* pointByMousePos(float posX, float posY) {
		Point p;
		p.x = posX;
		p.y = posY;
		sortDistance(points, &p);
		return points[0];
	}
	void translateWmouse(Point* _p, float posX, float posY) {
		Point* p = _p;
		if (sticks.size() >= 1) {
			auto findStick_1 = [&](Stick* _s) {
				return (_s->p0 == p); };
			auto foundStick_1 = std::find_if(sticks.begin(), sticks.end(), findStick_1);
			//if (foundStick_1 != sticks.end())
				//(*foundStick_1)->length = distanceNode((*foundStick_1)->p0, (*foundStick_1)->p1);

			auto findStick_2 = [&](Stick* _s) {
				return (_s->p1 == p); };
			auto foundStick_2 = std::find_if(sticks.begin(), sticks.end(), findStick_2);
			//if (foundStick_2 != sticks.end())
				//(*foundStick_2)->length = distanceNode((*foundStick_2)->p0, (*foundStick_2)->p1);
		}
		p->x = posX;
		p->y = posY;
	};
	void deleteWmouse(float posX, float posY) {
		std::vector<Stick*> tempSticks;
		for (int i = 0; i < sticks.size(); i++)
		{
			tempSticks.push_back(sticks[i]);
		}

		if (points.size() >= 1) {
			Point* p = pointByMousePos(posX, posY);
			if (sticks.size() >= 1)
			{
				auto findStick = [&](Stick* _s) {
					return ((_s->p0 == p || _s->p1 == p));
				};
				auto foundStick = std::find_if(tempSticks.begin(), tempSticks.end(), findStick);

				while (foundStick != tempSticks.end()) {
					Point* pNew = new Point();

					pNew->x = posX;
					pNew->y = posY;
					pNew->oldX = posX;
					pNew->oldY = posY;
					pNew->isPinned = false;
					points.push_back(pNew);

					if ((*foundStick)->p0 == p) {
						(*foundStick)->p0 = pNew;
					}
					else {
						(*foundStick)->p1 = pNew;
					}

					tempSticks.erase(foundStick);
					foundStick = std::find_if(tempSticks.begin(), tempSticks.end(), findStick);
				}
			}
			delete p;
			points.erase(points.begin());

		}

		std::cout << "sticks.size() " << sticks.size() << std::endl;
	}

};

int main(void) {

	sf::RenderWindow window(sf::VideoMode(W, H), "Verlet Iteration");
	window.setKeyRepeatEnabled(false);

	sf::RectangleShape background(sf::Vector2f(W - 100, H - 100));
	background.setFillColor(sf::Color(80, 80, 80));
	background.setPosition(50, 50);

	VerletIntegration vi;

	while (window.isOpen())
	{
		sf::Vector2i localMousePosition(sf::Mouse::getPosition(window));
		sf::Event event;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::LShift)
				{
					vi.selectedp = vi.pointByMousePos(localMousePosition.x, localMousePosition.y);
				}
			}

			if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					//vi.pinClick(localMousePosition.x, localMousePosition.y, true);
					vi.translateWmouse(vi.selectedp, localMousePosition.x, localMousePosition.y);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
					vi.deleteWmouse(localMousePosition.x, localMousePosition.y);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));

				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
					vi.selectedp = vi.pointByMousePos(localMousePosition.x, localMousePosition.y);
					vi.pinClick(localMousePosition.x, localMousePosition.y);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				else if (event.mouseButton.button == sf::Mouse::Left) {
					vi.initPoints(localMousePosition.x, localMousePosition.y);
					//std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}

			}
		}

		window.clear();
		window.draw(background);

		vi.updatePoints();
		for (int i = 0; i < 1; i++)
		{
			vi.updateSticks();
			vi.constrainPoints();
		}
		vi.renderSticks(&window);
		vi.renderPoints(&window);

		window.display();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}