
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
using namespace std;

// -------------------- Solar Panel --------------------
class SolarPanel {
private:
    double voltage;
    double current;
public:
    SolarPanel(double v=18.0, double i=2.0) : voltage(v), current(i) {}
    double getVoltage() { return voltage; }
    double getCurrent() { return current; }
    void update(double sunlightFactor) {
        voltage *= sunlightFactor;
        current *= sunlightFactor;
    }
};

// -------------------- Battery --------------------
class Battery {
private:
    double soc; // state of charge %
public:
    Battery(double initialSOC=50.0) : soc(initialSOC) {}
    void charge(double power) {
        soc += power * 0.01;
        if(soc > 100) soc = 100;
    }
    void discharge(double load) {
        soc -= load * 0.01;
        if(soc < 0) soc = 0;
    }
    double getSOC() { return soc; }
};

// -------------------- Base Controller --------------------
class ChargeController {
public:
    virtual void regulate(SolarPanel& sp, Battery& b) = 0; // pure virtual
    virtual ~ChargeController() {}
};

// -------------------- PWM Controller --------------------
class PWMController : public ChargeController {
public:
    void regulate(SolarPanel& sp, Battery& b) override {
        double power = sp.getVoltage() * sp.getCurrent();
        b.charge(power * 0.8); // less efficient
        cout << "[PWM] Power delivered: " << power*0.8 << " W\n";
    }
};

// -------------------- MPPT Controller --------------------
class MPPTController : public ChargeController {
public:
    void regulate(SolarPanel& sp, Battery& b) override {
        double voltage = sp.getVoltage();
        double current = sp.getCurrent();
        double power = voltage * current;

        // Simple Perturb & Observe logic
        static double prevPower = 0;
        static double step = 0.1;
        if(power > prevPower) voltage += step;
        else voltage -= step;
        prevPower = power;

        b.charge(power); // more efficient
        cout << "[MPPT] Power delivered: " << power << " W\n";
    }
};

// -------------------- Data Logger --------------------
class DataLogger {
private:
    ofstream logFile;
public:
    DataLogger(string filename="results.csv") {
        logFile << "Voltage,Current,Power,SOC\n";
    }
    void record(double v, double i, double p, double soc) {
        logFile << v << "," << i << "," << p << "," << soc << "\n";
    }
    ~DataLogger() {
        logFile.close();
    }
};


int main() {
    try {
        SolarPanel sp(18.0, 2.0);
        Battery b(50.0);
        MPPTController mppt;
        PWMController pwm;
        DataLogger logger;

        // Run simulation for 5 cycles
        for(int t=0; t<5; t++) {
            cout << "\nCycle " << t+1 << ":\n";
            sp.update(1.0 + 0.1*t); // simulate sunlight change

            double v = sp.getVoltage();
            double i = sp.getCurrent();
            double p = v * i;

            // Use MPPT controller
            mppt.regulate(sp, b);

            // Log data
            logger.record(v, i, p, b.getSOC());
        }
    } catch(exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}

