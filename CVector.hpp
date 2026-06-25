#pragma once
#include<SFML/Graphics.hpp>

class Vector
{
    public:
        float x = 0;
        float y = 0;
        Vector();
        Vector(float x , float y);
        bool operator == (const Vector& other) const;
        bool operator != (const Vector& other) const;

        Vector operator + (const Vector& other) const;
        Vector operator - (const Vector& other) const;
        Vector operator * (const float value ) const;
        Vector operator / (const float value ) const;

        void operator += (const Vector& other);
        void operator -= (const Vector& other);
        void operator *= (const float value);
        void operator /= (const float value);

        float distanceSquared(const Vector& other) const;
        float distance(const Vector& other) const;
        float lengthSquared() const;
        float length() const;
        Vector normalize() const;
        void print() const;
        void drawAsPoint(sf::RenderWindow& window , sf::Color color) const;
        void rotate(sf::Angle angle);

        sf::Vector2f toVector2f() const;
};