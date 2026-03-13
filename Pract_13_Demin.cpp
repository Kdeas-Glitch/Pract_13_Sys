#include <iostream>
#include <Windows.h>
#include <random>
#include <conio.h>
#include <iomanip>
#include <string>

struct Player {
    long health = 500000;
    int damage = 15000;
    int specialDamage = 32000;
    int attackCooldown = 2;
    int specialCooldown = 5;
    int defense = 20;
    int dodgeChance = 15;
    char name[64];
    long finaldamage=0;
};

struct Bayum {
    long long health = 90000000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 120000;
    int attackCooldown = 5;
    int specialCooldown = 10;
    long long fdamage;
};

bool Final = false;
bool FinalSAD = false;
int CountOfPlayers;
int currentpl = 0;
Player players[10];
Bayum BOSSKFC;
CRITICAL_SECTION cs;
CRITICAL_SECTION csboss;

DWORD WINAPI DamageBosss(LPVOID damage) {
    HANDLE hEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "BossAttack"
    );
    if (hEvent == NULL) {
        return 0;
    }
    while (BOSSKFC.health > 0) {
        DWORD res = WaitForSingleObject(hEvent, 15);
        WaitForSingleObject(hEvent, INFINITE);
        if (res != WAIT_TIMEOUT) {
            players[currentpl].health = players[currentpl].health - (int)damage * (100 - players[currentpl].defense) / 100;
            BOSSKFC.fdamage += (int)damage;
        }
    }
    return 0;
}


DWORD WINAPI DamageBosssSuper(LPVOID damage) {
    HANDLE hEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "BossAttackSuper"
    );
    if (hEvent == NULL) {
        return 0;
    }
    while (BOSSKFC.health > 0) {
        DWORD res = WaitForSingleObject(hEvent, 15);
        if (res != WAIT_TIMEOUT) {
            for (int j = 0; j < CountOfPlayers; j++) {
                if (players[j].health > 0) {
                    int dodge = rand() % 100;
                    if (dodge > players[currentpl].dodgeChance) {
                        players[j].health -= (int)damage;      
                        BOSSKFC.fdamage += (int)damage;
                    }
                }
            }
        }
    }
    return 0;
}




void DamageToBoss(int damage,int count) {
    HANDLE hEvent = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "PlayersAttack"
    );
    if (hEvent == NULL) {
        return ;
    }
    WaitForSingleObject(hEvent, INFINITE);
    BOSSKFC.health = BOSSKFC.health - damage * (100 - BOSSKFC.resist) / 100;
    players[count].finaldamage += damage * (100 - BOSSKFC.resist) / 100;
    if (BOSSKFC.health <= 0) {
        Final = true;
        BOSSKFC.health = 0;
    }
    SetEvent(hEvent);
}

DWORD WINAPI BOSS() {
    STARTUPINFO si;
    HANDLE hThread[2];
    DWORD IDThread[2];
    int supdamage = (BOSSKFC.specialDamage * (1 - 0.05 * (CountOfPlayers - 1)));
    hThread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DamageBosss, (void*)supdamage, HANDLE_FLAG_INHERIT, &IDThread[0]);
    if (hThread[0] == NULL) {
        return GetLastError();
    }
    hThread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DamageBosssSuper, (void*)BOSSKFC.damage, HANDLE_FLAG_INHERIT, &IDThread[1]);
    if (hThread[1] == NULL) {
        return GetLastError();
    }
    EnterCriticalSection(&csboss);
    Sleep(100);
    HANDLE hEventPlayer = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "PlayersAttack"
    );
    HANDLE hEventDamage = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "BossAttack"
    );
    HANDLE hEventSuperDamage = OpenEventA(
        EVENT_ALL_ACCESS,
        FALSE,
        "BossAttackSuper"
    );
    if (hEventPlayer == NULL) {
        Final = true;
        return 0;
    }
    if (hEventDamage == NULL) {
        Final = true;
        return 0;
    }
    if (hEventSuperDamage == NULL) {
        Final = true;
        return 0;
    }
    int i = 0;

    int countbasic = GetTickCount64();

    int countspec = GetTickCount64();
    int alive = 0;
    while (BOSSKFC.health > 0&&!FinalSAD) {
        for (int i = 0; i < 10; i++) {
            if (players[i].health > 0) {
                alive++;
            }
        }
        if (alive == 0) {
            FinalSAD = true;
            break;
        }
        while (true) {
            i = rand() % 10;
            if (players[i].health > 0) {
                currentpl = i;
                break;
            }
        }
        int realcount = GetTickCount64();
        if ((realcount - countspec) > BOSSKFC.specialCooldown*100) {
            PulseEvent(hEventSuperDamage);
            PulseEvent(hEventPlayer);
            countspec=GetTickCount64();
        }
        else
        if ((realcount - countbasic) > BOSSKFC.attackCooldown*100) {
            int dodge = rand() % 100;
            if (dodge > players[i].dodgeChance) {
                PulseEvent(hEventDamage);
            }
            countbasic = GetTickCount64();
        }
        else {
            Sleep(1);
        }
        alive = 0;
    }
    CloseHandle(hThread[0]);
    CloseHandle(hThread[1]);
    LeaveCriticalSection(&csboss);
    return 0;
};

