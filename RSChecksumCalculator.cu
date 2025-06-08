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
#include "RSChecksumCalculator.h"

using namespace std;
using namespace chrono;

const long long PID = 1321080;
const int DATA_ORDER_G = 1;
const int DATA_ORDER_A = 4;
const int DATA_ORDER_E = 7;
const int DATA_ORDER_M = 10;
const string CSV_HEADER = "Player frame,Enemy Frame,Player TID/SID,Enemy TID/SID,Species,Held Item,Moves,Pokeball,Egg,Enemy Mon";
const string MATCH_FILE = "gpuMatches.csv";
const string ACE_FILE = "gpuAces.csv";
const int DATA_SIZE = 18;
const int POKEBALL_COUNT = 12;
const int OUTPUT_SIZE = 10;
//entry count 12144 data count 218592 output count 121440
//enemyListSize = 506

struct ChecksumMatchResults {
    bool match;
    bool ace;
    long long keyXorData0;
    long long keyXorData3;
    long long keyXorData4;
    long long keyXorData10;
};

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
__global__
void calculateMatchCuda(long long entries, long long *dataTotal, long long *outputTotal) {
    for (long long entry = 0; entry < entries; entry++) {
        long long originalChecksum = ((dataTotal[(entry * DATA_SIZE) + 0] % 65536) + (dataTotal[(entry * DATA_SIZE) + 0] / 65536) + (dataTotal[(entry * DATA_SIZE) + 1] % 65536) + (dataTotal[(entry * DATA_SIZE) + 1] / 65536) + (dataTotal[(entry * DATA_SIZE) + 2] % 65536)
            + (dataTotal[(entry * DATA_SIZE) + 2] / 65536) + (dataTotal[(entry * DATA_SIZE) + 3] % 65536) + (dataTotal[(entry * DATA_SIZE) + 3] / 65536) + (dataTotal[(entry * DATA_SIZE) + 4] % 65536) + (dataTotal[(entry * DATA_SIZE) + 4] / 65536)
            + (dataTotal[(entry * DATA_SIZE) + 5] % 65536) + (dataTotal[(entry * DATA_SIZE) + 5] / 65536) + (dataTotal[(entry * DATA_SIZE) + 6] % 65536) + (dataTotal[(entry * DATA_SIZE) + 6] / 65536) + (dataTotal[(entry * DATA_SIZE) + 7] % 65536)
            + (dataTotal[(entry * DATA_SIZE) + 7] / 65536) + (dataTotal[(entry * DATA_SIZE) + 8] % 65536) + (dataTotal[(entry * DATA_SIZE) + 8] / 65536) + (dataTotal[(entry * DATA_SIZE) + 9] % 65536) + (dataTotal[(entry * DATA_SIZE) + 9] / 65536)
            + (dataTotal[(entry * DATA_SIZE) + 10] % 65536) + (dataTotal[(entry * DATA_SIZE) + 10] / 65536) + (dataTotal[(entry * DATA_SIZE) + 11] % 65536) + (dataTotal[(entry * DATA_SIZE) + 11] / 65536)) % 65536;

        long long keysXored = dataTotal[(entry * DATA_SIZE) + 12] ^ dataTotal[(entry * DATA_SIZE) + 13];

        long long newChecksum = (((keysXored ^ dataTotal[(entry * DATA_SIZE) + 0]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 0]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 1]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 1]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 2]) % 65536)
            + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 2]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 3]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 3]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 4]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 4]) / 65536)
            + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 5]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 5]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 6]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 6]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 7]) % 65536)
            + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 7]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 8]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 8]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 9]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 9]) / 65536)
            + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 10]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 10]) / 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 11]) % 65536) + ((keysXored ^ dataTotal[(entry * DATA_SIZE) + 11]) / 65536)) % 65536;

        if (originalChecksum == newChecksum) {
            bool ace = (((keysXored ^ dataTotal[(entry * DATA_SIZE) + 0]) % 65536) == 39710);
            outputTotal[(entry * OUTPUT_SIZE) + 0] = 1;                                               // long long match;
            outputTotal[(entry * OUTPUT_SIZE) + 1] = ace;                                             // long long ace;
            outputTotal[(entry * OUTPUT_SIZE) + 2] = keysXored ^ dataTotal[(entry * DATA_SIZE) + 0];  // long long keyXorData0;
            outputTotal[(entry * OUTPUT_SIZE) + 3] = keysXored ^ dataTotal[(entry * DATA_SIZE) + 3];  // long long keyXorData3;
            outputTotal[(entry * OUTPUT_SIZE) + 4] = keysXored ^ dataTotal[(entry * DATA_SIZE) + 4];  // long long keyXorData4;
            outputTotal[(entry * OUTPUT_SIZE) + 5] = keysXored ^ dataTotal[(entry * DATA_SIZE) + 10]; // long long keyXorData10;
            outputTotal[(entry * OUTPUT_SIZE) + 6] = dataTotal[(entry * DATA_SIZE) + 14];             // long long tid;
            outputTotal[(entry * OUTPUT_SIZE) + 7] = dataTotal[(entry * DATA_SIZE) + 15];             // long long frame;
            outputTotal[(entry * OUTPUT_SIZE) + 8] = dataTotal[(entry * DATA_SIZE) + 16];             // long long pokeballIndex;
            outputTotal[(entry * OUTPUT_SIZE) + 9] = dataTotal[(entry * DATA_SIZE) + 17];             // long long enemyListIndex;

        } else {
            outputTotal[(entry * OUTPUT_SIZE) + 0] = 0;  // long long match;
            outputTotal[(entry * OUTPUT_SIZE) + 1] = 0;  // long long ace;
            outputTotal[(entry * OUTPUT_SIZE) + 2] = 2;  // long long keyXorData0;
            outputTotal[(entry * OUTPUT_SIZE) + 3] = 3;  // long long keyXorData3;
            outputTotal[(entry * OUTPUT_SIZE) + 4] = 4;  // long long keyXorData4;
            outputTotal[(entry * OUTPUT_SIZE) + 5] = 5;  // long long keyXorData10;
            outputTotal[(entry * OUTPUT_SIZE) + 6] = 6;  // long long tid;
            outputTotal[(entry * OUTPUT_SIZE) + 7] = 7;  // long long frame;
            outputTotal[(entry * OUTPUT_SIZE) + 8] = 8;  // long long pokeballIndex;
            outputTotal[(entry * OUTPUT_SIZE) + 9] = 9;  // long long enemyListIndex;
        }
    }
}

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

    // Calculate Checksums
    cout << "Executing with TIDs " << arguments[0] << " to " << arguments[1] << " (inclusive) and the first " << arguments[2] << " frames." << endl;
    cout << "USING CUDA" << endl;
    calculateChecksumMatches(arguments[0], arguments[1], arguments[2], dataOrder, enemyList, enemyDict, otidVector);

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
        arguments.push_back(3575);
    }
    if (argc >= 3) {
        string arg = argv[2];
        int endingTid = stoi(arg);
        arguments.push_back(endingTid);
    } else {
        arguments.push_back(3575);
    }
    if (argc >= 4) {
        string arg = argv[3];
        int frameCount = stoi(arg);
        arguments.push_back(frameCount);
    } else {
        arguments.push_back(4000);
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
    if (args[0] > 100000) {
        cout << "TID upper bound exceeded, set to 100000." << endl;
        args[0] = 100000;
    }
    if (args[1] < args[0]) {
        cout << "TID range error, upper bound set to lower bound." << endl;
        args[1] = args[0];
    }
    if (args[1] > 100000) {
        cout << "TID upper bound exceeded, set to 100000." << endl;
        args[1] = 100000;
    }
    if (args[2] < 1) {
        cout << "Frame lower bound exceeded, set to 1." << endl;
        args[2] = 1;
    }
    if (args[2] > 100000) {
        cout << "Frame upper bound exceeded, set to 100000." << endl;
        args[2] = 100000;
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
 * Purpose: Loops through TIDs and frames and calcs
 *   checksums for each combination.
 * ******************************************************
 * Parameters:
 *   trainerIdStart: first TID to calc
 *   trainerIdEnd: last TID to calc
 *   frames: num frames to calc
 *   dataOrder: idk what this is
 *   enemyList: vector of enemy mons
 *   enemyDict: map of enemy mon to enemy data
 *   otidVector: vector of otid data
 * ******************************************************
*/
void calculateChecksumMatches(int trainerIdStart, int trainerIdEnd, int frames, string dataOrder[], vector<string> &enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector) {

    // Delete output files if they exist and create a new one.
    try {
        filesystem::remove(MATCH_FILE);
        filesystem::remove(ACE_FILE);   
    } catch (int errorCode) { }
    ofstream matchFile(MATCH_FILE);
    ofstream aceFile(ACE_FILE);
    matchFile << CSV_HEADER << endl;
    aceFile << CSV_HEADER << endl;

    // Trainer ID is inclusive. We don't do subtraction in TID like in python bc we don't need to account for header row.
    for (int tid = trainerIdStart; tid <= trainerIdEnd; tid++) {
        cout << "Checking tid " << tid << endl;

        string playerHex = intToHex(otidVector[tid][2], 4) + intToHex(otidVector[tid][1], 4).substr(2);
        long long playerLongLong = stoll(playerHex, 0, 16);
        long long playerKey = PID ^ playerLongLong;
        long long enemyListSize = enemyList.size();
        long long entryCount = frames * enemyListSize * POKEBALL_COUNT;
        long long dataCount = entryCount * DATA_SIZE;
        long long outputCount = entryCount * OUTPUT_SIZE;
        //long long *dataTotal = new long long[dataCount];
        //long long *outputTotal = new long long[outputCount];
        long long *dataTotal, *outputTotal;
        cudaMallocManaged(&dataTotal, dataCount * sizeof(long long));
        cudaMallocManaged(&outputTotal, outputCount * sizeof(long long));
		cout << "entry count " << entryCount << " data count " << dataCount << " output count " << outputCount << endl;
        
        long long *data = new long long[DATA_SIZE];

        // Start at frame 0. Python version starts at 1 bc of header column
        for (int frame = 0; frame < frames; frame++) {
            if (frame % 1 == 0) {
                cout << "Checking frame " << frame << endl;
            }

            string enemyHex = intToHex(otidVector[frame][1], 4) + intToHex(otidVector[frame][2], 4).substr(2);
            long long enemyLongLong = stoll(enemyHex, 0, 16);
            long long enemyKey = PID ^ enemyLongLong;

            data[12] = playerKey;
            data[13] = enemyKey;
            data[14] = tid;
            data[15] = frame;

            // Loop through all mons
            for (int enemyListIndex = 0; enemyListIndex < enemyListSize; enemyListIndex++) {
                string enemyMon = enemyList[enemyListIndex];
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
                
                data[17] = enemyListIndex;

                // Loop through pokeballs. We quit as soon as we find a match, even though there are likely more of the same pokeball.
                for (int pokeballIndex = 1; pokeballIndex < 13; pokeballIndex++) {
                    data[9] = stoll(llToBin(data[9], 32).substr(2, 1) + llToBin(pokeballIndex, 4).substr(2) + llToBin(data[9], 32).substr(7), 0, 2);
                    data[16] = pokeballIndex;

                    for (int dataIndex = 0; dataIndex < DATA_SIZE; dataIndex++) {
                        //cout << "Adding to " << (frame * enemyListSize * POKEBALL_COUNT * DATA_SIZE) + (enemyListIndex * POKEBALL_COUNT * DATA_SIZE) + ((pokeballIndex - 1) * DATA_SIZE) + dataIndex << endl;
                        dataTotal[(frame * enemyListSize * POKEBALL_COUNT * DATA_SIZE) + (enemyListIndex * POKEBALL_COUNT * DATA_SIZE) + ((pokeballIndex - 1) * DATA_SIZE) + dataIndex] = data[dataIndex];
                    }

                    /*
                    // [data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], playerKey, enemyKey, tid, frame, pokeballIndex, enemyListIndex]
                    for (int i = 0; i < DATA_SIZE; i++) {
                        cout << data[i] << " ";
                    }
                    cout << endl;*/
                }
            }
        }
        /*
        for (long long entry = 0; entry < entryCount; entry++) {
            cout << "data entry: " << entry << " ";
            //const string CSV_HEADER = "Player frame,Enemy Frame,Player TID/SID,Enemy TID/SID,Species,Held Item,Moves,Pokeball,Egg,Enemy Mon";
            cout << dataTotal[(entry * DATA_SIZE) + 0] << " " << dataTotal[(entry * DATA_SIZE) + 1] << " "
                << dataTotal[(entry * DATA_SIZE) + 2] << " " << dataTotal[(entry * DATA_SIZE) + 3] << " "
                << dataTotal[(entry * DATA_SIZE) + 4] << " " << dataTotal[(entry * DATA_SIZE) + 5] << " "
                << dataTotal[(entry * DATA_SIZE) + 6] << " " << dataTotal[(entry * DATA_SIZE) + 7] << " "
                << dataTotal[(entry * DATA_SIZE) + 8] << " " << dataTotal[(entry * DATA_SIZE) + 9] << " "
                << dataTotal[(entry * DATA_SIZE) + 10] << " " << dataTotal[(entry * DATA_SIZE) + 11] << " "
                << dataTotal[(entry * DATA_SIZE) + 12] << " " << dataTotal[(entry * DATA_SIZE) + 13] << " "
                << dataTotal[(entry * DATA_SIZE) + 14] << " " << dataTotal[(entry * DATA_SIZE) + 15] << " "
                << dataTotal[(entry * DATA_SIZE) + 16] << " " << dataTotal[(entry * DATA_SIZE) + 17] << " " << endl;
        }*/

        calculateMatchCuda<<<1, 1>>>(entryCount, dataTotal, outputTotal);
        //calculateMatchCuda(entryCount, dataTotal, outputTotal);

        cudaDeviceSynchronize();
        cout << "finished calcing checksums" << endl;
		cout << "entryCount: " << entryCount << endl;
        for (long long entry = 0; entry < entryCount; entry++) {
            /*cout << "entry: " << entry;
            //const string CSV_HEADER = "Player frame,Enemy Frame,Player TID/SID,Enemy TID/SID,Species,Held Item,Moves,Pokeball,Egg,Enemy Mon";
			cout << outputTotal[(entry * OUTPUT_SIZE) + 0] << " " << outputTotal[(entry * OUTPUT_SIZE) + 1] << " "
				<< outputTotal[(entry * OUTPUT_SIZE) + 2] << " " << outputTotal[(entry * OUTPUT_SIZE) + 3] << " "
				<< outputTotal[(entry * OUTPUT_SIZE) + 4] << " " << outputTotal[(entry * OUTPUT_SIZE) + 5] << " "
				<< outputTotal[(entry * OUTPUT_SIZE) + 6] << " " << outputTotal[(entry * OUTPUT_SIZE) + 7] << " "
				<< outputTotal[(entry * OUTPUT_SIZE) + 8] << " " << outputTotal[(entry * OUTPUT_SIZE) + 9] << endl;
            */
            
            if (outputTotal[(entry * OUTPUT_SIZE) + 0]) {
                string matchOut =
                    to_string(tid) + "," +
                    to_string(outputTotal[(entry * OUTPUT_SIZE) + 7]) + "," +
                    to_string(otidVector[tid][1]) + " " +
                    to_string(otidVector[tid][2]) + "," +
                    to_string(otidVector[outputTotal[(entry * OUTPUT_SIZE) + 7]][2]) + " " +
                    to_string(otidVector[outputTotal[(entry * OUTPUT_SIZE) + 7]][1]) + "," +
                    "0x" + intToHex(outputTotal[(entry * OUTPUT_SIZE) + 2], 8).substr(6) + "," +
                    intToHex(outputTotal[(entry * OUTPUT_SIZE) + 2], 8).substr(0, 6) + "," +
                    "0x" + intToHex(outputTotal[(entry * OUTPUT_SIZE) + 3], 8).substr(6) + " " +
                    intToHex(outputTotal[(entry * OUTPUT_SIZE) + 3], 8).substr(0, 6) + " " +
                    "0x" + intToHex(outputTotal[(entry * OUTPUT_SIZE) + 4], 8).substr(6) + " " +
                    intToHex(outputTotal[(entry * OUTPUT_SIZE) + 4], 8).substr(0, 6) + "," +
                    to_string(outputTotal[(entry * OUTPUT_SIZE) + 8]) + "," +
                    llToBin(outputTotal[(entry * OUTPUT_SIZE) + 5], 32).substr(3, 1) + "," +
                    enemyList[outputTotal[(entry * OUTPUT_SIZE) + 9]];
                matchFile << matchOut << endl;

                if (outputTotal[(entry * OUTPUT_SIZE) + 1]) {
                    aceFile << matchOut << endl;
                }
            }
                
        }

        cudaFree(dataTotal);
        cudaFree(outputTotal);
        delete [] data;
    }

    matchFile.close();
    aceFile.close();
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

template< typename T >
string intToHex(T i, int len)
{
    stringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(T) * 2)
        << std::hex << i;

    string hex = stream.str();
    return "0x" + hex.substr(hex.length() - len);
}

string llToBin(long long longlong, int len)
{
    return "0b" + bitset<32>(longlong).to_string().substr(32 - len);
}