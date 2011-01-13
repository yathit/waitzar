/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#pragma once

//Our FILETIME structure
#include <windows.h>
#undef min
#undef max

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <map>


//Some simple static functions for managing a series of log files
// We assume decent inlining removes these when the respective const flags are false.
class Logger {
private:
	//Generic constants
	const static unsigned int logTabDepth = 2;

	//Main log file: constants
	const static bool WZ_LOG_MAIN = false;
	const static char waitzarLogchar = 'L';
	const static std::string mainLogFileName;

	//Keymagic log file: constants
	const static bool WZ_LOG_KEYMAGIC = false;
	const static char keymagicLogchar = 'K';
	const static std::string keymagicLogFileName;

	//Uni2Zawgyi log file: constants
	const static bool WZ_LOG_UNI2ZAWGYI = false;
	const static char uni2ZawgyiLogchar = 'Z';
	const static std::string uni2ZawgyiLogFileName;

	//"Typing" log file: constants
	const static bool WZ_LOG_TYPING = false;
	const static char typingLogchar = 'T';
	const static std::string typingLogFileName;

private:
	//Variables
	static std::map< char, std::vector<FILETIME> > filetimeStacks;
	static std::map< char, std::string > filePaths;

private:
	//Helper function
	static unsigned long getTimeDifferenceMS(const FILETIME &st, const FILETIME &end);

public:
	//Exposed functionality
	static bool isLogging(char logLetter);
	static void resetLogFile(char logLetter);
	static void writeLogLine(char logLetter);
	static void writeLogLine(char logLetter, const std::wstring& logLine);
	static void startLogTimer(char logLetter);
	static void startLogTimer(char logLetter, const std::wstring& logLine);
	static void endLogTimer(char logLetter);
	static void endLogTimer(char logLetter, const std::wstring& logLine);
	static void markLogTime(char logLetter, const std::wstring& logLine);
};




/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
