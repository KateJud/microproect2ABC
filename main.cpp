#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <ctime>
#include <cstdlib>
//#include "windows.h"
#include <condition_variable>
#include <utility>

std::mutex m; // Мютекс
std::condition_variable cv;//Для wait()
//Дон Фредерико
//Дон Эрнардо
class Player {
    bool wasAttacker;
    bool lose = false;
    std::string name;
    double koeff;
    //[1000-5000]$
    int capital;//Начальный капитал генерируется в конструкторе
    //[100-500]ед здоровья
    int health;//Начальное здоровье генерируется в конструкторе

    static void writeThread(const std::string &threadName);//Выводит имя текущего потока на экран
    static void writeLose(Player *loser);//Информирует пользовател о проигрыше одного из донов
    void getDamage();//Наносит урон здоровью this
    void spendMoneyForAttack();//Тратит деньги на атаку
    std::string display();//Выводит информацию о текущем икроке на экран

public:
    explicit Player(std::string name);//Конструктор
    void attack(Player *defender, const std::string &threadName);//Атака
    bool isLose() const;//Проиграл или нет
    bool getWasAttacker() const;//Атаковал ли в предыдущем потоке?
};

void Player::writeThread(const std::string &threadName) {

    //Устанавливаем голубой цвет
//    HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
//    SetConsoleTextAttribute(hConsoleHandle, FOREGROUND_RED | FOREGROUND_BLUE);FOREGROUND_BLUE
    std::cout << threadName << std::endl;
    //Устанавливаем обратно белый цвет
//    SetConsoleTextAttribute(hConsoleHandle,
//                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void Player::writeLose(Player *loser) {

    //Устанавливаем красный цвет
    loser->lose = true;
//    HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
//    SetConsoleTextAttribute(hConsoleHandle, FOREGROUND_RED);
    std::cout << loser->name << " lose &_&" << std::endl;
    //Устанавливаем обратно белый
    // цвет
//    SetConsoleTextAttribute(hConsoleHandle,
//                            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

///Наносим урон здоровью this
void Player::getDamage() {
    health -= (int) koeff * 10;
    if (health <= 0) {
        writeLose(this);
    }
}

std::string Player::display() {
    return name +
           "\n\tcurrent health= " + std::to_string(health) +
           "\n\tcurrent capital= " + std::to_string(capital);
}

void Player::spendMoneyForAttack() {
    capital -= (int) koeff * 10;
    //Проигрыш
    if (capital <= 0) {
        writeLose(this);
    }
}


Player::Player(std::string name) {
    this->name = std::move(name);
    srand(time(nullptr)) ;
    health = rand() % (500 - 100) + 100; // диапазон равен от 100 до 500 включительно;
    capital = rand() % (5000 - 1000) + 1000;// диапазон равен от 1000 до 5000 включительно;
    koeff = 1.5;
    lose = false;//Не проиграли
    wasAttacker = true;//Устанавливаем true,чтобы любой из двух потоков начал свою детельность
}

void Player::attack(Player *defender, const std::string &threadName) {
    std::unique_lock<std::mutex> lk(m);
    //Ждем пока поток предыдущий поток не отработает (те defender must be attacker in previous action)
    while (!defender->getWasAttacker()) {
        cv.wait(lk);
    }
    //Проверяем еще раз на корректность
    if (!(this->isLose() || defender->isLose())) {
        writeThread(threadName);//Пишем имя потока
        spendMoneyForAttack();//Тратим деньги на атаку
        //С вероятностью 0.5 попадает в пративника и ранит его
        if ((rand() % 100) / 100.0 < 0.5) {
            defender->getDamage();
            //вывод инфы на экран
            std::cout << this->display() << " have attacked $_$" << std::endl;
            std::cout << defender->display() << " is defender o_o " << std::endl;//o_o
        } else {
            //вывод инфы на экран[или выводим в damage]
            std::cout << this->display() << " tried to attack but without fortune 0_o" << std::endl;
            std::cout << defender->display() << " is defender but nothing happened *_*" << std::endl;
        }
        //Меняем параметры которые регулируют работу потоков [чтоб потоки работали по очереди]
        wasAttacker = true;
        defender->wasAttacker = false;
    }
    //открываем все
    lk.unlock();
    cv.notify_all();
    // Sleep(100);
}

bool Player::isLose() const {
    return lose;
}

bool Player::getWasAttacker() const {
    return wasAttacker;
}

///Класс для функции потока
class myClass {
public:
    static void run(Player *attacker, Player *defender, const std::string &threadName);
};

void myClass::run(Player *attacker, Player *defender, const std::string &threadName) {
    //Потоки работают пока кто-то из донов не проиграет
    while (!(attacker->isLose() || defender->isLose())) {
        attacker->attack(defender, threadName);
    }
}

int main() {

    //Два дона
    //Наугад обстреливают
    //Стреляют пока не уничтожены цели
    // либо стоимость потраченных снарядов не привысит свой капитал
    //
    auto *an = new Player("Anchuari");
    auto *tar = new Player("Taranteri");
    std::thread tA(myClass::run, an, tar, "Thread1 ");//Поток в котором Анчуари нападает на Тарантери
    std::thread tt(myClass::run, tar, an, "Thread2 ");//Поток в котором Тарантери нападает на Тарантери Анчуари
    //Пока кто-то не проиграет

    //Объединяем потоки
    tA.join();
    tt.join();

    delete an;
    delete tar;
    std::cout << "I am finished to work!" << std::endl;
    return 0;
}
