#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "button.h"

bool get_name (); // Получает имя окна в фокусе, записывает результат в переменную name
void processing_events (); // Обработка событий
void redraw_win (); // Вывод в дисплей
std::wstring calc_percent (time_t val); // Вычисляет сколько процентов от всего времени состовляет val, записывает в строку
int8_t find_win (std::wstring &name); // Ищет в окно с именем name в списках, есл инаходит - возвращает тип окна. Если нет, то -1
void question (); // Выводит вопрос о добвалении окна в какую-либо группу
void edit (); // Выводит окно изменения имени окна
void readfile (); // Читает файл WindowGroup.conf и записывает информацию по вектроам
void writefile (); // Записывает файл WindowGroup.conf и записывает туда информацию из векторов

Window rwin;
Window pwin;
Display *disp;

std::wstring name;

time_t tmr, l_tmr;

sf::RenderWindow window;

sf::Font font;

time_t t_work = 0, t_rest = 0, t_other = 0;

int m_x,m_y;
bool m_pr; // Мышь, x, y, pressed

// В следующих 3 вектроах хранится по одной характеристике каждой записи об отнесении окна к каой-либо группе:
std::vector <std::wstring> list_of_name_win {L"TODO_Program_SFML",L"Work calculate", L"Question", L"Edit"}; // Лист имен окон

std::vector <int8_t> list_of_state_win {0,0,0,0}; // Лист группы окон (стоп-таймер:0, прочее:1, отдых:2, работа:3)

std::vector <int8_t> substr_or_no {0,0,0,0}; // Имя данного окна это название целиком или только чатсь? (0-целиком,1-часть)
// Это может пригодиться, когда нужно назначить какое либо приложение целиком в одну группу
// Например, браузер (окно браузера может принимать бесчисленное кол-во имен, все их не внести)
// Поэтому вносим только название браузера и все!

int handler (_XDisplay *d, XErrorEvent *ds) { // Обработчик ошибок X11Lib, не обрабатывает входные данные за ненадобностью
    return 0;
}

