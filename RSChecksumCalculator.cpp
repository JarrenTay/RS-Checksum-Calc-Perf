#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <format>
#include <filesystem>
#include <chrono>
#include "RSChecksumCalculator.h"

using namespace std;
using namespace chrono;

const long long PID = 1321080;
const int DATA_ORDER_G = 1;
const int DATA_ORDER_A = 4;
const int DATA_ORDER_E = 7;
const int DATA_ORDER_M = 10;
const string CSV_HEADER = "Player frame,Enemy Frame,Player TID/SID,Enemy TID/SID,Species,Held Item,Moves,Pokeball,Egg,Enemy Mon";
const string MATCH_FILE = "cppMatches.csv";
const string ACE_FILE = "cppAces.csv";

struct ChecksumMatchResults {
    bool match;
    bool ace;
    long long keyXorData0;
    long long keyXorData3;
    long long keyXorData4;
    long long keyXorData10;
};

int main() {
    steady_clock::time_point start = steady_clock::now();


    vector<string> enemyList = {};
    map<string, vector<long long>> enemyDict = dataFileToMap("enemyDataList.csv", enemyList);
    vector<vector<int>> otidVector = otidFileToVector("OTIDs.csv");
    string dataOrder[24] = {
        "GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA",
        "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
        "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG",
        "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"
    };

    // Edit this function call to search more TIDs and frames.
    calculateChecksumMatches(3575, 3577, 4000, dataOrder, enemyList, enemyDict, otidVector);

    steady_clock::time_point end = steady_clock::now();
    cout << "Time elapsed: " << (duration_cast<microseconds> (end - start).count()) / 1000000 << " seconds" << std::endl;
    return 0;
}

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

long long hexStringToIntLittleEndian(string hexString) {
    string reversedHexString = "";
    for (int i = 0; i * 2 < hexString.length(); i++) {
        reversedHexString = hexString.substr(i * 2, 2) + reversedHexString;
    }
    return stoll(reversedHexString, 0, 16);
}

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

