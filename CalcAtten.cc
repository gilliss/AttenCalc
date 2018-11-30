/*******
* CalcAtten.cc
*   A simple gamma-ray attenuation calculator.
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

// include the useful functions defined in the header file
#include "CalcAtten.hh"

// main body of program
int main(int argc, char* argv[])
{
    // read command line arguments
    if (argc < 2) {cout << "Usage: ./CalcAtten <macro>" << endl; exit(EXIT_FAILURE);}
    char* macroFileName = argv[1];

    // prep vars
    double I_init = 1.0;
    double I = I_init;
    double E = 0.0;

    // create ifstream for macro file and open the file
    ifstream ifs;
    ifs.open(macroFileName, ifstream::in);

    // read in the macro file
    if (ifs.is_open())
    {
      // prep vars for holding macro lines, and positions and substrings of macro lines
      string line, cmdType, cmdArg, cmdArg0, cmdArg1;
      string::size_type n = string::npos;

      // get line from macro
      while (getline(ifs, line))
      {
        // parse line in macro
        n = line.find(" ");
        if (n == string::npos) {cout << "Error: Unexpected macro format" << endl; exit(EXIT_FAILURE);}
        cmdType = line.substr(0, n); // substr returns [pos, pos+count)
        cmdArg = line.substr(n+1, string::npos);

        // parse Gamma(keV): command
        if (cmdType == "Gamma(keV):")
        {
          cout << "Setting gamma-ray energy to " << stof(cmdArg) << " keV" << endl;
          E = stof(cmdArg);
        }

        // parse Shield(type,cm): command
        if (cmdType == "Shield(type,cm):")
        {
          // further parse the arguments of the command
          n = cmdArg.find(",");
          cmdArg0 = cmdArg.substr(0, n); // returns [pos, pos+count)
          cmdArg1 = cmdArg.substr(n+1, string::npos);

          // calculate transmittance and remaining intensity
          cout << "Calculating intensity following " << cmdArg1 << " cm of " << cmdArg0 << endl;
          double T = Transmit(cmdArg0, cmdArg1, E);
          I = I * T;
          cout << "  Transmit frac, this layer: " << T << endl;
          cout << "  Remaining I = " << I << ", I_init = " << I_init << endl;
        }
      } // end while getline() loop
      // close macro file
      ifs.close();
    } // end file is_open() loop

    // exit program
    return 0;
}
