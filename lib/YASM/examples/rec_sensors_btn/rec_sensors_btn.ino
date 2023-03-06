//This is a pretty advanced example on state machines usage
//It's goal is to search DS18b20 sensors adresses and store them in arduino EEPROM
//for further use in a heating controller sketch.

//It use a state machine to control the sensor searching and recording and display a 
//menu on 20*4 alphanumeric LCD. It uses the provided BTN class to use a button as user input.
//It's an evolution of the rec_sensors example.

//the one wire bus is on pin 9, the button on pin 10 and switch to gnd when clicked



#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <yasm.h>
#include <btn.h>

// OneWire data wire is plugged into digital pin 9 on the Arduino
#define ONE_WIRE_BUS 9

#define TEMPERATURE_PRECISION 9

#define EEPROM_BASE_ADR 0 //base address for saving sensors addresses
#define PIN_SW 10
#define NBR_SONDE 9 //number of sensors to research and store in eeprom
uint8_t numSonde=1;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress adrSonde;

LiquidCrystal_I2C lcd(0x20, 4, 5, 6, 0, 1, 2, 3, 7, NEGATIVE);

YASM menu;
BTN btn;

void setup(void)
{
  //button input
  pinMode(PIN_SW,INPUT_PULLUP);

  // Start up the libraries
  sensors.begin();
  lcd.begin(4,20);
  
  lcd.backlight();
  lcd.clear();
  
  //initilizes the state machine
  menu.next(menu_start); 
}


void loop(void) //states machine use allow the loop to be minimal
{
	btn.update(!digitalRead(PIN_SW)); // ! because btn switch to gnd
    	menu.run(); 
}


///////////menu state machine////////////

void menu_start()
{
	if(menu.isFirstRun()) {
		lcd.clear();
		lcd.print("Appuyez pour rechercher sonde ");
		lcd.print(numSonde);
		lcd.print("/");
		lcd.print(NBR_SONDE);
	}
	if(btn.state(BTN_CLICK)) {
		menu.next(menu_recherche);
	}	
	if(btn.state(BTN_LONGCLICK)) {
		menu.next(menu_affiche);
		numSonde=1;
	}
	if(numSonde>NBR_SONDE) menu.next(menu_fin);
}

void menu_affiche()
{
	if(menu.isFirstRun()) {
		lcd.clear();
		for(uint8_t j=0; j<4;j++)
		{
			lcd.setCursor(0,j);
			lcd.print(numSonde);
			lcd.print("  ");
			EEPROM.get(EEPROM_BASE_ADR + (sizeof(adrSonde)*(numSonde-1)) , adrSonde);
			for (uint8_t i = 0; i < 8; i++)
			{
				// zero pad the address if necessary
				if (adrSonde[i] < 16) lcd.print("0");
				lcd.print(adrSonde[i], HEX);
			}
			numSonde++;
		}
	}
	if(btn.state(BTN_CLICK)) {
		menu.next(menu_affiche_next);
	}
	if(btn.state(BTN_LONGCLICK)) {
		menu.next(menu_start);
		numSonde=1;
	}
}

void menu_affiche_next()
{
		menu.next(menu_affiche);
}

void menu_fin()
{
	if(menu.isFirstRun()) {
		lcd.clear();
		lcd.print("Fin d'enregistrement");
	}	
	if(btn.state(BTN_LONGCLICK)) {
		menu.next(menu_affiche);
	}
}

void menu_recherche()
{
	if(menu.periodic(1.5E3)){
		//sensors.begin();
		lcd.clear();
		sensors.getDeviceCount();
		if (!sensors.getAddress(adrSonde, 0)) lcd.print("erreur sonde");
		else {
			sensors.setResolution(adrSonde, TEMPERATURE_PRECISION);
			lcd.print("sonde ");
			lcd.print(numSonde);
			lcd.print("/");
			lcd.print(NBR_SONDE);
			lcd.print("   ");
			sensors.requestTemperatures();
			lcd.print(sensors.getTempC(adrSonde));
			lcd.setCursor(0,1);
			for (uint8_t i = 0; i < 8; i++){
				// zero pad the address if necessary
				if (adrSonde[i] < 16) lcd.print("0");
				lcd.print(adrSonde[i], HEX);
			}
			lcd.setCursor(0,2);
			lcd.print("Appui court SAVE");
			lcd.setCursor(0,3);
			lcd.print("Appui long SUIVANTE");
		}
	}
	if(btn.state(BTN_CLICK)) {
		menu.next(menu_save);
	}
	if(btn.state(BTN_LONGCLICK)) {
		menu.next(menu_start);
		numSonde++;
	}
}

void menu_save()
{
	if(menu.isFirstRun()){
		lcd.clear();
		lcd.print("enreg. sonde ");
		lcd.print(numSonde);
		lcd.print("/");
		lcd.print(NBR_SONDE);
		lcd.print(" ?");
		lcd.setCursor(0,1);
		lcd.print("long=SAVE, court=RETOUR");
	}
	if(btn.state(BTN_CLICK)) {
		menu.next(menu_start);
	}	
	if(btn.state(BTN_LONGCLICK)){
		EEPROM.put(EEPROM_BASE_ADR + (sizeof(adrSonde)*(numSonde-1)), adrSonde);
		numSonde++;
		menu.next(menu_start);
	}
}
	
	
