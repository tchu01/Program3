#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "airportcpp.h"

using namespace std;

#define R 6371
#define TO_RAD (3.1415926536 / 180)

double haversine(double th1, double ph1, double th2, double ph2)
{
   double dx, dy, dz;
   ph1 -= ph2;
   ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

   dz = sin(th1) - sin(th2);
   dx = cos(ph1) * cos(th1) - cos(th2);
   dy = sin(ph1) * cos(th1);
   return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

void calculateDistances(double ** mat, int count, string *adj2Ap,
			map<string, Airport> alist) {

   for (int i = 0; i < count; i++) {
      for (int j = 0; j < count; j++) {
	 if (mat[i][j] == 1) {
	    mat[i][j] = haversine(alist[adj2Ap[i]].latitude,
				  alist[adj2Ap[i]].longitude,
				  alist[adj2Ap[j]].latitude,
				  alist[adj2Ap[j]].longitude);
	 }
      }
   }
}

/**
 * parseRoutes - parses the routes in the route.dat file and
 * calculates the distance between the ariports
 *
 * IN: mat - Adjacency matrix that contains the
 * distance. mat[src][dest] where src is the starting airport and dest
 * is the destination.
 * IN: fileName - name of the routes file
 * IN: alist - list of all the airports that is used to find the
 * coorindates of the airport in the routes file.
 */
int parseRoutes(double **mat, string fileName, string *adj2Ap,
		 map<string, Airport> alist) {
   char buf[256];
   char *token;
   string src, dest;
   int srcInx = 0, destInx = 0;
   int apSrc = 0, apDest = 0;
   map<string, int> lookup;
   int apCnt = 0, lkuCnt = 0;
   int totAp = 0;

   FILE *fp = fopen(fileName.c_str(), "r");

   if (fp == NULL) {
      printf("Error opening routes file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      token = strtok(buf, ",");
      strtok(NULL, ",");
      src = strtok(NULL, ",");
      strtok(NULL, ",");
      dest = strtok(NULL, ",");


      // determine if airport is on the adj matrix
      if (!lookup.count(src)) { // does not find src airport
	 lookup[src] = lkuCnt;
	 adj2Ap[apCnt++] = src;
	 apSrc = lkuCnt++;
	 totAp++;
      }
      else {
	 apSrc = lookup[src];
      }

      if (!lookup.count(dest)) { // does not find dest airport
	 lookup[dest] = lkuCnt;
	 adj2Ap[apCnt++] = dest;
	 apDest = lkuCnt++;
	 totAp++;
      }
      else {
	 apDest = lookup[dest];
      }

      mat[apSrc][apDest] = 1;

      // Haversine math here ~ real: 0.149 seconds
      //mat[src][dest] = haversine(alist[src].latitude, alist[src].longitude,
		//		 alist[dest].latitude, alist[dest].longitude);

      //if no mat use... ~ real: 0.044 seconds

      // 2/18 - Tim... just doing this, without calculation of haversine... ~ real: 0.120 seconds
   }
   fclose(fp);

   return totAp;
}

/**
 * parseAirport - parses the airport file
 *
 * IN: filename - name of the airport file to parse
 * IN: alist - hashmap of the airport and its airport code. Key is the
 * airport code and the airport struct is the airport data.
 *
 */
int parseAirports(string fileName, map<string, Airport>& alist) {
   FILE *fp = fopen(fileName.c_str(), "r");
   char buf[256];
   char *token;
   int id = 0;
   char DST;
   string name, city, country, airportCode, ICAO, tzDB;
   double latit = 0, longit = 0, altit = 0, timezone = 0;
   int cnt = 0;

   if(fp == NULL) {
      printf("Error opening airports file");
      exit(-1);
   }

   while(fgets(buf, sizeof(buf), fp)) {
      id = strtol(strtok(buf, ",\""), (char **)NULL, 10);
      name = strdup(strtok(NULL, ",\""));
      city = strdup(strtok(NULL, ",\""));
      country = strdup(strtok(NULL, ",\""));
      airportCode = strdup(strtok(NULL, ","));

      // Skip airports that do not have an airport code since there
      // will be no reference in the routes file
      if (!airportCode.compare("\"\"")) { // No airport code
	      continue;
      }
      else {
	      airportCode = airportCode.substr(1, airportCode.length()-2);
	      //cout << "new code = " + airportCode + "\n";
      }

      ICAO = strdup(strtok(NULL, ","));
      latit = strtod(strtok(NULL, ","),(char **)NULL); // lat
      longit = strtod(strtok(NULL, ","), (char **)NULL); // long
      altit = strtod(strtok(NULL, ","), (char **)NULL); // alt
      timezone = strtod(strtok(NULL, ","), (char **)NULL); // timezone
      DST = *(strtok(NULL, ","));
      tzDB = strdup(strtok(NULL, ","));

      Airport a = {id, name, city, country, airportCode, ICAO, latit, longit,
		   altit, timezone, DST, tzDB};

      alist[airportCode] = a;
      cnt++;
      //cout << alist[airportCode].name + "\n";
      //printf("id: %s\n", (*alist)[airportCode].name);
   }

   printf ("\n\n\nnumber of airports = %d\n",cnt);
   fclose(fp);
   return cnt;
}

void printMat (double **mat, int cnt) {

   for (int i = 0; i < cnt; i++) {
      cout <<i<<" : ";
      for (int j = 0; j < cnt; j++) {
	 cout<<mat[i][j]<< " ";
      }
      cout<<"\n\n";
   }

}

int main(int argc, char *argv[])
{
   std::map<string, Airport> airportList;

   int count = parseAirports("airports.dat", airportList);
   string adjMatToAp[count];

   // Construct 2d array for adjacency matrix
   double **adjMat = (double **)calloc(sizeof(double*), count);
   for (int i = 0; i < count; i++) {
      adjMat[i] = (double *)calloc(sizeof(double), count);
   }



   int totalApRoutes = parseRoutes(adjMat, "routes.dat", adjMatToAp, airportList);

   // for (int j = 0; j < 20; j++ ) {
   //    cout << "ariport " <<j <<" = " << adjMatToAp[j] << "\n";
   // }


   //printMat(adjMat, 20);

   //printf("lat = %s\n", airportList["GKA"].name);
   //cout << "dist = " << adjM["SFO"]["HKG"] << "\n";
   calculateDistances(adjMat, totalApRoutes, adjMatToAp, airportList);
   //printMat(adjMat, 20);
   return 0;
}
