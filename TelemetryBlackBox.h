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
    // Definicja zmiennych
    static const uint8_t bufferSize = 16;

    RocketState state;
    uint8_t currentIndex;
    uint8_t framesCount;
    uint16_t altitude;
    uint32_t frames[bufferSize];

    bool parachute;
    bool baro_error;

public :
    // Definicja konstruktora
    TelemetryBlackBox();

    void onRocketStateChange ( RocketState state ) ;
    void onParachuteDeployed () ;
    /**
    * @param error - Informuje czy wyst ąpił błąd barometru
    * @param value - Odczyt wysoko ści w stopach
    */
    void onBaroReading ( bool error , uint16_t value ) ;
    /**
    * @brief Pobiera bieżący stan , składa go w 32 - bitow ą ramk ę z sumą  kontroln ą
    i zapisuje do bufora .
    */
    void recordCurrentState () ;
    /**
    * @brief Funkcja pobiera ostatni ą ramk ę z bufora kołowego , dekoduje  ją oraz
    loguje zdekodowane warto ści w czytelny spos ób na konsol ę.
    * @Return Ostatnia ramka zapisana do bufora , jeśli xor się zgadza .
    0 xFFFFFFFF jeśli suma kontrolna jest niepoprawna .
    */
    uint32_t getLastDataframe () ;
};