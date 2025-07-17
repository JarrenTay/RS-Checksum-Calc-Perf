#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <bitset>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <set>
#include <stdlib.h>
#include "RSChecksumCalculator.h"
#include "ThreadPool.h"

using namespace std;
using namespace chrono;

const long long PID = 1321080;
const int DATA_ORDER_G = 1;
const int DATA_ORDER_A = 4;
const int DATA_ORDER_E = 7;
const int DATA_ORDER_M = 10;
const string CSV_HEADER = "Player frame,Enemy Frame,Player TID/SID,Enemy TID/SID,Species,Held Item,Moves,Pokeball,Egg,Enemy Mon,Move 1 PP,Move 2 PP,Move 3 PP,Move 4 PP,Enemy List Index";
const string MATCH_FOLDER = "./cppMatchesPP";
const string ACE_FOLDER = "./cppAcesPP";

struct ChecksumMatchResults {
    bool match;
    bool ace;
    long long keyXorData0;
    long long keyXorData3;
    long long keyXorData4;
    long long keyXorData10;
};

int main(int argc, char* argv[]) {
    steady_clock::time_point start = steady_clock::now();
    
    // Argument Parsing
    vector<int> arguments = parseArguments(argc, argv);
    handleArguments(arguments);

    // Parse Data Files
    vector<string> enemyList = {};
    map<string, vector<long long>> enemyDict = dataFileToMap("enemyDataList.csv", enemyList);
    vector<vector<int>> otidVector = otidFileToVector("OTIDs.csv");
    string dataOrder[24] = {
        "GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA",
        "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
        "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG",
        "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"
    };
    map<string, vector<int>> dataOrderOrder = dataOrderToMap("dataOrder.csv");

    // Create Directory
    try {
        filesystem::remove_all(MATCH_FOLDER);
        filesystem::remove_all(ACE_FOLDER);
    }
    catch (int _) {}
	try {
		filesystem::create_directory(MATCH_FOLDER);
		filesystem::create_directory(ACE_FOLDER);
	}
	catch (int errorCode) {
		cout << "Error creating output directories: " << errorCode << endl;
		return 1;
	}

    // Calculate checksums
    cout << "Calculating checksums" << endl;
    calculateChecksums(arguments, dataOrder, dataOrderOrder, enemyList, enemyDict, otidVector);

    cout << "Writing to file" << endl;
    // Combine checksum files
    combineChecksumFiles();

    // Check Time Elapsed
    steady_clock::time_point end = steady_clock::now();
    cout << "Time elapsed: " << (duration_cast<microseconds> (end - start).count()) / 1000000 << " seconds" << std::endl;
    return 0;
}

/* ******************************************************
 * Purpose: Parses passed arguments and assigns defaults
 * ******************************************************
 * Parameters:
 *   argc: Number of arguments
 *   argv: Char* array of arguments
 * ******************************************************
*/
vector<int> parseArguments(int argc, char* argv[]) {
    vector<int> arguments = vector<int>();
    if (argc >= 2) {
        string arg = argv[1];
        int startingTid = stoi(arg);
        arguments.push_back(startingTid);
    } else {
        arguments.push_back(100001);
    }
    if (argc >= 3) {
        string arg = argv[2];
        int endingTid = stoi(arg);
        arguments.push_back(endingTid);
    } else {
        arguments.push_back(100001);
    }
    if (argc >= 4) {
        string arg = argv[3];
        int frameCount = stoi(arg);
        arguments.push_back(frameCount);
    } else {
        arguments.push_back(4000);
    }
    if (argc >= 5) {
        string arg = argv[4];
        int threads = stoi(arg);
        arguments.push_back(threads);
    }
    else {
        arguments.push_back(1);
    }
    if (argc >= 6) {
        string arg = argv[5];
        int maxPpUsage = stoi(arg);
        arguments.push_back(maxPpUsage);
    }
    else {
        arguments.push_back(1);
    }
    if (argc >= 7) {
        string arg = argv[6];
        int maxEnemyIndex = stoi(arg);
        arguments.push_back(maxEnemyIndex);
    }
    else {
        arguments.push_back(100000);
    }
    if (argc >= 8) {
        string arg = argv[7];
        bool ruby = stoi(arg);
        arguments.push_back(ruby);
    }
    else {
        arguments.push_back(false);
    }
    return arguments;
}

