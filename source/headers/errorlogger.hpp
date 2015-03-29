
#ifndef ERRORLOGGER_HPP
#define ERRORLOGGER_HPP


/*Included headers*/
/*---------------------------------------------*/

/*---------------------------------------------*/

/*Included dependencies*/
/*---------------------------------------------*/
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
/*---------------------------------------------*/

/*Header content*/
/*=============================================*/

/*Used to log standard c++ erors, saves reports in errorlog.txt in the source folder*/
void errorlogger(const char* errormsg){
	time_t current = time(0);
	tm *date_raw = gmtime(&current);
	char *date = asctime(date_raw);

	std::ofstream errors; 
	errors.open("ErrorLog.txt", std::ofstream::app);
	if(errors.is_open()){
		errors << "Date: "<< date << "Error: " << errormsg << "\n" << std::endl;
		errors.close();
	}/*IF*/
	else{
		printf("Error opening external file Errorlog.txt\n");
		printf(" Error: %s\n Date: %s\n", errormsg, date );
	}/*ELSE*/
}/*errorLogger*/

/*=============================================*/

#endif