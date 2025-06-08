#ifndef CHECKSUM
#define CHECKSUM
#include <string>
#include <vector>
#include <map>

using namespace std;

int main(int argc, char* argv[]);
vector<int> parseArguments(int argc, char* argv[]);
void handleArguments(vector<int> &args);
map<string, vector<long long>> dataFileToMap(string fileName, vector<string> &enemyList);
long long hexStringToIntLittleEndian(string hexString);
vector<vector<int>> otidFileToVector(string fileName);
void calculateChecksumMatches(int trainerIdStart, int trainerIdEnd, int frames, string dataOrder[], vector<string> &enemyList, map<string, vector<long long>> enemyDict, vector<vector<int>> otidVector);
struct ChecksumMatchResults;
__global__ 
void calculateMatchCuda(long long entries, long long *dataTotal, long long *outputTotal);
ChecksumMatchResults calculateMatch(long long data[], long long playerKey, long long enemyKey);
string padStringNumber(string number);
template< typename T >
string intToHex(T i, int len);
string llToBin(long long longlong, int len);

#endif