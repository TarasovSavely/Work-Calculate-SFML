#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <ctime>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "button.h"

bool get_name (); // Получает имя окна в фокусе, записывает результат в переменную name
void processing_events (); // Обработка событий
void redraw_win (); // Вывод в дисплей
std::string calc_percent (time_t val); // Вычисляет сколько процентов от всего времени состовляет val, записывает в строку
bool find_str (wchar_t *str, wchar_t *f_str); // Ищет в строке-аргументе 1, строку-аргумент два

Window rwin;
Window pwin;
Display *disp;

wchar_t name [500];

time_t tmr;
tm *loctime;

sf::RenderWindow window;

sf::Font font;

time_t t_work = 0, t_rest = 0, t_other = 10;

int main(void) {
    window.create (sf::VideoMode(382, 110), "Work calculate", sf::Style::Default^sf::Style::Resize); // Создаем окно
    window.setFramerateLimit(120);

    if (!font.loadFromFile("font.ttf")) {
        std::cout<<"Font not found. Please, read README file."<<std::endl;
        return 1;
    }

    setlocale(LC_ALL, ""); // Адаптация под русский и не только
    disp = XOpenDisplay(NULL); // Устанавливаем связь с X-сервером

    while (window.isOpen()) {
        if (get_name ()) { // Получаем имя окна в фокусе
            tmr = time(NULL);
            std::wcout<<name<<std::endl;
        }
        redraw_win ();
        processing_events();
    }

    if (window.isOpen()) window.close(); // Если окно еще открыто, то закрыть
    if (disp) XCloseDisplay(disp); // Закрываем связь с X сервером

    return 0;
}

bool get_name() {

    bool ret = false;

    Window get_focus;
    int state_win;
    wchar_t** names;

    XGetInputFocus (disp, &get_focus, &state_win);

    XTextProperty name;
    int count;

    if (state_win == 1) {
        if (XGetWMName(disp, get_focus, &name)) {
            if (XwcTextPropertyToTextList (disp, &name, &names, &count)>=0) {
                if (count) {
                    wcscpy(::name,names[0]);
                    ret = true;
                }
                XwcFreeStringList (names);
            }
        }
    } else if (state_win == 2) {
        Window *cwins;
        unsigned int count2;

        XQueryTree(disp, get_focus, &rwin, &pwin, &cwins, &count2);

        if (XGetWMName(disp, pwin, &name)) {
            if (XwcTextPropertyToTextList (disp, &name, &names, &count)>=0) {
                if (count) {
                    wcscpy(::name,names[0]);
                    ret=true;
                }
                XwcFreeStringList (names);
            }
        }

        XFree (cwins);
    } else {
        std::cout<<"ERR1"<<std::endl;
    }

    return ret;
}

void processing_events () {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            window.close ();
            break;
        case sf::Event::LostFocus:
            window.setFramerateLimit(10);
            break;
        case sf::Event::GainedFocus:
            window.setFramerateLimit(600);
            break;
        default:
            break;
        }
    }
}

//Button btn;

void redraw_win () {
    sf::Text work, rest, other;

    window.clear(sf::Color (10,25,40));

    std::string str = "Working time: ";
    str = str + std::to_string (t_work/3600) +":"+std::to_string ((t_work%3600) / 60)+":"+std::to_string (t_work%60) + " (" + calc_percent(t_work) + ")";

    work.setFont(font);
    work.setString(str);
    work.setCharacterSize(25);
    work.setFillColor(sf::Color (0,250,0));
    work.setStyle(sf::Text::Bold | sf::Text::Italic);
    work.setPosition(sf::Vector2f (10.f,10.f));

    str = "Rest time: ";
    str = str + std::to_string (t_rest/3600) +":"+std::to_string ((t_rest%3600) / 60)+":"+std::to_string (t_rest%60) + " (" + calc_percent(t_rest) + ")";

    rest.setFont(font);
    rest.setString(str);
    rest.setCharacterSize(25);
    rest.setFillColor(sf::Color (250,0,0));
    rest.setStyle(sf::Text::Bold | sf::Text::Italic);
    rest.setPosition(sf::Vector2f (10.f,40.f));

    str = "Other time: ";
    str = str + std::to_string (t_other/3600) +":"+std::to_string ((t_other%3600) / 60)+":"+std::to_string (t_other%60) + " (" + calc_percent(t_other) + ")";;

    other.setFont(font);
    other.setString(str);
    other.setCharacterSize(25);
    other.setFillColor(sf::Color(0,0,250));
    other.setStyle(sf::Text::Bold | sf::Text::Italic);
    other.setPosition(sf::Vector2f (10.f,70.f));

    window.draw (work);
    window.draw (rest);
    window.draw (other);


    window.display();
}

std::string calc_percent (time_t val) {
    if (t_other+t_rest+t_work) {
        int ret = ((float)val/((float)t_other+t_rest+t_work))*1000.0;
        return std::to_string (ret/10) + "." + std::to_string (ret%10) + "%";
    } else {
        return "0.0%";
    }
}

bool find_str (wchar_t *str, wchar_t *f_str) {
    if (wcslen (str)<wcslen (f_str)) {
        return false;
    }

    size_t stat_find = 0;
    size_t n = wcslen(f_str);

    for (size_t i = 0; i<=wcslen (str)-wcslen(f_str); i++) {
        while (str [i+stat_find] == f_str [stat_find]) {
            stat_find++;
            if (stat_find == n) {
                return true;
            }
        }
        stat_find = 0;
    }
    return false;
}

