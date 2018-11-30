/*******
* CalcAtten.hh
*   Header file containing functions used in CalcAtten.cc, a simple gamma-ray attenuation calculator.
*
* Dependencies:
*   *Data.txt: files for the densities and mass attenuation coefficients (and mass energy-absorption coefficients) of various radiation absorbers
*   macro.txt: a macro file  specifying the radiation type and energy, and the layers of shielding
*
* Usage:
*   compile: g++ -g -Wall -oCalcAtten CalcAtten.cc
*   execute: ./CalcAtten macro.txt
*
* Ref:
*   https://physics.nist.gov/PhysRefData/XrayMassCoef/chap2.html
*   https://physics.nist.gov/PhysRefData/XrayMassCoef/tab1.html
*   https://physics.nist.gov/PhysRefData/XrayMassCoef/tab2.html
*   https://physics.nist.gov/PhysRefData/XrayMassCoef/tab3.html
*   https://physics.nist.gov/PhysRefData/XrayMassCoef/tab4.html
*
* Author:
*   Tom Gilliss (UNC, ENAP) 2018-07-09 for NCSSM project
*******/

#include <fstream> // input/output stream class to operate on files
#include <iostream> // standard input/output stream class
#include <string> // strings and string manipulations like find() and substr()
#include <vector> // storing arrays of values, with the array sizes changing on the fly
#include <cmath> // exp() and fabs() functions
#include <algorithm> // lower_bound() and upper_bound() algorithms
using namespace std; // implied namespace for std library objects

int Closest(vector<double>& vec, double val)
{
  /*******
  * Find entry closest to val in vec
  * Return the index to that entry
  * lower_bound returns iterator to first element in the range [first,last) which does not compare less than val
  * upper_bound returns iterator to first element in the range [first,last) which compares greater than val
  *******/

  vector<double>::iterator lb = lower_bound(vec.begin(), vec.end(), val) - 1; // subtracted off 1 index; see note above
  vector<double>::iterator ub = upper_bound(vec.begin(), vec.end(), val);
  cout << "  Closest energies in data for " << val << ": " << *lb << " " << *ub << endl;
  return (fabs(*ub - val) > fabs(*lb - val)) ? lb - vec.begin() : ub - vec.begin();
}

string DataFilePath(string absorber)
{
  return "Data/" + absorber + "Data.txt";
}

double Density(string absorber)
{
  /*******
  * Return the density of the given absorber
  *******/

  // create ifstream for data file and open file
  ifstream dataFile;
  dataFile.open(DataFilePath(absorber), ifstream::in);

  // open data file
  if (dataFile.is_open())
  {
    // prep vars for holding data lines, and positions and substrings of those lines
    string line, lineType, lineArg, lineArg0, lineArg1, lineArg2;
    string::size_type n = string::npos;

    // get line from data
    while (getline(dataFile, line))
    {
      // parse line in data file
      n = line.find(" ");
      if (n == string::npos) {cout << "Error: Unexpected data file format" << endl; exit(EXIT_FAILURE);}
      lineType = line.substr(0, n); // substr returns [pos, pos+count)
      lineArg = line.substr(n+1, string::npos);

      // parse Density(g/cm^3):
      if (lineType == "Density(g/cm^3):")
      {
        // if density found, close the file, and return the density, thereby exiting the function
        dataFile.close();
        return stof(lineArg);
      }
    }

    // if here, no density was found
    dataFile.close();
    cout << "Error: No density found in data file" << endl; exit(EXIT_FAILURE);
  }
  else {cout << "Error: File not open" << endl; exit(EXIT_FAILURE);}
}

double MassAttenCoeff(string absorber, double E)
{
  /*******
  * Return the mass attenuation coefficient of the given absorber, for a given radiation energy
  *******/

  // create ifstream for data file
  ifstream dataFile;
  dataFile.open(DataFilePath(absorber), ifstream::in);

  // create some vectors to store and search data
  vector<double> Es, MACs;

  // open data file
  if (dataFile.is_open())
  {
    // prep vars for holding data lines, and positions and substrings of those lines
    string line, lineType, lineArg, lineArg0, lineArg1, lineArg2;
    string::size_type n;

    // get line from data
    while (getline(dataFile, line))
    {
      // parse line in data file
      n = line.find(" ");
      if (n == string::npos) {cout << "Error: Unexpected data file format" << endl; exit(EXIT_FAILURE);}
      lineType = line.substr(0, n); // substr returns [pos, pos+count)
      lineArg = line.substr(n+1, string::npos);

      // parse MAC(MeV,cm^2/g,cm^2/g):
      if (lineType == "MAC(MeV,cm^2/g,cm^2/g):")
      {
        // further parse the line of data
        n = lineArg.find(" ");
        lineArg0 = lineArg.substr(0, n); // energy
        lineArg1 = lineArg.substr(n+1, lineArg.find(" ", n+1)); // mass attenuation coefficient
        lineArg2 = lineArg.substr(lineArg.find(" ", n+1)+1, lineArg.find(" ", lineArg.find(" ", n+1)+1)); // mass energy-absorption coefficient

        // fill corresponding vectors of energies and mass attenuation coefficients
        Es.push_back(stof(lineArg0)); // these energies from the data file are in MeV
        MACs.push_back(stof(lineArg1));
      }
    } // end while getline() loop
    dataFile.close();
  } // end file is_open() loop
  else {cout << "Error: File not open" << endl; exit(EXIT_FAILURE);}

  // find and return the closest available MAC
  int i = Closest(Es, E/1000.); // E/1000. serves to convert from keV to MeV
  cout << "  Energy and MassAttenCoeff used for " << absorber << " " << E << ": " <<  Es[i] << " " << MACs[i] << endl;
  return MACs[i]; // convert E from keV to MeV for comparison with data file
}

double Transmit(string absorber, string thickness, double E)
{
  /*******
  * Return the fraction of beam transmitted
  * c = mass attenuation coeff (cm^2/g)
  * rho = density of absorber material (g/cm^3)
  * t = thickness of absorber material (cm)
  * E = radiation energy (keV)
  *******/

  double t = stof(thickness);
  double rho = Density(absorber);
  double c = MassAttenCoeff(absorber, E);
  return exp(-1 * c * rho * t);
}
