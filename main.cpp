#include "TelemetryBlackBox.h"

int main() {
    TelemetryBlackBox box;
    
    // Przykładowe użycie:
    box.onRocketStateChange(ARM);
    box.onBaroReading(false, 1500);
    box.recordCurrentState();

    return 0;
}