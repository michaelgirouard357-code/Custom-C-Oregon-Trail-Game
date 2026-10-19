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
#include "Functions.h"
// Enable ANSI color support on Windows consoles
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#undef max
#undef min
using namespace std;
void showImage(const string& path, const string& caption = "") {
	sf::Texture texture;
	if (!texture.loadFromFile(path)) {
		cout << "(Could not load image: " << path << ")" << endl;
		return;
	}
	sf::Sprite sprite(texture);
	sf::RenderWindow window(sf::VideoMode(texture.getSize()), caption.empty() ? "Oregon Trail" : caption);
	while (window.isOpen()) {
		while (const std::optional<sf::Event> event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>() || event->is<sf::Event::KeyPressed>())
				window.close();
		}
		window.clear();
		window.draw(sprite);
		window.display();
	}
}
int supplyChoiceFunction() {
	cout << "Please select the category of supplies you would like to purchase: " << endl;
	cout << "1. Food" << endl;
	cout << "2. Ammunition" << endl;
	cout << "3. Clothing" << endl;
	cout << "4. Oxen" << endl;
	cout << "5. Spare Parts" << endl;
	cout << "7. Medicine" << endl;
	cout << "6. View inventory" << endl;
	cout << "0. Finish shopping" << endl;
	int supplyChoice;
	cin >> supplyChoice;
	return supplyChoice;
}
void meetLocals(Inventory& inv, vector<PartyMember>& party,
	mt19937& rng, int& startingGold) {
	vector<string> names = { "Martha","Elias","Jonah","Agnes","Samuel","Lydia","Hank","Beatrice","Clara","Walter" };
	shuffle(names.begin(), names.end(), rng);
	int npcCount = 3;
	vector<NPC> locals;
	for (int i = 0; i < npcCount; ++i) {
		NPC n;
		n.name = names[i];
		int rolePick = rng() % 3;
		n.role = (rolePick == 0) ? "Trader" : (rolePick == 1) ? "Healer" : "Storyteller";
		locals.push_back(n);
	}
	cout << "You walk into the marketplace and see some locals:" << endl;
	PlaySound(TEXT("C:\\Windows\\Media\\chord.wav"), NULL, SND_FILENAME | SND_ASYNC);
	for (int i = 0; i < (int)locals.size(); ++i) cout << i + 1 << ") " << locals[i].name << " (" << locals[i].role << ")" << endl;
	cout << "Enter number to talk or 0 to step away: ";
	string pick; getline(cin, pick);
	int idx = -1; try { idx = stoi(pick) - 1; }
	catch (...) { idx = -1; }
	if (idx < 0 || idx >= (int)locals.size()) { cout << "You decide not to engage with the locals." << endl; return; }
	NPC& npc = locals[idx];
	cout << "You approach " << npc.name << " the " << npc.role << "." << endl;
	if (npc.role == "Trader") {
		// Trader offers a small randomized deal
		int offerType = rng() % 4; // 0=food,1=bullets,2=spare,3=medicine
		if (offerType == 0) {
			int amt = (rng() % 3 + 1) * 25; int cost = amt / 2; // cheap food
			cout << npc.name << ": I can sell you " << amt << " lbs of food for " << cost << " gold. Buy? (y/n): ";
			string a; getline(cin, a);
			if (!a.empty() && (a == "y" || a == "Y") && startingGold >= cost) { inv.foodPounds += amt; startingGold -= cost; cout << "Purchased " << amt << " lbs of food." << endl; }
			else cout << "No deal made." << endl;
		}
		else if (offerType == 1) {
			int amt = (rng() % 3 + 1) * 10; int cost = amt / 2;
			cout << npc.name << ": I have " << amt << " bullets for " << cost << " gold. Buy? (y/n): "; string a; getline(cin, a);
			if (!a.empty() && (a == "y" || a == "Y") && startingGold >= cost) { inv.bullets += amt; startingGold -= cost; cout << "Purchased " << amt << " bullets." << endl; }
			else cout << "No deal made." << endl;
		}
		else if (offerType == 2) {
			int amt = 1 + (rng() % 2); int cost = 40 * amt;
			cout << npc.name << ": Spare parts, " << amt << " for " << cost << " gold. Buy? (y/n): "; string a; getline(cin, a);
			if (!a.empty() && (a == "y" || a == "Y") && startingGold >= cost) { inv.spareParts += amt; startingGold -= cost; cout << "Purchased " << amt << " spare parts." << endl; }
			else cout << "No deal made." << endl;
		}
		else {
			int amt = 1 + (rng() % 3); int cost = 18 * amt;
			cout << npc.name << ": I can sell " << amt << " medicine kits for " << cost << " gold. Buy? (y/n): "; string a; getline(cin, a);
			if (!a.empty() && (a == "y" || a == "Y") && startingGold >= cost) { inv.medicineKits += amt; startingGold -= cost; cout << "Purchased " << amt << " medicine kits." << endl; }
			else cout << "No deal made." << endl;
		}
	}
	else if (npc.role == "Healer") {
		// Healer can treat a sick member for a fee
		bool anySick = false; for (size_t i = 0; i < party.size(); ++i) if (party[i].alive && party[i].severity > 0) anySick = true;
		if (!anySick) { cout << npc.name << ": Nobody looks sick. I have nothing to do." << endl; return; }
		cout << npc.name << ": I can tend to one sick person and reduce severity by 1 for 15 gold. Treat someone? (y/n): "; string a; getline(cin, a);
		if (a.empty() || (a != "y" && a != "Y")) { cout << "You decline treatment." << endl; return; }
		cout << "Select who to treat:" << endl; for (size_t i = 0; i < party.size(); ++i) { cout << i + 1 << ") " << party[i].name; if (!party[i].alive) cout << " (dead)"; else if (party[i].severity > 0) cout << " (sick sev " << party[i].severity << ")"; cout << endl; }
		string sel; getline(cin, sel); int who = -1; try { who = stoi(sel) - 1; }
		catch (...) { who = -1; }
		if (who < 0 || who >= (int)party.size() || !party[who].alive || party[who].severity == 0) { cout << "Invalid selection or no treatment needed." << endl; return; }
		if (startingGold < 15) { cout << "You can't afford the healer." << endl; return; }
		startingGold -= 15; party[who].severity = max(0, party[who].severity - 1); if (party[who].severity == 0) { cout << party[who].name << " has been healed and is now healthy." << endl; }
		else cout << party[who].name << " has been treated; severity now " << party[who].severity << "." << endl;
	}
	else { // Storyteller
		vector<string> rumors = { "There's a shortcut through the north pass.", "A cholera outbreak was reported ahead.", "Good hunting grounds in the next 80 miles.", "A wealthy trader visits every 200 miles." };
		cout << npc.name << ": \"" << rumors[rng() % rumors.size()] << "\"" << endl;
	}
}
void renderScene(const Inventory& inv, const vector<PartyMember>& party, bool wagonBroken) {
	// Try to use ANSI colors (green healthy, yellow sick, red dead)
	const string RESET = "\x1b[0m";
	const string RED = "\x1b[31m";
	const string GREEN = "\x1b[32m";
	const string YELLOW = "\x1b[33m";
	cout << "\n--- Scene ---" << endl;
	const int colWidth = 14; // width per party member column
	// Helper lambda to print a string centered in a column with optional color
	auto printPadded = [&](const string& content, int width, const string& color = "") {
		int len = (int)content.size();
		int padLeft = max(0, (width - len) / 2);
		int padRight = max(0, width - len - padLeft);
		for (int i = 0; i < padLeft; ++i) cout << ' ';
		if (!color.empty()) cout << color << content << RESET;
		else cout << content;
		for (int i = 0; i < padRight; ++i) cout << ' ';
		};
	// Draw party stick figures in a row (3 lines)
	for (size_t r = 0; r < 3; ++r) {
		for (size_t i = 0; i < party.size(); ++i) {
			string s;
			if (!party[i].alive) {
				if (r == 0) s = "  X  ";
				else if (r == 1) s = " /|\\ ";
				else s = " / \\ ";
				printPadded(s, colWidth, RED);
			}
			else {
				if (r == 0) s = "  o  ";
				else if (r == 1) s = " /|\\ ";
				else s = " / \\ ";
				// color by health
				if (party[i].severity > 0) printPadded(s, colWidth, YELLOW);
				else printPadded(s, colWidth, GREEN);
			}
		}
		cout << endl;
	}
	// Names and status under figures
	for (size_t i = 0; i < party.size(); ++i) {
		string name = party[i].name.substr(0, min<size_t>(party[i].name.size(), 14));
		string status = "";
		string color = GREEN;
		if (!party[i].alive) { status = " (dead)"; color = RED; }
		else if (party[i].severity > 0) { status = " (sick)"; color = YELLOW; }
		string label = name + status;
		printPadded(label, colWidth, color);
	}
	cout << endl;
	// Draw oxen (simple row of '-O-' characters)
	cout << "Oxen: ";
	int oxToShow = inv.oxen;
	for (int i = 0; i < oxToShow; ++i) cout << "'-O-' ";
	if (oxToShow == 0) cout << "(none)";
	cout << endl;
	// Draw wagon ASCII (show broken variant if wagonBroken)
	if (wagonBroken) {
		cout << "Wagon:  __[=X==]__   (broken)" << endl;
		cout << "   __      _/\\__" << endl;
		cout << "  /  \\____/     \\" << endl;
	}
	else {
		cout << "Wagon:  __[=====]__" << endl;
		cout << "-------------" << endl;
	}
}
void displaySettlementArt(bool isMedicalStation) {
	// Show a real image of the corresponding building alongside the ASCII art.
	// Images are expected in an "assets" folder next to the .exe:
	//   assets/hospital.png    - shown when arriving at a medical station
	//   assets/settlement.png  - shown when arriving at a regular settlement
	if (isMedicalStation) {
		showImage("assets/hospital.png", "Medical Center");
	}
	else {
		showImage("assets/settlement.png", "Settlement");
	}
	if (isMedicalStation) {
		cout << R"(
	  __________________________
	 |       MEDICAL CENTER     |
	 |  +--------------------+  |
	 |  |        +           |  |
	 |  |      +++           |  |
	 |  |   +++++++++        |  |
	 |  |      +++           |  |
	 |  |        +           |  |
	 |  +--------------------+  |
	 |          |  |            |
	 |__________|__|____________|
)" << endl;
	}
	else {
		cout << R"(
		  /\                  /\                  /\
		 /  \                /  \                /  \
		/____\              /____\              /____\
		| [] |              | [] |              | [] |
		|    |              |    |              |    |
	 ___|____|___        ___|____|___        ___|____|___
	|            |      |            |      |            |
	|  SETTLEMENT|      |  MARKET    |      |  TOWN      |
	|____________|      |____________|      |____________|
)" << endl;
	}
}
void displayDayCounter(int totalDays) {
	cout << "\nDay counter: " << totalDays << " day(s) traveled." << endl;
	int days = totalDays;
	int years = days / 365;
	days %= 365;
	int months = days / 30;
	days %= 30;
	int weeks = days / 7;
	days %= 7;
	// Build parts
	vector<string> parts;
	auto addPart = [&](int value, const string& sing, const string& plur) {
		if (value <= 0) return;
		parts.push_back(to_string(value) + " " + (value == 1 ? sing : plur));
		};
	// Add parts in order: years, months, weeks, days
	addPart(years, "year", "years");
	addPart(months, "month", "months");
	addPart(weeks, "week", "weeks");
	addPart(days, "day", "days");
	if (parts.empty()) {
		cout << "(less than one day)\n";
		return;
	}
	// Join with commas and an "and" before the last element
	for (size_t i = 0; i < parts.size(); ++i) {
		if (i > 0) {
			if (i + 1 == parts.size()) cout << " and ";
			else cout << ", ";
		}
		// Print the part
		cout << parts[i];
	}
	// End with a newline
	cout << "\n";
}
void consumeClothingForSeverity(Inventory& inv, int prevSeverity, int newSeverity, const string& name) {
	if (prevSeverity < 3 && newSeverity == 3) {
		if (inv.clothingSets > 0) {
			inv.clothingSets -= 1;
			cout << "A set of clothing was used to keep " << name << " warm during their severe illness. Clothing sets remaining: " << inv.clothingSets << "." << endl;
		}
		else {
			cout << name << " needed extra warmth, but you have no clothing sets left." << endl;
		}
	}
}
void consumeClothingForDeath(Inventory& inv, const string& name) {
	int toUse = min(2, inv.clothingSets);
	if (toUse > 0) {
		inv.clothingSets -= toUse;
		cout << toUse << " set(s) of clothing were used for " << name << "'s burial. Clothing sets remaining: " << inv.clothingSets << "." << endl;
	}
}
void displayInventory(const Inventory& inv, int gold) {
	cout << "\n--- Inventory Summary ---" << endl;
	cout << "Food: " << inv.foodPounds << " lbs" << endl;
	cout << "Bullets: " << inv.bullets << endl;
	cout << "Clothing sets: " << inv.clothingSets << endl;
	cout << "Oxen: " << inv.oxen << endl;
	cout << "Spare parts: " << inv.spareParts << endl;
	cout << "Medicine kits: " << inv.medicineKits << endl;
	cout << "Wood: " << inv.woodPounds << " lbs" << endl;
	cout << "Gold: " << gold << endl;
	cout << "-------------------------\n" << endl;
}
