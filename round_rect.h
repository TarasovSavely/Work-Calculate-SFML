#ifndef ROUND_RECT
#define ROUND_RECT

#define _USE_MATH_DEFINES
#include <cmath>

struct  Rectangle : public sf::ConvexShape {
    Rectangle(const sf::FloatRect rt, const float rs)
        : rect(rt), radius(rs) {
        init();
        setPosition(rt.left, rt.top);
        sf::Shape::update ();
    }

    virtual size_t getPointCount () const {
        return m.size();
    }
    virtual sf::Vector2f getPoint (size_t i) const {
        return m[i];
    }

    sf::FloatRect rect;

private:

    float radius;
    sf::Vector2f position{0.f, 0.f};

    std::vector<sf::Vector2f> m;
    unsigned         PointCount;

    void init() {
        std::vector<sf::Vector2f> sin_4;
        {
            const float PREC = M_PI_2 / 15;
            for(  float f = 0.0; f < M_PI_2; f += PREC) {
                sin_4.push_back({sinf(f) * radius, cosf(f)*radius});
            }
        }

        std::vector<sf::Vector2f> sin_4_reverse(sin_4.rbegin(), sin_4.rend());

        const float WIDE2   = rect.width  - radius;
        const float HEIGHT2 = rect.height - radius;

        for(const auto& e : sin_4_reverse) {
            m.push_back({WIDE2   + e.x, HEIGHT2 + e.y});
        }

        for(const auto& e : sin_4) {
            m.push_back({radius  - e.x, HEIGHT2 + e.y});
        }

        for(const auto& e : sin_4_reverse) {
            m.push_back({ radius - e.x, radius - e.y});
        }

        for(const auto& e : sin_4) {
            m.push_back({ WIDE2  + e.x, radius - e.y});
        }
    }
};

#endif
