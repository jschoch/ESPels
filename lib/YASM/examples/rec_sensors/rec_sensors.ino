//This is a pretty advanced example on state machines usage
//It's goal is to search DS18b20 sensors adresses and store them in arduino EEPROM
//for further use in a heating controller sketch.

//It uses two states machines, one for checking a button state to get user input
//events, and one to control the sensor searching and recording, and create a menu
//on 20*4 alphanumeric LCD.

//One possible upgrade would be to create a helper function to return the button
//event value instead of checking directly the storage variable.
//And by the way this is done in the "rec_sensors_btn" example, where it uses the provided
//BTN class instead of dealing with the button in the sketch.

//the one wire bus is on pin 9, the button on pin 10 and switch to gnd when clicked



#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <yasm.h>

// OneWire data wire is plugged into digital pin 9 on the Arduino
#define ONE_WIRE_BUS 9

#define TEMPERATURE_PRECISION 9

#define EEPROM_BASE_ADR 0 //base address for saving sensors addresses
#define PIN_SW 10
#define NBR_SONDE 9 //number of sensors to research and store in eeprom
uint8_t numSonde=1;

//button possible events
#define BTN_NONE 0
#define BTN_CLICK 1
#define BTN_LONGCLICK 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress adrSonde;

LiquidCrystal_I2C lcd(0x20, 4, 5, 6, 0, 1, 2, 3, 7, NEGATIVE);

YASM menu;
YASM btn;

uint8_t flagbtn=0;
uint8_t btnstate=BTN_NONE;

void setup(void)
{
  //button input
  pinMode(PIN_SW,INPUT_PULLUP);

  // Start up the libraries
  sensors.begin();
  lcd.begin(4,20);
  
  lcd.backlight();
  lcd.clear();
  
  //initilizes the states machines
  menu.next(menu_start); 
  btn.next(btn_wait);

}


void loop(void) //states machines use allow the loop to be minimal
{
	flagbtn = !digitalRead(PIN_SW); // ! because button switch to GND
	btn.run();
	menu.run();

}

/////////////button state machine///////////////

void btn_wait()
{
	if(btn.elapsed(100)) if(flagbtn) btn.next(btn_debounce);
}

void btn_debounce()
{
	if(!flagbtn) btn.next(btn_wait);
	if(btn.elapsed(5)) btn.next(btn_check);
}

void btn_check()
{
	if(btn.elapsed(1E3)) { 
		btn.next(btn_longpress); 
		btnstate = BTN_LONGCLICK;
		return;
	}
	if(!flagbtn) {
		btn.next(btn_wait);
		btnstate = BTN_CLICK;
	}
}

void btn_longpress()
{
	if(!flagbtn) btn.next(btn_wait);
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
	if(btnstate==BTN_CLICK) {
		btnstate=BTN_NONE;
		menu.next(menu_recherche);
		lcd.clear();
	}	
	if(btnstate==BTN_LONGCLICK) {
		btnstate=BTN_NONE;
		menu.next(menu_affiche);
		numSonde=1;
		//lcd.clear();
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
	if(btnstate==BTN_CLICK) {
		btnstate=BTN_NONE;
		menu.next(menu_affiche_next);
		lcd.clear();
	}
	if(btnstate==BTN_LONGCLICK) {
		btnstate=BTN_NONE;
		menu.next(menu_start);
		lcd.clear();
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
	if(btnstate==BTN_LONGCLICK) {
		btnstate=BTN_NONE;
		menu.next(menu_affiche);
		lcd.clear();
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
	if(btnstate==BTN_CLICK) {
		btnstate=BTN_NONE;
		menu.next(menu_save);
	}
	if(btnstate==BTN_LONGCLICK) {
		btnstate=BTN_NONE;
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
	if(btnstate==BTN_CLICK){
		btnstate=BTN_NONE;
		menu.next(menu_start);
	}	
	if(btnstate==BTN_LONGCLICK){
		btnstate=BTN_NONE;
		EEPROM.put(EEPROM_BASE_ADR + (sizeof(adrSonde)*(numSonde-1)), adrSonde);
		numSonde++;
		menu.next(menu_start);
	}
}
	
	
