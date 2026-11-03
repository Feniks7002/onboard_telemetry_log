#pragma once
#include <cstdint>

enum RocketState : uint8_t {
    DISARM = 0 ,
    ARM = 1 ,
    FLIGHT = 2 ,
    PARACHUTE = 3 ,
    LANDED = 4
};

class TelemetryBlackBox {
private:
    static const uint8_t bufforSize = 16;

    RocketState state;
    uint8_t currentIndex;
    uint8_t framesCount;
    uint16_t altitude;
    uint32_t frames[bufforSize] = {};

    bool parachute;
    bool baro_error;
    
public :
    TelemetryBlackBox() {
        state = DISARM;
        currentIndex = 0;
        framesCount = 0;
        altitude = 0;
        parachute = false;
        baro_error = false;
    }

    void onRocketStateChange ( RocketState state ) {
        this->state = state;
    }

    void onParachuteDeployed () {
        parachute = true;
    }

    /**
    * @param error - Informuje czy wystąpił błąd barometru
    * @param value - Odczyt wysokości w stopach
    */
    void onBaroReading ( bool error , uint16_t value ) {
        baro_error = error;
        altitude = value;
    }

    /**
    * @brief Pobiera bieżący stan, składa go w 32 - bitową ramkę z sumą  kontrolną i zapisuje do bufora .
    */
    void recordCurrentState () {
        uint32_t frame = 0;
        uint16_t encodedAltitude = altitude;

        frame |= state;

        if (parachute) {
            frame |= (1u << 3);
        }

        if (baro_error) {
            frame |= (1u << 4);
            encodedAltitude = 0xFFFFFFFF;
        } else if (state == DISARM || state == LANDED) {
            encodedAltitude = 0;
        }

        frame |= ((uint32_t)encodedAltitude << 8);

        uint8_t bajt0 = frame & 0xFF;
        uint8_t bajt1 = (frame >> 8) & 0xFF;
        uint8_t bajt2 = (frame >> 16) & 0xFF;
        uint8_t checksum = bajt0 ^ bajt1 ^ bajt2;

        frame |= ((uint32_t)checksum << 24);
        
        frames[currentIndex] = frame;
        currentIndex = (currentIndex + 1) % 16;

        if (currentIndex < 16) {
            framesCount++;
        }
    }

    /**
    * @brief Funkcja pobiera ostatni ą ramk ę z bufora kołowego , dekoduje  ją oraz
    loguje zdekodowane warto ści w czytelny spos ób na konsol ę.
    * @return Ostatnia ramka zapisana do bufora , jeśli xor się zgadza .
    0 xFFFFFFFF jeśli suma kontrolna jest niepoprawna .
    */
    uint32_t getLastDataframe () {
        if (framesCount == 0) {
            return 0xFFFFFFFF;
        }

        uint8_t lastIndex = (currentIndex - 1 + 16) % 16;
        uint32_t lastFrame = frames[lastIndex];

        uint8_t bajt0 = lastFrame & 0xFF;
        uint8_t bajt1 = (lastFrame >> 8) & 0xFF;
        uint8_t bajt2 = (lastFrame >> 16) & 0xFF;
        
        uint8_t checksum = (lastFrame >> 24) % 0xFF;
        uint8_t checksum2 = bajt0 ^ bajt1 ^ bajt2;

        if (checksum == checksum2) {
            return lastFrame;
        } else {
            return 0xFFFFFFFF;
        }
    }
};