/* ******************************************************
 * Purpose: Checks for tid and frame argument validity
 * ******************************************************
 * Parameters:
 *   args: Vector [TID start, TID end, Frame amount]
 * ******************************************************
*/
void handleArguments(vector<int> &args) {
    if (args[0] < 0) {
        cout << "TID lower bound exceeded, set to 0." << endl;
        args[0] = 0;
    }
    if (args[0] > 100001) {
        cout << "TID upper bound exceeded, set to 100000." << endl;
        args[0] = 100001;
    }
    if (args[1] < args[0]) {
        cout << "TID range error, upper bound set to lower bound." << endl;
        args[1] = args[0];
    }
    if (args[1] > 100001) {
        cout << "TID upper bound exceeded, set to 100000." << endl;
        args[1] = 100001;
    }
    if (args[2] < 1) {
        cout << "Frame lower bound exceeded, set to 1." << endl;
        args[2] = 1;
    }
    if (args[2] > 100000) {
        cout << "Frame upper bound exceeded, set to 100000." << endl;
        args[2] = 100000;
    }
    if (args[3] < 1) {
        cout << "Thread count lower bound exceeded, set to 1." << endl;
        args[3] = 1;
    }
    if (args[3] > thread::hardware_concurrency()) {
        cout << "Thread count upper bound exceeded, set to hardware upper limit of " << thread::hardware_concurrency() << endl;
        args[3] = thread::hardware_concurrency();
    }
    if (args[4] < 0) {
        cout << "Max PP Usage lower bound exceeded, set to 0." << endl;
        args[4] = 0;
    }
    if (args[4] > 40) {
        cout << "Thread count upper bound exceeded, set to hardware upper limit of " << 40 << endl;
        args[4] = 40;
    }
    if (args[5] < 0) {
        cout << "Max enemy index lower bound exceeded, set to 1." << endl;
        args[5] = 1;
    }
}

/* ******************************************************
 * Purpose: Parses enemyDataList.csv
 * ******************************************************
 * Parameters:
 *   fileName: Name of file containing enemy data
 *   enemyList: Outputs vector of enemy mons
 * ******************************************************
*/
map<string, vector<long long>> dataFileToMap(string fileName, vector<string> &enemyList) {
    string enemyDataRawLine = "";
    ifstream enemyDataFile(fileName);
    map<string, vector<long long>> enemyDict = map<string, vector<long long>>();
    const string DELIMITER = ",";

    while (getline(enemyDataFile, enemyDataRawLine)) {
        string enemyMon = enemyDataRawLine.substr(0, enemyDataRawLine.find(DELIMITER));
        string enemyData = enemyDataRawLine.substr(enemyDataRawLine.find(DELIMITER) + 1);
        vector<long long> enemyDataVector = vector<long long>();
        
        long long pieceOne = hexStringToIntLittleEndian(enemyData.substr(0, 8));
        enemyDataVector.push_back(pieceOne);
        for (int i = 0; i < 12; i++) {
            int pieceTwoStart = (i + 8) * 8;
            long long pieceTwo = hexStringToIntLittleEndian(enemyData.substr(pieceTwoStart, 8));
            long long pieceThree = hexStringToIntLittleEndian(enemyData.substr(8 , 8));
            enemyDataVector.push_back(pieceTwo ^ pieceOne ^ pieceThree);
        }
        enemyDict.insert(pair<string, vector<long long>>(enemyMon, enemyDataVector));
        enemyList.push_back(enemyMon);
    }
    return enemyDict;
}

/* ******************************************************
 * Purpose: Converts Data Order into a map
 * ******************************************************
 * Parameters:
 *   fileName: Name of file containing data order
 * ******************************************************
*/
map<string, vector<int>> dataOrderToMap(string fileName) {
    string dataOrderRawLine = "";
    ifstream dataOrderFile(fileName);
    map<string, vector<int>> dataOrderDict = map<string, vector<int>>();
    const string DELIMITER = ",";

    while (getline(dataOrderFile, dataOrderRawLine)) {
        string dataOrderString = dataOrderRawLine.substr(0, dataOrderRawLine.find(DELIMITER));
        vector<int> dataOrderOrderVector = vector<int>();

        int startIndex = dataOrderRawLine.find(DELIMITER);
        string dataOrderPiece = "";
        for (int i = 0; i < 12; i++) {
            dataOrderPiece = dataOrderRawLine.substr(startIndex + 1, dataOrderRawLine.find(DELIMITER, startIndex + 1));
            startIndex = dataOrderRawLine.find(DELIMITER, startIndex + 1);
            dataOrderOrderVector.push_back(stoi(dataOrderPiece));
        }
        dataOrderDict.insert(pair<string, vector<int>>(dataOrderString, dataOrderOrderVector));
    }
    return dataOrderDict;
}