void calculateChecksumMatches(int trainerIdStart, int trainerIdEnd, int frames, string dataOrder[], vector<string> &enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector) {

    try {
        filesystem::remove(MATCH_FILE);
        filesystem::remove(ACE_FILE);   
    } catch (int errorCode) { }
    
    ofstream matchFile(MATCH_FILE);
    ofstream aceFile(ACE_FILE);
    matchFile << CSV_HEADER << endl;
    aceFile << CSV_HEADER << endl;

    // Note, python version doesn't allow 1 and does subtraction to account for header column
    for (int tid = trainerIdStart; tid < trainerIdEnd; tid++) {
        cout << "Checking tid " << tid << endl;

        string playerHex = padStringNumber(format("0x{:4x}", otidVector[tid][2]) + format("{:4x}", otidVector[tid][1]));
        long long playerLongLong = stoll(playerHex, 0, 16);
        long long playerKey = PID ^ playerLongLong;

        // Start at frame 0 because python version starts at 1 bc of header column
        for (int frame = 0; frame < frames; frame++) {
            if (frame % 500 == 0) {
                cout << "Checking frame " << frame << endl;
            }

            string enemyHex = padStringNumber(format("0x{:4x}", otidVector[frame][1]) + format("{:4x}", otidVector[frame][2]));
            long long enemyLongLong = stoll(enemyHex, 0, 16);
            long long enemyKey = PID ^ enemyLongLong;
            long long data[12] = {};

            //for (auto const& [key, val] : enemyDict) {
            for (int enemyListIndex = 0; enemyListIndex < enemyList.size(); enemyListIndex++) {
                string enemyMon = enemyList[enemyListIndex];
                //vector<long long> enemyMonData = val;
                vector<long long> enemyMonData = enemyDict[enemyMon];
                string dataOrderString = dataOrder[enemyMonData[0] % 24];

                for (int dataOrderCharIndex = 0; dataOrderCharIndex < dataOrderString.length(); dataOrderCharIndex++) {
                    int enemyMonIndex = 0;
                    switch (dataOrderString[dataOrderCharIndex]) {
                        case 'G':
                            enemyMonIndex = DATA_ORDER_G;
                            break;
                        case 'A':
                            enemyMonIndex = DATA_ORDER_A;
                            break;
                        case 'E':
                            enemyMonIndex = DATA_ORDER_E;
                            break;
                        case 'M':
                            enemyMonIndex = DATA_ORDER_M;
                            break;
                    }
                    data[dataOrderCharIndex * 3] = enemyMonData[enemyMonIndex];
                    data[(dataOrderCharIndex * 3) + 1] = enemyMonData[enemyMonIndex + 1];
                    data[(dataOrderCharIndex * 3) + 2] = enemyMonData[enemyMonIndex + 2];
                }

                for (int pokeballIndex = 1; pokeballIndex < 13; pokeballIndex++) {
                    data[9] = stoll(padStringNumber(format("{:34b}", data[9]).substr(2, 1) + format("{:6b}", pokeballIndex).substr(2) + format("{:34b}", data[9]).substr(7)), 0, 2);

                    ChecksumMatchResults matchResults = calculateMatch(data, playerKey, enemyKey);
                    if (matchResults.match) {
                        string matchOut = format("{},{},{} {},{} {},0x{},{},0x{} {} 0x{} {},{},{},{}\n",
                            to_string(tid),
                            to_string(frame),
                            otidVector[tid][1],
                            otidVector[tid][2],
                            otidVector[frame][2],
                            otidVector[frame][1],
                            padStringNumber(format("{:8x}", matchResults.keyXorData0).substr(4, 4)),
                            padStringNumber(format("0x{:8x}", matchResults.keyXorData0).substr(0, 6)),
                            padStringNumber(format("0x{:8x}", matchResults.keyXorData3).substr(6)),
                            padStringNumber(format("0x{:8x}", matchResults.keyXorData3).substr(0, 6)),
                            padStringNumber(format("0x{:8x}", matchResults.keyXorData4).substr(6)),
                            padStringNumber(format("0x{:8x}", matchResults.keyXorData4).substr(0, 6)),
                            to_string(pokeballIndex),
                            padStringNumber(format("{:34b}", matchResults.keyXorData10).substr(3, 1)),
                            enemyMon);
                        matchFile << matchOut;

                        if (matchResults.ace) {
                            string aceOut = format("{},{},{} {},{} {},0x{},{},0x{} {} 0x{} {},{},{},{}\n",
                                to_string(tid),
                                to_string(frame),
                                otidVector[tid][1],
                                otidVector[tid][2],
                                otidVector[frame][2],
                                otidVector[frame][1],
                                padStringNumber(format("{:8x}", matchResults.keyXorData0).substr(4, 4)),
                                padStringNumber(format("0x{:8x}", matchResults.keyXorData0).substr(0, 6)),
                                padStringNumber(format("0x{:8x}", matchResults.keyXorData3).substr(6)),
                                padStringNumber(format("0x{:8x}", matchResults.keyXorData3).substr(0, 6)),
                                padStringNumber(format("0x{:8x}", matchResults.keyXorData4).substr(6)),
                                padStringNumber(format("0x{:8x}", matchResults.keyXorData4).substr(0, 6)),
                                to_string(pokeballIndex),
                                padStringNumber(format("{:34b}", matchResults.keyXorData10).substr(3, 1)),
                                enemyMon);
                            aceFile << aceOut;
                        }

                        break;
                    }
                }
            }
        }
    }

    matchFile.close();
    aceFile.close();
}

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

string padStringNumber(string number) {
    string outNumber = number;
    for (int charIndex = 0; charIndex < number.length(); charIndex++) {
        if (number[charIndex] == ' ') {
            outNumber[charIndex] = '0';
        }
    }
    return outNumber;
}