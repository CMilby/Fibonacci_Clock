
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
#define DS3231_I2C_ADDRESS 0x68

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

void setup( void ) {
  Wire.begin();
  Serial.begin( 9600 );
  // Serial.println( "RGB LED Controller" );

  // while ( !Serial );
  // delay( 200 );

  // Pins for LEDs
  pinMode( 2, OUTPUT );
  pinMode( 3, OUTPUT );
  pinMode( 4, OUTPUT );
  pinMode( 5, OUTPUT );
  pinMode( 6, OUTPUT );
  pinMode( 7, OUTPUT );

  // PWM Pins
  pinMode( 9 , OUTPUT );
  pinMode( 10 , OUTPUT );
  pinMode( 11 , OUTPUT );

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

  // Set Initial Time Here
  // setDS3231Time( 0, 48, 10, 5, 15, 12, 16 );

  randomSeed( analogRead( A0 ) );
  ResetHours();
}

void loop( void ) {
  byte mySecond, myMinute, myHour, myDayOfWeek, myDayOfMonth, myMonth, myYear;
  readDS3231Time( &mySecond, &myMinute, &myHour, &myDayOfWeek, &myDayOfMonth, &myMonth, &myYear);
  displayTime( mySecond, myMinute, myHour, myDayOfWeek, myDayOfMonth, myMonth, myYear );
  
  if ( myHour > 12 ) {
    myHour -= 12;
  }

  ParseHours( myHour );
  ParseMins( myMinute );

  delay( 1000 );
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
  bool isDone = false;
  while ( myMins != 0 ) {
    myIndex = myRNR[ myTotal ];
    int myValue = Values[ myIndex ];

    if ( myMins - myValue < 0 ) {
      myTotal--;

      if ( myTotal == -1 ) {
        ResetMins();

        ParseMins( pMins );
        isDone = true;
        break;
      }

      continue;
    }

    if ( !isDone ) {
      SetLight( myRNR[ myTotal ], GetColor( myRNR[ myTotal ] ) );
      myTotal--;
      myMins -= myValue;
    }
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
  bool isDone = false;
  
  while ( myHours != 0 ) {
    myIndex = myRNR[ myTotal ];
    int myValue = Values[ myIndex ];

    if ( myHours - myValue < 0 ) {
      myTotal--;

      if ( myTotal == -1 ) {
        ResetHours();

        ParseHours( pHours );
        isDone = true;
        break;
      }

      continue;
    }

    if ( !isDone ) {
      SetLight( myRNR[ myTotal ], HOURS );
      myTotal--;
      myHours -= myValue;
    }
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
    switch ( divisor ) {
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
    switch (divisor) {
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

byte decToBcd( byte p_val ) {
  return ( ( p_val / 10 * 16 ) + ( p_val % 10 ) );
}

byte bcdToDec( byte p_val ) {
  return ( ( p_val / 16 * 10 ) + ( p_val % 16 ) );
}

void setDS3231Time( byte p_second, byte p_minute, byte p_hour, byte p_dayOfWeek, byte p_dayOfMonth, byte p_month, byte p_year ) {
  Wire.beginTransmission( DS3231_I2C_ADDRESS );
  Wire.write( 0 );
  Wire.write( decToBcd( p_second ) ); // set seconds
  Wire.write( decToBcd( p_minute ) ); // set minutes
  Wire.write( decToBcd( p_hour ) ); // set hours
  Wire.write( decToBcd( p_dayOfWeek ) ); // set day of week (1=Sunday, 7=Saturday)
  Wire.write( decToBcd( p_dayOfMonth ) ); // set date (1 to 31)
  Wire.write( decToBcd( p_month ) ); // set month
  Wire.write( decToBcd( p_year ) ); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231Time( byte *p_second, byte *p_minute, byte *p_hour, byte *p_dayOfWeek, byte *p_dayOfMonth, byte *p_month, byte *p_year ) {
  Wire.beginTransmission( DS3231_I2C_ADDRESS );
  Wire.write( 0 );
  Wire.endTransmission();
  Wire.requestFrom( DS3231_I2C_ADDRESS, 7 );

  *p_second = bcdToDec ( Wire.read() & 0x7f );
  *p_minute = bcdToDec ( Wire.read() );
  *p_hour = bcdToDec ( Wire.read() & 0x3f );
  *p_dayOfWeek = bcdToDec ( Wire.read() );
  *p_dayOfMonth = bcdToDec ( Wire.read() );
  *p_month = bcdToDec ( Wire.read() );
  *p_year = bcdToDec ( Wire.read() );
}

void displayTime( byte p_second, byte p_minute, byte p_hour, byte p_dayOfWeek, byte p_dayOfMonth, byte p_month, byte p_year ) {
  Serial.print( p_hour, DEC );
  Serial.print( ":" );
  if ( p_minute < 10 ) {
    Serial.print("0");
  }
  Serial.print( p_minute, DEC );
  Serial.print( ":" );
  if ( p_second < 10 ) {
    Serial.print("0");
  }

  Serial.print( p_second, DEC );
  Serial.print( " ");
  Serial.print( p_dayOfMonth, DEC );
  Serial.print( "/");
  Serial.print( p_month, DEC );
  Serial.print( "/");
  Serial.print( p_year, DEC );
  Serial.print( " Day of week: ");
  switch ( p_dayOfWeek ) {
    case 1:
      Serial.println("Sunday");
      break;
    case 2:
      Serial.println("Monday");
      break;
    case 3:
      Serial.println("Tuesday");
      break;
    case 4:
      Serial.println("Wednesday");
      break;
    case 5:
      Serial.println("Thursday");
      break;
    case 6:
      Serial.println("Friday");
      break;
    case 7:
      Serial.println("Saturday");
      break;
  }
}


