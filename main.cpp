#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <ctime>
#include <vector>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "button.h"

bool get_name (); // Получает имя окна в фокусе, записывает результат в переменную name
void processing_events (); // Обработка событий
void redraw_win (); // Вывод в дисплей
std::string calc_percent (time_t val); // Вычисляет сколько процентов от всего времени состовляет val, записывает в строку
bool find_str (wchar_t *str, wchar_t *f_str); // Ищет в строке-аргументе 1, строку-аргумент два
int8_t find_win (std::string &name); // Ищет в окно с именем name в списках, есл инаходит - возвращает тип окна. Если нет, то -1
void question (); // Выводит вопрос о добвалении ока в какую-либо группу

Window rwin;
Window pwin;
Display *disp;

std::string name;

time_t tmr, l_tmr;

sf::RenderWindow window;

sf::Font font;

time_t t_work = 0, t_rest = 0, t_other = 0;

int m_x,m_y;
bool m_pr; // Мышь, x, y, pressed

std::vector <std::string> list_of_name_win; // Лист имен окон
std::vector <int8_t> list_of_state_win; // Лист состояний окон (стоп-таймер, прочее, отдых, работа)

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

        tmr = time(NULL);
        if (tmr!=l_tmr && get_name ()) { // Получаем имя окна в фокусе
            l_tmr = tmr;
            switch (find_win(name)) {
            case 1:
                ++t_other;
                break;
            case 2:
                ++t_rest;
                break;
            case 3:
                ++t_work;
                break;
            case -1:
                question ();
                break;
            }
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
    char** names;

    XGetInputFocus (disp, &get_focus, &state_win);

    XTextProperty name;
    int count;

    if (state_win == 1) {
        if (XGetWMName(disp, get_focus, &name)) {
            if (XmbTextPropertyToTextList (disp, &name, &names, &count)>=0) {
                if (count) {
                    ::name = names[0];
                    ret = true;
                }
                XFreeStringList (names);
            }
        }
    } else if (state_win == 2) {
        Window *cwins;
        unsigned int count2;

        XQueryTree(disp, get_focus, &rwin, &pwin, &cwins, &count2);

        if (XGetWMName(disp, pwin, &name)) {
            if (XmbTextPropertyToTextList (disp, &name, &names, &count)>=0) {
                if (count) {
                    ::name = names[0];
                    ret=true;
                }
                XFreeStringList (names);
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
            window.setFramerateLimit(120);
            break;
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Left) {
                m_pr = true;
            }
            break;
        case sf::Event::MouseButtonReleased:
            if (event.mouseButton.button == sf::Mouse::Left) {
                m_pr = false;
            }
            break;
        case sf::Event::MouseMoved:
            m_x = event.mouseMove.x;
            m_y = event.mouseMove.y;
            break;
        default:
            break;
        }
    }
}

