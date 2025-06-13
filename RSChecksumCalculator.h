#ifndef CHECKSUM
#define CHECKSUM
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
#include "RSChecksumCalculator.h"
#include "ThreadPool.h"

using namespace std;

int main(int argc, char* argv[]);
vector<int> parseArguments(int argc, char* argv[]);
void handleArguments(vector<int> &args);
map<string, vector<long long>> dataFileToMap(string fileName, vector<string> &enemyList);
map<string, vector<int>> dataOrderToMap(string fileName);
long long hexStringToIntLittleEndian(string hexString);
vector<vector<int>> otidFileToVector(string fileName);
void calculateChecksums(vector<int> arguments, const string dataOrder[], map<string, vector<int>> dataOrderOrder, vector<string> enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector);
void calculateChecksumMatchesThread(int tid, int frames, const string dataOrder[], map<string, vector<int>> dataOrderOrder, vector<string> &enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector);
struct ChecksumMatchResults;
ChecksumMatchResults calculateMatch(long long data[], long long playerKey, long long enemyKey);
void combineChecksumFiles();
string padStringNumber(string number);
template< typename T >
string intToHex(T i, int len);
string llToBin(long long longlong, int len);

#endif