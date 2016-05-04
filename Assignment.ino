/*
Owen Kane C13383511
Hugo Hollis C13375106
*/
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information
#include <DHT.h>//dht libary
#include <LiquidCrystal.h>

#define DHTPIN A3    
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
LiquidCrystal lcd(8, 9, 6, 5, 3, 2);

int numRuns = 1;  // Execution count, so this doesn't run forever
int maxRuns = 12; // Maximum number of times the Choreo should be executed, 
                  //set to 12, as there are twelve 5 minute periods in an hour
boolean emailMsg = true;
float alertTemp = 25;

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);
  dht.begin();
  
  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);

  int wifiStatus = WL_IDLE_STATUS;

  // Determine if the WiFi Shield is present
  Serial.print("\n\nShield:");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("FAIL");

    // If there's no WiFi shield, stop here
    while(true);
  }

  Serial.println("OK");

  // Try to connect to the local WiFi network
  while(wifiStatus != WL_CONNECTED) {
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(WIFI_SSID, WPA_PASSWORD);

    if (wifiStatus == WL_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
    delay(5000);
  }

  Serial.println("Setup complete.\n");
}

void loop() {
  if (numRuns <= maxRuns) {
    float humidity = dht.readHumidity();//Get data from he DHT sensor 
    float temperature = dht.readTemperature();
    int sensorValue = analogRead(A0);//Get the light data from the pin it is connected to

    String comma = ",";
    String RowDataValue = temperature + comma + humidity + comma + sensorValue;//Format the string
    Serial.println("Entering data :" + RowDataValue);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humidity: "); 
    lcd.print((int) humidity);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("Temp.: "); 
    lcd.print((int) temperature);
    lcd.println("*C       ");
    delay(750);
    
    Serial.println("Running AppendRow - Run #" + String(numRuns++));

    TembooChoreo AppendRowChoreo(client);

    // Invoke the Temboo client
    AppendRowChoreo.begin();

    // Set Temboo account credentials
    AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
    AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set Choreo inputs
    AppendRowChoreo.addInput("RowData", RowDataValue);
    String SpreadsheetTitleValue = "Assignment";
    AppendRowChoreo.addInput("SpreadsheetTitle", SpreadsheetTitleValue);
    String RefreshTokenValue = "1/M06QnyB8_IXVL9y2iICuxwSyneLkbIMUGEZBGh5WQPgMEudVrK5jSpoR30zcRFq6";
    AppendRowChoreo.addInput("RefreshToken", RefreshTokenValue);
    String ClientSecretValue = "Hz_m92JL43Rjx15sVXyf6azV";
    AppendRowChoreo.addInput("ClientSecret", ClientSecretValue);
    String ClientIDValue = "937043918210-fgsg1ob4so5ja889j6ocb9v6a6do0mgv.apps.googleusercontent.com";
    AppendRowChoreo.addInput("ClientID", ClientIDValue);

    // Identify the Choreo to run
    AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");

    // Run the Choreo; when results are available, print them to serial
    AppendRowChoreo.run();

    while(AppendRowChoreo.available()) {
      char c = AppendRowChoreo.read();
      Serial.print(c);
    }
    AppendRowChoreo.close();

    if(numRuns >= maxRuns){//Only send email if in montoring time
      emailMsg = false;
    }
  }

  if(emailMsg = true && dht.readTemperature() > alertTemp){//Needs to call email() outside "maxruns" 
                                                           //if statement as causes bad request errors otherwise.
    Serial.print("Temperature over Alert temperature, sending email...");
    email();
  }
  
  Serial.println("\nWaiting...\n");
  delay(300000); // wait 5 mins between AppendRow calls
}

void email(){
    TembooChoreo SendEmailChoreo(client);

    // Invoke the Temboo client
    SendEmailChoreo.begin();

    // Set Temboo account credentials
    SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
    SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set Choreo inputs
    String FromAddressValue = "owen.a.kane@gmail.com";
    SendEmailChoreo.addInput("FromAddress", FromAddressValue);
    String UsernameValue = "owen.a.kane@gmail.com";
    SendEmailChoreo.addInput("Username", UsernameValue);
    String ToAddressValue = "owen.a.kane@gmail.com";
    SendEmailChoreo.addInput("ToAddress", ToAddressValue);
     String SubjectValue = "Temperature Alert!";
    SendEmailChoreo.addInput("Subject", SubjectValue);
    String PasswordValue = "zsoahpawrkrgceno";
    SendEmailChoreo.addInput("Password", PasswordValue);
    String MessageBodyValue = "Temperature over set alert temperature!";
    SendEmailChoreo.addInput("MessageBody", MessageBodyValue);

    // Identify the Choreo to run
    SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");

    // Run the Choreo; when results are available, print them to serial
    SendEmailChoreo.run();

    while(SendEmailChoreo.available()) {
      char c = SendEmailChoreo.read();
      Serial.print(c);
    }
    SendEmailChoreo.close();
}


