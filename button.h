#ifndef BUTTON_H
#define BUTTON_H

#include "round_rect.h"

class Button {
private:
    Rectangle rect, sh_shadow;

    sf::Color col1, col2, col3;
    bool shadow = false, multicolor = false;

    uint8_t press;

public:
    // Конструктор: прямоугольник, скругление, цвет обычный
    Button (sf::FloatRect rc, float r, sf::Color color, float left = 3, float top = 3) {
        rc.left -= left;  // Т.к. первый аргумент зачастую это результат работы функции
        rc.top -=top;     // getGlobalBounds, применяемой к тесту, то эти строки
        rc.width += left*2;  // позволяют сделать кнопку более "простороной"
        rc.height += top*2; // и более красивой
        rect = Rectangle (rc,r);
        col1 = color;
    }

    // Нажата ли кнопка (x мыши, y мыши, нажата ли кнопка мыши)
    bool is_pressed (int x, int y, bool pr_btn) {
        press = rect.rect.contains (x,y) + (pr_btn&&rect.rect.contains (x,y));
        return press==2;
    }

    // Для корректной отрисовки в режиме multicolor рекомендуется перед отрисовкой вызвать is_pressed
    void draw (sf::RenderWindow &window) {
        if (multicolor) {
            switch (press) {
            case 0:
                rect.setFillColor (col1);
                break;
            case 1:
                rect.setFillColor (col2);
                break;
            case 2:
                rect.setFillColor (col3);
                break;
            }
        } else {
            rect.setFillColor (col1);
        }
        if (shadow) {
            window.draw (sh_shadow);
        }
        window.draw (rect);
    }

    // Включить режим multicolor, т.е. отдельные цвета для кнопки при:
    // Обычном режиме
    // Наведении на нее курсора
    // Нажатии на нее
    // Аргументы:
    // Цвет при наведении на кнопку
    // Цвет при нажатии на кнопку
    void set_multicolor (sf::Color col_2, sf::Color col_3) {
        multicolor = true;
        col2 = col_2;
        col3 = col_3;
    }

    // Включить отображение тени от кнопки (цвет тени, смещение тени относительно тела кнопки)
    void set_shadow (sf::Color color_shadow = sf::Color (100,100,100,100), float offs_x = 3.f, float offs_y = 3.f) {
        shadow = true;

        sh_shadow = rect;
        sh_shadow.setFillColor(color_shadow);
        sh_shadow.setPosition(rect.getPosition ().x+offs_x, rect.getPosition ().y+offs_y);
    }
};

#endif
