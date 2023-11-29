#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#define PIN                 5
#define NUMPIXELS           (8*8)
#define WEBSITE_HEAD        "\
                            <!DOCTYPE html>\n\
                            <html lang=\"en\">\n\
                            <head>\n\
                              <meta charset=\"UTF-8\">\n\
                              <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
                              <title>Button Grid</title>\n\
                              <style>\n\
                                body {\n\
                                  display: flex;\n\
                                  justify-content: center;\n\
                                  align-items: center;\n\
                                  height: 100vh;\n\
                                  font-family: Helvetica;\n\
                                  margin: 0px auto;\n\
                                  text-align: center;\n\
                                }\n\n\
                                .grid-container {\n\
                                  display: grid;\n\
                                  grid-template-columns: repeat(8, 50px); \n\
                                  grid-gap: 5px; \n\
                                }\n\n\
                                .grid-container a{\n\
                                  padding: 10px 10px;\n\
                                  border: none;\n\
                                  cursor: pointer;\n\
                                  text-decoration: none;\n\
                                }\n\n\
                                .buttonon{\n\
                                  background-color: #00FF00; \n\
                                  color: #000000; \n\
                                  font-size: 30px; \n\
                                }\n\n\
                                .buttonoff{\n\
                                  background-color: #FF0000; \n\
                                  color: #000000; \n\
                                  font-size: 30px; \n\
                                }\n\n\
                                .buttongeneral{\n\
                                  background-color: #333333; \n\
                                  color: #FFFFFF; \n\
                                  font-size: 15px; \n\
                                }\n\
                              </style>\n\
                            </head>"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

bool ledState[64];

bool tictactoe[64] = {0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,1,1,1,1,1,1,1,1,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,1,1,1,1,1,1,1,1,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0};

const char* ssid     = "Phone max";
const char* password = "espwebserver";

WiFiServer server(80);

String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  strip.begin();
  strip.show();
}

void loop(){
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            if (header.indexOf("GET /reset") >= 0) {
              for (int i = 0; i < 64; i++) {
                strip.setPixelColor(i, strip.Color(0, 0, 0));
                ledState[i] = false;
              }
            } else if (header.indexOf("GET /tictactoe") >= 0) {
              for (int c = 0; c < 3; c++) {
                int color;
                if (c == 0)
                   color = strip.Color(0, 255, 0);
                else if (c == 1)
                  color = strip.Color(200, 128, 0);
                else if (c == 2)
                  color = strip.Color(255, 0, 0);
                for (int i = 0; i < 64; i++) {
                  if (tictactoe[i]) {
                    strip.setPixelColor(i, color);
                    ledState[i] = true;
                    strip.show();
                    delay(15);
                  }
                  else {
                    strip.setPixelColor(i, strip.Color(0, 0, 0));
                    ledState[i] = false;
                    strip.show();
                  }
                }
              }
            } else {
                for (int i = 0; i < 64; i++) {
                String onCommand = "GET /" + String(i) + "/on";
                String offCommand = "GET /" + String(i) + "/off";

                if (header.indexOf(onCommand) >= 0) {
                    strip.setPixelColor(i, strip.Color(255, 0, 0));
                    ledState[i] = true;
                    break;
                } else if (header.indexOf(offCommand) >= 0) {
                    strip.setPixelColor(i, strip.Color(0, 0, 0));
                    ledState[i] = false;
                    break;
                }
              }
              strip.show();
            }
            
            // Display the HTML web page
            client.println(WEBSITE_HEAD);
            
            // Web Page Heading
            client.println("<body>");
            String link;
            String button;
            client.println("<div class=\"grid-container\">");
            for (int i = 0; i < 8; i++) {
              for (int j = 0; j < 8; j++)
              {
                int ledId = i * 8 + j;
                if (ledState[ledId]){
                  link = String(ledId) + "/off";
                  button = "buttonoff";
                } else {
                  link = String(ledId) + "/on";
                  button = "buttonon";
                }
                client.println("<a class=\"" + button + "\" href=\"/" + link + "\">" + String(ledId) + "</a>");
              }
            }
            client.println("<a class=\"buttongeneral\" href=\"/reset\"> RST </a>");
            client.println("<a class=\"buttongeneral\" href=\"/tictactoe\"> TTT </a>");
            client.println("</div>");

            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}