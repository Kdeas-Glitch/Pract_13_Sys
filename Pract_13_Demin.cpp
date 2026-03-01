#include <iostream>
#include <Windows.h>
#include <random>
#include <iomanip>
#include <string>

struct Player {
    long health = 500000;
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
    long long health = 900000000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldown = 5;
    int specialCooldown = 10;
};

bool Final = false;
int CountOfPlayers;
Player players[10];
Bayum BOSSKFC;
CRITICAL_SECTION cs;
CRITICAL_SECTION csboss;

void DamageBoss(int damage,int count) {
    
    HANDLE hEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "PlayersAttack"
    );
    if (hEvent == NULL) {
        return ;
    }
    WaitForSingleObject(hEvent, INFINITE);
    ResetEvent(hEvent);
    BOSSKFC.health = BOSSKFC.health - damage * (100 - BOSSKFC.resist) / 100;
    players[count].finaldamage += damage * (100 - BOSSKFC.resist) / 100;
    if (BOSSKFC.health <= 0) {
        Final = true;
        BOSSKFC.health = 0;
    }
    ResetEvent(hEvent);

}

DWORD WINAPI BOSS() {
    HANDLE hEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "PlayersAttack"
    );
    Sleep(100);
    if (hEvent == NULL) {
        return 0;
    }
    EnterCriticalSection(&csboss);

    int i = 0;

    int countbasic = GetTickCount64();

    int countspec = GetTickCount64();

    while (BOSSKFC.health > 0) {
        while (true) {
            i = rand() % 10;
            if (players[i].health > 0) {
                break;
            }
        }
        int realcount = GetTickCount64();
        if (realcount - countspec > BOSSKFC.specialCooldown) {
            for (int j = 0; j < CountOfPlayers; j++) {
                players[j].health -= BOSSKFC.specialDamage * (1 - 0.05 * (CountOfPlayers - 1));
            }
            SetEvent(hEvent);
        }
        if (realcount - countbasic > BOSSKFC.attackCooldown*100) {
            int dodge = rand() % 100;
            if (dodge > players[i].dodgeChance) {
                players[i].health = players[i].health-BOSSKFC.damage * (100 - players[i].defense) / 100;
            }
            countbasic = GetTickCount64();
        }
        else {
            Sleep(1);
        }
    }

    LeaveCriticalSection(&csboss);
    return 0;
};

DWORD WINAPI player(LPVOID pl) {
    int count = (int)pl;
    if (count == 5) {
        players[count].finaldamage++;
    }
    int countbasic = GetTickCount64();
    int countspec=GetTickCount64();


    while (players[count].health > 0 && !Final) {

        int realcount = GetTickCount64();
        if (realcount - countspec> players[count].specialCooldown) {
            DamageBoss(players[count].specialDamage, count);
            countspec = GetTickCount64();

        }
        else if(realcount-countbasic> players[count].attackCooldown) {
            DamageBoss(players[count].damage, count);
            countbasic = GetTickCount64();
        }
        /*else{
            Sleep(1);
            if (realcount - countbasic - players[count].attackCooldown > 0)
            {
                std::cout << "cast sadasdasdasdsadasd";
                Sleep(realcount - countbasic - players[count].attackCooldown);
            }
            DamageBoss(players[count].damage, count);
            countbasic = GetTickCount64();

        }*/
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
    Player winners[3];
    for (int i = 0; i < CountOfPlayers; i++) {
        std::string str = std::to_string(i+1) + " игрок";
        strcpy_s(players[i].name, str.c_str());
        hThread[i] = CreateThread(NULL, 0, player, (void*)i, 0, &IDThread[i]);
        if (hThread == NULL) {
            return GetLastError();
        }
    }
    hBOSS= CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BOSS, NULL, 0, &IDBOSS);
    if (hBOSS == NULL) {
        return GetLastError();
    }
    HANDLE hEvent = CreateEventA(
        NULL,
        TRUE,
        TRUE,
        "PlayersAttack"
    );

    if (hEvent == NULL) {
        return GetLastError();
    }
    Sleep(20);
    while(!Final) {
        for (int i = 0; i < CountOfPlayers; i++) {
            std::cout << std::setw(2) << i+1 <<" игрок" << "      ХП: " << players[i].health << "      УРОН: " << players[i].finaldamage << std::endl ;
        }
        std::cout << std::endl;
        std::cout <<"ХП Босса:     " << BOSSKFC.health << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        Sleep(1000);
    }
    WaitForMultipleObjects(CountOfPlayers,hThread,TRUE,INFINITE);
    WaitForSingleObject(hBOSS,INFINITE);
    winners[0] = players[0];
    winners[1] = players[1];
    winners[2] = players[2];
    std::cout << "WWW ПОБЕДА!!!"<<std::endl;
    for (int i = 0; i < CountOfPlayers; i++) {
        if (winners[0].finaldamage < players[i].finaldamage) {
            winners[2] = winners[1];
            winners[1] = winners[0];
            winners[0] = players[i];
       }

    }
    std::cout << "MVP's:" << std::endl;
    std::cout<<"1: " << winners[0].name << " " << winners[0].finaldamage << std::endl;
    std::cout<<"2: " << winners[1].name << " " << winners[1].finaldamage << std::endl;
    std::cout<<"3: " << winners[2].name << " " << winners[2].finaldamage << std::endl;
    DeleteCriticalSection(&cs);
    DeleteCriticalSection(&csboss);
}