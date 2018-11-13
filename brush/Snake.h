#ifndef SNAKE_H
#define SNAKE_H

#include <QThread>
#include <list>
#include <memory>
#include "SupportCanvas2D.h"
#include "BGRA.h"

class SupportCanvas2D;

class Snake;

class SnakeManager :  public QObject
{
    Q_OBJECT
public:
    SnakeManager(SupportCanvas2D* canvas);
    virtual ~SnakeManager();

    void addSnake(int x, int y, BGRA color);

    void addFood(int x, int y);

    void clear();

    void eat(int x, int y);

    void copyCanvasIfNeeded();

    void repaint(int x, int y, int w, int h);

    int fix(int cur, int max);

    std::list<std::pair<int, int>>& getFoodLocs();

private:

    std::vector<Snake*> m_snakes;

    std::list<std::pair<int, int>> m_food;

    SupportCanvas2D* m_canvas;

    BGRA* m_canvasCopy;

    const int m_foodW = 10;
    const int m_foodH = 10;

    const BGRA m_foodColor = BGRA(255, 255, 25);

    QMutex mtx;

public slots:
    void update();
};

typedef enum Direction
{
    NORTH, SOUTH, EAST, WEST
} Direction;


class BodySegment
{
public:
    BodySegment(BodySegment* next, BGRA color, int x = 0, int y = 0)
        : m_next(next), m_color(color), m_x(x), m_y(y)
    {

    }

    void moveTo(SupportCanvas2D* canvas, int x, int y);

    void move(SupportCanvas2D* canvas);

    void paint(SupportCanvas2D* canvas);

    int getX()
    {
        return m_x;
    }

    int getY()
    {
        return m_y;
    }

private:

    BodySegment* m_next;

    BGRA m_color;
    int m_x;
    int m_y;
};

class Snake
{
public:
    Snake(int x, int y, SupportCanvas2D* canvas, SnakeManager* manager, BGRA color);


    double calcRadius(int x1, int y1, int x2, int y2);

    void paint();

    void move(Direction d, int x = 10);

    static int fix(int cur, int max);

    void update();


private:

    Direction m_d;


    int m_x;
    int m_y;

    SupportCanvas2D* m_canvas;

    SnakeManager* m_snakeManager;

    BGRA m_color;

    BGRA* m_canvasCopy;

    std::vector<std::unique_ptr<BodySegment>> m_segments;


};

#endif // SNAKE_H