int main(void) {
    window.create (sf::VideoMode(382, 110), "Work calculate", sf::Style::Default^sf::Style::Resize); // Создаем окно
    window.setFramerateLimit(120);

    XSetErrorHandler(handler);

    if (!font.loadFromFile("font.ttf")) {
        std::cout<<"Font not found. Please, read README file."<<std::endl;
        return 1;
    }

    setlocale(LC_ALL, ""); // Адаптация под русский и не только
    disp = XOpenDisplay(NULL); // Устанавливаем связь с X-сервером

    readfile ();

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
    wchar_t** names;

    XGetInputFocus (disp, &get_focus, &state_win);

    XTextProperty name;
    int count;

    if (state_win == 1) {
        if (XGetWMName(disp, get_focus, &name)) {
            if (XwcTextPropertyToTextList (disp, &name, &names, &count)>=0) {
                if (count) {
                    ::name = names[0];
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
                    ::name = names[0];
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
            writefile();
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

    std::wstring str = L"Working time: ";
    str = str + std::to_wstring (t_work/3600) +L":"+std::to_wstring ((t_work%3600) / 60)+L":"+std::to_wstring (t_work%60) + L" (" + calc_percent(t_work) + L")";

    work.setFont(font);
    work.setString(str);
    work.setCharacterSize(25);
    work.setFillColor(sf::Color (20,215,20));
    work.setStyle(sf::Text::Bold | sf::Text::Italic);
    work.setPosition(sf::Vector2f (10.f,10.f));

    str = L"Rest time: ";
    str = str + std::to_wstring (t_rest/3600) +L":"+std::to_wstring ((t_rest%3600) / 60)+L":"+std::to_wstring (t_rest%60) + L" (" + calc_percent(t_rest) + L")";

    rest.setFont(font);
    rest.setString(str);
    rest.setCharacterSize(25);
    rest.setFillColor(sf::Color (235,10,10));
    rest.setStyle(sf::Text::Bold | sf::Text::Italic);
    rest.setPosition(sf::Vector2f (10.f,40.f));

    str = L"Other time: ";
    str = str + std::to_wstring (t_other/3600) +L":"+std::to_wstring ((t_other%3600) / 60)+L":"+std::to_wstring (t_other%60) + L" (" + calc_percent(t_other) + L")";;

    other.setFont(font);
    other.setString(str);
    other.setCharacterSize(25);
    other.setFillColor(sf::Color(30,0,245));
    other.setStyle(sf::Text::Bold | sf::Text::Italic);
    other.setPosition(sf::Vector2f (10.f,70.f));

    window.draw (work);
    window.draw (rest);
    window.draw (other);

    window.display();
}

std::wstring calc_percent (time_t val) {
    if (t_other+t_rest+t_work) {
        int ret = ((float)val/((float)t_other+t_rest+t_work))*1000.0;
        return std::to_wstring (ret/10) + L"." + std::to_wstring (ret%10) + L"%";
    } else {
        return L"0.0%";
    }
}

int8_t find_win (std::wstring &name) {
    for (size_t i = 0; i<list_of_name_win.size (); i++) {
        if (substr_or_no[i] ? (name.find(list_of_name_win [i])!=std::string::npos) : (name==list_of_name_win [i])) {
            return list_of_state_win [i];
        }
    }
    return -1;
}

void question () {
    sf::RenderWindow que_win (sf::VideoMode(390, 75), "Question", sf::Style::Default^sf::Style::Resize);
    que_win.setFramerateLimit(120);

    sf::Text txt;
    int m_x = 0, m_y = 0, m_pr = 0;

    list_of_name_win.push_back(name);
    substr_or_no.push_back (0);

    txt.setFont(font);
    bool large;

    if (name.size ()>11) {
        large = true;
    } else {
        large = false;
    }
    std::wstring str = L"The \"" + (large?std::wstring (name.begin (),name.begin ()+11):std::wstring(name.begin(), name.end())) + (large?L"...":L"") + L"\" window is not assigned to any group.\n\r"
                       "Which group should he be assigned to?";
    txt.setString (str.c_str());
    txt.setCharacterSize(14);
    txt.setFillColor(sf::Color (0,0,0));
    txt.setPosition(sf::Vector2f (10.f,10.f));


    sf::Text text_work, text_rest, text_other, text_StopTimer, text_edit;

    text_work.setFont(font);
    text_work.setString("Work");
    text_work.setCharacterSize(17);
    text_work.setFillColor(sf::Color(0,0,0));
    text_work.setPosition(sf::Vector2f (11.f,49.f));

    text_rest.setFont(font);
    text_rest.setString("Rest");
    text_rest.setCharacterSize(17);
    text_rest.setFillColor(sf::Color(0,0,0));
    text_rest.setPosition(sf::Vector2f (70.f,48.f));

    text_other.setFont(font);
    text_other.setString("Other");
    text_other.setCharacterSize(17);
    text_other.setFillColor(sf::Color(0,0,0));
    text_other.setPosition(sf::Vector2f (122.f,49.f));

    text_StopTimer.setFont(font);
    text_StopTimer.setString("Stop-Timer");
    text_StopTimer.setCharacterSize(17);
    text_StopTimer.setFillColor(sf::Color(0,0,0));
    text_StopTimer.setPosition(sf::Vector2f (185.f,47.f));

    text_edit.setFont(font);
    text_edit.setString("Edit name");
    text_edit.setCharacterSize(17);
    text_edit.setFillColor(sf::Color(0,0,0));
    text_edit.setPosition(sf::Vector2f (292.f,49.f));

    Button btn_work (text_work.getGlobalBounds(),5.f,sf::Color (10,245,10), 3, 4),
           btn_rest (text_rest.getGlobalBounds(),5.f,sf::Color (245,10,10), 3, 5),
           btn_other(text_other.getGlobalBounds(),5.f,sf::Color (117,187,253), 3, 4),
           btn_stoptimer(text_StopTimer.getGlobalBounds(),5.f,sf::Color (255,223,0)),
           btn_edit (text_edit.getGlobalBounds (), 5.f, sf::Color (120,120,120), 3, 4);

    while (que_win.isOpen()) {
        que_win.clear (sf::Color::White);

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

        if (btn_edit.is_pressed(m_x,m_y,m_pr)) {
            edit ();
            substr_or_no [substr_or_no.size ()-1] = 1;
            if (name.size ()>11) {
                large = true;
            } else {
                large = false;
            }
            std::wstring str = L"The \"" + (large?std::wstring (name.begin (),name.begin ()+11):std::wstring(name.begin(), name.end())) + (large?L"...":L"") + L"\" window is not assigned to any group.\n\r"
                               "Which group should he be assigned to?";
            txt.setString (str.c_str());
            txt.setCharacterSize(14);
            txt.setFillColor(sf::Color (0,0,0));
            txt.setPosition(sf::Vector2f (10.f,10.f));

            list_of_name_win [list_of_name_win.size ()-1] = name;
        }

        btn_work.draw(que_win);
        btn_rest.draw(que_win);
        btn_stoptimer.draw(que_win);
        btn_other.draw(que_win);
        btn_edit.draw (que_win);

        que_win.draw (txt);
        que_win.draw (text_work);
        que_win.draw (text_rest);
        que_win.draw (text_other);
        que_win.draw (text_StopTimer);
        que_win.draw (text_edit);

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

void edit () {
    sf::RenderWindow win_edit (sf::VideoMode(390, 37), "Edit", sf::Style::Default^sf::Style::Resize);
    win_edit.setFramerateLimit(120);

    std::size_t pos = 0;

    while (win_edit.isOpen()) {
        win_edit.clear (sf::Color (255,255,255));

        sf::Text text_edit;
        text_edit.setFont(font);
        text_edit.setString(L">"+name.substr(pos,44));
        text_edit.setCharacterSize(17);
        text_edit.setFillColor(sf::Color(0,0,0));
        text_edit.setPosition(sf::Vector2f (10.f,10.f));

        win_edit.draw(text_edit);

        win_edit.display();

        sf::Event event;
        while (win_edit.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                win_edit.close ();
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
            case sf::Event::TextEntered:
                if (event.text.unicode!=8) {
                    name [pos] = event.text.unicode;
                    pos++;
                    if (pos==name.size()) {
                        pos = name.size()-1;
                    }
                }
                break;
            case sf::Event::KeyPressed:
                if (event.key.code==sf::Keyboard::Right) {
                    pos++;
                    if (pos==name.size()) {
                        pos = name.size()-1;
                    }
                } else if (event.key.code==sf::Keyboard::Left) {
                    if (pos>0) {
                        pos--;
                    }
                } else if (event.key.code==sf::Keyboard::Backspace) {
                    if (pos<name.size ()) {
                        name.erase(pos,1);
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}

void readfile () {
    std::ifstream fin ("WindowGroup.conf", std::ios_base::binary);
    if (fin.is_open ()) {
        size_t num;
        fin.read ((char*)&num, sizeof (size_t));
        for (size_t i = 4; i<num; i++) {
            size_t sz_txt;
            fin.read((char*)&sz_txt, sizeof (size_t));
            int8_t p1;
            fin.read((char*)&p1, 1);
            list_of_state_win.push_back (p1);

            fin.read((char*)&p1,1);
            substr_or_no.push_back (p1);

            wchar_t *name = new wchar_t [sz_txt+1];
            name [sz_txt] = '\0';
            fin.read((char*)name, sizeof (wchar_t)*sz_txt);
            list_of_name_win.push_back (name);
            delete [] name;
        }
        fin.close ();
    }
}

void writefile () {
    std::ofstream fout ("WindowGroup.conf", std::ios_base::binary);
    if (fout.is_open ()) {
        size_t sz = list_of_name_win.size ();
        fout.write ((char*) &sz,sizeof (size_t));
        for (size_t i = 4; i<list_of_name_win.size (); i++) {
            sz = list_of_name_win[i].size ();
            fout.write((char*)&sz, sizeof (size_t));
            fout.write((char*)&(list_of_state_win[i]), 1);
            fout.write((char*)&(substr_or_no[i]), 1);
            fout.write((char*)list_of_name_win [i].c_str (), sizeof (wchar_t)*list_of_name_win [i].size());
        }
        fout.close ();
    } else {
        std::wcout<<"I can't create file WindowGroup.conf"<<std::endl;
    }
}
