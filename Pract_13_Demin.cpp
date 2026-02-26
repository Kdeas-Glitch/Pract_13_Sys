#include <iostream>
#include <Windows.h>
#include <random>
#include <iomanip>
#include <string>

struct Player {
    long  health = 500000;
    int damage = 12000;
    int specialDamage = 30000;
    int attackCooldown = 2;
    int specialCooldown = 5;
    int defense = 20;
    int dodgeChance = 15;
    char name[64];
    long finaldamage=0;
};
struct Bayum {
    long long health = 9000000000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldown = 5;
    int specialCooldown = 10;
};
bool Final = false;
int CountOfPlayers;
Player players[10];
Bayum KFCBOSS;
CRITICAL_SECTION cs;
CRITICAL_SECTION csboss;
DWORD WINAPI BOSS(LPVOID bs) {
    Sleep(100);
    EnterCriticalSection(&csboss);
    Bayum boss = *static_cast<Bayum*>(bs);
    std::cout << boss.health << std::endl;
    LeaveCriticalSection(&csboss);
    return 0;
};
DWORD WINAPI player(LPVOID pl) {
    Player* play = static_cast<Player*>(pl);
    HANDLE hEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE, 
        "PlayersAttack"
    );
    if (hEvent == NULL) {
        return GetLastError();
    }
    SetEvent(hEvent);
    int countbasic = GetTickCount64();
    int countspec=GetTickCount64();
    while (play->health > 0&&!Final) {
        WaitForSingleObject(hEvent, INFINITE);
        EnterCriticalSection(&cs);
        int realcount = GetTickCount64();
        if (realcount - countspec> play->specialCooldown) {
            KFCBOSS.health = KFCBOSS.health - play->specialDamage * (100 - KFCBOSS.resist) / 100;
            play->finaldamage= play->finaldamage+(play->specialDamage * (100 - KFCBOSS.resist) / 100);
            countspec = GetTickCount64();
        }
        else if(realcount-countbasic>play->attackCooldown) {
            KFCBOSS.health = KFCBOSS.health - play->damage * (100 - KFCBOSS.resist) / 100;
            play->finaldamage = play->finaldamage + (play->damage * (100 - KFCBOSS.resist) / 100);
            countbasic = GetTickCount64();
        }
        else{
            if(realcount - countbasic - play->attackCooldown>0)
            Sleep(realcount - countbasic - play->attackCooldown);
            else
            {
                Sleep(1);
            }
            KFCBOSS.health = KFCBOSS.health- play->damage * (100 - KFCBOSS.resist) / 100;
            play->finaldamage = play->finaldamage + (play->damage * (100 - KFCBOSS.resist) / 100);
            countbasic = GetTickCount64();
        }
        LeaveCriticalSection(&cs);
        SetEvent(hEvent);
    }
    return 0;
};


int main()
{
    setlocale(0, "rus");
    InitializeCriticalSection(&cs);
    InitializeCriticalSection(&csboss);
    std::cout << "Введите колличество игроков 1-10:";
    std::cin >> CountOfPlayers;
    if (CountOfPlayers < 1 || CountOfPlayers>10) {
        std::cout << "Ну сказанно же 1-10!";
        return 0;
    }
    HANDLE hThread[10];
    DWORD IDThread[10];

    HANDLE hBOSS;
    DWORD IDBOSS;
    for (int i = 0; i < CountOfPlayers; i++) {
        std::string str = std::to_string(i) + " игрок";
        strcpy_s(players[i].name, str.c_str());
        hThread[i] = CreateThread(NULL, 0, player, (void*)&players[i], 0, &IDThread[i]);
        if (hThread == NULL) {
            return GetLastError();
        }
        SetThreadPriority(hThread[i], THREAD_PRIORITY_BELOW_NORMAL);
    }
    hBOSS= CreateThread(NULL, 0, BOSS, (void*)&KFCBOSS, 0, &IDBOSS);
    if (hBOSS == NULL) {
        return GetLastError();
    }
    HANDLE hEvent = CreateEventA(
        NULL,
        FALSE,
        FALSE,
        "PlayersAttack"
    );

    if (hEvent == NULL) {
        return GetLastError();
    }
    while(!Final) {
        for (int i = 0; i < CountOfPlayers; i++) {
            std::cout << players[i].name<< " " << players[i].health<<" " << players[i].finaldamage << std::endl;
        }

        std::cout << KFCBOSS.health << std::endl;
        Sleep(1000);
    }
    WaitForMultipleObjects(CountOfPlayers,hThread,TRUE,INFINITE);
    WaitForSingleObject(hBOSS,INFINITE);

    DeleteCriticalSection(&cs);
    DeleteCriticalSection(&csboss);
}