#include<iostream>
#include<windows.h>
#include<conio.h>
#include<thread>
using namespace std;
using namespace std::chrono_literals;

#define MIN_TANK_VOLUME 20
#define MAX_TANK_VOLUME 120
//директива define (Определить) создаёт макроопределение (макрос)

class Tank
{
	const int VOLUME;
	double fuel_level;
public:
	const int get_VOLUME()const
	{
		return VOLUME;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}
	void fill(double amount)
	{
		if (amount < 0)
			return;
		if (fuel_level + amount < VOLUME)
		{
			fuel_level += amount;
		}
		else
		{
			fuel_level = VOLUME;
		}
	}
	double give_fuel(double amount)
	{
		if (amount < 0)
			return fuel_level;
		fuel_level -= amount;
		if (fuel_level < 0)
			fuel_level = 0;
		return fuel_level;
	}
	Tank(int volume):VOLUME(
		volume<MIN_TANK_VOLUME?MIN_TANK_VOLUME:
		volume>MAX_TANK_VOLUME?MAX_TANK_VOLUME:
		volume)
	{
		fuel_level = 0;
		cout << "Tank is ready " << this << endl;
	}
	~Tank()
	{
		cout << "Tank is over: " << this << endl;
	}
	void info()const
	{
		cout << "Tank volume: " << VOLUME << " liters.\n";
		cout << "Fuel level: " << fuel_level << " liters.\n";
	}
};

