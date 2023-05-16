#include <WiFi.h>
#include <WiFiMulti.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>
#include <WebServer.h>


LiquidCrystal_PCF8574 lcd(0x27); // 20 chars and 4 line display

/* Network Settings */
char SVR_NAME[] = "publicdxcluster";
#define SVR_PORT 7300

/* Define your callsign, passcode*/
#define callsign "YOURCALL"
#define passcode ""

int REPORT_INTERVAL = 10;
#define TO_LINE  10000

WiFiMulti wifiMulti;

// Use WiFiClient class to create TCP connections
WiFiClient client;
WebServer server(80);

int LED_BUILTIN = 2;

float h = 0;
float t = 0;
float b = 0;
float v = 0;

int fp = 0;
int fps = 0;
String stats = "";

void setup()
{
  Serial.begin(115200);
  delay(10);

  Wire.begin();
  Wire.beginTransmission(0x27);
  int error = Wire.endTransmission();
  //Serial.print("Error: ");
  //Serial.print(error);

  if (error == 0) {
    Serial.println("LCD found.");
    lcd.begin(20, 4); // initialize the lcd
    lcd.setBacklight(255);
    lcd.home();
    lcd.clear();

  } else {
    Serial.println(": LCD not found.");
  }

  pinMode(LED_BUILTIN, OUTPUT);

  lcd.setCursor(7, 0);
  lcd.print (callsign);
  lcd.setCursor(5, 1);
  lcd.print ("DX CLUSTER");

  wifiMulti.addAP("wifitry1", "code");
  wifiMulti.addAP("wifitry2", "code");

  lcd.setCursor(0, 2);
  lcd.print ("Connecting Wifi...");

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() != WL_CONNECTED) {
    blink (1000);
    blink (1000);
    blink (1000);
    ESP.restart();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  blink (200);
  lcd.setCursor(0, 2);
  lcd.print ("WiFi connected     ");


  Wire.begin();

  server.on("/", handleRoot);

  server.on("/reset", []() {
    server.send(200, "text/html", "Reset in 3 second");
    blink (1000);
    blink (1000);
    blink (1000);
    ESP.restart();
  });

  server.onNotFound(handleNotFound);

  server.begin();
}

void loop()
{
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    Serial.println("Restarting in 10 seconds");
    lcd.setCursor(0, 2);
    lcd.print ("WiFi NOT connected");
    lcd.setCursor(0, 3);
    lcd.print ("Restarting in 10 seconds");
    blink (1000);
    blink (1000);
    blink (1000);
    delay(7000);
    ESP.restart();
  }




  if ( client.connect(SVR_NAME, SVR_PORT) )
  {
    Serial.println("DX Cluster connected");
    lcd.setCursor(0, 3);
    lcd.print ("DX Cluster connected");
    blink (1000);

    Stream* serverpromt = &client;

    if (wait4content(serverpromt, "login:", 6))
    {
      client.println(callsign);
      delay(1000);


      // Read all the lines of the reply from server and print them to Serial
      while (wait4content(&client, "Hello", 5)) {

        while (true) {
          String line = client.readStringUntil('\r');
          if (wait4content(&client, "DX de ", 6)) {

            line.replace("DX de ", "");
            line.replace("     ", " ");
            line.replace("    ", " ");
            line.replace("   ", " ");
            line.replace("  ", " ");

            Serial.println(line);

            if (line.length() > 20) {
              lcd.home();
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print (line.substring(0, 20));
              lcd.setCursor(0, 1);
              lcd.print (line.substring(20, 40));
              lcd.setCursor(0, 2);
              lcd.print (line.substring(40, 60));

              //SES
              if (line.indexOf("vent ") > 0 or line.indexOf(" point") > 0 or line.indexOf(" SX") > 0 or line.indexOf("pecial ") > 0 or line.indexOf(" SPECIAL ") > 0 or line.indexOf(" SES ") > 0 or line.indexOf(" ses ") > 0 or line.indexOf("iploma ") > 0 or line.indexOf(" DIPLOMA ") > 0 or line.indexOf("ward") > 0 or line.indexOf("AWARD") > 0) {
                fp = 0;
                while (line.charAt(fp) != ':' and fp < 20) {
                  fp++;
                }
                fp++;
                fp++;

                fps = fp;

                while (line.charAt(fps) != ' ' and fps < 40) {
                  fps++;
                }
                fps++;
                fps++;
                while (line.charAt(fps) != ' ' and fps < 40) {
                  fps++;
                }
                stats = "SES " + line.substring(fp, fps);
                blink (1000);
              }

              //SOTA
              if (line.indexOf(" SOTA ") > 0 or line.indexOf(" sota ") > 0 or line.indexOf(" Sota ") > 0 ) {
                fp = 0;
                while (line.charAt(fp) != ':' and fp < 20) {
                  fp++;
                }
                fp++;
                fp++;

                fps = fp;

                while (line.charAt(fps) != ' ' and fps < 40) {
                  fps++;
                }
                fps++;
                fps++;
                while (line.charAt(fps) != ' ' and fps < 40) {
                  fps++;
                }
                stats = "SOTA " + line.substring(fp, fps);
                blink (1000);
              }

              lcd.setCursor(0, 3);
              lcd.print (stats.substring(0, 20));
              Serial.println(stats);
            }
          }
        }
      }

      client.stop();
      Serial.println("Server disconnected");



    }  //  if prompt
    else
    {
      Serial.println("No prompt from the server.");
    }
  }//  if connect
  else
  {
    Serial.println("Can not connect to the server.");
  }




  delay(100);
}

////////////////////////////////////////////////////////////////////////////////////////////// End Loop ///////////////////////////////////////////

void handleRoot() {
  String message = "<html>\n";

  message += "</html>\n";

  server.send(200, "text/html", message);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void blink( int timeon) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(timeon);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}



boolean wait4content(Stream * stream, char *target, int targetLen)
{
  size_t index = 0;  // maximum target string length is 64k bytes!
  int c;
  boolean ret = false;
  unsigned long timeBegin;
  delay(50);
  timeBegin = millis();

  while ( true )
  {
    //  wait and read one byte
    while ( !stream->available() )
    {
      if ( millis() - timeBegin > TO_LINE )
      {
        break;
      }
      delay(2);
    }
    if ( stream->available() ) {
      c = stream->read();

      Serial.write(c);

      //  judge the byte
      if ( c == target[index] )
      {
        index ++;
        if ( !target[index] )
          // return true if all chars in the target match
        {
          ret = true;
          break;
        }
      }
      else if ( c >= 0 )
      {
        index = 0;  // reset index if any char does not match
      } else //  timed-out for one byte
      {
        break;
      }
    }
    else  //  timed-out
    {
      break;
    }
  }
  Serial.println("");
  return ret;
}
