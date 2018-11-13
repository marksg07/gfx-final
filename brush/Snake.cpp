#include <QTimer>
#include <QMutex>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Snake.h"

inline Direction op_dir(Direction d)
{
    if (d == NORTH)
    {
        return SOUTH;
    }

    if (d == SOUTH)
    {
        return NORTH;
    }

    if (d == WEST)
    {
        return EAST;
    }

    return WEST;
}

SnakeManager::SnakeManager(SupportCanvas2D* canvas)
    : m_canvas(canvas), m_canvasCopy(nullptr)
{
    srand(time(NULL));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(100);
}

void SnakeManager::addSnake(int x, int y, BGRA color)
{

    copyCanvasIfNeeded();

    Snake* snake = new Snake(x, y, m_canvas, this, color);


    m_snakes.push_back(snake);


}

int SnakeManager::fix(int cur, int max)
{
    if (cur < 0)
    {
        return cur + max;
    }

    if (cur >= max)
    {
        return cur - max;
    }

    return cur;
}

void SnakeManager::addFood(int x, int y)
{
    copyCanvasIfNeeded();

    x = std::max(1, std::min(m_canvas->width() - m_foodW - 5, x));
    y = std::max(1, std::min(m_canvas->height() - m_foodH - 5, y));


    m_food.push_back(std::make_pair(x, y));


    BGRA* pix = m_canvas->data();

    for(int r = y; r < y + m_foodH; r++)
    {
        for(int c = x; c < x + m_foodW; c++)
        {
            int i = (r * m_canvas->width()) + c;

            pix[i] = m_foodColor;
        }
    }
}


void SnakeManager::update()
{
    if (m_canvasCopy != nullptr && !m_snakes.empty())
    {
        memcpy(m_canvas->data(), m_canvasCopy, m_canvas->width() * m_canvas->height() * sizeof(BGRA));
        m_canvas->update();
    }

    BGRA* pix = m_canvas->data();
    for(std::pair<int, int>& food : m_food)
    {
        int x = food.first;
        int y = food.second;

        for(int r = y; r < y + m_foodH; r++)
        {
            for(int c = x; c < x + m_foodW; c++)
            {
                int i = (r * m_canvas->width()) + c;

                pix[i] = m_foodColor;
            }
        }
    }

    for(Snake* snake : m_snakes)
    {
        snake->update();
    }
}

void SnakeManager::copyCanvasIfNeeded()
{

    if (m_canvasCopy == nullptr)
    {
        int h = m_canvas->height();
        int w = m_canvas->width();

        m_canvasCopy = new BGRA[h * w];
        memcpy(m_canvasCopy, m_canvas->data(), h * w * sizeof(BGRA));
    }


}

void SnakeManager::clear()
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;



    if (m_canvasCopy != nullptr)
    {
        memcpy(m_canvas->data(), m_canvasCopy, m_canvas->width() * m_canvas->height() * sizeof(BGRA));
        m_canvas->update();
    }

    for(Snake* snake : m_snakes)
    {
        delete snake;
    }

    m_snakes.clear();
    m_food.clear();

    delete m_canvasCopy;
    m_canvasCopy = nullptr;


}

SnakeManager::~SnakeManager()
{
    clear();
}

std::list<std::pair<int, int> >& SnakeManager::getFoodLocs()
{
    return m_food;
}

void SnakeManager::repaint(int x, int y, int w, int h)
{
    if (m_canvasCopy == nullptr)
    {
        return;
    }

    int ch = m_canvas->height();
    int cw = m_canvas->width();
    BGRA* pix = m_canvas->data();

    for(int i = y; i < y + h + 1; i++)
    {
        // Replace with memcpy
        for(int j = x; j < x + w + 1; j++)
        {
            int idx = (fix(i, ch) * cw) + fix(j, cw);

            pix[idx] = m_canvasCopy[idx];
        }
    }


}


Snake::Snake(int x, int y, SupportCanvas2D* canvas, SnakeManager* manager, BGRA color)
    : m_x(x), m_y(y), m_canvas(canvas), m_snakeManager(manager), m_color(color)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;

    m_segments.push_back(std::make_unique<BodySegment>(nullptr, m_color, x, y));

    m_canvasCopy = nullptr;
}

int Snake::fix(int cur, int max)
{
    if (cur < 0)
    {
        return cur + max;
    }

    if (cur >= max)
    {
        return cur - max;
    }

    return cur;
}