DWORD WINAPI player(LPVOID pl) {
    Sleep(1);
    int count = (int)pl;
    if (count == 5) {
        players[count].finaldamage++;
    }
    int countbasic = GetTickCount64();
    int countspec=GetTickCount64();


    while (players[count].health > 0 && !Final) {

        int realcount = GetTickCount64();
        if (realcount - countspec> players[count].specialCooldown) {
            DamageToBoss(players[count].specialDamage, count);
            countspec = GetTickCount64();

        }
        else if(realcount-countbasic> players[count].attackCooldown) {
            DamageToBoss(players[count].damage, count);
            countbasic = GetTickCount64();
        }
        /*else{
            Sleep(1);
            if (realcount - countbasic - players[count].attackCooldown > 0)
            {
                std::cout << "cast sadasdasdasdsadasd";
                Sleep(realcount - countbasic - players[count].attackCooldown);
            }
            DamageToBoss(players[count].damage, count);
            countbasic = GetTickCount64();

        }*/
    }
    int counts = 0;
    for (int i = 0; i < 10; i++) {
        if (players[i].health <= 0) {
            counts++;
        }
    }
    if (counts == 10) {
        FinalSAD = true;
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
        if (hThread[i] == NULL) {
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
    HANDLE hEventBoss = CreateEventA(
        NULL,
        TRUE,
        FALSE,
        "BossAttack"
    );
    HANDLE hEventBossSuper = CreateEventA(
        NULL,
        TRUE,
        FALSE,
        "BossAttackSuper"
    );

    if (hEvent == NULL) {
        return GetLastError();
    }
    if (hEventBoss == NULL) {
        return GetLastError();
    }
    if (hEventBossSuper == NULL) {
        return GetLastError();
    }
    Sleep(20);
    while(!Final&&!FinalSAD) {
        for (int i = 0; i < CountOfPlayers; i++) {
            std::cout << std::setw(2) << i+1 <<" игрок" << "      ХП: " << players[i].health << "      УРОН: " << players[i].finaldamage << std::endl ;
        }
        std::cout << std::endl;
        std::cout <<"ХП Босса:     " << BOSSKFC.health << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        Sleep(1000);
    }
    if (FinalSAD) {
        std::cout << "LLL Проигрыш!!!" << std::endl;
        std::cout << "MVP's:" << std::endl;
        std::cout << "1: BOSSKFC" " " << BOSSKFC.fdamage << std::endl;
    }
    else {
        winners[0] = players[0];
        winners[1] = players[1];
        winners[2] = players[2];
        std::cout << "WWW ПОБЕДА!!!" << std::endl;
        for (int i = 0; i < CountOfPlayers; i++) {
            if (winners[0].finaldamage < players[i].finaldamage) {
                winners[2] = winners[1];
                winners[1] = winners[0];
                winners[0] = players[i];
            }

        }
        std::cout << "MVP's:" << std::endl;
        std::cout << "1: " << winners[0].name << " " << winners[0].finaldamage << std::endl;
        std::cout << "2: " << winners[1].name << " " << winners[1].finaldamage << std::endl;
        std::cout << "3: " << winners[2].name << " " << winners[2].finaldamage << std::endl;
    }
    WaitForMultipleObjects(CountOfPlayers,hThread,TRUE,INFINITE);
    WaitForSingleObject(hBOSS,INFINITE);
    for (int i = 0; i < 10; i++) {
        CloseHandle(hThread[i]);
    }
    CloseHandle(hBOSS);
    DeleteCriticalSection(&cs);
    DeleteCriticalSection(&csboss);
    CloseHandle(hEvent);
    CloseHandle(hEventBoss);
    CloseHandle(hEventBossSuper);
}