void redraw_win () {
    sf::Text work, rest, other;

    window.clear(sf::Color (255,255,255));

    std::string str = "Working time: ";
    str = str + std::to_string (t_work/3600) +":"+std::to_string ((t_work%3600) / 60)+":"+std::to_string (t_work%60) + " (" + calc_percent(t_work) + ")";

    work.setFont(font);
    work.setString(str);
    work.setCharacterSize(25);
    work.setFillColor(sf::Color (20,215,20));
    work.setStyle(sf::Text::Bold | sf::Text::Italic);
    work.setPosition(sf::Vector2f (10.f,10.f));

    str = "Rest time: ";
    str = str + std::to_string (t_rest/3600) +":"+std::to_string ((t_rest%3600) / 60)+":"+std::to_string (t_rest%60) + " (" + calc_percent(t_rest) + ")";

    rest.setFont(font);
    rest.setString(str);
    rest.setCharacterSize(25);
    rest.setFillColor(sf::Color (235,10,10));
    rest.setStyle(sf::Text::Bold | sf::Text::Italic);
    rest.setPosition(sf::Vector2f (10.f,40.f));

    str = "Other time: ";
    str = str + std::to_string (t_other/3600) +":"+std::to_string ((t_other%3600) / 60)+":"+std::to_string (t_other%60) + " (" + calc_percent(t_other) + ")";;

    other.setFont(font);
    other.setString(str);
    other.setCharacterSize(25);
    other.setFillColor(sf::Color(30,0,245));
    other.setStyle(sf::Text::Bold | sf::Text::Italic);
    other.setPosition(sf::Vector2f (10.f,70.f));

    Button btn (work.getGlobalBounds(),3.f,sf::Color (255,223,0));
    btn.set_shadow();
    btn.set_multicolor(sf::Color (255.0/1.5, 223.0/1.5, 0), sf::Color (225/3, 223/3, 0));
    std::cout<<btn.is_pressed(m_x,m_y, m_pr)<<std::endl;
    btn.draw(window);

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

int8_t find_win (std::string &name) {
    for (size_t i = 0; i<list_of_name_win.size (); i++) {
        if (list_of_name_win [i] == name) {
            return list_of_state_win [i];
        }
    }
    return -1;
}

void question () {
    sf::RenderWindow que_win (sf::VideoMode(390, 75), "Question", sf::Style::Default^sf::Style::Resize);
    window.setFramerateLimit(120);

    sf::Text txt;
    int m_x, m_y, m_pr;

    list_of_name_win.push_back(name);

    txt.setFont(font);
    bool large = false;
    if (name.size ()>11) {
        name.resize(11);
        large = true;
    }
    txt.setString("The \"" + name + (large?"...":"") + "\" window is not assigned to any group.\n\r"
                  "Which group should he be assigned to?");
    txt.setCharacterSize(14);
    txt.setFillColor(sf::Color (0,0,0));
    txt.setPosition(sf::Vector2f (10.f,10.f));

    sf::Text text_work, text_rest, text_other, text_StopTimer;

    text_work.setFont(font);
    text_work.setString("Work");
    text_work.setCharacterSize(17);
    text_work.setFillColor(sf::Color(0,0,0));
    text_work.setPosition(sf::Vector2f (10.f,48.f));

    text_rest.setFont(font);
    text_rest.setString("Rest");
    text_rest.setCharacterSize(17);
    text_rest.setFillColor(sf::Color(0,0,0));
    text_rest.setPosition(sf::Vector2f (70.f,48.f));

    text_other.setFont(font);
    text_other.setString("Other");
    text_other.setCharacterSize(17);
    text_other.setFillColor(sf::Color(0,0,0));
    text_other.setPosition(sf::Vector2f (130.f,48.f));

    text_StopTimer.setFont(font);
    text_StopTimer.setString("Stop-Timer");
    text_StopTimer.setCharacterSize(17);
    text_StopTimer.setFillColor(sf::Color(0,0,0));
    text_StopTimer.setPosition(sf::Vector2f (200.f,48.f));

    Button btn_work (text_work.getGlobalBounds(),5.f,sf::Color (10,245,10)),
           btn_rest (text_rest.getGlobalBounds(),5.f,sf::Color (245,10,10)),
           btn_other(text_other.getGlobalBounds(),5.f,sf::Color (0,0,150)),
           btn_stoptimer(text_StopTimer.getGlobalBounds(),5.f,sf::Color (255,223,0));

    while (que_win.isOpen()) {
        que_win.clear (sf::Color::White);
        // (стоп-таймер, прочее, отдых, работа)
        if (btn_work.is_pressed(m_x,m_y,m_pr)) {
            list_of_state_win.push_back(3);
            que_win.close ();
            return;
        }
        if (btn_rest.is_pressed(m_x,m_y,m_pr)) {
            list_of_state_win.push_back(2);
            que_win.close ();
            return;
        }
        if (btn_stoptimer.is_pressed(m_x,m_y,m_pr)) {
            list_of_state_win.push_back(0);
            que_win.close ();
            return;
        }
        if (btn_other.is_pressed(m_x,m_y,m_pr)) {
            list_of_state_win.push_back(1);
            que_win.close ();
            return;
        }

        btn_work.draw(que_win);
        btn_rest.draw(que_win);
        btn_stoptimer.draw(que_win);
        btn_other.draw(que_win);

        que_win.draw (txt);
        que_win.draw (text_work);
        que_win.draw (text_rest);
        que_win.draw (text_other);
        que_win.draw (text_StopTimer);
        que_win.display ();

        sf::Event event;
        while (que_win.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                list_of_state_win.push_back(0);
                que_win.close ();
                break;
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    m_pr = true;
                }
                break;
            case sf::Event::MouseButtonReleased:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    m_pr = false;
                }
                break;
            case sf::Event::MouseMoved:
                m_x = event.mouseMove.x;
                m_y = event.mouseMove.y;
                break;
            default:
                break;
            }
        }

    }
}

