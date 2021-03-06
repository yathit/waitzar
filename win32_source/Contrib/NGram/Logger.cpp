/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */

#include "Logger.h"




//Consts:
const std::string Logger::mainLogFileName =        "wz_log.txt";
const std::string Logger::keymagicLogFileName =    "wz_log_keymagic.txt";
const std::string Logger::uni2ZawgyiLogFileName =  "wz_log_uni2zawgyi.txt";
const std::string Logger::typingLogFileName =      "wz_log_typing.txt";
const std::string Logger::configLogFileName =      "wz_log_config.txt";

//For some reason, if we define these in the header file then our
// map initializer list can't find them.
const char Logger::waitzarLogchar =    'L';
const char Logger::keymagicLogchar =   'K';
const char Logger::uni2ZawgyiLogchar = 'Z';
const char Logger::typingLogchar =     'T';
const char Logger::configLogchar =     'C';

//Variables:
std::map< char, std::vector<FILETIME> > Logger::filetimeStacks;
std::map< char, std::string > Logger::filePaths = {
		{waitzarLogchar, mainLogFileName},
		{keymagicLogchar, keymagicLogFileName},
		{typingLogchar, typingLogFileName},
		{uni2ZawgyiLogchar, uni2ZawgyiLogFileName},
		{configLogchar, configLogFileName}
};

//Helper 1
bool Logger::isLogging(char logLetter)
{
	if (WZ_LOG_MAIN && (logLetter==waitzarLogchar))
		return true;
	if (WZ_LOG_KEYMAGIC && (logLetter==keymagicLogchar))
		return true;
	if (WZ_LOG_UNI2ZAWGYI && (logLetter==uni2ZawgyiLogchar))
		return true;
	if (WZ_LOG_TYPING && (logLetter==typingLogchar))
		return true;
	if (WZ_LOG_CONFIG && (logLetter==configLogchar))
		return true;
	return false;
}

//Helper 2 (to-do: fix the timer if the high words differ)
unsigned long Logger::getTimeDifferenceMS(const FILETIME &st, const FILETIME &end)
{
	if (st.dwHighDateTime != end.dwHighDateTime)
		return 12345; //TODO: Fix this
	return (end.dwLowDateTime - st.dwLowDateTime)/10000L;
}



//Reset
void Logger::resetLogFile(char logLetter) 
{
	if (isLogging(logLetter)) {
		//Reset the saved filetimes
		filetimeStacks[logLetter].clear();

		//Ensure the file path has been added.
		//filePaths[logLetter] = (logLetter==waitzarLogchar) ? mainLogFileName : (logLetter==keymagicLogchar) ? keymagicLogFileName : (logLetter==typingLogchar) ? typingLogFileName : uni2ZawgyiLogFileName;

		//Reset log file contents
		std::ofstream log(filePaths[logLetter].c_str(), std::ios::out);
		log.close();
	}
}

//Write a single line (indent based on number of running timers; just ignore tabs for no timings)
//Escapes unicode
void Logger::writeLogLine(char logLetter, const std::wstring& logLine)
{
	if (isLogging(logLetter)) {
		//Open file
		std::ofstream log(filePaths[logLetter].c_str(), std::ios::app);

		//Append main line if non-empty.
		if (!logLine.empty()) {
			//Indent
			std::wstring newLine = std::wstring(filetimeStacks[logLetter].size()*logTabDepth, L' ') + logLine;

			//Generate line
			std::stringstream msg;
			for (size_t i=0; i<newLine.length(); i++) {
				if (newLine[i]<=0xFF && newLine[i]!=0x00)
					msg <<(char)newLine[i];
				else
					msg <<"\\u" <<std::hex <<std::uppercase <<newLine[i] <<std::dec <<std::nouppercase;
			}

			//Write line
			log <<msg.str();
		}

		//Write a trailing newline anyway
		log <<std::endl;
		log.close();
	}
}
void Logger::writeLogLine(char logLetter)
{
	Logger::writeLogLine(logLetter, L"");
}


//Start the timer with an optional line in the log file
void Logger::startLogTimer(char logLetter, const std::wstring& logLine)
{
	//Write line
	if (!logLine.empty())
		writeLogLine(logLetter, logLine);

	//Start timer
	FILETIME time;
	GetSystemTimeAsFileTime(&time);
	filetimeStacks[logLetter].push_back(time);
}
void Logger::startLogTimer(char logLetter)
{
	Logger::startLogTimer(logLetter, L"");
}


//Stop the timer with an optional line in the log file
void Logger::endLogTimer(char logLetter, const std::wstring& logLine)
{
	//Stop timer
	filetimeStacks[logLetter].pop_back();

	//Write line
	if (!logLine.empty())
		writeLogLine(logLetter, logLine);
}
void Logger::endLogTimer(char logLetter)
{
	Logger::endLogTimer(logLetter, L"");
}



//Mark a given time with a log line
void Logger::markLogTime(char logLetter, const std::wstring& logLine)
{
	//Generate a timed line
	std::wstringstream linePrefix;
	if (filetimeStacks[logLetter].empty())
		linePrefix <<L"<NULL> ms - ";
	else {
		//Time the event
		FILETIME endTime;
		GetSystemTimeAsFileTime(&endTime);
		FILETIME startTime = filetimeStacks[logLetter][filetimeStacks[logLetter].size()-1];
		DWORD timeMS = getTimeDifferenceMS(startTime, endTime);
		linePrefix <<timeMS <<L" ms - ";
		filetimeStacks[logLetter][filetimeStacks[logLetter].size()-1] = endTime;
	}

	//Write line
	writeLogLine(logLetter, linePrefix.str() + logLine);
}




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
