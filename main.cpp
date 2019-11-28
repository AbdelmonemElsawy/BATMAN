#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>

using namespace std;

void * check(void * arg);
void * execute(void * b);

class BAT
{
private:
    int number;
    string direction;
public:
    BAT(int num, string dir)
    {
        this -> number = num + 1;
        this -> direction = dir;
    }

    int get_num()
    {
        return number;
    }

    int get_direction_num()
    {
        if(direction == "North")
            return 0;

        else if (direction == "East")
            return 1;

        else if (direction == "South")
            return 2;

        return 3;// for west.
    }
    string get_direction()
    {
        return direction;
    }
};

class Monitor
{
private:
    pthread_mutex_t lock;

    //condition variables to queue BATs arriving from one direction.
    pthread_cond_t direction_queue[4];

    //condition variables to let BATs from the right have precedence to cross.
    pthread_cond_t crossing_queue[4];

    int counter = 0;
    bool crossing[4];

public:
    Monitor()
    {

        for(int i = 0; i < 4; i++)
        {
            crossing[i] = false;
            pthread_cond_init(&direction_queue[i], NULL);
            pthread_cond_init(&crossing_queue[i], NULL);
        }

        pthread_mutex_init(&lock, NULL);
    }

    void arrive(BAT bat)
    {
        pthread_mutex_lock(&lock);
        int dir = bat.get_direction_num();
        if(crossing[dir])
        {
            pthread_cond_wait(&direction_queue[dir], & lock);
        }
        crossing[dir] = true;
        cout << "BAT " << bat.get_num() << " from " << bat.get_direction() << " arrives at crossing." << endl;
        counter ++;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }

    void cross(BAT bat)
    {
        pthread_mutex_lock(&lock);
        int dir = bat.get_direction_num();
        if(crossing[(dir + 3) % 4])
        {
            pthread_cond_wait(&crossing_queue[dir], & lock);
        }
        cout << "BAT " << bat.get_num() << " from " << bat.get_direction() << " leaving crossing." << endl;
        crossing[dir] = false;
        counter --;
        sleep(1);
        pthread_mutex_unlock(&lock);
    }

    void leave(BAT bat)
    {
        pthread_mutex_lock(&lock);
        int dir = bat.get_direction_num();
        pthread_cond_signal(&crossing_queue[(dir + 1) % 4]);

        pthread_cond_signal(&direction_queue[dir]);

        pthread_mutex_unlock(&lock);
    }
    void check()
    {
        pthread_mutex_lock(&lock);
        if (counter == 4)
        {
            cout << "DEADLOCK: BAT jam detected, signaling North to go" << endl;
            pthread_cond_signal(&crossing_queue[0]);
        }
        pthread_mutex_unlock(&lock);
    }

};

int main(int argc, char **argv)
{
    if(argc > 1)
    {
        string s = (string) argv[1];
        pthread_t threads[s.size()];
        pthread_t check_thread;
        pthread_create(&check_thread, NULL, check, NULL);
        for(int i = 0; i < s.size(); i++)
        {
            BAT *bat;
            if(s[i] == 'n')
                bat = new BAT(i, "North");
            else if(s[i] == 's')
                bat = new BAT(i, "South");
            else if(s[i] == 'e')
                bat = new BAT(i, "East");
            else
                bat = new BAT(i, "West");
            pthread_create(&threads[i],NULL, &execute,bat);
        }

        for(int i = 0; i < s.size(); i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
    return 0;
}

Monitor BatMan;

void * execute(void * b)
{
    BAT bat = *(BAT *) b;
    BatMan.arrive(bat);
    BatMan.cross(bat);
    BatMan.leave(bat);
}

void * check(void * arg)
{
    while(true)
    {
        BatMan.check();
        sleep(3);
    }
}
