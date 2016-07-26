
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>

#define PrescalerOverflowValue 4

#define NUM_LEDS 6
#define NUM_MODES 2

#define OFF 0
#define HOURS 1
#define MINS 2
#define BOTH 3

#define RED 0
#define GREEN 1
#define BLUE 2

#define TIME_HEADER "T"

unsigned char Prescaler = 0;

unsigned char CurrentLED = 1;
unsigned char LEDValues[ NUM_LEDS ][ 3 ];

int Colors[ NUM_MODES ][ 4 ][ 3 ] = {
    {
        {
            255, 255, 255 // OFF
        },
        {
            255, 0, 0 // HOURS
        },
        {
            0, 255, 0 // MINS
        },
        {
            0, 0, 255 // BOTH
        }
    },
    {
        {
            255, 255, 255 // OFF
        },
        {
            255, 255, 0 // HOURS
        },
        {
            0, 255, 255 // MINS
        },
        {
            255, 0, 255 // BOTH
        }
    }
};

int Mode = 0;
int LastHour = -1;
int LastMins = -1;

int Values[] = { 1, 1, 2, 3, 5 };

tmElements_t tTime;

void setup( void ) {
  Serial.begin( 9600 );
  Serial.println( "RGB LED Controller" );
  
  while ( !Serial );
  delay( 200 );

  // setTime( 20, 45, 30, 29, 6, 16 );
  // RTC.set( now() );
  
  /*if ( getTime( __TIME__ ) ) { 
     if ( RTC.write( tTime ) ) {
      Serial.println( "We good" );
     }
  }*/
  
    // Pins for LEDs
    pinMode( 2, OUTPUT );
    pinMode( 3, OUTPUT );
    pinMode( 4, OUTPUT );
    pinMode( 5, OUTPUT );
    pinMode( 6, OUTPUT );
    pinMode( 7, OUTPUT );
    
    // PWM Pins
    pinMode( 9 ,OUTPUT );
    pinMode( 10 ,OUTPUT );
    pinMode( 11 ,OUTPUT );
    
    // Set PWM Pins to HIGH
    digitalWrite( 9, 1 );
    digitalWrite( 10, 1 );
    digitalWrite( 11, 1 );
    
    // Enable Multiplex
    setPwmFrequency( 9, 8 );
    setPwmFrequency( 10, 8 );
    setPwmFrequency( 11, 8 );
    analogWrite( 9, 255 );
    analogWrite( 10, 255 );
    analogWrite( 11, 255 );
    
    TIMSK2 = 1 << TOIE2;
  
    randomSeed( analogRead( A0 ) );
    ResetHours();
}