#define MIN_ENGINE_CONSUMPTION 3
#define MAX_ENGINE_CONSUMPTION 30
class Engine
{
	const double CONSUMPTION;
	double consumption_per_second;
	bool isStarted;
public:
	double get_consumption()const
	{
		return CONSUMPTION;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	Engine(double consumption) 
		:CONSUMPTION(
			consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION: 
			consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION: 
			consumption)
	{
		this->consumption_per_second = CONSUMPTION * 3e-5;
		isStarted = false;
		cout << "Engine is ready: " << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over: " << this << endl;
	}
	void start()
	{
		isStarted = true;
	}
	void stop()
	{
		isStarted = false;
	}
	bool started()const
	{
		return isStarted;
	}
	void info()const
	{
		cout << "Engine status: " << (isStarted ? "started" : "not stated") << endl;
		cout << "Consumpition: " << CONSUMPTION << " liters/100km" << endl;
		cout << "Consumpition: " << consumption_per_second << " liters per second" << endl;
	}
};

#define Enter 13
#define Escape 27

#define MAX_SPEED_LOW_LIMIT		50
#define MAX_SPEED_HIGH_LIMIT	447
class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
	struct
	{
		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread free_wheeling_thread;
	}threads;
	int speed;	
	const int MAX_SPEED;
	const int ACCELERATION;
public:
	Car(int consumption = 10, int volume = 60, int max_speed = 250) :
		engine(consumption),
		tank(volume),
		driver_inside(false),
		speed(0),
		ACCELERATION(MAX_SPEED / 20),
		MAX_SPEED
		(
			max_speed < MAX_SPEED_LOW_LIMIT ? MAX_SPEED_LOW_LIMIT :
			max_speed > MAX_SPEED_HIGH_LIMIT ? MAX_SPEED_HIGH_LIMIT :
			max_speed
		)		
	{
		cout << "Car is ready " << this << endl;
	}
	~Car()
	{
		cout << "Car is over" << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		if (speed == 0)
		{
			driver_inside = false;
			if (threads.panel_thread.joinable())
				threads.panel_thread.join();
			system("cls");
			cout << "Out of the car" << endl;
		}			
	}
	void start()
	{
		if (tank.get_fuel_level() && driver_inside)
		{
			engine.start();
			threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}		
	}
	void stop()
	{
		engine.stop();
		if (threads.engine_idle_thread.joinable())
			threads.engine_idle_thread.join();
	}
	void accelerate()
	{
		if (engine.started())
		{
			if (!threads.free_wheeling_thread.joinable())
				threads.free_wheeling_thread = thread(&Car::free_wheeling, this);
			speed += ACCELERATION;
			if (speed > MAX_SPEED) speed = MAX_SPEED;			
			this_thread::sleep_for(1s);
		}
	}
	void slow_down()
	{
		if (speed - ACCELERATION < 0)
		{
			speed = 0;
			//if (threads.free_wheeling_thread.joinable())
				//threads.free_wheeling_thread.join();
		}			
		else
			speed -= ACCELERATION;		
		this_thread::sleep_for(1s);
	}
	//////////////для устранения ошибки при нажатии Esc
	void wind_stop() 
	{
		speed = 0;
		if (threads.free_wheeling_thread.joinable())
			threads.free_wheeling_thread.join();
	}
	//////////////
	void control()
	{
		char key;
		do
		{
			key = 0;
			if (_kbhit())
				key = _getch();
			switch (key)
			{
			case 'F':
			case 'f': 
			{
				if (driver_inside)
				{
					cout << "Get out of car!\a" << endl;
				}
				else
				{
					double fuel;
					cout << "Input fuel amount: "; 
					cin >> fuel;
					tank.fill(fuel);
				}
			}
			break;
			case Enter: driver_inside ? get_out() : get_in(); break;
			case 'I': case 'i': engine.started() ? stop() : start(); break;
			case 'W': case 'w': accelerate(); break;
			case 'S': case 's': slow_down(); break;
			case Escape:speed = 0; stop(); get_out();  break;
			}
			if (tank.get_fuel_level() == 0)
				stop();
			if (speed == 0)
				wind_stop();
		} while (key != 27);
	}
	void free_wheeling()
	{
		while (speed)
		{
			if (speed - 1 < 0)
				speed = 0;
			else
				speed--;
			this_thread::sleep_for(1s);						
		}		
	}
	double calculate_current_cons()
	{
		double current_consumption = 0;
		if (speed == 0)
			current_consumption = 0;
		else if (speed >= 1 && speed <= 60)
			current_consumption = 0.002;
		else if (speed >= 61 && speed <= 100)
			current_consumption = 0.0014;
		else if (speed >= 101 && speed <= 140)
			current_consumption = 0.002;
		else if (speed >= 141 && speed <= 200)
			current_consumption = 0.0025;
		else if (speed >= 201 && speed <= 250)
			current_consumption = 0.003;
		else if (speed >= 251)
			current_consumption = 0.004;
		return current_consumption;
	}
	void engine_idle()
	{		
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_second() + calculate_current_cons()))
		{
			this_thread::sleep_for(1s);
		}
	}
	void panel()const
	{
		while (driver_inside)
		{
			system("cls");	
			//cout << "CURRENT CONSUMPTION: " <<  << endl;
			for (int i = 0; i < speed; i++)	cout << "|";
			cout << endl;
			cout << "Speed: " << speed << " km/h\n";
			cout << "Engine is " << (engine.started() ? "started" : "not started") << endl;
			cout << "Fuel level:\t" << tank.get_fuel_level() << " liters.\t";
			if (tank.get_fuel_level() < 5)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 0xCF);
				cout << " LOW FUEL " << endl;
				SetConsoleTextAttribute(hConsole, 0x0F);
			}
			else
			{
				cout << endl;
			}
			//Sleep(300)
			std::this_thread::sleep_for(100ms);
		}
		return;
	}
	void info()const
	{
		engine.info();
		tank.info();
	}
};

//int check_volume(int volume)
//{
//	if (volume > MAX_TANK_VOLUME)
//		volume = MAX_TANK_VOLUME;
//	if (volume < MIN_TANK_VOLUME)
//		volume = MIN_TANK_VOLUME;
//	return volume;
//}

//#define TANK_CHECK
//#define ENGINE

void main()
{
	setlocale(LC_ALL, "");	
#if defined TANK_CHECK
	//если определено TANK_CHECK, то следующий код до дерективы #endif будет виден компилятору
	Tank tank(check_volume(150));
	int fuel;
	do
	{
		cout << "Enter fuel amount: "; cin >> fuel;
		tank.fill(fuel);
		tank.info();
	} while (fuel > 0);
#endif	// TANK_CHECK
#ifdef ENGINE
	Engine engine(10);
	engine.info();
#endif //ENGINE
	Car bmw;
	bmw.info();
	bmw.control();
}