# Work-Calculate-SFML
This is a program for calculating working time and rest time.

## English documentation
### Work logic:
This is a simple time counter. The program has several timers:
+ Working time timer.
+ Rest timer.
+ Timer for doing other things.

Their value is displayed on the main screen of the program. The timers are run one by one depending on the name of the currently focused window. The window names are divided into 3 groups corresponding to each timer + 1 group called "stop timer". The program window named "Work calculate" belongs to the "stop timer" group. When focus is moved to a window that does not belong to any group, a window opens with the question "Which group does this window belong to?" (the so-called "Question" window).

### Functions:
Window names are stored in a list. The list consists of entries. Each entry contains 3 properties:
+ The name of the window or part of it.
+ Number of the group to which the window belongs.
+ This property determines what is stored in property 1: either this is the entire window name or part of the window name.
All three properties can be configured in the "Question" window.

## Русская документация

### Логика работы:
Это простой счетчик времени. В программе есть несколько таймеров:
+ Таймер рабочего времени.
+ Таймер времени отдыха.
+ Таймер занятий прочими делами.

Их значение отображается на главном экране программы. Таймера запускаются поочередно в зависимости отимени текущего окна в фокусе. Имена окон делятся на 3 группы, соответствующие каждому таймеру + 1 группа под названием "стоп-таймер". Окно программы с именем "Work calculate" относится к группе "стоп-таймер". При переводе фокуса на окно, не относящееся ни к одной группе открывается окно с вопросом "К какой группе отнести это окно?" (так называемое окно "Вопроса").

### Функции:
Имена окон харнятся в списке. Список состоит из записей. Каждая запись содержит 3 свойства:
+ Имя окна или его части.
+ Номер группы к которой относится окно.
+ Это свойство определяет, что харнится в свойстве 1: либо это имя окна целиком, либо часть имени окна.
Все три свойства можно настроить в окне "Вопроса".