/* ******************************************************
 * Purpose: Converts a hex string to little endian
 * ******************************************************
 * Parameters:
 *   hexString: Hex string to convert
 * ******************************************************
*/
long long hexStringToIntLittleEndian(string hexString) {
    string reversedHexString = "";
    for (int i = 0; i * 2 < hexString.length(); i++) {
        reversedHexString = hexString.substr(i * 2, 2) + reversedHexString;
    }
    return stoll(reversedHexString, 0, 16);
}

/* ******************************************************
 * Purpose: Parses Otid file into a 2d array
 * ******************************************************
 * Parameters:
 *   fileName: Hex string to convert
 * ******************************************************
*/
vector<vector<int>> otidFileToVector(string fileName) {
    string otidDataRawLine = "";
    ifstream otidDataFile(fileName);
    vector<vector<int>> otidVector = vector<vector<int>>();
    const string DELIMITER = ",";
    bool headerRow = true;

    while (getline(otidDataFile, otidDataRawLine)) {
        if (headerRow) {
            headerRow = false;
            continue;
        }

        int firstCommaIndex = otidDataRawLine.find(DELIMITER);
        int secondCommaIndex = otidDataRawLine.find(DELIMITER, firstCommaIndex + 1);
        int thirdCommaIndex = otidDataRawLine.find(DELIMITER, secondCommaIndex + 1);
        string advancesString = otidDataRawLine.substr(0, firstCommaIndex);
        string tidString = otidDataRawLine.substr(firstCommaIndex + 1, secondCommaIndex);
        string sidString = otidDataRawLine.substr(secondCommaIndex + 1, thirdCommaIndex);
        vector<int> otidRow = vector<int>();
        
        otidRow.push_back(stoi(advancesString));
        otidRow.push_back(stoi(tidString));
        otidRow.push_back(stoi(sidString));

        otidVector.push_back(otidRow);
    }
    return otidVector;
}

/* ******************************************************
 * Purpose: Organizes threads to calculate checksums
 * ******************************************************
 * Parameters:
 *   arguments: Arguments from command line
 *     [0] : TID Start
 *     [1] : TID End
 *     [2] : Frames to calculate
 *     [3] : Number of threads
 *     [4] : Max PP Usage
 *     [5] : Max enemy index
 *     [6] : Ruby or Sapphire
 *   dataOrder: idk what this is
 *   enemyList: vector of enemy mons
 *   enemyDict: map of enemy mon to enemy data
 *   otidVector: vector of otid data
 * ******************************************************
*/
void calculateChecksums(vector<int> arguments, const string dataOrder[], map<string, vector<int>> dataOrderOrder, vector<string> enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector) {
    // Calculate Checksums
    cout << "Executing with TIDs " << arguments[0] << " to " << arguments[1] << " (inclusive) and the first " << arguments[2] << " frames" << " using " << arguments[3] << " threads." << endl;
    cout << "Maximum PP Usage " << arguments[4] << " and max enemy index of " << arguments[5] << "." << endl;
    if (arguments[6]) {
        cout << "Calculating for Ruby." << endl;
    }
    else {
        cout << "Calculating for Sapphire." << endl;
    }

    this_thread::sleep_for(chrono::milliseconds(2500));

    ThreadPool pool(arguments[3]);
    for (int tid = arguments[0]; tid <= arguments[1]; tid++) {
        
        int frames = arguments[2];
        int enemyListSize = min(arguments[5], (int) enemyList.size());
        int dataOrderStringLength = 4;

        // Trainer ID is inclusive. We don't do subtraction in TID like in python bc we don't need to account for header row.
        string playerHex = intToHex(otidVector[tid][2], 4) + intToHex(otidVector[tid][1], 4).substr(2);
        long long playerLongLong = stoll(playerHex, 0, 16);
        long long playerKey = PID ^ playerLongLong;

        // Start at frame 0. Python version starts at 1 bc of header column
        for (int frame = 0; frame < frames; frame++) {

            string enemyHex = intToHex(otidVector[frame][1], 4) + intToHex(otidVector[frame][2], 4).substr(2);
            long long enemyLongLong = stoll(enemyHex, 0, 16);
            long long enemyKey = PID ^ enemyLongLong;

            // Loop through all mons
            for (int enemyListIndex = 0; enemyListIndex < enemyListSize; enemyListIndex++) {

                pool.enqueue([=, &dataOrderOrder, &enemyList, &enemyDict, &otidVector]() {
                    calculateChecksumEnemyMonThread(tid, frame, playerKey, enemyKey, dataOrder, ref(dataOrderOrder), ref(enemyList), ref(enemyDict), ref(otidVector), enemyListIndex, arguments[4], arguments[6]);
                    });
            }
        }
    }
    // Wait for all threads to finish.
    pool.stopAndWait();
                
}