void loop( void ) {
  tmElements_t tm;

  if ( RTC.read( tm ) ) {
    if ( tm.Hour > 12 ) {
      ParseHours( tm.Hour - 12 );
    } else {
      ParseHours( tm.Hour );
    }
    ParseMins( tm.Minute );
    
    Serial.print( "Ok, Time = " );
    print2digits( tm.Hour );
    Serial.write( ':' );
    print2digits( tm.Minute );
    Serial.write( ':' );
    print2digits( tm.Second );
    Serial.print( ", Date (D/M/Y) = " );
    Serial.print( tm.Day );
    Serial.write( '/' );
    Serial.print( tm.Month );
    Serial.write( '/');
    Serial.print( tmYearToCalendar( tm.Year ) );
    Serial.println();
    
  } else {
    if ( RTC.chipPresent() ) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    
    delay( 10000 );
  }
  
  delay( 3000 );
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

bool getTime(const char *str)
{
  int myHour, myMin, mySec;

  if (sscanf(str, "%d:%d:%d", &myHour, &myMin, &mySec) != 3) return false;
  tTime.Hour = myHour;
  tTime.Minute = myMin;
  tTime.Second = mySec;
  return true;
}

void SetLight( int pIndex, int pColors ) {
  if ( pIndex == 4 ) {
    SetLight( 5, Colors[ Mode ][ pColors ][ RED ], Colors[ Mode ][ pColors ][ GREEN ], Colors[ Mode ][ pColors ][ BLUE ] );
  }
  SetLight( pIndex, Colors[ Mode ][ pColors ][ RED ], Colors[ Mode ][ pColors ][ GREEN ], Colors[ Mode ][ pColors ][ BLUE ] );
}

void SetLight( int pIndex, int pRed, int pGreen, int pBlue ) {
  LEDValues[ pIndex ][ 0 ] = pRed;
  LEDValues[ pIndex ][ 1 ] = pGreen;
  LEDValues[ pIndex ][ 2 ] = pBlue;
}

void ResetHours( void ) {
    for ( int i = 0; i < 6; i++ ) {
        SetLight( i, Colors[ Mode ][ OFF ][ RED ], Colors[ Mode ][ OFF ][ GREEN ], Colors[ Mode ][ OFF ][ BLUE ] );
    }
}

void ResetMins( void ) {
  for ( int i = 0; i < 6; i++ ) {
    if ( LEDValues[ i ][ 0 ] == Colors[ Mode ][ BOTH ][ RED ] && 
       LEDValues[ i ][ 1 ] == Colors[ Mode ][ BOTH ][ GREEN ] && 
       LEDValues[ i ][ 2 ] == Colors[ Mode ][ BOTH ][ BLUE ] ) {
      SetLight( i, HOURS );
    }
  }
  
  for ( int i = 0; i < 6; i++ ) {
    if ( LEDValues[ i ][ 0 ] == Colors[ Mode ][ MINS ][ RED ] && 
       LEDValues[ i ][ 1 ] == Colors[ Mode ][ MINS ][ GREEN ] && 
       LEDValues[ i ][ 2 ] == Colors[ Mode ][ MINS ][ BLUE ] ) {
      SetLight( i, OFF );
    }
  }
}

int GetColor( int pIndex ) {
  if ( LEDValues[ pIndex ][ 0 ] == Colors[ Mode ][ HOURS ][ RED ] && 
     LEDValues[ pIndex ][ 1 ] == Colors[ Mode ][ HOURS ][ GREEN ] && 
     LEDValues[ pIndex ][ 2 ] == Colors[ Mode ][ HOURS ][ BLUE ] ) {
    return BOTH;
  }
  return MINS;
}

void ParseMins( int pMins ) {
  int myMins = pMins / 5;
  
    if ( LastMins == myMins ) {
        return;
    }
    
    int myRNR[] = { 0, 1, 2, 3, 4 };
    for ( int i = 0; i < 50; i++ ) {
        int myFirst = random( 5 );
        int mySecond;
        do {
            mySecond = random( 5 );
        } while ( mySecond == myFirst );
        
        int myTemp = myRNR[ myFirst ];
        myRNR[ myFirst ] = myRNR[ mySecond ];
        myRNR[ mySecond ] = myTemp;
    }
  
  int myIndex;
    int myTotal = 4;
    while ( myMins != 0 ) {
        myIndex = myRNR[ myTotal ];
        int myValue = Values[ myIndex ];
        
        if ( myMins - myValue < 0 ) {
            myTotal--;
            
            if ( myTotal == -1 ) {
                ResetMins();
                
                ParseMins( pMins );
                return;
            }
            
            continue;
        }
        
        SetLight( myRNR[ myTotal ], GetColor( myRNR[ myTotal ] ) );
        myTotal--;
        myMins -= myValue;
    }
  
  LastMins = pMins / 5;
}

void ParseHours( int pHours ) {
    if ( LastHour == pHours ) {
        return;
    }
    
    int myRNR[] = { 0, 1, 2, 3, 4 };
    for ( int i = 0; i < 50; i++ ) {
        int myFirst = random( 5 );
        int mySecond;
        do {
            mySecond = random( 5 );
        } while ( mySecond == myFirst );
        
        int myTemp = myRNR[ myFirst ];
        myRNR[ myFirst ] = myRNR[ mySecond ];
        myRNR[ mySecond ] = myTemp;
    }
    
    int myHours = pHours;
    int myIndex;
    int myTotal = 4;
    while ( myHours != 0 ) {
        myIndex = myRNR[ myTotal ];
        int myValue = Values[ myIndex ];
        
        if ( myHours - myValue < 0 ) {
            myTotal--;
            
            if ( myTotal == -1 ) {
                ResetHours();
                
                ParseHours( pHours );
                return;
            }
            
            continue;
        }
        
        SetLight( myRNR[ myTotal ], HOURS ); 
        myTotal--;
        myHours -= myValue;
    }
  
  LastHour = pHours;
}

ISR( TIMER2_OVF_vect ) {
    if ( Prescaler < PrescalerOverflowValue ) {
        Prescaler++;
    } else {
        Prescaler = 0;
        Multiplex();
    }
}

void Multiplex( void ) {
    PORTD &= 0b00000011; // Control pin 0-5
    PORTB &= 0b11101110; // Control pin 6-7
    
    analogWrite( 9, 255 - LEDValues[ CurrentLED ][ 0 ] );
    analogWrite( 10, 255 - LEDValues[ CurrentLED ][ 1 ] );
    analogWrite( 11, 255 - LEDValues[ CurrentLED ][ 2 ] );
    
    switch ( CurrentLED ) {
        case 0:
            //digitalWrite(2, 1); // Turn on LED 1
            PORTD |= 0b00000100;
            break;
        case 1:
            //digitalWrite(3, 1); // Turn on LED 1
            PORTD |= 0b00001000;
            break;
        case 2:
            //digitalWrite(4, 1); // Turn on LED 1
            PORTD |= 0b00010000;
            break;
        case 3:
            //digitalWrite(5, 1); // Turn on LED 1
            PORTD |= 0b00100000;
            break;
        case 4:
            //digitalWrite(6, 1); // Turn on LED 1
            PORTD |= 0b01000000;
            break;
        case 5:
            //digitalWrite(7, 1); // Turn on LED 1
            PORTD |= 0b10000000;
            break;
        case 6:
            //digitalWrite(8, 1); // Turn on LED 1
            PORTB |= 0b00000001;
            break;
        case 7:
            //digitalWrite(12, 1); // Turn on LED 1
            PORTB |= 0b00010000;
            break;
    }
    
    CurrentLED++;
    if ( CurrentLED >= NUM_LEDS ) {
        CurrentLED = 0;
  }
}

void setPwmFrequency( int pin, int divisor ) {
    byte mode;
    if ( pin == 5 || pin == 6 || pin == 9 || pin == 10 ) {
        switch( divisor ) {
            case 1: mode = 0x01; break;
            case 8: mode = 0x02; break;
            case 64: mode = 0x03; break;
            case 256: mode = 0x04; break;
            case 1024: mode = 0x05; break;
            default: return;
        }
        if ( pin == 5 || pin == 6 ) {
            TCCR0B = TCCR0B & 0b11111000 | mode;
        } else {
            TCCR1B = TCCR1B & 0b11111000 | mode;
        }
    } else if ( pin == 3 || pin == 11 ) {
        switch(divisor) {
            case 1: mode = 0x01; break;
            case 8: mode = 0x02; break;
            case 32: mode = 0x03; break;
            case 64: mode = 0x04; break;
            case 128: mode = 0x05; break;
            case 256: mode = 0x06; break;
            case 1024: mode = 0x7; break;
            default: return;
        }
        TCCR2B = TCCR2B & 0b11111000 | mode;
    }
}

/*unsigned long processSyncMessage( void ) {
  unsigned long pcTime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1, 2013
  
  if ( Serial.find( TIME_HEADER ) ) {
    pcTime = Serial.parseInt();
    return pcTime;
    if ( pcTime < DEFAULT_TIME ) {
      pcTime = 0L;
    }
  }
  return pcTime;
}*/

