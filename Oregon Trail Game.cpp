// This game is a text-based simulation of the Oregon Trail, where players manage resources and make decisions to reach Oregon safely.
// The code includes features such as supply purchasing, party management, health events, and travel mechanics.
// The code is structured to allow for both manual and automatic day advancement, with detailed feedback on the party's status and inventory.
// The code uses ANSI escape codes for colored output in the console, enhancing the visual representation of party members' health status.
// The game includes a simple ASCII art representation of the party, oxen, and wagon, providing a visual element to the text-based interface.
// The code is designed to be cross-platform, with specific handling for Windows consoles to enable ANSI color support.
// The game includes a variety of supply categories, each with different purchasing options and costs, allowing players to strategize their resource management.
// The game tracks the passage of time in days, months, and years, providing a realistic simulation of the journey to Oregon.
// The game includes a difficulty setting that affects hunting success rates and yields, adding an additional layer of challenge for players.
// The game includes a medical event system that randomly affects party members, requiring players to manage health and resources effectively.
// The game includes a system for tracking the severity of illnesses and the effectiveness of treatments, adding depth to the health management aspect of the game.
// The game includes a system for tracking the carrying capacity of the wagon and the effects of overloading, adding realism to the resource management aspect of the game.
// The game includes a system for tracking the status of the wagon, including the possibility of it breaking down and requiring repairs, adding an additional layer of challenge for players.
// The game includes a system for tracking the status of oxen, including the possibility of them dying, which affects the party's ability to travel and carry supplies.
// The game includes a system for tracking the status of party members, including their health, sickness, and mortality, adding depth to the character management aspect of the game.
// The game includes a system for tracking the passage of time and the effects of seasons on travel and health, adding realism to the simulation.
// The game includes a system for tracking the player's gold and the costs of supplies, adding an economic management aspect to the game.
// The game includes a system for tracking the player's progress towards Oregon, including the distance traveled and the remaining distance, providing a clear goal for players to work towards.
// The game includes a system for tracking the player's choices and their consequences, allowing for a dynamic and interactive gameplay experience.
// The game includes a system for tracking the player's decisions and their impact on the party's survival, adding a strategic element to the gameplay.
// The game includes a system for tracking the player's decisions and their impact on the party's health, resources, and progress, adding depth to the gameplay experience.
//Included Libraries:
	// The code includes standard libraries for input/output, mathematical functions, string manipulation, vector operations, random number generation, time management, threading, and algorithmic operations.
	//iostream: for console input and output operations
	//cmath: for mathematical functions and operations
	//string: for string manipulation and operations
	//vector: for dynamic array operations and management
	//random: for random number generation and distribution
	//ctime: for time-related functions and operations
	//limits: for numeric limits and properties of fundamental types
	//thread: for multithreading and concurrent execution
	//chrono: for time-related functions and operations
	//algorithm: for standard algorithms and operations on containers
	//iomanip: for input/output manipulators and formatting
	//windows.h: for Windows-specific functions and operations, including console handling and system calls
	//array: for array class support
	//SFML\Graphics.hpp: To allow a separate window to open with an image file
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
// Enable ANSI color support on Windows consoles
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#undef max
#undef min
//simplifying every line (don't need to clarify std every time)
using namespace std;
// Internal inventory to track purchased items (not shown to the player)
struct Inventory {
	int foodPounds = 0;
	int bullets = 0;
	int clothingSets = 0;
	int oxen = 0;
	int spareParts = 0;
	int medicineKits = 0;
	int woodPounds = 0;
};
void showImage(const std::string& path, const std::string& caption = "") {
	sf::Texture texture;
	if (!texture.loadFromFile(path)) {
		std::cout << "(Could not load image: " << path << ")" << std::endl;
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
// Function to display the supply menu and get the player's choice
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
// Simple NPC meeting system for settlements
struct NPC {
	string name;
	string role; // "Trader", "Healer", "Storyteller"
};
void meetLocals(Inventory &inv, vector<string> &party, vector<bool> &alive, vector<int> &severity, std::mt19937 &rng, int &startingGold) {
	vector<string> names = {"Martha","Elias","Jonah","Agnes","Samuel","Lydia","Hank","Beatrice","Clara","Walter"};
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
	for (int i = 0; i < (int)locals.size(); ++i) cout << i+1 << ") " << locals[i].name << " (" << locals[i].role << ")" << endl;
	cout << "Enter number to talk or 0 to step away: ";
	string pick; getline(cin, pick);
	int idx = -1; try { idx = stoi(pick) - 1; } catch(...) { idx = -1; }
	if (idx < 0 || idx >= (int)locals.size()) { cout << "You decide not to engage with the locals." << endl; return; }
	NPC &npc = locals[idx];
	cout << "You approach " << npc.name << " the " << npc.role << "." << endl;
	if (npc.role == "Trader") {
		// Trader offers a small randomized deal
		int offerType = rng() % 4; // 0=food,1=bullets,2=spare,3=medicine
		if (offerType == 0) {
			int amt = (rng() % 3 + 1) * 25; int cost = amt / 2; // cheap food
			cout << npc.name << ": I can sell you " << amt << " lbs of food for " << cost << " gold. Buy? (y/n): ";
			string a; getline(cin, a);
			if (!a.empty() && (a=="y"||a=="Y") && startingGold >= cost) { inv.foodPounds += amt; startingGold -= cost; cout << "Purchased " << amt << " lbs of food." << endl; }
			else cout << "No deal made." << endl;
		} else if (offerType == 1) {
			int amt = (rng() % 3 + 1) * 10; int cost = amt / 2;
			cout << npc.name << ": I have " << amt << " bullets for " << cost << " gold. Buy? (y/n): "; string a; getline(cin,a);
			if (!a.empty() && (a=="y"||a=="Y") && startingGold >= cost) { inv.bullets += amt; startingGold -= cost; cout << "Purchased " << amt << " bullets." << endl; } else cout << "No deal made." << endl;
		} else if (offerType == 2) {
			int amt = 1 + (rng()%2); int cost = 40 * amt;
			cout << npc.name << ": Spare parts, " << amt << " for " << cost << " gold. Buy? (y/n): "; string a; getline(cin,a);
			if (!a.empty() && (a=="y"||a=="Y") && startingGold >= cost) { inv.spareParts += amt; startingGold -= cost; cout << "Purchased " << amt << " spare parts." << endl; } else cout << "No deal made." << endl;
		} else {
			int amt = 1 + (rng()%3); int cost = 18 * amt;
			cout << npc.name << ": I can sell " << amt << " medicine kits for " << cost << " gold. Buy? (y/n): "; string a; getline(cin,a);
			if (!a.empty() && (a=="y"||a=="Y") && startingGold >= cost) { inv.medicineKits += amt; startingGold -= cost; cout << "Purchased " << amt << " medicine kits." << endl; } else cout << "No deal made." << endl;
		}
	} else if (npc.role == "Healer") {
		// Healer can treat a sick member for a fee
		bool anySick = false; for (size_t i = 0; i < party.size(); ++i) if (alive[i] && severity[i] > 0) anySick = true;
		if (!anySick) { cout << npc.name << ": Nobody looks sick. I have nothing to do." << endl; return; }
		cout << npc.name << ": I can tend to one sick person and reduce severity by 1 for 15 gold. Treat someone? (y/n): "; string a; getline(cin,a);
		if (a.empty() || (a!="y" && a!="Y")) { cout << "You decline treatment." << endl; return; }
		cout << "Select who to treat:" << endl; for (size_t i = 0; i < party.size(); ++i) { cout << i+1 << ") " << party[i]; if (!alive[i]) cout << " (dead)"; else if (severity[i]>0) cout << " (sick sev "<<severity[i]<<")"; cout << endl; }
		string sel; getline(cin, sel); int who=-1; try{ who = stoi(sel)-1; } catch(...) { who=-1; }
		if (who<0 || who >= (int)party.size() || !alive[who] || severity[who]==0) { cout << "Invalid selection or no treatment needed." << endl; return; }
		if (startingGold < 15) { cout << "You can't afford the healer." << endl; return; }
		startingGold -= 15; severity[who] = max(0, severity[who]-1); if (severity[who]==0) { cout << party[who] << " has been healed and is now healthy." << endl; } else cout << party[who] << " has been treated; severity now "<<severity[who]<<"."<<endl;
	} else { // Storyteller
		vector<string> rumors = {"There's a shortcut through the north pass.", "A cholera outbreak was reported ahead.", "Good hunting grounds in the next 80 miles.", "A wealthy trader visits every 200 miles."};
		cout << npc.name << ": \"" << rumors[rng() % rumors.size()] << "\"" << endl;
	}
}
// Render a simple ASCII scene showing party, oxen, and wagon
void renderScene(const Inventory &inv, const vector<string> &party, const vector<bool> &alive, const vector<int> &severity, bool wagonBroken) {
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
			if (!alive[i]) {
				if (r == 0) s = "  X  ";
				else if (r == 1) s = " /|\\ ";
				else s = " / \\ ";
				printPadded(s, colWidth, RED);
			} else {
				if (r == 0) s = "  o  ";
				else if (r == 1) s = " /|\\ ";
				else s = " / \\ ";
				// color by health
				if (severity[i] > 0) printPadded(s, colWidth, YELLOW);
				else printPadded(s, colWidth, GREEN);
			}
		}
		cout << endl;
	}
	// Names and status under figures
	for (size_t i = 0; i < party.size(); ++i) {
		string name = party[i].substr(0, min<size_t>(party[i].size(), 14));
		string status = "";
		string color = GREEN;
		if (!alive[i]) { status = " (dead)"; color = RED; }
		else if (severity[i] > 0) { status = " (sick)"; color = YELLOW; }
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
	} else {
		cout << "Wagon:  __[=====]__" << endl;
		cout << "-------------" << endl;
	}
}
// Opens a small window and shows an image file. Waits for the player to
// press any key or close the window before returning control to the console.
// If the image can't be found/loaded, it fails quietly and just returns,
// so a missing file never crashes the game or blocks play.
// Display the type of location the party has reached.
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
	} else {
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
// Display days traveled in mixed units (years, months, weeks, days)
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
	auto addPart = [&](int value, const string &sing, const string &plur) {
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
// Consumes one set of clothing when a party member's illness reaches its most
// severe stage (severity 3), representing extra blankets/layers needed for
// care. Only fires on the transition INTO severity 3, not every day spent
// there, so pass the severity value from before and after the update.
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
// Consumes up to 2 sets of clothing when a party member dies, representing
// clothing used for burial. Uses fewer if that's all that's left.
void consumeClothingForDeath(Inventory& inv, const string& name) {
	int toUse = min(2, inv.clothingSets);
	if (toUse > 0) {
		inv.clothingSets -= toUse;
		cout << toUse << " set(s) of clothing were used for " << name << "'s burial. Clothing sets remaining: " << inv.clothingSets << "." << endl;
	}
}
// Print a visible summary of the internal inventory when the player requests it
void displayInventory(const Inventory &inv, int gold) {
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
int main() {
	// Enable ANSI color support on Windows consoles
#ifdef _WIN32
		auto enableVirtualTerminal = []() {
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hOut == INVALID_HANDLE_VALUE) return;
			DWORD dwMode = 0;
			if (!GetConsoleMode(hOut, &dwMode)) return;
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hOut, dwMode);
	};
	enableVirtualTerminal();
#endif
	// Display the title of the game and the "welcome message"
	cout << "Welcome to the Oregon Trail Game!" << endl;
	cout << "You are about to embark on a journey to the Oregon Territory in the 19th century." << endl;
	cout << "You will face many challenges along the way, including harsh weather, disease, and limited resources." << endl;
	cout << "Your goal is to reach Oregon safely and successfully." << endl;
	cout << "Good luck on your journey!" << endl;
	string playerNameOne, playerNameTwo, playerNameThree, playerNameFour, playerNameFive;
	cout << "Please enter 5 names, one for each member of your party: ";
	cin >> playerNameOne >> playerNameTwo >> playerNameThree >> playerNameFour >> playerNameFive;
	cout << "Your party members are: " << playerNameOne << ", " << playerNameTwo << ", " << playerNameThree << ", " << playerNameFour << ", and " << playerNameFive << "." << endl;
	cout << "Please select the career you desire from the list: " << endl;
	cout << "Regardless of your career choice, you will start with 1000 gold, 500 pounds of food, and 100 pounds of wood. The banker will have the 5000 gold and the farmer will start with 750 gold." << endl;
	int careerChoice;
	cout << "1. Banker" << endl;
	cout << "2. Carpenter" << endl;
	cout << "3. Farmer" << endl;
	// Get the player's career choice
	cin >> careerChoice;
	// Declare the variable for starting gold and start month
	int startingGold = 0;
	int startingMonth;
	// Declare the variable for player status (alive or dead)
	int playerStatus = 1; // 1 = alive, 0 = dead
	cout << "Please select the month you would like to start your journey (1-12): ";
	// Get the player's starting month
	cin >> startingMonth;
	Inventory inv; // tracks what the player buys
	// Set starting gold based on career choice
	if (careerChoice == 1) {
		cout << "You have chosen to be a Banker and because of this, you start with the most money. Specifically 5000 gold." << endl;
		startingGold = 5000;
	}
	else if (careerChoice == 2) {
		cout << "You have chosen to be a Carpenter and because of this, you start with the most amount of wood to repair your wagon, but the second least amount of gold." << endl;
		startingGold = 2500;
	}
	else if (careerChoice == 3) {
		cout << "You have chosen to be a Farmer and because of this, you start with the least amount of money and wood, but the most amount of food." << endl;
		startingGold = 750;
	}
	else {
		cout << "Invalid choice. Please select a valid career." << endl;
	}
	// Repeat the supply menu after each purchase until the user chooses to finish
	while (true) {
		int supplyChoice = supplyChoiceFunction();
		// Handle the user's supply choice
		if (supplyChoice == 0) {
			cout << "Finished shopping. Time to start your journey!" << endl;
			break;
		}
		// Display inventory if the user chooses to view it
		if (supplyChoice == 6) {
			displayInventory(inv, startingGold);
		}
		// Handle each supply category purchase
		if (supplyChoice == 1) {
			cout << "You have chosen to purchase food." << endl;
			cout << "You will need to purchase enough food to last your party for the entire journey. It is recommended to purchase at least 100 pounds of food per person." << endl;
			cout << "1.50 pounds of food - 25 gold" << endl;
			cout << "2.100 pounds of food - 50 gold" << endl;
			cout << "3.200 pounds of food - 100 gold" << endl;
			int foodChoice;
			cin >> foodChoice;
			if (foodChoice == 1) {
				// Handle the user's food choice
				cout << "You have chosen to purchase 50 pounds of food for 25 gold." << endl;
			startingGold -= 25; // Deduct the cost of food from starting gold
			inv.foodPounds += 50;
			}
			else if (foodChoice == 2) {
				// Handle the user's food choice
				cout << "You have chosen to purchase 100 pounds of food for 50 gold." << endl;
			startingGold -= 50; // Deduct the cost of food from starting gold
			inv.foodPounds += 100;
			}
			else if (foodChoice == 3) {
				// Handle the user's food choice
				cout << "You have chosen to purchase 200 pounds of food for 100 gold." << endl;
			startingGold -= 100; // Deduct the cost of food from starting gold
			inv.foodPounds += 200;
			}
			else {
				cout << "Invalid choice. Please select a valid food option." << endl;
		}
}
		else if (supplyChoice == 2) {
			// Handle ammunition purchase
			cout << "You have chosen to purchase ammunition." << endl;
			cout << "You will need to purchase enough ammunition to hunt for food and defend your party from potential threats. It is recommended to purchase at least 20 bullets per person." << endl;
			cout << "1. 20 bullets - 10 gold" << endl;
			cout << "2. 50 bullets - 25 gold" << endl;
			cout << "3.	100 bullets - 50 gold" << endl;
			int ammoChoice;
			cin >> ammoChoice;
			if (ammoChoice == 1) {
				// Handle the user's ammunition choice
				cout << "You have chosen to purchase 20 bullets for 10 gold." << endl;
				startingGold -= 10; // Deduct the cost of ammunition from starting gold
				inv.bullets += 20;
			}
			else if (ammoChoice == 2) {
				// Handle the user's ammunition choice
				cout << "You have chosen to purchase 50 bullets for 25 gold." << endl;
				startingGold -= 25; // Deduct the cost of ammunition from starting gold
				inv.bullets += 50;
			}
			else if (ammoChoice == 3) {
				// Handle the user's ammunition choice
				cout << "You have chosen to purchase 100 bullets for 50 gold." << endl;
				startingGold -= 50; // Deduct the cost of ammunition from starting gold
				inv.bullets += 100;
			}
			else {
				cout << "Invalid choice. Please select a valid ammunition option." << endl;
			}
		}
		else if (supplyChoice == 3) {
			// Handle clothing purchase
			cout << "You have chosen to purchase clothing." << endl;
			cout << "You will need to purchase enough clothing to keep your party warm and protected from the elements. It is recommended to purchase at least 2 sets of clothing per person." << endl;
			cout << "1. 5 sets of clothing - 20 gold" << endl;
			cout << "2. 10 sets of clothing - 40 gold" << endl;
			cout << "3. 15 sets of clothing - 60 gold" << endl;
			int clothingChoice;
			cin >> clothingChoice;
			if (clothingChoice == 1) {
				// Handle the user's clothing choice
				cout << "You have chosen to purchase 5 sets of clothing for 20 gold." << endl;
				startingGold -= 20; // Deduct the cost of clothing from starting gold
				inv.clothingSets += 5;
			}
			else if (clothingChoice == 2) {
				// Handle the user's clothing choice
				cout << "You have chosen to purchase 10 sets of clothing for 40 gold." << endl;
				startingGold -= 40; // Deduct the cost of clothing from starting gold
				inv.clothingSets += 10;
			}
			else if (clothingChoice == 3) {
				// Handle the user's clothing choice
				cout << "You have chosen to purchase 15 sets of clothing for 60 gold." << endl;
				startingGold -= 60; // Deduct the cost of clothing from starting gold
				inv.clothingSets += 15;
			}
			else {
				cout << "Invalid choice. Please select a valid clothing option." << endl;
			}
		}
		else if (supplyChoice == 4) {
			// Handle oxen purchase
			cout << "You have chosen to purchase oxen." << endl;
			cout << "You will need to purchase enough oxen to pull your wagon. It is recommended to purchase at least 2 oxen per wagon." << endl;
			cout << "1. 2 oxen - 100 gold" << endl;
			cout << "2. 4 oxen - 200 gold" << endl;
			cout << "3. 6 oxen - 300 gold" << endl;
			int oxenChoice;
			cin >> oxenChoice;
			if (oxenChoice == 1) {
				// Handle the user's oxen choice
				cout << "You have chosen to purchase 2 oxen for 100 gold." << endl;
				startingGold -= 100; // Deduct the cost of oxen from starting gold
				inv.oxen += 2;
			}
			else if (oxenChoice == 2) {
				// Handle the user's oxen choice
				cout << "You have chosen to purchase 4 oxen for 200 gold." << endl;
				startingGold -= 200; // Deduct the cost of oxen from starting gold
				inv.oxen += 4;
			}
			else if (oxenChoice == 3) {
				// Handle the user's oxen choice
				cout << "You have chosen to purchase 6 oxen for 300 gold." << endl;
				startingGold -= 300; // Deduct the cost of oxen from starting gold
				inv.oxen += 6;
			}
			else {
				cout << "Invalid choice. Please select a valid oxen option." << endl;
			}
		}
		else if (supplyChoice == 5) {
			// Handle spare parts purchase
			cout << "You have chosen to purchase spare parts." << endl;
			cout << "You will need to purchase enough spare parts to repair your wagon in case of damage. It is recommended to purchase at least 1 set of spare parts per wagon." << endl;
			cout << "1. 1 set of spare parts - 50 gold" << endl;
			cout << "2. 2 sets of spare parts - 100 gold" << endl;
			cout << "3. 3 sets of spare parts - 150 gold" << endl;
			int partsChoice;
			cin >> partsChoice;
			if (partsChoice == 1) {
				// Handle the user's spare parts choice
				cout << "You have chosen to purchase 1 set of spare parts for 50 gold." << endl;
				startingGold -= 50; // Deduct the cost of spare parts from starting gold
				inv.spareParts += 1;
			}
			else if (partsChoice == 2) {
				// Handle the user's spare parts choice
				cout << "You have chosen to purchase 2 sets of spare parts for 100 gold." << endl;
				startingGold -= 100; // Deduct the cost of spare parts from starting gold
				inv.spareParts += 2;
			}
			else if (partsChoice == 3) {
				// Handle the user's spare parts choice
				cout << "You have chosen to purchase 3 sets of spare parts for 150 gold." << endl;
				startingGold -= 150; // Deduct the cost of spare parts from starting gold
				inv.spareParts += 3;
			}
			else {
				cout << "Invalid choice. Please select a valid spare parts option." << endl;
			}	
		}
		else if (supplyChoice == 7) {
			// Handle medicine kits purchase
			cout << "You have chosen to purchase medicine kits." << endl;
			cout << "Medicine kits can be used to treat sick party members. Each kit reduces severity by one stage." << endl;
			cout << "1. 1 kit - 20 gold" << endl;
			cout << "2. 3 kits - 50 gold" << endl;
			cout << "3. 5 kits - 80 gold" << endl;
			int medChoice; 
			cin >> medChoice;
			// Handle the user's medicine kits choice
			if (medChoice == 1) { 
				// Handle the user's medicine kits choice
				cout << "Purchased 1 medicine kit." << endl; 
				startingGold -= 20; inv.medicineKits += 1; 
			}
			else if (medChoice == 2) { 
				// Handle the user's medicine kits choice
				cout << "Purchased 3 medicine kits." << endl; 
				startingGold -= 50; inv.medicineKits += 3; 
			}
			else if (medChoice == 3) { 
				// Handle the user's medicine kits choice
				cout << "Purchased 5 medicine kits." << endl; 
				startingGold -= 80; inv.medicineKits += 5; 
			}
			else { 
				cout << "Invalid choice. Please select a valid medicine option." << endl; 
			}
		}
		else {
			cout << "Invalid choice. Please select a valid supply category." << endl;
		}
		// After each purchase, show remaining gold and prompt
		cout << "Remaining gold: " << startingGold << endl;
		// Check carrying capacity and warn if overloaded
		{
			double load = 0.0;
			load += inv.foodPounds; // lbs
			load += inv.woodPounds; // lbs
			load += inv.spareParts * 30; // estimate 30 lbs per spare parts set
			load += inv.clothingSets * 5; // estimate 5 lbs per clothing set
			load += inv.medicineKits * 5; // estimate 5 lbs per kit
			// bullets negligible but add small weight
			load += inv.bullets * 0.1;
			double capacity = inv.oxen * 500 + 200; // wagon base capacity + oxen
			cout << "Estimated carried weight: " << static_cast<int>(load) << " lbs. Capacity: " << static_cast<int>(capacity) << " lbs." << endl;
			if (load > capacity) cout << "Warning: Your current supplies exceed your wagon's carrying capacity. Consider buying more oxen or removing items." << endl;
		}
		cout << "Enter 0 to finish shopping or choose another category." << endl;
	}
	if (((startingMonth > 1) && (startingMonth < 3)) || (startingMonth >= 11)) {
		cout << "You have chosen to start your journey in a season with wintery conditions." << endl;
	}
	else if (startingMonth >= 3 && startingMonth <= 6) {
		cout << "You have chosen to start your journey in spring." << endl;
	}
	else if (startingMonth >= 7 && startingMonth <= 8) {
		cout << "You have chosen to start your journey in summer." << endl;
	}
	else {
		cout << "You have chosen to start your journey in fall." << endl;
	}
	int currentMonth = startingMonth;
	int milesTraveled = 0;
	int milesToOregon = 2000; // Total distance to Oregon
	int daysOnTrail = 0;
	// After shopping, record the initial food capacity for thresholds
	// (will be set after shopping loop ends; placeholder assigned here)
	int initialFoodCapacity = 0;
	// Track last miles traveled for village and hunting events
	int lastVillageMiles = 0;
	int lastHuntMiles = 0;
	// Track consecutive days the wagon is overloaded (for slowdown mechanics)
	int overloadWarningDays = 0;
	double overloadPenaltyMultiplier = 1.0; // multiplicative penalty applied to speed when overloaded long-term
	// Difficulty settings for hunting success and yield
	int difficulty = 2;
	double huntSuccessProb = 0.65;
	int huntYieldMin = 30;
	int huntYieldMax = 80;
	// Now set initialFoodCapacity and prompt difficulty based on post-shopping inventory
	initialFoodCapacity = max(1, inv.foodPounds);
	cout << "Select difficulty: 1=Easy, 2=Medium, 3=Hard: ";
	string diffLine; getline(cin, diffLine);
	try { difficulty = stoi(diffLine); } catch(...) { difficulty = 2; }
	// Set hunting parameters based on difficulty
	if (difficulty == 1) {
		huntSuccessProb = 0.85; huntYieldMin = 20; huntYieldMax = 50;
	} else if (difficulty == 2) {
		huntSuccessProb = 0.65; huntYieldMin = 30; huntYieldMax = 80;
	} else {
		huntSuccessProb = 0.50; huntYieldMin = 20; huntYieldMax = 150;
	}
	// Party data structures for medical events
	vector<string> party = { playerNameOne, playerNameTwo, playerNameThree, playerNameFour, playerNameFive };
	vector<bool> sick(party.size(), false);
	vector<string> sicknessDesc(party.size(), "");
	vector<bool> alive(party.size(), true);
	// Random engine for medical events
	mt19937 rng(static_cast<unsigned>(time(nullptr)));
	// Configure medical event spacing based on chosen difficulty:
	// Easy: rarer events; Medium: ~3 times per week (every 2-3 days); Hard: daily events
	int medMin = 4, medMax = 10;
	if (difficulty == 3) { // Hard
		medMin = 1; medMax = 1; // event every day
	} else if (difficulty == 2) { // Medium
		medMin = 2; medMax = 3; // event every 2-3 days (~3/week)
	} else { // Easy
		medMin = 4; medMax = 10; // rarer
	}
	uniform_int_distribution<int> medicalDist(medMin, medMax);
	int daysUntilMedical = medicalDist(rng);
	uniform_int_distribution<int> memberDist(0, static_cast<int>(party.size()) - 1);
	vector<string> illnesses = { "fever", "dysentery", "broken leg", "snake bite", "pneumonia" };
	uniform_int_distribution<int> illnessDist(0, static_cast<int>(illnesses.size()) - 1);
	bool wagonBroken = false;
	double speedMultiplier = 1.0; // cumulative speed modifier (1.0 = normal)
	uniform_int_distribution<int> oxDeathDist(0, 99); // for ox death chance roll
	uniform_int_distribution<int> wagonBreakDist(0, 99); // for wagon break chance roll
	// Clear leftover newline before getline loop
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "You are now on the trail. Your journey has begun!" << endl;
	// Choose travel mode: manual or auto-advance
	cout << "Select travel mode: (1) Manual (press Enter each day), (2) Auto-advance" << endl;
	int travelMode = 1;
	// Loop until valid input is received
	while (true) {
		cout << "Enter 1 or 2: ";
		string modeLine;
		getline(cin, modeLine);
		if (modeLine == "1" || modeLine == "2") { travelMode = stoi(modeLine); break; }
	}
	int autoDaysToRun = 0; // 0 means run until user stops or completion
	int autoDelayMs = 200; // default delay between auto days
	// If auto mode is selected, prompt for number of days to simulate and delay
	if (travelMode == 2) {
		cout << "Auto-advance selected. Enter number of days to simulate (0 = run until you stop): ";
		string daysLine;
		getline(cin, daysLine);
		try { autoDaysToRun = stoi(daysLine); } catch(...) { autoDaysToRun = 0; }
		cout << "Enter delay between auto-days in milliseconds (e.g. 200): ";
		string delayLine;
		getline(cin, delayLine);
		try { autoDelayMs = stoi(delayLine); } catch(...) { autoDelayMs = 200; }
	}
	// Health severity: 0 = healthy, 1 = mild, 2 = moderate, 3 = severe
	vector<int> severity(party.size(), 0);
	// Track dysentery countdown (days until death) per member; 0 = not infected
	vector<int> dysenteryDays(party.size(), 0);
	// Track dysentery treatment progress (days remaining of treatment) per member; 0 = not in treatment
	vector<int> dysenteryTreatmentDays(party.size(), 0);
	// Track auto mode days run so far
	int autoDaysRunNow = 0;
	// Main simulation loop
	bool stopSim = false;
	while (playerStatus == 1 && !stopSim) {
		// If manual mode, prompt user for action each day
		if (travelMode == 1) {
			cout << "(Enter=advance 1 day, r=rest, s=status, a=auto, q=quit) > ";
			string cmd;
			getline(cin, cmd);
			// Process command if not empty
			if (!cmd.empty()) {
				// Convert command to lowercase for easier comparison
				transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
				// Handle commands
				if (cmd == "q" || cmd == "Q") { cout << "Stopping trail simulation." << endl; break; }
				if (cmd == "s" || cmd == "S") {
					displayInventory(inv, startingGold);
					for (size_t i = 0; i < party.size(); ++i) if (!alive[i]) cout << party[i] << " (dead)\n";
					continue;
				}
				// Handle rest command: reduces severity for sick members
				if (cmd == "r" || cmd == "R") {
					// Rest: 70% chance to reduce severity by 1 for sick members
					cout << "You rest for a day. Sick members may recover faster." << endl;
					// Apply recovery chance to each sick and alive party member
					for (size_t i = 0; i < party.size(); ++i) {
						if (severity[i] > 0 && alive[i]) {
							int roll = rng() % 100;
							if (roll < 70) { severity[i] = max(0, severity[i] - 1); if (severity[i] == 0) sick[i] = false; }
						}
					}
					// Advance one day after resting
				}
			if (cmd == "t" || cmd == "T") {
				// Treat a sick party member using a medicine kit
				cout << "Select a party member to treat:\n";
				// List party members with their health status
				for (size_t i = 0; i < party.size(); ++i) {
					cout << i+1 << ". " << party[i];
					if (!alive[i]) cout << " (dead)";
					else if (severity[i] > 0) cout << " (sick: " << sicknessDesc[i] << ", severity " << severity[i] << ")";
					cout << "\n";
				}
				cout << "Enter number: ";
				string idxLine; getline(cin, idxLine);
				int idx = 0;
				// Convert input to index (1-based to 0-based)
				try { idx = stoi(idxLine) - 1; } catch(...) { idx = -1; }
			// Validate index and apply treatment logic
			if (idx >= 0 && idx < (int)party.size()) {
				if (!alive[idx]) cout << party[idx] << " is dead and cannot be treated.\n";
				else if (severity[idx] == 0) cout << party[idx] << " is not sick.\n";
				else if (sicknessDesc[idx] == "dysentery") {
					cout << "Medicine kits and field treatment cannot cure dysentery. Only a medical station can cure it (3 days required)." << endl;
				} else if (inv.medicineKits <= 0) cout << "You have no medicine kits to use.\n";
				else {
					inv.medicineKits -= 1;
					severity[idx] = max(0, severity[idx] - 1);
					if (severity[idx] == 0) { sick[idx] = false; sicknessDesc[idx] = ""; cout << party[idx] << " has been cured by medicine." << endl; renderScene(inv, party, alive, severity, wagonBroken); }
					else { cout << party[idx] << " has been treated and is now severity " << severity[idx] << "." << endl; renderScene(inv, party, alive, severity, wagonBroken); }
				}
			} else cout << "Invalid selection.\n";
				continue;
			}
			// Handle auto mode command: switch to auto-advance
				if (cmd == "a" || cmd == "A") {
					// switch to auto mode
					travelMode = 2;
					cout << "Switching to auto mode. Enter days to simulate (0=until stop): ";
					string d; getline(cin,d); try{ autoDaysToRun = stoi(d);}catch(...){autoDaysToRun=0;}
					// Allow the user to change auto-run speed when switching to auto mode
					cout << "Enter delay between auto-days in milliseconds (e.g. 200): ";
					string delayLine2; getline(cin, delayLine2);
					try { autoDelayMs = stoi(delayLine2); } catch(...) { /* keep previous value */ }
				}
				// Handle repair command: use spare parts to fix the wagon
				if (cmd == "repair" || cmd == "fix") {
					if (!wagonBroken) { cout << "Your wagon is not broken." << endl; continue; }
					if (inv.spareParts > 0) {
						inv.spareParts -= 1;
						wagonBroken = false;
						speedMultiplier = 1.0; // restore normal speed
						cout << "You used a spare part to repair the wagon. Travel speed restored." << endl;
					} else {
						cout << "You have no spare parts to repair the wagon." << endl;
					}
					continue;
				}
			}
		}
		// Advance one day (both manual Enter or auto)
		daysOnTrail++;
		if (travelMode == 2) autoDaysRunNow++;
		// Daily food consumption based on living party members
		int livingCount = 0;
		for (size_t i = 0; i < alive.size(); ++i) if (alive[i]) livingCount++;
		int dailyConsumptionPerPerson = 3; // pounds per person per day
		int dailyConsumption = livingCount * dailyConsumptionPerPerson;
		if (inv.foodPounds >= dailyConsumption) {
			inv.foodPounds -= dailyConsumption;
		} else {
			// Not enough food: everyone eats what remains and sickness risk increases
			if (inv.foodPounds > 0) {
				cout << "You ran out of food during the day; your party ate what remained (" << inv.foodPounds << " lbs)." << endl;
				inv.foodPounds = 0;
			}
			cout << "Food shortage! Party members are at higher risk of illness or worsening conditions." << endl;
			for (size_t i = 0; i < party.size(); ++i) {
				if (alive[i]) {
					int prevSeverity = severity[i];
					severity[i] = min(3, severity[i] + 1);
					sick[i] = (severity[i] > 0);
					consumeClothingForSeverity(inv, prevSeverity, severity[i], party[i]);
				}
			}
		}
		// Update miles traveled based on oxen and apply speed multiplier
		double baseMiles = 2.0;
		if (inv.oxen >= 6) baseMiles = 20.0;
		else if (inv.oxen >= 4) baseMiles = 16.0;
		else if (inv.oxen >= 2) baseMiles = 12.0;
		else if (inv.oxen == 1) baseMiles = 6.0;
		// Check carrying capacity and apply overload warnings / exponential penalty after a week
		{
			double load = 0.0;
			load += inv.foodPounds; // lbs
			load += inv.woodPounds; // lbs
			load += inv.spareParts * 30; // estimate 30 lbs per spare parts set
			load += inv.clothingSets * 5; // estimate 5 lbs per clothing set
			load += inv.medicineKits * 5; // estimate 5 lbs per kit
			load += inv.bullets * 0.1; // bullets
			double capacity = inv.oxen * 500 + 200; // wagon base capacity + oxen
			// Check if overloaded and apply warnings and penalties
			if (load > capacity) {
				overloadWarningDays++;
				cout << "Warning: Your current supplies exceed your wagon's carrying capacity. Days overloaded: " << overloadWarningDays << "." << endl;
				if (overloadWarningDays > 7) {
					int extraDays = overloadWarningDays - 7;
					// exponential decay: multiply speed by 0.995 per extra day (~0.5% per day compounding)
					double dailyFactor = 0.995;
					overloadPenaltyMultiplier = pow(dailyFactor, static_cast<double>(extraDays));
					if (overloadPenaltyMultiplier < 0.01) overloadPenaltyMultiplier = 0.01; // floor at 1% speed
					double percentSlow = (1.0 - overloadPenaltyMultiplier) * 100.0;
					cout << "Overload penalty applied. Speed reduced by " << static_cast<int>(percentSlow) << "% (multiplier=" << overloadPenaltyMultiplier << ")." << endl;
				} else {
					cout << "Initial warnings only - remove items or buy more oxen to avoid slowdowns after a week." << endl;
				}
			} else {
				if (overloadWarningDays > 0) cout << "You are now under capacity; overload warnings reset." << endl;
				overloadWarningDays = 0;
				overloadPenaltyMultiplier = 1.0;
			}
		}
		// Calculate miles traveled today with speed multiplier and overload penalty
		double milesTodayD = baseMiles * speedMultiplier * overloadPenaltyMultiplier;
		int milesToday = static_cast<int>(max(1.0, round(milesTodayD)));
		milesTraveled += milesToday;
		// Check if the party has reached Oregon
		if (milesTraveled >= milesToOregon) {
			milesTraveled = milesToOregon; // clamp so the summary doesn't overshoot
			cout << "\n*******************************************" << endl;
			cout << "*   CONGRATULATIONS, YOU MADE IT TO OREGON!  *" << endl;
			cout << "*******************************************\n" << endl;
			cout << "After " << daysOnTrail << " days and " << milesTraveled << " miles, your party has reached the Oregon Territory!" << endl;
			//Count and list who survived the journey
			int oregonSurvivorCount = 0;
			for (size_t i = 0; i < party.size(); ++i) {
				if (alive[i]) cout << "  - " << party[i] << " (survived)" << endl;
				else cout << "  - " << party[i] << " (did not survive)" << endl;
			}
			stopSim = true;
			break; // exit main simulation loop so the final summary runs
		}
		// Village every ~100 miles
		// Check if a village is reached (every 100 miles)
		if (milesTraveled - lastVillageMiles >= 100) {
			lastVillageMiles = (milesTraveled / 100) * 100;
			cout << "You have reached a small settlement at mile " << milesTraveled << ". Would you like to stop and resupply? (y/n): ";
			string ans;
				if (travelMode == 2) {
					// pause auto-run so the player can choose in the settlement
					cout << "Auto-run paused at the settlement so you can make resupply choices." << endl;
					travelMode = 1; // force manual mode so user can interact
				}
				getline(cin, ans);
				// If the player chooses to stop, enter the settlement
			if (!ans.empty() && (ans == "y" || ans == "Y")) {
				// Determine if this settlement is a medical station (small chance)
				bool isMedicalStation = (rng() % 100) < 33; // 33% chance (1/3)
				displaySettlementArt(isMedicalStation);
				if (isMedicalStation) cout << "You have found a medical station at this settlement. Dysentery can be treated here (3 days required)." << endl;
				else cout << "Welcome to the settlement. You may purchase multiple items before you leave." << endl;
				// Show the player's current inventory when arriving at a settlement
				displayInventory(inv, startingGold);
				// Enter settlement shopping loop
				while (true) {
					cout << "Available supplies:" << endl;
					cout << "1. Food bundles: 50lbs-30g, 100lbs-55g, 200lbs-100g" << endl;
					cout << "2. Ammunition: 20-12g, 50-30g, 100-55g" << endl;
					cout << "3. Spare parts: 1-60g, 2-110g" << endl;
					cout << "4. Oxen: 2-120g, 4-230g" << endl;
					cout << "5. Medicine kits: 1-25g, 3-60g" << endl;
					cout << "6. Sell items from your inventory" << endl;
					cout << "T) Talk to locals" << endl;
					// If this is a medical station, offer the option to stay for treatment
					if (isMedicalStation) {
						cout << "S) Stay one day at the medical station" << endl;
						cout << "0. Leave settlement" << endl;
						cout << "Remaining gold: " << startingGold << endl;
						cout << "Enter the number of the category to purchase (0 to leave), or T to talk: ";
					}
			string sc; 
			getline(cin, sc);
			// Talk to locals shortcut
			if (sc == "T" || sc == "t") { 
				meetLocals(inv, party, alive, severity, rng, startingGold); 
				continue; 
			}
			int shopChoice = 0; try { shopChoice = stoi(sc); } catch(...) { shopChoice = -1; }
					if (shopChoice == 0) { cout << "Leaving settlement." << endl; break; }
					// Handle medical station stay (stay 1 day advances time at station)
					if (isMedicalStation && (sc == "S" || sc == "s")) {
						// Advance a single day while at the medical station
						daysOnTrail++;
						// consume food for the day
						int livingCountLocal = 0; for (size_t ii=0; ii<alive.size(); ++ii) if (alive[ii]) livingCountLocal++;
						int dailyConsumptionLocal = livingCountLocal * 3;
						if (inv.foodPounds >= dailyConsumptionLocal) inv.foodPounds -= dailyConsumptionLocal;
						else { if (inv.foodPounds>0) { cout<<"You used remaining "<<inv.foodPounds<<" lbs of food."<<endl; inv.foodPounds=0; } cout<<"Food shortage while at station."<<endl; }
						// Progress dysentery countdown and any treatments
						for (size_t ii = 0; ii < party.size(); ++ii) {
							if (!alive[ii]) continue;
							if (dysenteryDays[ii] > 0) {
								// Start treatment the first day this person stays at the station
								if (dysenteryTreatmentDays[ii] <= 0) {
									dysenteryTreatmentDays[ii] = 3;
									cout << party[ii] << " has begun treatment for dysentery. 3 days required." << endl;
								}
								dysenteryDays[ii] -= 1;
								if (dysenteryTreatmentDays[ii] > 0) {
									dysenteryTreatmentDays[ii] -= 1;
									cout << party[ii] << " is receiving treatment. Days remaining: " << dysenteryTreatmentDays[ii] << "." << endl;
									if (dysenteryTreatmentDays[ii] <= 0) {
										// cured
										dysenteryDays[ii] = 0;
										severity[ii] = 0; sick[ii] = false; sicknessDesc[ii] = "";
										cout << party[ii] << " has been cured of dysentery at the medical station." << endl;
									}
								}
								// check death after treatment progression
								if (dysenteryDays[ii] <= 0 && dysenteryTreatmentDays[ii] <= 0) {
									// died earlier or cured; nothing
								} else if (dysenteryDays[ii] <= 0 && dysenteryTreatmentDays[ii] <= 0) {
									// placeholder
								} else if (dysenteryDays[ii] <= 0) {
									alive[ii] = false; sick[ii] = false; severity[ii] = 0; sicknessDesc[ii] = "";
									cout << "Tragic news: " << party[ii] << " has died from dysentery while at the station." << endl;
									consumeClothingForDeath(inv, party[ii]);
								}
							}
						}
						// show a simple daily station summary
						displayDayCounter(daysOnTrail);
						renderScene(inv, party, alive, severity, wagonBroken);
						continue;
					}
					// Handle purchases and sales
					if (shopChoice == 1) {
						cout << "Choose bundle: 1=50lbs(30g), 2=100lbs(55g), 3=200lbs(100g): "; string b; getline(cin,b); int bc=0; try{bc=stoi(b);}catch(...){bc=0;} if (bc==1 && startingGold>=30){inv.foodPounds+=50; startingGold-=30; cout<<"Purchased 50 lbs."<<endl;} else if (bc==2 && startingGold>=55){inv.foodPounds+=100; startingGold-=55; cout<<"Purchased 100 lbs."<<endl;} else if (bc==3 && startingGold>=100){inv.foodPounds+=200; startingGold-=100; cout<<"Purchased 200 lbs."<<endl;} else cout<<"Cannot purchase or invalid selection."<<endl;
					}
					else if (shopChoice == 2) {
						cout << "Choose ammo: 1=20(12g), 2=50(30g), 3=100(55g): "; string a; getline(cin,a); int ac=0; try{ac=stoi(a);}catch(...){ac=0;} if (ac==1 && startingGold>=12){inv.bullets+=20; startingGold-=12; cout<<"Purchased 20 bullets."<<endl;} else if (ac==2 && startingGold>=30){inv.bullets+=50; startingGold-=30; cout<<"Purchased 50 bullets."<<endl;} else if (ac==3 && startingGold>=55){inv.bullets+=100; startingGold-=55; cout<<"Purchased 100 bullets."<<endl;} else cout<<"Cannot purchase or invalid selection."<<endl;
					}
					else if (shopChoice == 3) {
						cout << "Choose spare parts: 1=1(60g), 2=2(110g): "; string p; getline(cin,p); int pc=0; try{pc=stoi(p);}catch(...){pc=0;} if (pc==1 && startingGold>=60){inv.spareParts+=1; startingGold-=60; cout<<"Purchased 1 spare part."<<endl;} else if (pc==2 && startingGold>=110){inv.spareParts+=2; startingGold-=110; cout<<"Purchased 2 spare parts."<<endl;} else cout<<"Cannot purchase or invalid selection."<<endl;
					}
					else if (shopChoice == 4) {
						cout << "Choose oxen: 1=2(120g), 2=4(230g): "; string o; getline(cin,o); int oc=0; try{oc=stoi(o);}catch(...){oc=0;} if (oc==1 && startingGold>=120){inv.oxen+=2; startingGold-=120; cout<<"Purchased 2 oxen."<<endl;} else if (oc==2 && startingGold>=230){inv.oxen+=4; startingGold-=230; cout<<"Purchased 4 oxen."<<endl;} else cout<<"Cannot purchase or invalid selection."<<endl;
					}
					else if (shopChoice == 5) {
						cout << "Choose medicine: 1=1(25g), 2=3(60g): "; string m; getline(cin,m); int mc=0; try{mc=stoi(m);}catch(...){mc=0;} if (mc==1 && startingGold>=25){inv.medicineKits+=1; startingGold-=25; cout<<"Purchased 1 medicine kit."<<endl;} else if (mc==2 && startingGold>=60){inv.medicineKits+=3; startingGold-=60; cout<<"Purchased 3 medicine kits."<<endl;} else cout<<"Cannot purchase or invalid selection."<<endl;
					}
					else if (shopChoice == 6) {
						// Selling menu
						cout << "Sell which item?\n";
						cout << "1) Food (sell by pounds)\n";
						cout << "2) Bullets\n";
						cout << "3) Spare parts\n";
						cout << "4) Oxen\n";
						cout << "5) Medicine kits\n";
						cout << "0) Cancel\n";
						cout << "Enter choice: "; string sellChoiceLine; getline(cin, sellChoiceLine);
						// Convert input to integer with error handling
						int sellChoice = -1; 
						try { 
							sellChoice = stoi(sellChoiceLine); 
						} 
						catch(...) { 
							sellChoice = -1; 
						}
						if (sellChoice == 0) { 
							cout << "Cancel selling." << endl; 
						}
						else if (sellChoice == 1) {
							cout << "Enter pounds of food to sell: "; 
							string amt; 
							getline(cin, amt); 
							int pounds=0; 
							try {
								pounds=stoi(amt);
							}
							catch(...) {
								pounds=0;
							} 
							if (pounds>0 && inv.foodPounds>=pounds) {
								// Prevent selling below recommended starting food (200 lbs per person)
								int recommendedFood = 200 * static_cast<int>(party.size());
								if (inv.foodPounds - pounds < recommendedFood) {
									cout << "You cannot sell that much food; must keep at least " << recommendedFood << " lbs for recommended starting supplies." << endl;
								} else {
									double pricePerLb = 0.3; // sell at 0.3 gold per lb
									int goldGain = static_cast<int>(round(pounds * pricePerLb));
									inv.foodPounds -= pounds; startingGold += goldGain; cout << "Sold "<<pounds<<" lbs of food for "<<goldGain<<" gold."<<endl;
								}
							} else cout<<"Cannot sell that amount of food."<<endl;
						}
						else if (sellChoice == 2) {
							cout << "Enter number of bullets to sell: "; string amt; getline(cin, amt); int num=0; try{num=stoi(amt);}catch(...){num=0;} if (num>0 && inv.bullets>=num) {
								// Prevent selling below recommended bullets (100 per person)
								int recommendedBullets = 100 * static_cast<int>(party.size());
								if (inv.bullets - num < recommendedBullets) {
									cout << "You cannot sell that many bullets; must keep at least " << recommendedBullets << " bullets for recommended starting supplies." << endl;
								} else {
									double pricePerBullet = 0.3; int goldGain = static_cast<int>(round(num * pricePerBullet)); inv.bullets -= num; startingGold += goldGain; cout<<"Sold "<<num<<" bullets for "<<goldGain<<" gold."<<endl;
								}
							} else cout<<"Cannot sell that many bullets."<<endl;
						}
						else if (sellChoice == 3) {
							cout << "Enter number of spare parts to sell: "; string amt; getline(cin, amt); int num=0; try{num=stoi(amt);}catch(...){num=0;} if (num>0 && inv.spareParts>=num) { int goldGain = num * 30; inv.spareParts -= num; startingGold += goldGain; cout<<"Sold "<<num<<" spare parts for "<<goldGain<<" gold."<<endl; } else cout<<"Cannot sell that many spare parts."<<endl;
						}
						else if (sellChoice == 4) {
							cout << "Enter number of oxen to sell: "; string amt; getline(cin, amt); int num=0; try{num=stoi(amt);}catch(...){num=0;} if (num>0 && inv.oxen>=num) { 
								// Prevent selling below recommended starting amount
								int recommendedOxen = 2 * static_cast<int>(party.size()); // 2 oxen per person recommended
								if (inv.oxen - num < recommendedOxen) {
									cout << "You cannot sell below the recommended starting amount of " << recommendedOxen << " oxen." << endl;
								} else {
									int goldGain = num * 50; inv.oxen -= num; startingGold += goldGain; cout<<"Sold "<<num<<" oxen for "<<goldGain<<" gold."<<endl;
								}
							} else cout<<"Cannot sell that many oxen."<<endl;
						}
						else if (sellChoice == 5) {
							cout << "Enter number of medicine kits to sell: "; string amt; getline(cin, amt); int num=0; try{num=stoi(amt);}catch(...){num=0;} if (num>0 && inv.medicineKits>=num) { int goldGain = num * 12; inv.medicineKits -= num; startingGold += goldGain; cout<<"Sold "<<num<<" medicine kits for "<<goldGain<<" gold."<<endl; } else cout<<"Cannot sell that many medicine kits."<<endl;
						}
						else {
							cout << "Invalid sell option." << endl;
						}
					}
					else {
						cout << "Invalid option. Please choose a number from the menu." << endl;
					}
					// After each purchase attempt, show capacity and warn if overloaded
					double load = 0.0;
					load += inv.foodPounds; // lbs
					load += inv.woodPounds; // lbs
					load += inv.spareParts * 30; // estimate 30 lbs per spare parts set
					load += inv.clothingSets * 5; // estimate 5 lbs per clothing set
					load += inv.medicineKits * 5; // estimate 5 lbs per kit
					load += inv.bullets * 0.1; // bullets
					double capacity = inv.oxen * 500 + 200; // wagon base capacity + oxen
					cout << "Estimated carried weight: " << static_cast<int>(load) << " lbs. Capacity: " << static_cast<int>(capacity) << " lbs." << endl;
					if (load > capacity) cout << "Warning: Your current supplies exceed your wagon's carrying capacity. Consider buying more oxen or removing items." << endl;
					// loop continues, allow the player to buy more items or leave
				}
			}
		}
		// Hunting trigger when low on food every ~50 miles
		if (inv.foodPounds < initialFoodCapacity * 0.25 && milesTraveled - lastHuntMiles >= 50) {
			lastHuntMiles = milesTraveled;
			cout << "Food is low (" << inv.foodPounds << " lbs). Opportunity to hunt encountered. Attempt hunt? (y/n): ";
			string huntAns;
			if (travelMode == 2) huntAns = "y"; else getline(cin, huntAns);
			if (!huntAns.empty() && (huntAns == "y" || huntAns == "Y")) {
				int roll = rng() % 100;
				// Auto-hunting always uses medium settings
				double useSuccessProb = (travelMode == 2) ? 0.65 : huntSuccessProb;
				int useMin = (travelMode == 2) ? 30 : huntYieldMin;
				int useMax = (travelMode == 2) ? 80 : huntYieldMax;
				int successThresh = (int)round(useSuccessProb * 100.0);
				if (roll < successThresh) {
					uniform_int_distribution<int> yieldDist(useMin, useMax);
					int gain = yieldDist(rng);
					inv.foodPounds += gain;
					cout << "Hunt successful! You gained " << gain << " lbs of food." << endl;
				} else {
					cout << "Hunt failed. You returned with no food." << endl;
				}
			}
		}
		// Small daily chance an oxen dies
		if (inv.oxen > 0) {
			int oxRoll = oxDeathDist(rng);
			if (oxRoll < 3) { // 3% chance
				inv.oxen -= 1;
				// apply small random slowdown 1-2%
				int slowdown = (rng() % 2) + 1; // 1 or 2
				speedMultiplier *= (1.0 - slowdown / 100.0);
				cout << "A misfortune: one of your oxen has died. Remaining oxen: " << inv.oxen << ". Travel speed reduced by " << slowdown << "%." << endl;
				renderScene(inv, party, alive, severity, wagonBroken);
				// If you have at least 2 oxen left, wagons still pull; otherwise heavy slowdown
				if (inv.oxen < 2) {
					speedMultiplier *= 0.6; // big penalty when fewer than 2 oxen
					cout << "You have fewer than 2 oxen; travel is severely slowed." << endl;
				}
			}
		}
		// Small daily chance the wagon breaks
		// Only if it is not already broken
		if (!wagonBroken) {
			int wbRoll = wagonBreakDist(rng);
			// 2% chance per day
			if (wbRoll < 2) { // 2% chance per day
				cout << "Oh no! Your wagon has broken down." << endl;
				// If spare parts are available, prompt to use one to repair
				if (inv.spareParts > 0) {
					if (travelMode == 1) {
						cout << "You have " << inv.spareParts << " spare parts. Use one to repair now? (y/n): ";
						string ans; getline(cin, ans);
						if (!ans.empty() && (ans == "y" || ans == "Y")) {
						inv.spareParts -= 1;
						cout << "You used a spare part to repair the wagon." << endl;
						// Wagon repaired, no speed penalty
						renderScene(inv, party, alive, severity, wagonBroken);
						} else {
						wagonBroken = true;
						speedMultiplier *= 0.5;
						cout << "You chose not to repair now. Travel speed halved until repaired." << endl;
						renderScene(inv, party, alive, severity, wagonBroken);
						}
					} else {
						// auto mode: auto-repair if parts available
						inv.spareParts -= 1;
						cout << "Wagon broke but a spare part was used automatically to repair it." << endl;
						renderScene(inv, party, alive, severity, wagonBroken);
					}
				} else {
					wagonBroken = true;
					speedMultiplier *= 0.5;
					cout << "You have no spare parts. Travel speed halved until repaired." << endl;
				}
			}
		}
		// daily summary
		displayDayCounter(daysOnTrail);
		renderScene(inv, party, alive, severity, wagonBroken);
		// Update month based on days
		if (daysOnTrail % 30 == 0) {
			currentMonth++;
			if (currentMonth > 12) currentMonth = 1;
		}
		// Daily health updates: sick members may worsen or improve
		for (size_t i = 0; i < party.size(); ++i) {
			if (!alive[i]) continue;
			if (severity[i] > 0) {
				int roll = rng() % 100;
				// chance to worsen depends on current severity (made more likely)
				int worsenChance = 8 * severity[i]; // 8%,16%,24%
				int recoverChance = 8; // base ~8% daily recovery
				int prevSeverityForDay = severity[i];
				if (roll < worsenChance) {
					severity[i] = min(3, severity[i] + 1);
					cout << party[i] << "'s condition has worsened to severity " << severity[i] << "." << endl;
					consumeClothingForSeverity(inv, prevSeverityForDay, severity[i], party[i]);
				}
				else if (roll < worsenChance + recoverChance) {
					severity[i] = max(0, severity[i] - 1);
					if (severity[i] == 0) { sick[i] = false; sicknessDesc[i] = ""; cout << party[i] << " has recovered." << endl; }
					else cout << party[i] << " has improved to severity " << severity[i] << "." << endl;
				}
				// if severe, chance of death
				if (severity[i] >= 3) {
					int deathRoll = rng() % 100;
					if (deathRoll < 15) { // 15% daily death chance at severity 3
						alive[i] = false; sick[i] = false; severity[i] = 0;
						cout << "Tragic news: " << party[i] << " has died from their illness." << endl;
						consumeClothingForDeath(inv, party[i]);
						renderScene(inv, party, alive, severity, wagonBroken);
					}
				}
				// dysentery countdown handling
				if (dysenteryDays[i] > 0) {
					dysenteryDays[i] -= 1;
					if (dysenteryDays[i] <= 0) {
						// person dies from dysentery if countdown hits zero
						alive[i] = false; sick[i] = false; severity[i] = 0; sicknessDesc[i] = "";
						cout << "Tragic news: " << party[i] << " has died from dysentery." << endl;
						consumeClothingForDeath(inv, party[i]);
						renderScene(inv, party, alive, severity, wagonBroken);
					}
				}
			}
		}
	// Additional daily chance for a new illness event (independent of scheduled events)
	// Only enabled on Easy difficulty to preserve Medium/Hard scheduling behavior
	if (difficulty == 1) {
		// Small chance each day that a random alive party member falls ill
		int dailyIllnessRoll = rng() % 100;
		if (dailyIllnessRoll < 8) { // ~8% chance per day
			int attemptsLocal = 0;
			int idxLocal = -1;
			while (attemptsLocal < 10) {
				int candidate = memberDist(rng);
				if (alive[candidate]) { idxLocal = candidate; break; }
				attemptsLocal++;
			}
			if (idxLocal != -1) {
				int ill = illnessDist(rng);
				int prevSeverityLocal = severity[idxLocal];
				if (severity[idxLocal] == 0) severity[idxLocal] = 1; else severity[idxLocal] = min(3, severity[idxLocal] + 1);
				sick[idxLocal] = (severity[idxLocal] > 0);
				sicknessDesc[idxLocal] = illnesses[ill];
				cout << "Daily event: " << party[idxLocal] << " has fallen ill with " << sicknessDesc[idxLocal] << " (severity " << severity[idxLocal] << ")!" << endl;
				consumeClothingForSeverity(inv, prevSeverityLocal, severity[idxLocal], party[idxLocal]);
				renderScene(inv, party, alive, severity, wagonBroken);
				if (sicknessDesc[idxLocal] == "dysentery") {
					dysenteryDays[idxLocal] = 10;
					cout << "Warning: " << party[idxLocal] << " has contracted dysentery. Without treatment they will die in 10 days." << endl;
				} else {
					// Auto-apply medicine for non-dysentery illnesses
					if (inv.medicineKits > 0 && severity[idxLocal] > 0) {
						inv.medicineKits -= 1;
						severity[idxLocal] = max(0, severity[idxLocal] - 1);
						if (severity[idxLocal] == 0) {
							sick[idxLocal] = false; sicknessDesc[idxLocal] = ""; cout << party[idxLocal] << " was automatically treated with a medicine kit and has recovered." << endl;
						} else {
							cout << party[idxLocal] << " was automatically treated with a medicine kit; severity is now " << severity[idxLocal] << "." << endl;
						}
					}
				}
			}
		}
	}
	// Check for medical event (new illness)
	daysUntilMedical--;
		if (daysUntilMedical <= 0) {
			// Pick a random alive party member
			int attempts = 0;
			int idx = -1;
			while (attempts < 10) {
				int candidate = memberDist(rng);
				if (alive[candidate]) { idx = candidate; break; }
				attempts++;
			}
			if (idx != -1) {
				int ill = illnessDist(rng);
				// if already sick increase severity, else start at mild
				int prevSeverityScheduled = severity[idx];
				if (severity[idx] == 0) severity[idx] = 1; else severity[idx] = min(3, severity[idx] + 1);
				sick[idx] = (severity[idx] > 0);
				sicknessDesc[idx] = illnesses[ill];
				// If dysentery is contracted, it is always fatal
				cout << "Event: " << party[idx] << " has fallen ill with " << sicknessDesc[idx] << " (severity " << severity[idx] << ")!" << endl;
				consumeClothingForSeverity(inv, prevSeverityScheduled, severity[idx], party[idx]);
				renderScene(inv, party, alive, severity, wagonBroken);if (sicknessDesc[idx] == "dysentery") {
					// Dysentery: start a 10-day countdown until death (unless cured in time)
					dysenteryDays[idx] = 10;
					cout << "Warning: " << party[idx] << " has contracted dysentery. Without treatment they will die in 10 days." << endl;
				} else {
					// Auto-apply medicine for non-dysentery illnesses
					if (inv.medicineKits > 0 && severity[idx] > 0) {
						inv.medicineKits -= 1;
						severity[idx] = max(0, severity[idx] - 1);
						if (severity[idx] == 0) {
							sick[idx] = false;
							sicknessDesc[idx] = "";
							cout << party[idx] << " was automatically treated with a medicine kit and has recovered." << endl;
						} else {
							cout << party[idx] << " was automatically treated with a medicine kit; severity is now " << severity[idx] << "." << endl;
						}
					}
				}
			}
			// schedule next medical event
			daysUntilMedical = medicalDist(rng);
		}
		// Check if entire party is dead; if so end the game
		{
			bool anyAlive = false;
			for (size_t i = 0; i < alive.size(); ++i) {
				if (alive[i]) { anyAlive = true; break; }
			}
			if (!anyAlive) {
				cout << "All party members have died. The game is over." << endl;
				// Show final summary, save score, and offer restart
				stopSim = true;
				playerStatus = 0;
				break; // exit main simulation loop
			}
		}
		// Display status snapshot each week
		if (daysOnTrail % 7 == 0) {
			cout << "Current month: " << currentMonth << endl;
			cout << "Days on trail: " << daysOnTrail << endl;
			// Show current inventory
			displayDayCounter(daysOnTrail);
			cout << "Miles traveled: " << milesTraveled << endl;
			cout << "Miles remaining to Oregon: " << (milesToOregon - milesTraveled) << endl;
			// show any sick party members
			for (size_t i = 0; i < party.size(); ++i) {
				if (!alive[i]) cout << party[i] << " is dead." << endl;
				else if (sick[i]) cout << party[i] << " is sick (" << sicknessDesc[i] << ") severity " << severity[i] << endl;
			}
		}
		// If auto mode, handle sleeping and stop conditions
		if (travelMode == 2) {
			// small pause so output is readable
			this_thread::sleep_for(chrono::milliseconds(autoDelayMs));
			if (autoDaysToRun > 0 && autoDaysRunNow >= autoDaysToRun) {
				cout << "Auto-run completed " << autoDaysRunNow << " days. Switching back to manual mode." << endl;
				travelMode = 1; // hand control back to the player instead of ending the game
				autoDaysRunNow = 0; // reset so a future auto batch starts counting fresh
			}
		}
	}
	cout << "\n--- Final Summary ---" << endl;
	cout << "Days on trail: " << daysOnTrail << endl;
	cout << "Miles traveled: " << milesTraveled << " / " << milesToOregon << endl;
	int survivors = 0;
	for (size_t i = 0; i < party.size(); ++i) if (alive[i]) survivors++;
	cout << "Survivors: " << survivors << " / " << party.size() << endl;
	displayInventory(inv, startingGold);
	ofstream scoreFile("scores.txt", ios::app);
	if (scoreFile) {
		time_t now = time(nullptr);
		char dateBuffer[26]{};
		ctime_s(dateBuffer, sizeof(dateBuffer), &now);
		scoreFile << "Days: " << daysOnTrail << ", Miles: " << milesTraveled
			<< ", Survivors: " << survivors << ", Gold: " << startingGold
			<< ", Date: " << dateBuffer;
		cout << "Score saved to scores.txt" << endl;
	} else {
		cout << "Could not open scores.txt to save score." << endl;
	}
	cout << "Would you like to play again? (y/n): ";
	string replayAnswer;
	getline(cin, replayAnswer);
	if (!replayAnswer.empty() && (replayAnswer == "y" || replayAnswer == "Y")) {
		cout << "Restarting game..." << endl;
		// Clear the visible screen (2J), the scrollback history (3J), and move the cursor to the top-left (H)
		cout << "\x1b[2J\x1b[3J\x1b[H" << flush;
		return main();
	}
	cout << "Thanks for playing." << endl;
	return 0;
}