/* ******************************************************
 * Purpose: Calculates checksum for an enemy mon
 * ******************************************************
*/
void calculateChecksumEnemyMonThread(int tid, int frame, long long playerKey, long long enemyKey, const string dataOrder[], map<string, vector<int>> dataOrderOrder, vector<string> enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector, int enemyListIndex, int maxPpUsage, bool ruby) {

    // Delete output files if they exist and create a new one.
    string matchFilePath = MATCH_FOLDER + "/" + to_string(frame) + "_" + to_string(enemyListIndex) + ".csv";
    string aceFilePath = ACE_FOLDER + "/" + to_string(frame) + "_" + to_string(enemyListIndex) + ".csv";
    try {
        filesystem::remove(matchFilePath);
        filesystem::remove(aceFilePath);
    }
    catch (int errorCode) {}
    ofstream matchFile(matchFilePath);
    ofstream aceFile(aceFilePath);


    long long data[12];
    string enemyMon = enemyList[enemyListIndex];
    vector<long long> enemyMonData = enemyDict[enemyMon];
    string dataOrderString = dataOrder[enemyMonData[0] % 24];
    vector<int> usedDataOrder = dataOrderOrder[dataOrderString];

    set<int> validPokeballs = { 2, 3, 4, 8, 9, 10, 12 };

    bool found = false;

    for (int dataOrderIndex = 0; dataOrderIndex < 12; dataOrderIndex++) {
        data[dataOrderIndex] = enemyMonData[usedDataOrder[dataOrderIndex]];
    }

    if (ruby) {
        long long data9Piece1 = data[9] & 0b11111000000000000000000000000000;
        long long data9Piece2 =           0b00000001000000000000000000000000;
        long long data9Piece3 = data[9] & 0b00000000011111111111111111111111;
        data[9] = data9Piece1 + data9Piece2 + data9Piece3;
    }

    long long ppMoveMax[4] = {};
    long long ppMoveMin[4] = {};

    ppMoveMax[0] = data[5] & 0b00000000000000000000000011111111;
    ppMoveMax[1] = (data[5] & 0b00000000000000001111111100000000) >> 8;
    ppMoveMax[2] = (data[5] & 0b00000000111111110000000000000000) >> 16;
    ppMoveMax[3] = (data[5] & 0b11111111000000000000000000000000) >> 24;

    for (int moveIndex = 0; moveIndex < 4; moveIndex++) {
        ppMoveMin[moveIndex] = max((long long)0, ppMoveMax[moveIndex] - maxPpUsage);
    }

    for (int ppMove0 = ppMoveMin[0]; ppMove0 < ppMoveMax[0] + 1; ppMove0++) {
        for (int ppMove1 = ppMoveMin[1]; ppMove1 < ppMoveMax[1] + 1; ppMove1++) {
            for (int ppMove2 = ppMoveMin[2]; ppMove2 < ppMoveMax[2] + 1; ppMove2++) {
                for (int ppMove3 = ppMoveMin[3]; ppMove3 < ppMoveMax[3] + 1; ppMove3++) {
                    data[5] = (ppMove3 << 24) + (ppMove2 << 16) + (ppMove1 << 8) + ppMove0;

                    // Loop through pokeballs. We quit as soon as we find a match, even though there are likely more of the same pokeball.
                    for (long long pokeballIndex = 1; pokeballIndex < 13; pokeballIndex++) {

                        // If not a valid pokeball, skip
                        if (validPokeballs.find(pokeballIndex) == validPokeballs.end()) {
                            continue;
                        }

                        long long data9Piece1 = data[9] & 0b10000111111111111111111111111111; // llToBin(data[9], 32).substr(2, 1) Get first bit
                        long long data9Piece2 = pokeballIndex << 27; // Shift bits over 27 to be next to Piece 1
                        data[9] = data9Piece1 + data9Piece2;
                        //const string CSV_HEADER = "Player frame,Enemy Frame,Player TID/SID,Enemy TID/SID,Species,Held Item,Moves,Pokeball,Egg,Enemy Mon,Move 1 PP,Move 2 PP,Move 3 PP,Move 4 PP,Enemy List Index";
                        ChecksumMatchResults matchResults = calculateMatch(data, playerKey, enemyKey);
                        if (matchResults.match) {
                            string matchOut =
                                to_string(tid) + "," +                                          // Player Frame
                                to_string(frame) + "," +                                        // Enemy Frame
                                to_string(otidVector[tid][1]) + " " +                           // Player TID
                                to_string(otidVector[tid][2]) + "," +                           // Player SID
                                to_string(otidVector[frame][2]) + " " +                         // Enemy TID
                                to_string(otidVector[frame][1]) + "," +                         // Enemy SID
                                "0x" + intToHex(matchResults.keyXorData0, 8).substr(6) + "," +  // Species
                                intToHex(matchResults.keyXorData0, 8).substr(0, 6) + "," +      // Held Item
                                "0x" + intToHex(matchResults.keyXorData3, 8).substr(6) + " " +  // Moves 1
                                intToHex(matchResults.keyXorData3, 8).substr(0, 6) + " " +      // Moves 2
                                "0x" + intToHex(matchResults.keyXorData4, 8).substr(6) + " " +  // Moves 3
                                intToHex(matchResults.keyXorData4, 8).substr(0, 6) + "," +      // Moves 4
                                to_string(pokeballIndex) + "," +                                // Pokeball
                                llToBin(matchResults.keyXorData10, 32).substr(3, 1) + "," +     // Egg
                                enemyMon + "," +                                                // Enemy Mon
                                to_string(ppMove0) + "," +                                      // Move 1 PP
                                to_string(ppMove1) + "," +                                      // Move 2 PP
                                to_string(ppMove2) + "," +                                      // Move 3 PP
                                to_string(ppMove3) + "," +                                      // Move 4 PP
                                to_string(enemyListIndex);                                      // Enemy List Index

                            found = true;

                            matchFile << matchOut << endl;

                            if (matchResults.ace) {
                                aceFile << matchOut << endl;
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "Finished frame " + to_string(frame) + " enemyMon " + to_string(enemyListIndex) + "\n";
    matchFile.close();
    aceFile.close();

    if (!found) {
        try {
            filesystem::remove(matchFilePath);
            filesystem::remove(aceFilePath);
        }
        catch (int errorCode) {}
    }
}

/* ******************************************************
 * Purpose: Calculates checksum based on data array and
 *   player and enemy key
 * ******************************************************
 * Parameters:
 *   data: Array of enemy mon data
 *   playerKey: Player key
 *   enemyKey: Enemy key
 * ******************************************************
*/
ChecksumMatchResults calculateMatch(long long data[], long long playerKey, long long enemyKey) {

    long long originalChecksum = ((data[0] % 65536) + (data[0] / 65536) + (data[1] % 65536) + (data[1] / 65536) + (data[2] % 65536)
        + (data[2] / 65536) + (data[3] % 65536) + (data[3] / 65536) + (data[4] % 65536) + (data[4] / 65536)
        + (data[5] % 65536) + (data[5] / 65536) + (data[6] % 65536) + (data[6] / 65536) + (data[7] % 65536)
        + (data[7] / 65536) + (data[8] % 65536) + (data[8] / 65536) + (data[9] % 65536) + (data[9] / 65536)
        + (data[10] % 65536) + (data[10] / 65536) + (data[11] % 65536) + (data[11] / 65536)) % 65536;

    long long keysXored = playerKey ^ enemyKey;

    long long newChecksum = (((keysXored ^ data[0]) % 65536) + ((keysXored ^ data[0]) / 65536) + ((keysXored ^ data[1]) % 65536) + ((keysXored ^ data[1]) / 65536) + ((keysXored ^ data[2]) % 65536)
        + ((keysXored ^ data[2]) / 65536) + ((keysXored ^ data[3]) % 65536) + ((keysXored ^ data[3]) / 65536) + ((keysXored ^ data[4]) % 65536) + ((keysXored ^ data[4]) / 65536)
        + ((keysXored ^ data[5]) % 65536) + ((keysXored ^ data[5]) / 65536) + ((keysXored ^ data[6]) % 65536) + ((keysXored ^ data[6]) / 65536) + ((keysXored ^ data[7]) % 65536)
        + ((keysXored ^ data[7]) / 65536) + ((keysXored ^ data[8]) % 65536) + ((keysXored ^ data[8]) / 65536) + ((keysXored ^ data[9]) % 65536) + ((keysXored ^ data[9]) / 65536)
        + ((keysXored ^ data[10]) % 65536) + ((keysXored ^ data[10]) / 65536) + ((keysXored ^ data[11]) % 65536) + ((keysXored ^ data[11]) / 65536)) % 65536;

    if (originalChecksum == newChecksum) {
        bool ace = (((keysXored ^ data[0]) % 65536) == 39710);
        ChecksumMatchResults matchResults = {
            true,                   // bool match;
            ace,                    // bool ace;
            keysXored ^ data[0],    // long long keyXorData0;
            keysXored ^ data[3],    // long long keyXorData3;
            keysXored ^ data[4],    // long long keyXorData4;
            keysXored ^ data[10],   // long long keyXorData10;
        };

        return matchResults;
    } else {
        ChecksumMatchResults matchResults = {
            false,  // bool match;
            false,  // bool ace;
            0,      // long long keyXorData0;
            0,      // long long keyXorData3;
            0,      // long long keyXorData4;
            0,      // long long keyXorData10;
        };

        return matchResults;
    }
}

/* ******************************************************
 * Purpose: Combines checksum files into one file
 * ******************************************************
*/
void combineChecksumFiles() {
	// Combine all match files into one
	ofstream combinedMatchFile("./combinedPPMatches.csv");
    combinedMatchFile << CSV_HEADER << endl;
	for (const auto& entry : filesystem::directory_iterator(MATCH_FOLDER)) {
		if (entry.path().extension() == ".csv" && entry.path().filename() != "combined.csv") {
			ifstream matchFile(entry.path());
			string line;
			while (getline(matchFile, line)) {
                combinedMatchFile << line << endl;
			}
		}
	}
    combinedMatchFile.close();

    // Combine all ace files into one
    ofstream combinedAceFile("./combinedPPAces.csv");
    combinedAceFile << CSV_HEADER << endl;
    for (const auto& entry : filesystem::directory_iterator(ACE_FOLDER)) {
        if (entry.path().extension() == ".csv" && entry.path().filename() != "combined.csv") {
            ifstream aceFile(entry.path());
            string line;
            while (getline(aceFile, line)) {
                combinedAceFile << line << endl;
            }
        }
    }
    combinedAceFile.close();
}

/* ******************************************************
 * Purpose: Replaces spaces in a number string with 0
 * ******************************************************
 * Parameters:
 *   number: string to pad
 * ******************************************************
*/
string padStringNumber(string number) {
    string outNumber = number;
    for (int charIndex = 0; charIndex < number.length(); charIndex++) {
        if (number[charIndex] == ' ') {
            outNumber[charIndex] = '0';
        }
    }
    return outNumber;
}

/* ******************************************************
 * Purpose: Converts int to hex string
 * ******************************************************
 * Parameters:
 *   i: integer to convert (can be an int-like type)
 *   len: Length of the hex string to return after 0x
 * ******************************************************
*/
template< typename T >
string intToHex(T i, int len)
{
    stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;
  
    string hex = stream.str();
    return "0x" + hex.substr(hex.length() - len);
}

/* ******************************************************
 * Purpose: Converts long long to binary string
 * ******************************************************
 * Parameters:
 *   longlong: long long to convert
 *   len: Length of the hex string to return after 0b
 * ******************************************************
*/
string llToBin(long long longlong, int len)
{
    return "0b" + bitset<32>(longlong).to_string().substr(32 - len);
}