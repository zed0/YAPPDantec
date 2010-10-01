#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Serial.h"
using namespace std;

bool doCommand(string command);
bool sendCommand(string command);
bool sendReady();
bool sendHome(bool x, bool y, bool z);
bool sendCalibrate(int cal_x, int cal_y, int cal_z);
bool sendMove(int newXPos, int newYPos, int newZPos);
template<class T> string toString(const T& t);
template<class T> T fromString(const string& s);

CSerial serialPort;
int port;
int baudRate;
string filename = "myFile.txt";

int positionX;
int positionY;
int positionZ;
int calibrationX = 10;
int calibrationY = 10;
int calibrationZ = 10;
int speed = 250;

int main(int argc, char *argv[])
{
	filename = "myFile.txt";
	port = 5;
	baudRate = 9600;
	if(argc == 1)
	{
		cout << "No parameters specified, using defaults." << endl;
		cout << "To specify use DantecTraverse.exe [port [filename [baudRate]]]" << endl;
	}
	else if(argc >= 2)
	{
		port = fromString<int>(argv[1]);
	}
	else if(argc >= 3)
	{
		filename = argv[2];
	}
	else if(argc >= 4)
	{
		baudRate = fromString<int>(argv[3]);
	}
	cout << "Parameters are: COM " << port << "; filename: \"" << filename << "\"; Baud Rate: " << baudRate << endl;

	if(!serialPort.Open(port, baudRate))
	{
		cout << "Cannot open serial port " << port << " with Baud Rate " << baudRate << endl;
		throw 1;
	}
	while(1)
	{
		//checks if the file ends with a full stop on a seperate line, this signifies that YAPP is finished writing to the file
		ifstream iFile(filename.c_str());
		string content((istreambuf_iterator<char>(iFile)), istreambuf_iterator<char>());
		if(content.rfind("\n.") != string::npos)
		{
			cout << content << endl;
			doCommand(content);
			ofstream oFile(filename.c_str(), fstream::trunc);
		}
		int waiting = serialPort.ReadDataWaiting();
		if(waiting)
		{
			char* buffer = new char[waiting];
			serialPort.ReadData(buffer, waiting);
			string incomming = buffer;
			cout << "Incoming: " << buffer << endl;
			delete buffer;
		}
	}
}

bool sendReady()
{
	string message = "@07";
	return sendCommand(message);
}

bool sendHome(bool x, bool y, bool z)
{
	int axes = 4*z + 2*y + x;
	string message = "@0R" + toString<int>(axes);
	return sendCommand(message);
}

bool sendGetPos()
{
	string message = "@0P";
	return sendCommand(message);
}

bool sendCalibrate(int newCalX, int newCalY, int newCalZ)
{
	string message = "@0d";
	calibrationX = newCalX;
	calibrationY = newCalY;
	calibrationZ = newCalZ;
	message.append(toString<int>(speed*newCalX) + ",");
	message.append(toString<int>(speed*newCalY) + ",");
	message.append(toString<int>(speed*newCalZ));
	return sendCommand(message);
}

bool sendMove(int newXPos, int newYPos, int newZPos)
{
	string message = "@0M";
	message.append(toString<int>(newXPos*calibrationX) + ",");
	message.append(toString<int>(speed*calibrationX) + ",");
	message.append(toString<int>(newYPos*calibrationY) + ",");
	message.append(toString<int>(speed*calibrationY) + ",");
	message.append(toString<int>(newZPos*calibrationZ) + ",");
	message.append(toString<int>(speed*calibrationZ) + ",");
	message += "0,";
	message += toString(speed*10);
	return sendCommand(message);
}

bool sendCommand(string command)
{
	cout << "Sending: " << command << endl;
	command.append("\r");
	if(serialPort.SendData(command.c_str(), command.size()) == command.size())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool doCommand(string fullCommand)
{
	stringstream stream;
	stream << fullCommand;
	string firstCommand;
	stream >> firstCommand;
	if(firstCommand == "READY")
	{
		sendReady();
	}
	else if(firstCommand == "HOME")
	{
		sendHome(true, true, true);
	}
	else if(firstCommand == "GETPOS")
	{
		sendGetPos();
	}
	else if(firstCommand == "CALIBRATE")
	{
		string newCalX;
		string newCalY;
		string newCalZ;
		getline(stream, newCalX, ',');
		getline(stream, newCalY, ',');
		getline(stream, newCalZ, ',');
		sendCalibrate(fromString<int>(newCalX), fromString<int>(newCalY), fromString<int>(newCalZ));
	}
	else if(firstCommand == "MOVE")
	{
		string newXPos;
		string newYPos;
		string newZPos;
		getline(stream, newXPos, ',');
		getline(stream, newYPos, ',');
		getline(stream, newZPos, ',');
		sendMove(fromString<int>(newXPos), fromString<int>(newYPos), fromString<int>(newZPos));
	}
}

template<class T> string toString(const T& t)
{
	ostringstream stream;
	stream << t;
	return stream.str();
}

template<class T> T fromString(const string& s)
{
	istringstream stream(s);
	T t;
	stream >> t;
	return t;
}
