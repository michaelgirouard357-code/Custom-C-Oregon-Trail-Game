#pragma once
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <limits>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <array>
#include <SFML/Graphics.hpp>
#include "Party Member.h"
using namespace std;
struct Inventory {
	int foodPounds = 0;
	int bullets = 0;
	int clothingSets = 0;
	int oxen = 0;
	int spareParts = 0;
	int medicineKits = 0;
	int woodPounds = 0;
};
struct NPC {
	string name;
	string role; // "Trader", "Healer", "Storyteller"
};
void showImage(const string& path, const string& caption = "");
int supplyChoiceFunction();
void meetLocals(Inventory& inv, vector<PartyMember>& party,
	mt19937& rng, int& startingGold);
void renderScene(const Inventory& inv, const vector<PartyMember>& party, bool wagonBroken);
void displaySettlementArt(bool isMedicalStation);
void displayDayCounter(int totalDays);
void consumeClothingForSeverity(Inventory& inv, int prevSeverity, int newSeverity, const string& name);
void consumeClothingForDeath(Inventory& inv, const string& name);
void displayInventory(const Inventory& inv, int startingGold);