void Snake::move(Direction d, int x)
{

    switch(d)
    {
    case NORTH:
        m_y -= x;
        break;
    case SOUTH:
        m_y += x;
        break;
    case EAST:
        m_x += x;
        break;
    case WEST:
        m_x -= x;
        break;
    }

    m_d = d;


    m_y = Snake::fix(m_y, m_canvas->height());
    m_x = Snake::fix(m_x, m_canvas->width());


    for(int i = m_segments.size() - 1; i > 0; i--)
    {
        m_segments[i]->move(m_canvas);
    }

    m_segments[0]->moveTo(m_canvas, m_x, m_y);
    paint();
    //m_canvas->update();
}

double Snake::calcRadius(int x1, int y1, int x2, int y2)
{
    double x_diff = (x2 - x1) * (x2 - x1);
    double y_diff = (y2 - y1) * (y2 - y1);

    return sqrt(x_diff + y_diff);
}

void Snake::paint()
{
    for(size_t i = 0; i < m_segments.size(); i++)
    {
        m_segments[i]->paint(m_canvas);
    }

    m_canvas->update();
}

void BodySegment::paint(SupportCanvas2D* canvas)
{
    int h = canvas->height();
    int w = canvas->width();
    BGRA* pix = canvas->data();

    for(int i = m_y; i <= m_y + 10; i++)
    {
        for(int j = m_x; j <= m_x + 10; j++)
        {

            int idx = (Snake::fix(i, h) * w) + Snake::fix(j, w);

            if (idx > w * h)
            {

                //std::cout << "err: " << i << ", " << j << " : " << idx << std::endl;
            }

            pix[idx] = m_color;
        }
    }
}

void BodySegment::moveTo(SupportCanvas2D* canvas, int x, int y)
{
    m_y = Snake::fix(y, canvas->height());
    m_x = Snake::fix(x, canvas->width());
}

void BodySegment::move(SupportCanvas2D* canvas)
{
    if (m_next != nullptr)
    {
        moveTo(canvas, m_next->getX(), m_next->getY());
    }
}

void SnakeManager::eat(int x, int y)
{
    m_food.remove_if([=](std::pair<int, int> p) { return (p.first == x) && (p.second == y); });

    repaint(x, y, m_foodW, m_foodH);
}

void Snake::update()
{

    m_canvas->update();

    std::list<std::pair<int, int>>& foodLocs = m_snakeManager->getFoodLocs();
    if (!foodLocs.empty())
    {
        std::pair<int, int> closest = *foodLocs.begin();
        double sDist = calcRadius(closest.first, closest.second, m_x, m_y);

        int i = 0;
        for(auto it = foodLocs.begin(); it != foodLocs.end(); it++)
        {
            std::pair<int, int> food = *it;
            double dist = calcRadius(food.first, food.second, m_x, m_y);

            if (dist < sDist)
            {
                sDist = dist;
                closest = food;
            }

            i++;
        }

        if (sDist <= 15)
        {
            m_snakeManager->eat(closest.first, closest.second);
            paint();
            m_segments.push_back(std::make_unique<BodySegment>(m_segments[m_segments.size() - 1].get(), m_color));

            while(1)
            {
                Direction d = (Direction) (rand() % 4);

                if (d != op_dir(m_d))
                {
                    m_d = d;
                    break;
                }
            }

        } else {

            int x_diff = closest.first - m_x;
            int y_diff = closest.second - m_y;

            Direction d;
            if ((rand() % 100) >= ((abs(x_diff) >= abs(y_diff) ? 25 : 75)))
            {
                d = (x_diff > 0) ? EAST : WEST;

                if (m_d == op_dir(d))
                {
                    d = (y_diff > 0) ? SOUTH : NORTH;
                }

            } else {
                d = (y_diff > 0) ? SOUTH : NORTH;

                if (m_d == op_dir(d))
                {
                    d = (x_diff > 0) ? EAST : WEST;
                }
            }

            move(d, 10); //std::min((int) sDist, (rand() % 5) + 5));

        }

    } else {

        int change = rand() % 10;

        if (change == 0)
        {
            while(1)
            {
                Direction d = (Direction) (rand() % 4);

                if (d != op_dir(m_d))
                {
                    m_d = d;
                    break;
                }
            }
        }

        move(m_d, 10); // (rand() % 5) + 5);
    }


}


